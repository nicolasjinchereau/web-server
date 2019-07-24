/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <stdexcept>
#include <net/sockets/SocketAcceptAwaiter.h>
#include <net/sockets/OSSockets.h>
#include <net/sockets/SocketController.h>
#include <net/sockets/socket_error.h>

using namespace std;

SocketAcceptAwaiter::SocketAcceptAwaiter(int socket)
    : socket(socket)
{
    
}

SocketAcceptAwaiter::~SocketAcceptAwaiter()
{
    
}

bool SocketAcceptAwaiter::ready()
{
    return false;
}

void SocketAcceptAwaiter::suspend(std::experimental::coroutine_handle<> handle)
{
    this->handle = handle;

    SocketController::instance.Accept(
        socket, this,
        [](int result, int error, void* context) {
            auto awaiter = (SocketAcceptAwaiter*)context;
            awaiter->result = result;
            awaiter->error = error;
            awaiter->handle.resume();
        });
}

Socket SocketAcceptAwaiter::resume()
{
    if (result == -1)
        throw socket_error("accept operation failed", error);
    
    return Socket(result);
}
