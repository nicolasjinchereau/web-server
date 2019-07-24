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

struct SocketSendAwaiter : public Awaiter<int>
{
    std::experimental::coroutine_handle<> handle;
    const char* bufferPtr;
    size_t bufferSize;
    int socket = -1;
    int result = 0;
    int error = 0;

    SocketSendAwaiter() = delete;
    SocketSendAwaiter(int socket, const char* bufferPtr, size_t bufferSize);
    
    SocketSendAwaiter(const SocketSendAwaiter&) = delete;
    SocketSendAwaiter& operator=(const SocketSendAwaiter&) = delete;

    SocketSendAwaiter(SocketSendAwaiter&&) = default;
    SocketSendAwaiter& operator=(SocketSendAwaiter&&) = default;

    bool ready() override;
    void suspend(std::experimental::coroutine_handle<> handle) override;
    int resume() override;
};
