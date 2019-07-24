/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <type_traits>
#include <memory>
#include <net/sockets/Socket.h>
#include <net/sockets/OSSockets.h>
#include <net/sockets/SocketConnectAwaiter.h>
#include <net/sockets/SocketSendAwaiter.h>
#include <net/sockets/SocketRecvAwaiter.h>
#include <net/sockets/SocketAcceptAwaiter.h>

using namespace std;
using namespace chrono;
using namespace experimental;

int GetNativeAddressFamily(AddressFamily af)
{
    if (af == AddressFamily::InterNetwork) return AF_INET;
    else if (af == AddressFamily::InterNetworkV6) return AF_INET6;
    else throw std::runtime_error("invalid AddressFamily");
}

int GetNativeSocketType(SocketType st)
{
    if (st == SocketType::Stream) return SOCK_STREAM;
    else if (st == SocketType::Dgram) return SOCK_DGRAM;
    else if (st == SocketType::Raw) return SOCK_RAW;
    else if (st == SocketType::Rdm) return SOCK_RDM;
    else if (st == SocketType::Seqpacket) return SOCK_SEQPACKET;
    else throw std::runtime_error("invalid SocketType");
}

int GetNativeProtocolType(ProtocolType pt)
{
    if (pt == ProtocolType::Unknown) return -1;
    else if (pt == ProtocolType::Unspecified) return 0;
    else if (pt == ProtocolType::IP) return IPPROTO_ICMP;
    else if (pt == ProtocolType::Icmp) return IPPROTO_ICMP;
    else if (pt == ProtocolType::Igmp) return IPPROTO_IGMP;
    else if (pt == ProtocolType::Ggp) return IPPROTO_GGP;
    else if (pt == ProtocolType::IPv4) return IPPROTO_IPV4;
    else if (pt == ProtocolType::St) return IPPROTO_ST;
    else if (pt == ProtocolType::TCP) return IPPROTO_TCP;
    else if (pt == ProtocolType::Egp) return IPPROTO_EGP;
    else if (pt == ProtocolType::Igp) return IPPROTO_IGP;
    else if (pt == ProtocolType::Rdp) return IPPROTO_RDP;
    else if (pt == ProtocolType::Pup) return IPPROTO_PUP;
    else if (pt == ProtocolType::Udp) return IPPROTO_UDP;
    else if (pt == ProtocolType::Idp) return IPPROTO_IDP;
    else if (pt == ProtocolType::IPv6) return IPPROTO_IPV6;
    else if (pt == ProtocolType::IPv6HopByHopOptions) return 0;
    else if (pt == ProtocolType::IPv6RoutingHeader) return IPPROTO_ROUTING;
    else if (pt == ProtocolType::IPv6FragmentHeader) return IPPROTO_FRAGMENT;
    else if (pt == ProtocolType::IPSecEncapsulatingSecurityPayload) return IPPROTO_ESP;
    else if (pt == ProtocolType::IPSecAuthenticationHeader) return IPPROTO_AH;
    else if (pt == ProtocolType::IcmpV6) return IPPROTO_ICMPV6;
    else if (pt == ProtocolType::IPv6NoNextHeader) return IPPROTO_NONE;
    else if (pt == ProtocolType::IPv6DestinationOptions) return IPPROTO_DSTOPTS;
    else if (pt == ProtocolType::ND) return IPPROTO_ND;
    else if (pt == ProtocolType::Pim) return IPPROTO_PIM;
    else if (pt == ProtocolType::Pgm) return IPPROTO_PGM;
    else if (pt == ProtocolType::SCTP) return IPPROTO_SCTP;
    else if (pt == ProtocolType::Raw) return IPPROTO_RAW;
    else throw std::runtime_error("invalid ProtocolType");
}

#if defined(_WIN32)
void InitWinsock()
{
    WSAData wsdata;
    if (WSAStartup(WINSOCK_VERSION, &wsdata) != 0)
        throw std::runtime_error("failed to initialze winsock");

    if (wsdata.wVersion != WINSOCK_VERSION)
    {
        WSACleanup();
        throw std::runtime_error("failed to initialze winsock");
    }
}

void TermWinsock() {
    WSACleanup();
}
#else
void InitWinsock(){}
void TermWinsock(){}
#endif

Socket::Socket()
{
    InitWinsock();
}

Socket::Socket(AddressFamily family, SocketType type, ProtocolType protocol)
{
    InitWinsock();

    auto af = GetNativeAddressFamily(family);
    auto ty = GetNativeSocketType(type);
    auto pr = GetNativeProtocolType(protocol);

    _handle = (int)socket(af, ty, pr);
    if(_handle == InvalidSocket)
        throw runtime_error("failed to create socket");
}

Socket::Socket(int handle) {
    InitWinsock();
    _handle = handle;
    SetBlocking(true);
}

Socket::~Socket()
{
    if(_handle != InvalidSocket)
        close(_handle);

    TermWinsock();
}

Socket::Socket(Socket&& socket) noexcept {
    InitWinsock();
    _handle = socket._handle;
    _blocking = socket._blocking;
    socket._handle = InvalidSocket;
    socket._blocking = true;
}

Socket& Socket::operator=(Socket&& socket) noexcept {
    _handle = socket._handle;
    _blocking = socket._blocking;
    socket._handle = InvalidSocket;
    socket._blocking = true;
    return *this;
}

int Socket::handle() const {
    return _handle;
}

bool Socket::valid() const {
    return _handle != InvalidSocket;
}

bool Socket::blocking() const {
    return _blocking;
}

void Socket::SetBlocking(bool value)
{
    if(_handle == InvalidSocket)
        throw runtime_error("invalid socket");
    
    _blocking = value;

    unsigned long nonBlockingMode = _blocking ? 0 : 1;
    if(ioctl(_handle, FIONBIO, &nonBlockingMode) == SocketError)
        throw runtime_error("failed to set socket blocking option");
}

void Socket::SetTcpNoDelay(bool value)
{
    int noDelay = value ? 0 : 1;
    int ret = setsockopt(_handle, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay, sizeof(int));
}

void Socket::Bind(int port, const char* address, bool reuseAddress)
{
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if(address != nullptr)
        inet_pton(AF_INET, address, &addr.sin_addr);
    else
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(reuseAddress)
    {
        int yes = 1;
        if(setsockopt(_handle, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == SocketError)
            throw runtime_error("failed to set socket option: reuse address");
    }
    
    if(::bind(_handle, (sockaddr*)&addr, sizeof(sockaddr_in)) == SocketError)
        throw runtime_error("failed to bind");
}

void Socket::Listen()
{
    if(listen(_handle, SOMAXCONN) == SocketError)
        throw runtime_error("failed to listen");
}

void Socket::Connect(int port, const char* address)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, address, &addr.sin_addr);

    if(connect(_handle, (sockaddr*)&addr, sizeof(sockaddr_in)) == SocketError)
        throw runtime_error("failed to connect");
}

Socket Socket::Accept()
{
    if(_handle == InvalidSocket)
        throw runtime_error("invalid socket");
    
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);

    int ret = (int)accept((SOCKET)_handle, (sockaddr*)&addr, &len);
    if(ret == InvalidSocket)
        throw runtime_error("failed to accept connection");

    return Socket(ret);
}

void Socket::Close()
{
    if(_handle != InvalidSocket) {
        close(_handle);
        _handle = InvalidSocket;
    }
}

int Socket::Recv(char *buffer, size_t length)
{
    if(_handle == InvalidSocket)
        throw runtime_error("invalid socket");
    
    int ret = recv(_handle, buffer, (int)length, 0);
    if(ret == SocketError)
    {
        int code = errno;
        if(code == S_EWOULDBLOCK)
            return -1;
        else
            throw runtime_error("receive operation failed");
    }

    return ret;
}

int Socket::Recv(vector<char>& buffer) {
    return Recv(buffer.data(), buffer.size());
}

int Socket::Send(const char *buffer, size_t length)
{
    if(_handle == InvalidSocket)
        throw runtime_error("invalid socket");
    
    int ret = send(_handle, buffer, (int)length, 0);
    if(ret == SocketError)
    {
        int code = errno;
        if(code == S_EWOULDBLOCK)
            return -1;
        else
            throw runtime_error("send operation failed");
    }

    return ret;
}

int Socket::Send(const vector<char>& buffer) {
    return Send(buffer.data(), buffer.size());
}

int Socket::Poll(SocketPollMode mode, milliseconds timeout)
{
    if(_handle == InvalidSocket)
        throw runtime_error("invalid socket");

    timeval* pTimeout = nullptr;
    timeval tv;

    if(timeout.count() >= 0)
    {
        long long secs = duration_cast<seconds>(timeout).count();
        long long usecs = duration_cast<microseconds>(timeout).count() - secs * micro::den;
        tv = { (long)secs, (int)usecs };
        pTimeout = &tv;
    }

    fd_set fdSet = { 1, (SOCKET)_handle };
    int ret = -1;

    switch(mode)
    {
    case SocketPollMode::Read:
        ret = select(0, &fdSet, 0, 0, pTimeout);
        break;

    case SocketPollMode::Write:
        ret = select(0, 0, &fdSet, 0, pTimeout);
        break;

    case SocketPollMode::Error:
        ret = select(0, 0, 0, &fdSet, pTimeout);
        break;
    }

    if(ret < 0)
        throw runtime_error("select operation failed");

    return ret;
}

Task<void> Socket::ConnectAsync(int port, const std::string& address)
{
    ThrowIfBlocking();
    return Task<void>(std::make_shared<SocketConnectAwaiter>(_handle, port, address));
}

Task<Socket> Socket::AcceptAsync()
{
    ThrowIfBlocking();
    return Task<Socket>(std::make_shared<SocketAcceptAwaiter>(_handle));
}

Task<int> Socket::SendAsync(const char* bufferPtr, size_t bufferSize)
{
    ThrowIfBlocking();
    return Task<int>(std::make_shared<SocketSendAwaiter>(_handle, bufferPtr, bufferSize));
}

Task<int> Socket::RecvAsync(char* bufferPtr, size_t bufferSize)
{
    ThrowIfBlocking();
    return Task<int>(std::make_shared<SocketRecvAwaiter>(_handle, bufferPtr, bufferSize));
}

string Socket::GetHostIP(const string& host)
{
	addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	addrinfo* result;
    string ret;

	if (getaddrinfo(host.c_str(), 0, &hints, &result) == 0)
	{
		for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			char buffer[INET6_ADDRSTRLEN];
            auto addr = (struct sockaddr_in*)ptr->ai_addr;
            
            ret = inet_ntop(
				AF_INET,
                &addr->sin_addr,
				buffer,
				sizeof(buffer));
		}

        freeaddrinfo(result);
	}

	return ret;
}

void Socket::ThrowIfBlocking() const
{
    if (_blocking)
        throw std::runtime_error("socket must be in non-blocking mode");
}
