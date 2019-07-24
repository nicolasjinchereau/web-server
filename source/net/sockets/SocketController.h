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
#include <net/sockets/SocketWaiter.h>
#include <experimental/coroutine>
#include <functional>
#include <memory>

enum class SocketResult
{
    Completed,
    Blocked,
    Failed,
    HungUp
};

// result: 0 for success, -1 for error.
// error: the errno/WSAGetLastError if result is -1
typedef void(*SocketCallback)(int result, int error, void* context);

struct SocketOperation
{
    Dispatcher* dispatcher = nullptr;
    int socket = 0;
    char* bufferPtr;
    size_t bufferSize;
    void* context = nullptr;
    int error = 0;
    SocketCallback callback = nullptr;
};

class SocketController
{
public:
    SocketController();
    ~SocketController();
    
    void Connect(int socket, const std::string& ip, int port, void* context, SocketCallback callback);
    void Accept(int socket, void* context, SocketCallback callback);
    void Send(int socket, const char* bufferPtr, size_t size, void* context, SocketCallback callback);
    void Receive(int socket, char* bufferPtr, size_t size, void* context, SocketCallback callback);

    static SocketController instance;
private:
    static void ContinueConnect(void* operation, intmax_t result);
    static void ContinueAccept(void* operation, intmax_t result);
    static void ContinueSend(void* operation, intmax_t result);
    static void ContinueRecv(void* operation, intmax_t result);

    static void FinalizeConnect(void* operation, intmax_t result);
    static void FinalizeAccept(void* operation, intmax_t result);
    static void FinalizeSend(void* operation, intmax_t result);
    static void FinalizeReceive(void* operation, intmax_t result);

    SocketWaiter socketWaiter;
};
