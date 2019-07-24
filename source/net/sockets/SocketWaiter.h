/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <cstdint>
#include <net/sockets/Socket.h>
#include <net/sockets/OSSockets.h>
#include <system/Dispatcher.h>
#include <experimental/coroutine>
#include <functional>
#include <memory>

enum class SocketOperationType
{
    Error = -1,
    Connect,
    Accept,
    Send,
    Recv,
};

struct SocketWaitInfo
{
    SocketOperationType type;
    int socket;
    void* context;
    void(*callback)(void* context, intmax_t num);
    Dispatcher* dispatcher;
};

class SocketWaiter
{
    static constexpr int RequestWakePort = 32190;
    static constexpr int SendWakePort = 32191;
    static constexpr const char* LoopbackAddress = "127.0.0.1";

public:
    std::atomic<bool> run = false;
    std::thread runLoopThread;
    Socket wakeSockets[2];
    std::vector<SocketWaitInfo> sockets;
    std::vector<pollfd> pollfds;
    std::mutex mut;
    
    SocketWaiter();
    ~SocketWaiter();
    
    // callback: result = 0 for success, result = -1 for failure
    // dispatcher: must not be null
    // context: optional context pointer returned in callback
    bool Wait(
        SocketOperationType type,
        int socket,
        void* context,
        void(*callback)(void* context, intmax_t result),
        Dispatcher* dispatcher
    );

private:
    void Wake();
    void RunLoop();
    void UpdateOperations();
    void WaitForEvents();
};
