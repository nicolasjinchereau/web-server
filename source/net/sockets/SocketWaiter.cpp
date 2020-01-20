/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <net/sockets/Socket.h>
#include <net/sockets/SocketWaiter.h>
#include <net/sockets/OSSockets.h>
#include <system/format.h>
#include <system/Console.h>
#include <mutex>
#include <thread>
#include <algorithm>

namespace
{
    short pollEventTypes[4] = {
        POLLWRNORM, // Connect
        POLLRDNORM, // Accept
        POLLWRNORM, // Send
        POLLRDNORM  // Recv
    };
}

SocketWaiter::SocketWaiter()
{
    Socket tmp(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::TCP);
    tmp.Bind(RequestWakePort, nullptr, true);
    tmp.Listen();
    
    wakeSockets[0] = Socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::TCP);
    wakeSockets[0].Connect(RequestWakePort, LoopbackAddress);
    wakeSockets[1] = tmp.Accept();
    wakeSockets[1].SetBlocking(false);
    
    tmp.Close();
    
    sockets.reserve(64);
    pollfds.reserve(64);

    sockets.push_back(SocketWaitInfo{ SocketOperationType::Recv, wakeSockets[1].handle(), nullptr, nullptr, nullptr });
    pollfds.push_back(pollfd{ (Socket::HandleType)wakeSockets[1].handle(), POLLRDNORM, 0 });

    run = true;
    runLoopThread = std::thread(&SocketWaiter::RunLoop, this);
}

SocketWaiter::~SocketWaiter()
{
    run = false;
    Wake();
    runLoopThread.join();
}

bool SocketWaiter::Wait(SocketOperationType type, int socket, void* context, void(*callback)(void* context, intmax_t num), Dispatcher* dispatcher)
{
    std::lock_guard<std::mutex> lk(mut);
    sockets.push_back(SocketWaitInfo{ type, socket, context, callback, dispatcher });
    
    Wake();

    return true;
}

void SocketWaiter::Wake() {
    wakeSockets[0].Send("w", 1);
}

void SocketWaiter::RunLoop()
{
    while (run) {
        UpdateOperations();
        WaitForEvents();
    }
}

void SocketWaiter::UpdateOperations()
{
    std::lock_guard<std::mutex> lk(mut);

    for (size_t i = 1; i < pollfds.size(); ++i)
    {
        auto& info = sockets[i];
        auto revents = pollfds[i].revents;

        if (revents) {
            int result = (revents & (POLLRDNORM | POLLWRNORM | POLLHUP)) ? 0 : -1;
            info.dispatcher->InvokeAsync(info.callback, info.context, result);
        }
    }

    for (size_t i = pollfds.size() - 1; i != 0; --i)
    {
        if (pollfds[i].revents)
            sockets.erase(sockets.begin() + i);
    }

    pollfds.clear();

    for (size_t i = 0; i < sockets.size(); ++i)
    {
        auto& info = sockets[i];
        auto events = pollEventTypes[(int)info.type];
        auto socket = (Socket::HandleType)info.socket;

        pollfds.push_back(pollfd{ socket, events, 0 });
    }
}

void SocketWaiter::WaitForEvents()
{
    // clear the wake socket
    char wakeBytes[1024];
    while (wakeSockets[1].Recv(wakeBytes, sizeof(wakeBytes)) > 0) {}

    // poll for socket events
    int millis = -1;
    int ret = poll(pollfds.data(), (nfds_t)pollfds.size(), millis);
    if (ret == Socket::SocketError)
        Console::WriteLine("failed to poll sockets: %", (int)errno);
}
