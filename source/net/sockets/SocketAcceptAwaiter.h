/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdint>
#include <chrono>
#include <vector>
#include <exception>
#include <string>
#include <vector>
#include <experimental/coroutine>
#include <system/Dispatcher.h>
#include <system/Task.h>
#include <system/Awaiter.h>
#include <net/sockets/Socket.h>
#include <net/sockets/Socket.h>

class SocketAcceptAwaiter : public Awaiter<Socket>
{
    std::experimental::coroutine_handle<> handle;
    int socket = -1;
    int result = 0;
    int error = 0;

public:
    SocketAcceptAwaiter(int socket);
    ~SocketAcceptAwaiter();

    SocketAcceptAwaiter(const SocketAcceptAwaiter&) = delete;
    SocketAcceptAwaiter& operator=(const SocketAcceptAwaiter&) = delete;

    SocketAcceptAwaiter(SocketAcceptAwaiter&&) = default;
    SocketAcceptAwaiter& operator=(SocketAcceptAwaiter&&) = default;

    virtual bool ready();
    virtual void suspend(std::experimental::coroutine_handle<> handle);
    virtual Socket resume();
};
