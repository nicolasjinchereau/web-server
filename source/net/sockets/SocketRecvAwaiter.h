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

struct SocketRecvAwaiter : public Awaiter<int>
{
    std::experimental::coroutine_handle<> handle;
    char* bufferPtr;
    size_t bufferSize;
    int socket = -1;
    int result = 0;
    int error = 0;

    SocketRecvAwaiter() = delete;
    SocketRecvAwaiter(int socket, char* bufferPtr, size_t bufferSize);

    SocketRecvAwaiter(const SocketRecvAwaiter&) = delete;
    SocketRecvAwaiter& operator=(const SocketRecvAwaiter&) = delete;

    SocketRecvAwaiter(SocketRecvAwaiter&&) = default;
    SocketRecvAwaiter& operator=(SocketRecvAwaiter&&) = default;

    bool ready() override;
    void suspend(std::experimental::coroutine_handle<> handle) override;
    int resume() override;
};
