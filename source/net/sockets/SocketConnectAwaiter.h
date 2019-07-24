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

struct SocketConnectAwaiter : public Awaiter<void>
{
    std::experimental::coroutine_handle<> handle;
    int socket = -1;
    int port = 0;
    std::string ip;
    int result = 0;
    int error = 0;

    SocketConnectAwaiter() = delete;
    SocketConnectAwaiter(int socket, int port, const std::string& ip);
    
    SocketConnectAwaiter(const SocketConnectAwaiter&) = delete;
    SocketConnectAwaiter& operator=(const SocketConnectAwaiter&) = delete;
    
    SocketConnectAwaiter(SocketConnectAwaiter&&) = default;
    SocketConnectAwaiter& operator=(SocketConnectAwaiter&&) = default;

    bool ready() override;
    void suspend(std::experimental::coroutine_handle<> handle) override;
    
    // return: 1 for success, 0 for failure
    void resume() override;
};
