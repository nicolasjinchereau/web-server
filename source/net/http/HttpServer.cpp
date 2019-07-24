/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <net/http/MimeTypes.h>
#include <net/http/HttpServer.h>
#include <net/sockets/socket_error.h>
#include <system/format.h>
#include <system/Spinlock.h>
#include <system/Console.h>
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

using namespace std;
using namespace chrono;

HttpServer::HttpServer()
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

HttpServer::~HttpServer()
{
    Stop();

#ifdef _WIN32
    WSACleanup();
#endif
}

void HttpServer::Start(int port, const string& docsPath, size_t threadCount)
{
    Stop();

    try
    {
        Console::WriteLine("Server starting");

        this->port = port;
        this->httpdocs = docsPath;
        this->run = true;

        if(this->httpdocs.back() == '\\')
            this->httpdocs.pop_back();

        // create worker threads to handle incoming requests
        auto threadCount = thread::hardware_concurrency();

        for (size_t i = 0; i < threadCount; ++i)
            requestThreads.push_back(std::thread(&HttpServer::RequestDispatchEntryPoint, this));

        // start a looping coroutine to accept incoming connections and add them to the queue
        ListenForConnections();
    }
    catch(exception&)
    {
        Stop();
        throw;
    }
}

void HttpServer::Stop()
{
    if(run)
    {
        run = false;
        listenSocket.Close();

        turnstyle.PermitAll();

        for (int i = 0; i < requestThreads.size(); ++i)
            requestThreads[i].join();

        requestThreads.clear();
        clientSockets.clear();

        port = 0;
        httpdocs.clear();

        Console::WriteLine("Server stopped");
    }
}

Task<void> HttpServer::ListenForConnections()
{
    listenSocket = Socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::TCP);
    listenSocket.SetBlocking(false);
    listenSocket.Bind(port);
    listenSocket.Listen();

    while (run)
    {
        try
        {
            Socket socket = co_await listenSocket.AcceptAsync();

            socket.SetBlocking(false);
            socket.SetTcpNoDelay(true);

            Console::WriteLine(socket, "client connected");

            // add client socket to queue, to be picked up by a worker thread
            EnqueueClient(std::move(socket));
        }
        catch (exception& ex)
        {
            Console::WriteLine(ex.what());
        }
    }
}

void HttpServer::RequestDispatchEntryPoint()
{
    Dispatcher::current().InvokeAsync([](auto p, auto n) { ((HttpServer*)p)->GetRequests(); }, this);
    Dispatcher::current().Run();
}

Task<void> HttpServer::GetRequests()
{
    try
    {
        while (run)
        {
            // wait for a new client connection
            co_await turnstyle;

            // pull next client from queue and begin accepting requests
            Socket socket = GetNextClient();

            if (run && socket.valid())
            {
                // start a looping coroutine to process requests for this client
                AcceptRequests(std::move(socket));
            }
        }
    }
    catch(exception& ex)
    {
        Console::WriteLine(ex.what());
    }

    Dispatcher::current().Quit();
}

void HttpServer::EnqueueClient(Socket socket)
{
    std::lock_guard<mutex> lk(mut);
    clientSockets.push_back(std::move(socket));
    turnstyle.PermitOne();
}

Socket HttpServer::GetNextClient()
{
    std::lock_guard<mutex> lk(mut);

    Socket socket;

    if (!clientSockets.empty()) {
        socket = std::move(clientSockets.front());
        clientSockets.pop_front();
    }

    return socket;
}

Task<void> HttpServer::AcceptRequests(Socket socket)
{
    try
    {
        time_point timeout = steady_clock::time_point::min();
        bool keepAlive = true;
        std::vector<char> requestBuffer;

        while (keepAlive)
        {
            requestBuffer.resize(BufferSize);
            
            Console::WriteLine(socket, "waiting for request");
            
            int received = co_await socket.RecvAsync(requestBuffer.data(), requestBuffer.size());
            if (received == 0)
            {
                Console::WriteLine(socket, "client disconnected");
                break;
            }

            requestBuffer.resize(received);

            HttpRequest req;

            if (!req.Parse(requestBuffer)) {
                Console::WriteLine(socket, "bad request");
                co_await SendError(socket, HttpStatus::BadRequest, keepAlive);
                continue;
            }

            if (req.method != HttpMethod::Get) {
                Console::WriteLine(socket, "method not allowed");
                co_await SendError(socket, HttpStatus::MethodNotAllowed, keepAlive);
                continue;
            }

            auto connection = req.fields.find("Connection");
            if (connection != req.fields.end() && connection->second == "close")
            {
                Console::WriteLine(socket, "Connection: close");
                keepAlive = false;
            }

            string localPath = httpdocs + Http::DecodeURL(req.uri);
            if (localPath.back() == '/')
                localPath += defaultPage;

#ifdef _WIN32
            for (char& ch : localPath)
            {
                if (ch == '/')
                    ch = '\\';
            }
#endif

            Console::WriteLine(socket, "requested file - %", req.uri);

            ifstream fin(localPath, ios::in | ios::binary);
            if (!fin.is_open()) {
                Console::WriteLine(socket, "file not found - %", req.uri);
                co_await SendError(socket, HttpStatus::NotFound, keepAlive);
                continue;
            }

            fin.seekg(0, ios::end);
            size_t fileSize = (size_t)fin.tellg();
            fin.seekg(0, ios::beg);

            auto fileExtension = localPath.substr(localPath.find_last_of(".") + 1);

            vector<Http::ContentRange> ranges;

            auto rangeField = req.fields.find("Range");
            if (rangeField != req.fields.end())
                ranges = Http::ParseRange(rangeField->second);

            size_t rangeStart;
            size_t rangeEnd;
            int hasRanges = GetRangeInfo(ranges, fileSize, &rangeStart, &rangeEnd);

            HttpResponse resp;
            resp.fields["Content-Type"] = MimeTypes::TypeFor(fileExtension);
            resp.fields["Content-Encoding"] = "identity";
            resp.fields["Connection"] = keepAlive ? "keep-alive" : "close";
            resp.fields["Accept-Ranges"] = "bytes";

            size_t contentLength = 0;

            if (hasRanges == 1)
            {
                contentLength = rangeEnd - rangeStart + 1;
                resp.status = HttpStatus::PartialContent;
                resp.fields["Content-Length"] = to_string(contentLength);
                resp.fields["Content-Range"] = format("bytes %-%/%", rangeStart, rangeEnd, fileSize);
                fin.seekg(rangeStart);
            }
            else if (hasRanges == 0)
            {
                contentLength = fileSize;
                resp.status = HttpStatus::OK;
                resp.fields["Content-Length"] = to_string(contentLength);
            }
            else // hasRanges == -1
            {
                Console::WriteLine(socket, "range not satisfiable");
                co_await SendError(socket, HttpStatus::RequestedRangeNotSatisfiable, keepAlive);
                continue;
            }

            co_await SendFile(socket, std::move(resp), std::move(fin), contentLength);

            Console::WriteLine(socket, "successfully sent file - %", req.uri);
        }

        Console::WriteLine(socket, "exited request loop");
    }
    catch (exception& ex) {
        Console::WriteLine(socket, ex.what());
    }
}

Task<void> HttpServer::SendError(Socket& socket, HttpStatus status, bool keepAlive)
{
    try
    {
        Console::WriteLine(socket, "Send Error");
        std::vector<char> buffer;
        auto resp = HttpResponse::Create(status, keepAlive);
        resp.Serialize(buffer);

        // send response (buffer already includes body)
        const char* bufferPtr = buffer.data();
        size_t bufferSize = buffer.size();

        while (bufferSize != 0)
        {
            int sent = co_await socket.SendAsync(bufferPtr, bufferSize);
            bufferPtr += sent;
            bufferSize -= sent;
        }
    }
    catch (exception& ex) {
        Console::WriteLine(ex.what());
    }
}

Task<void> HttpServer::SendFile(Socket& socket, HttpResponse response, std::ifstream fin, size_t contentLength)
{
    std::vector<char> buffer;
    response.Serialize(buffer);
    
    Console::WriteLine(socket, "sending response..");

    // send response header
    {
        const char* bufferPtr = buffer.data();
        size_t bufferSize = buffer.size();

        while (bufferSize != 0)
        {
            int sent = co_await socket.SendAsync(bufferPtr, bufferSize);
            bufferPtr += sent;
            bufferSize -= sent;
        }
    }

    // send response body (the file)
    while (contentLength > 0)
    {
        size_t readCount = std::min(BufferSize, contentLength);
        buffer.resize(readCount);
        contentLength -= readCount;
        
        if(!fin.read(buffer.data(), readCount)) {
            Console::WriteLine(socket, "failed to read from file");
            return;
        }

        const char* bufferPtr = buffer.data();
        size_t bufferSize = buffer.size();

        while (bufferSize != 0)
        {
            int sent = co_await socket.SendAsync(bufferPtr, bufferSize);
            bufferPtr += sent;
            bufferSize -= sent;
        }
    }
}

int HttpServer::GetRangeInfo(std::vector<Http::ContentRange>& ranges, size_t fileSize, size_t* rangeStart, size_t* rangeEnd)
{
    if (ranges.empty())
        return 0;

    assert(rangeStart);
    assert(rangeEnd);

    auto& range = ranges[0];

    size_t start;
    size_t end;

    if (range.first && range.second) {
        start = *range.first;
        end = *range.second;
    }
    else if (range.first) {
        start = *range.first;
        end = fileSize - 1;
    }
    else if (range.second) {
        start = fileSize - *range.second;
        end = fileSize - 1;
    }
    else {
        start = 0;
        end = fileSize - 1;
    }

    if (start > end ||
        start >= fileSize ||
        end >= fileSize)
    {
        return -1;
    }

    *rangeStart = start;
    *rangeEnd = end;

    return 1;
}
