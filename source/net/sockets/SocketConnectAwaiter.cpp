/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <stdexcept>
#include <net/sockets/SocketConnectAwaiter.h>
#include <net/sockets/OSSockets.h>
#include <net/sockets/SocketController.h>
#include <net/sockets/socket_error.h>

using namespace std;

SocketConnectAwaiter::SocketConnectAwaiter(int socket, int port, const std::string& ip)
{
    this->socket = socket;
    this->port = port;
    this->ip = ip;
}

bool SocketConnectAwaiter::ready() {
    return false;
}

void SocketConnectAwaiter::suspend(std::experimental::coroutine_handle<> handle)
{
    this->handle = handle;

    SocketController::instance.Connect(
        socket, ip, port, this,
        [](int result, int error, void* context) {
            auto awaiter = (SocketConnectAwaiter*)context;
            awaiter->result = result;
            awaiter->error = error;
            awaiter->handle.resume();
        });
}

void SocketConnectAwaiter::resume()
{
    if (result == -1)
        throw socket_error("connect operation failed", error);
}
