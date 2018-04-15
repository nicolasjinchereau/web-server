/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Server.h"
#include "Socket.h"
#include "Http.h"
#include "MimeTypes.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <atomic>
#include <queue>
#include "Spinlock.h"
#include "format.h"

using namespace std;
using namespace chrono;

#ifdef _WIN32
inline int poll(pollfd* fdArray, unsigned int fds, int timeout) {
    return WSAPoll(fdArray, fds, timeout);
}
#endif

Server::Server()
{
#ifdef _WIN32
    WSAData wsdata;
    if(WSAStartup(WINSOCK_VERSION, &wsdata) != 0) {
        throw runtime_error("failed to initialze winsock");
    }

    if(wsdata.wVersion != WINSOCK_VERSION) {
        WSACleanup();
        throw runtime_error("failed to initialze winsock");
    }
#endif
}

Server::~Server()
{
    Stop();

#ifdef _WIN32
    WSACleanup();
#endif
}

void Server::Start(int port, const string& docsPath, size_t threadCount)
{
    Stop();

    try
    {
        Server::Log(nullptr, "Server starting");

        this->port = port;
        this->httpdocs = docsPath;
        this->run = true;

        if(this->httpdocs.back() == '\\')
            this->httpdocs.pop_back();

        idleSessions.reserve(64);
        activeSessions.reserve(64);

        Socket tmp(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::TCP);
        tmp.Bind(RequestWakePort, nullptr, true);
        tmp.Listen();

        idleThreadWakeSockets[0] = Socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::TCP);
        idleThreadWakeSockets[0].Connect(RequestWakePort, LoopbackAddress);
        idleThreadWakeSockets[1] = tmp.Accept();
        idleThreadWakeSockets[1].SetNonBlocking(true);

        tmp.Close();

        if(threadCount == 0)
            threadCount = thread::hardware_concurrency();

        for(unsigned i = 0; i < threadCount; ++i)
            activeThreads.emplace_back(thread(&Server::ActiveThreadRunLoop, this));

        idleThread = thread(&Server::IdleThreadRunLoop, this);
        listenThread = thread(&Server::ListenThreadRunLoop, this);
    }
    catch(exception&)
    {
        Stop();
        throw;
    }
}

void Server::Stop()
{
    if(run)
    {
        run = false;

        listenSocket.Close();
        idleThreadWakeSockets[0].Close();
        idleThreadWakeSockets[1].Close();
        activeThreadWakeCond.notify_all();

        listenThread.join();
        idleThread.join();

        for(auto& thread : activeThreads)
            thread.join();

        port = 0;
        httpdocs.clear();
        activeThreads.clear();

        Server::Log(nullptr, "Server stopped");
    }
}

void Server::Log(const Session* session, const string& message)
{
    static Spinlock spin;
    lock_guard<Spinlock> lk(spin);

    if(session)
        cout << setfill(' ') << setw(6) << (uint64_t)session->socket.handle() << ": ";
    else
        cout << string(6, '-') << ": ";

    cout << message << endl;
    cout.flush();
}

void Server::ListenThreadRunLoop()
{
    listenSocket = Socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::TCP);
    listenSocket.Bind(port);
    listenSocket.Listen();

    while(run)
    {
        if(listenSocket.Poll(SocketPollMode::Accept, 500ms) == 1 && run)
        {
            try
            {
                auto session = make_shared<Session>(listenSocket.Accept());
                Server::Log(session.get(), "connected");

                session->socket.SetNonBlocking(true);
                session->socket.SetTcpNoDelay(true);
                session->state = SessionState::Request;
                session->timeout = steady_clock::now() + SessionTimeout;

                {
                    lock_guard<Spinlock> lk(lock);
                    idleSessions.push_back(session);
                    WakeIdleThread();
                }
            }
            catch(exception&)
            {

            }
        }
    }
}

void Server::WakeIdleThread() {
    idleThreadWakeSockets[0].Send("w", 1);
}

void Server::WakeActiveThreads() {
    activeThreadWakeCond.notify_all();
}

void Server::IdleThreadRunLoop()
{
    vector<pollfd> pollInfo;
    vector<shared_ptr<Session>> keep;
    keep.reserve(idleSessions.capacity());

    while(run)
    {
        {
            lock_guard<Spinlock> lk(lock);

            int sessionsActivated = 0;
            auto now = steady_clock::now();

            {
                int i = 0;

                for(; i < (int)pollInfo.size() - 1; ++i)
                {
                    auto& session = idleSessions[i];

                    if(pollInfo[i].revents == 0)
                    {
                        if(now < session->timeout)
                        {
                            keep.push_back(move(session));
                        }
                        else
                        {
                            Log(session.get(), "disconnecting (timeout)");
                        }
                    }
                    else
                    {
                        if((pollInfo[i].revents & (POLLRDNORM | POLLWRNORM)) != 0)
                        {
                            activeSessions.push_back(move(session));
                            ++sessionsActivated;
                        }
                        else
                        {
                            Log(session.get(), "disconnecting");
                        }
                    }
                }

                for(auto sz = idleSessions.size(); i != sz; ++i)
                    keep.push_back(move(idleSessions[i]));
            }

            idleSessions.swap(keep);
            keep.clear();
            pollInfo.clear();

            if(sessionsActivated > 0)
                WakeActiveThreads();

            int waitTypes[2] = { POLLRDNORM, POLLWRNORM };
            pollInfo.resize(idleSessions.size() + 1);

            for(int i = 0; i < (int)pollInfo.size() - 1; ++i) {
                pollInfo[i].fd = idleSessions[i]->socket.handle();
                pollInfo[i].events = waitTypes[(int)idleSessions[i]->state];
            }

            pollInfo.back().fd = idleThreadWakeSockets[1].handle();
            pollInfo.back().events = POLLRDNORM;
        }

        int ret = poll(pollInfo.data(), (ULONG)pollInfo.size(), (int)SessionTimeout.count() + 100);
        if(ret == SOCKET_ERROR)
            throw runtime_error("failed to poll sockets: "s + to_string(WSAGetLastError()));

        if(!run)
            return;

        char wakeBytes[1024];
        while(idleThreadWakeSockets[1].Recv(wakeBytes, sizeof(wakeBytes)) > 0){}
    }
}

void Server::ActiveThreadRunLoop()
{
    shared_ptr<Session> session;
    milliseconds timeSliceLength = 5ms;

    while(run)
    {
        bool stillActive = false;

        if(session)
        {
            try
            {
                if(session->state == SessionState::Request) {
                    stillActive = ReceiveRequest(session);
                }
                else if(session->state == SessionState::Response) {
                    stillActive = SendResponse(session, timeSliceLength);
                }
            }
            catch(SocketException& ex) {
                Log(session.get(), format("% (%)", ex.what(), ex.code()));
                session->state = SessionState::Done;
            }
            catch(exception& ex) {
                Log(session.get(), ex.what());
                session->state = SessionState::Done;
            }
        }

        {
            unique_lock<Spinlock> lk(lock);

            if(session)
            {
                if(session->state != SessionState::Done)
                {
                    if(stillActive)
                    {
                        activeSessions.push_back(move(session));
                    }
                    else
                    {
                        session->timeout = steady_clock::now() + SessionTimeout;
                        idleSessions.push_back(move(session));
                        WakeIdleThread();
                    }
                }
                else
                {
                    Log(session.get(), "disconnecting");
                }

                session = nullptr;
            }

            while(activeSessions.empty() && run)
                activeThreadWakeCond.wait(lk);

            if(run)
            {
                timeSliceLength = min(1000ms / activeSessions.size(), MaxTimeSlice);
                session = move(activeSessions.front());
                activeSessions.pop_front();
            }
        }
    }
}

bool Server::ReceiveRequest(const shared_ptr<Session>& session)
{
    session->buffer.resize(BufferSize);

    int ret = session->socket.Recv(session->buffer);
    if(ret == -1) {
        return false;
    }
    else if(ret == 0) {
        session->state = SessionState::Done;
        return false;
    }

    session->buffer.resize(ret);

    HttpRequest req;

    if(!req.Parse(session->buffer))
    {
        auto resp = HttpResponse::Create(HttpStatus::BadRequest, session->keepAlive);
        resp.Serialize(session->buffer);
        session->bufferOffset = 0;
        session->contentLength = resp.content.size();
        session->fin = ifstream();
        session->state = SessionState::Response;
        return true;
    }

    if(req.method != HttpMethod::Get)
    {
        auto resp = HttpResponse::Create(HttpStatus::MethodNotAllowed, session->keepAlive);
        resp.Serialize(session->buffer);
        session->bufferOffset = 0;
        session->contentLength = resp.content.size();
        session->fin = ifstream();
        session->state = SessionState::Response;
        return true;
    }

    auto connection = req.fields.find("Connection");
    if(connection != req.fields.end() && connection->second == "close")
        session->keepAlive = false;

    string localPath = httpdocs + Http::DecodeURL(req.uri);

    if(localPath.back() == '/')
        localPath += defaultPage;

    for(char& ch : localPath)
    {
        if(ch == '/')
            ch = '\\';
    }

    ifstream fin(localPath, ios::in | ios::binary);
    if(!fin.is_open())
    {
        auto resp = HttpResponse::Create(HttpStatus::NotFound, session->keepAlive);
        resp.Serialize(session->buffer);
        session->bufferOffset = 0;
        session->contentLength = resp.content.size();
        session->fin = ifstream();
        session->state = SessionState::Response;
        return true;
    }

    fin.seekg(0, ios::end);
    size_t fileSize = (size_t)fin.tellg();
    fin.seekg(0, ios::beg);

    auto fileExtension = localPath.substr(localPath.find_last_of(".") + 1);

    HttpResponse resp;
    resp.fields["Content-Type"] = MimeTypes::TypeFor(fileExtension);
    resp.fields["Content-Encoding"] = "identity";
    resp.fields["Connection"] = session->keepAlive ? "keep-alive" : "close";
    resp.fields["Accept-Ranges"] = "bytes";

    size_t contentLength;

    vector<Http::ContentRange> ranges;

    auto rangeField = req.fields.find("Range");
    if(rangeField != req.fields.end())
        ranges = Http::ParseRange(rangeField->second);

    if(!ranges.empty())
    {
        auto& range = ranges[0];

        size_t rangeStart;
        size_t rangeEnd;

        if(range.first && range.second) {
            rangeStart = *range.first;
            rangeEnd = *range.second;
        }
        else if(range.first) {
            rangeStart = *range.first;
            rangeEnd = fileSize - 1;
        }
        else if(range.second) {
            rangeStart = fileSize - *range.second;
            rangeEnd = fileSize - 1;
        }
        else {
            rangeStart = 0;
            rangeEnd = fileSize - 1;
        }

        if(rangeStart > rangeEnd ||
           rangeStart >= fileSize ||
           rangeEnd >= fileSize)
        {
            auto resp = HttpResponse::Create(HttpStatus::RequestedRangeNotSatisfiable, session->keepAlive);
            resp.Serialize(session->buffer);
            session->bufferOffset = 0;
            session->contentLength = resp.content.size();
            session->fin = ifstream();
            session->state = SessionState::Response;
            return true;
        }

        contentLength = rangeEnd - rangeStart + 1;

        resp.status = HttpStatus::PartialContent;
        resp.fields["Content-Length"] = to_string(contentLength);
        resp.fields["Content-Range"] = format("bytes %-%/%", rangeStart, rangeEnd, fileSize);
        fin.seekg(rangeStart);
    }
    else
    {
        contentLength = fileSize;

        resp.status = HttpStatus::OK;
        resp.fields["Content-Length"] = to_string(contentLength);
    }

    resp.Serialize(session->buffer);
    session->bufferOffset = 0;
    session->contentLength = contentLength;
    session->fin = move(fin);
    session->state = SessionState::Response;

    return true;
}

bool Server::SendResponse(const shared_ptr<Session>& session, milliseconds timeSliceLength)
{
    auto timeSliceEnd = steady_clock::now() + timeSliceLength;

    do
    {
        if(session->bufferOffset == session->buffer.size() &&
           session->contentLength > 0)
        {
            size_t readCount = min(BufferSize, session->contentLength);
            session->buffer.resize(readCount);

            if(!session->fin.read(session->buffer.data(), readCount))
            {
                Log(session.get(), "failed to read from file");
                session->state = session->keepAlive ? SessionState::Request : SessionState::Done;
                return false;
            }

            session->contentLength -= readCount;
            session->bufferOffset = 0;
        }

        char* pBuffer = session->buffer.data() + session->bufferOffset;
        size_t size = session->buffer.size() - session->bufferOffset;

        int sent = session->socket.Send(pBuffer, size);
        if(sent == -1)
            return false;

        session->bufferOffset += sent;

        if(session->bufferOffset == session->buffer.size() &&
           session->contentLength == 0)
        {
            session->state = session->keepAlive ? SessionState::Request : SessionState::Done;
            break;
        }

    } while(steady_clock::now() < timeSliceEnd);

    return true;
}
