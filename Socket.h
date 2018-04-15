/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdint>
#include <chrono>
#include <vector>
#include <exception>
#include <string>

enum class AddressFamily
{
    Unknown = -1,
    Unspecified,
    AppleTalk,
    Atm,
    Banyan,
    Ccitt,
    Chaos,
    Cluster,
    DataKit,
    DataLink,
    DecNet,
    Ecma,
    FireFox,
    HyperChannel,
    Ieee12844,
    ImpLink,
    InterNetwork,
    InterNetworkV6,
    Ipx,
    Irda,
    Iso,
    Lat,
    NetBios,
    NetworkDesigners,
    NS,
    Osi,
    Pup,
    Sna,
    Unix,
    VoiceView
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
    Accept,
    Read,
    Write,
    Error,
};

class SocketException : public std::exception
{
    int _code;
public:
    SocketException() : SocketException(-1){}
    SocketException(int code) : SocketException(code, "socket error"){}
    SocketException(int code, const char* message) : _code(code), std::exception(message){}
    SocketException(int code, const std::string& message) : SocketException(code, message.c_str()){}
    
    int code() const {
        return _code;
    }
};

class Socket
{
    uintptr_t _handle;
public:
    using milliseconds = std::chrono::milliseconds;
    
    Socket();
    Socket(AddressFamily family, SocketType type, ProtocolType protocol);
    Socket(uintptr_t handle);
    ~Socket();

    Socket(Socket&& socket);
    Socket& operator=(Socket&& socket);

    Socket(const Socket& sock) = delete;
    Socket& operator=(const Socket& sock) = delete;

    uintptr_t handle() const;
    bool valid() const;

    void SetNonBlocking(bool value);
    void SetTcpNoDelay(bool value);
    void Bind(int port, const char* address = nullptr, bool reuseAddress = false);
    void Listen();
    void Connect(int port, const char* address);
    Socket Accept();
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

    static std::string GetHostIP(const std::string& host);
};
