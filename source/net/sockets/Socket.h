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
#include <memory>
#include <experimental/coroutine>
#include <system/Dispatcher.h>
#include <system/Task.h>
#include <cstdint>

enum class AddressFamily
{
    Unknown = -1,
    InterNetwork,
    InterNetworkV6
};

enum class SocketType
{
    Stream,
    Dgram,
    Raw,
    Rdm,
    Seqpacket
};

enum class ProtocolType
{
    Unknown = -1,
    Unspecified,
    IP,
    Icmp,
    Igmp,
    Ggp,
    IPv4,
    St,
    TCP,
    Cbt,
    Egp,
    Igp,
    Rdp,
    Pup,
    Udp,
    Idp,
    IPv6,
    IPv6HopByHopOptions,
    IPv6RoutingHeader,
    IPv6FragmentHeader,
    IPSecEncapsulatingSecurityPayload,
    IPSecAuthenticationHeader,
    IcmpV6,
    IPv6NoNextHeader,
    IPv6DestinationOptions,
    ND,
    Iclfxbm,
    Pim,
    Pgm,
    L2TP,
    SCTP,
    Raw
};

enum class SocketPollMode
{
    Read,
    Write,
    Close,
    Error,
    Accept = Read,
    Connect = Write
};

class Socket
{
public:
    using milliseconds = std::chrono::milliseconds;
    
#ifdef _WIN32
    typedef uint64_t HandleType;
#else
    typedef int HandleType;
#endif

    static constexpr int InvalidSocket = ~0;
    static constexpr int SocketError = -1;

    Socket();
    Socket(AddressFamily family, SocketType type, ProtocolType protocol);
    Socket(int handle);
    ~Socket();

    Socket(const Socket& sock) = delete;
    Socket& operator=(const Socket& sock) = delete;

    Socket(Socket&& socket) noexcept;
    Socket& operator=(Socket&& socket) noexcept;

    int handle() const;
    bool valid() const;
    bool blocking() const;

    void SetBlocking(bool value);
    void SetTcpNoDelay(bool value);
    void Bind(int port, const char* address = nullptr, bool reuseAddress = false);
    void Listen();
    Socket Accept();
    void Connect(int port, const char* address);
    void Close();

    ///<summary>Returns the number of bytes received, or -1 if the socket
    ///is set to non-blocking mode and the operation would have blocked.</summary>
    ///<exception cref="SocketException">Thrown for all errors except EWOULDBLOCK</exception>
    int Recv(char* buffer, size_t length);

    ///<summary>Returns the number of bytes received, or -1 if the socket
    ///is set to non-blocking mode and the operation would have blocked.</summary>
    ///<exception cref="SocketException">Thrown for all errors except EWOULDBLOCK</exception>
    int Recv(std::vector<char>& buffer);

    ///<summary>Returns the number of bytes sent, or -1 if the socket
    ///is set to non-blocking mode and the operation would have blocked.</summary>
    ///<exception cref="SocketException">Thrown for all errors except EWOULDBLOCK</exception>
    int Send(const char* buffer, size_t length);

    ///<summary>Returns the number of bytes sent, or -1 if the socket
    ///is set to non-blocking mode and the operation would have blocked.</summary>
    ///<exception cref="SocketException">Thrown for all errors except EWOULDBLOCK</exception>
    int Send(const std::vector<char>& buffer);

    ///<summary>Returns the number of sockets ready</summary>
    ///<exception cref="SocketException">The operation failed</exception>
    int Poll(SocketPollMode mode, milliseconds timeout = milliseconds(-1));

    // throws socket_error on failure
    Task<void> ConnectAsync(int port, const std::string& address);
    
    // returns the newly connected socket
    // throws socket_error on failure
    Task<Socket> AcceptAsync();
    
    // returns the buffer argument for reuse
    // throws socket_error on failure
    // buffer must live until call completes
    Task<int> SendAsync(const char* bufferPtr, size_t bufferSize);

    // returns the buffer argument containing received data
    // throws socket_error on failure
    // buffer must live until call completes
    Task<int> RecvAsync(char* bufferPtr, size_t bufferSize);

    static std::string GetHostIP(const std::string& host);

private:
    void ThrowIfBlocking() const;

    int _handle = -1;
    bool _blocking = true;
};
