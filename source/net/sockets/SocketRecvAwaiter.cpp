/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <stdexcept>
#include <net/sockets/SocketRecvAwaiter.h>
#include <net/sockets/OSSockets.h>
#include <net/sockets/SocketController.h>
#include <net/sockets/socket_error.h>

using namespace std;

SocketRecvAwaiter::SocketRecvAwaiter(int socket, char* bufferPtr, size_t bufferSize)
    : socket(socket), bufferPtr(bufferPtr), bufferSize(bufferSize)
{
}

bool SocketRecvAwaiter::ready() {
    return false;
}

void SocketRecvAwaiter::suspend(std::experimental::coroutine_handle<> handle)
{
    this->handle = handle;

    SocketController::instance.Receive(
        socket, bufferPtr, bufferSize, this,
        [](int result, int error, void* context) {
            auto awaiter = (SocketRecvAwaiter*)context;
            awaiter->result = result;
            awaiter->error = error;
            awaiter->handle.resume();
        });
}

int SocketRecvAwaiter::resume()
{
    if (result == -1)
        throw socket_error("recv operation failed", error);

    return result;
}
