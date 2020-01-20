/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <net/sockets/Socket.h>
#include <net/sockets/SocketController.h>
#include <net/sockets/OSSockets.h>
#include <mutex>
#include <thread>
#include <algorithm>

SocketController SocketController::instance;

SocketController::SocketController()
{
}

SocketController::~SocketController()
{
}

void SocketController::Connect(
    int socket,
    const std::string& ip,
    int port,
    void* context,
    SocketCallback callback
)
{
    auto op = new SocketOperation{ &Dispatcher::current(), socket, nullptr, 0, context, 0, callback };

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    int ret = connect((Socket::HandleType)socket, (sockaddr*)&addr, sizeof(sockaddr_in));
    if (ret == Socket::SocketError)
    {
        int err = errno;
        if (err == S_EWOULDBLOCK)
        {
            socketWaiter.Wait(SocketOperationType::Connect, socket, op, &SocketController::ContinueConnect, op->dispatcher);
        }
        else
        {
            op->error = err;
            Dispatcher::current().InvokeAsync(&FinalizeConnect, op, -1);
        }
    }
    else
    {
        Dispatcher::current().InvokeAsync(&FinalizeConnect, op, 0);
    }
}

void SocketController::Accept(
    int socket,
    void* context,
    SocketCallback callback
)
{
    auto op = new SocketOperation{ &Dispatcher::current(), socket, nullptr, 0, context, 0, callback };

    sockaddr_in addr{};
    socklen_t len = sizeof(addr);

    int clientSocket = (int)accept((Socket::HandleType)socket, (sockaddr*)&addr, &len);
    if (clientSocket == Socket::InvalidSocket)
    {
        int err = errno;
        if (err == S_EWOULDBLOCK || err == EAGAIN)
        {
            socketWaiter.Wait(SocketOperationType::Accept, socket, op, &SocketController::ContinueAccept, op->dispatcher);
        }
        else
        {
            op->error = err;
            Dispatcher::current().InvokeAsync(&FinalizeAccept, op, -1);
        }
    }
    else
    {
        Dispatcher::current().InvokeAsync(&FinalizeAccept, op, clientSocket);
    }
}

void SocketController::Send(
    int socket,
    const char* bufferPtr,
    size_t bufferSize,
    void* context,
    SocketCallback callback
)
{
    auto op = new SocketOperation{ &Dispatcher::current(), socket, (char*)bufferPtr, bufferSize, context, 0, callback };
    
    int sent = send((Socket::HandleType)socket, bufferPtr, (int)bufferSize, 0);
    if (sent == Socket::SocketError)
    {
        int err = errno;
        if (err == S_EWOULDBLOCK)
        {
            socketWaiter.Wait(SocketOperationType::Send, socket, op, &SocketController::ContinueSend, op->dispatcher);
        }
        else
        {
            op->error = err;
            Dispatcher::current().InvokeAsync(&FinalizeSend, op, -1);
        }
    }
    else
    {
        Dispatcher::current().InvokeAsync(&FinalizeSend, op, sent);
    }
}

void SocketController::Receive(
    int socket,
    char* bufferPtr,
    size_t bufferSize,
    void* context,
    SocketCallback callback
)
{
    auto op = new SocketOperation{ &Dispatcher::current(), socket, bufferPtr, bufferSize, context, 0, callback };

    int received = recv((Socket::HandleType)socket, bufferPtr, (int)bufferSize, 0);
    if (received == Socket::SocketError)
    {
        int err = errno;
        if (err == S_EWOULDBLOCK)
        {
            socketWaiter.Wait(SocketOperationType::Recv, socket, op, &SocketController::ContinueRecv, op->dispatcher);
        }
        else
        {
            op->error = err;
            Dispatcher::current().InvokeAsync(&FinalizeReceive, op, -1);
        }
    }
    else
    {
        op->error = 0;
        Dispatcher::current().InvokeAsync(&FinalizeReceive, op, received);
    }
}

void SocketController::ContinueConnect(void* operation, intmax_t result)
{
    auto op = (SocketOperation*)operation;

    if (result == -1)
    {
        int error;
        socklen_t sz = sizeof(error);
        int ret = getsockopt((Socket::HandleType)op->socket, SOL_SOCKET, SO_ERROR, (char*)&error, &sz);
        op->error = error;
    }

    Dispatcher::current().InvokeAsync(&FinalizeConnect, op, result);
}

void SocketController::ContinueAccept(void* operation, intmax_t result)
{
    auto op = (SocketOperation*)operation;

    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    int clientSocket = (int)accept((Socket::HandleType)op->socket, (sockaddr*)&addr, &len);

    if (clientSocket == -1)
        op->error = errno;
    
    Dispatcher::current().InvokeAsync(&FinalizeAccept, op, clientSocket);
}

void SocketController::ContinueSend(void* operation, intmax_t result)
{
    auto op = (SocketOperation*)operation;

    int sent = send((Socket::HandleType)op->socket, op->bufferPtr, (int)op->bufferSize, 0);
    if (sent == -1)
        op->error = errno;

    Dispatcher::current().InvokeAsync(&FinalizeSend, op, sent);
}

void SocketController::ContinueRecv(void* operation, intmax_t result)
{
    auto op = (SocketOperation*)operation;

    int received = recv((Socket::HandleType)op->socket, op->bufferPtr, (int)op->bufferSize, 0);
    if (received == -1)
        op->error = errno;
    
    Dispatcher::current().InvokeAsync(&FinalizeReceive, op, received);
}

void SocketController::FinalizeConnect(void* operation, intmax_t result)
{
    auto op = std::unique_ptr<SocketOperation>((SocketOperation*)operation);
    op->callback((int)result, op->error, op->context);
}

void SocketController::FinalizeAccept(void* operation, intmax_t result)
{
    auto op = std::unique_ptr<SocketOperation>((SocketOperation*)operation);
    op->callback((int)result, op->error, op->context);
}

void SocketController::FinalizeSend(void* operation, intmax_t result)
{
    auto op = std::unique_ptr<SocketOperation>((SocketOperation*)operation);
    op->callback((int)result, op->error, op->context);
}

void SocketController::FinalizeReceive(void* operation, intmax_t result)
{
    auto op = std::unique_ptr<SocketOperation>((SocketOperation*)operation);
    op->callback((int)result, op->error, op->context);
}
