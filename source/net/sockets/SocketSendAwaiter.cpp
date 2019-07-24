/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <stdexcept>
#include <net/sockets/SocketSendAwaiter.h>
#include <net/sockets/OSSockets.h>
#include <net/sockets/SocketController.h>
#include <net/sockets/socket_error.h>

using namespace std;

SocketSendAwaiter::SocketSendAwaiter(int socket, const char* bufferPtr, size_t bufferSize)
    : socket(socket), bufferPtr(bufferPtr), bufferSize(bufferSize)
{
}

bool SocketSendAwaiter::ready() {
    return false;
}

void SocketSendAwaiter::suspend(std::experimental::coroutine_handle<> handle)
{
    this->handle = handle;

    SocketController::instance.Send(
        socket, bufferPtr, bufferSize, this,
        [](int result, int error, void* context) {
            auto awaiter = (SocketSendAwaiter*)context;
            awaiter->result = result;
            awaiter->error = error;
            awaiter->handle.resume();
        });
}

int SocketSendAwaiter::resume()
{
    if (result == -1)
        throw socket_error("send operation failed", error);

    return result;
}
