/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Socket.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <stdexcept>
#include <unordered_map>
using namespace std;
using namespace chrono;

unordered_map<AddressFamily, int> _socketAddressFamilyMap {
    { AddressFamily::Unknown, AF_UNKNOWN1 },
    { AddressFamily::Unspecified, AF_UNSPEC },
    { AddressFamily::AppleTalk, AF_APPLETALK },
    { AddressFamily::Atm, AF_ATM },
    { AddressFamily::Banyan, AF_BAN },
    { AddressFamily::Ccitt, AF_CCITT },
    { AddressFamily::Chaos, AF_CHAOS },
    { AddressFamily::Cluster, AF_CLUSTER },
    { AddressFamily::DataKit, AF_DATAKIT },
    { AddressFamily::DataLink, AF_DLI },
    { AddressFamily::DecNet, AF_DECnet },
    { AddressFamily::Ecma, AF_ECMA },
    { AddressFamily::FireFox, AF_FIREFOX },
    { AddressFamily::HyperChannel, AF_HYLINK },
    { AddressFamily::Ieee12844, AF_12844 },
    { AddressFamily::ImpLink, AF_IMPLINK },
    { AddressFamily::InterNetwork, AF_INET },
    { AddressFamily::InterNetworkV6, AF_INET6 },
    { AddressFamily::Ipx, AF_IPX },
    { AddressFamily::Irda, AF_IRDA },
    { AddressFamily::Iso, AF_ISO },
    { AddressFamily::Lat, AF_LAT },
    { AddressFamily::NetBios, AF_NETBIOS },
    { AddressFamily::NetworkDesigners, AF_NETDES },
    { AddressFamily::NS, AF_NS },
    { AddressFamily::Osi, AF_OSI },
    { AddressFamily::Pup, AF_PUP },
    { AddressFamily::Sna, AF_SNA },
    { AddressFamily::Unix, AF_UNIX },
    { AddressFamily::VoiceView, AF_VOICEVIEW },
};

unordered_map<SocketType, int> _socketTypeMap {
    { SocketType::Stream, SOCK_STREAM },
    { SocketType::Dgram, SOCK_DGRAM },
    { SocketType::Raw, SOCK_RAW },
    { SocketType::Rdm, SOCK_RDM },
    { SocketType::Seqpacket, SOCK_SEQPACKET },
};

unordered_map<ProtocolType, int> _socketProtocolTypeMap {
    { ProtocolType::Unknown, -1 },
    { ProtocolType::Unspecified, 0 },
    { ProtocolType::IP, 0 },
    { ProtocolType::Icmp, IPPROTO_ICMP },
    { ProtocolType::Igmp, IPPROTO_IGMP },
    { ProtocolType::Ggp, IPPROTO_GGP },
    { ProtocolType::IPv4, IPPROTO_IPV4 },
    { ProtocolType::St, IPPROTO_ST },
    { ProtocolType::TCP, IPPROTO_TCP },
    { ProtocolType::Cbt, IPPROTO_CBT },
    { ProtocolType::Egp, IPPROTO_EGP },
    { ProtocolType::Igp, IPPROTO_IGP },
    { ProtocolType::Rdp, IPPROTO_RDP },
    { ProtocolType::Pup, IPPROTO_PUP },
    { ProtocolType::Udp, IPPROTO_UDP },
    { ProtocolType::Idp, IPPROTO_IDP },
    { ProtocolType::IPv6, IPPROTO_IPV6 },
    { ProtocolType::IPv6HopByHopOptions, 0 },
    { ProtocolType::IPv6RoutingHeader, IPPROTO_ROUTING },
    { ProtocolType::IPv6FragmentHeader, IPPROTO_FRAGMENT },
    { ProtocolType::IPSecEncapsulatingSecurityPayload, IPPROTO_ESP },
    { ProtocolType::IPSecAuthenticationHeader, IPPROTO_AH },
    { ProtocolType::IcmpV6, IPPROTO_ICMPV6 },
    { ProtocolType::IPv6NoNextHeader, IPPROTO_NONE },
    { ProtocolType::IPv6DestinationOptions, IPPROTO_DSTOPTS },
    { ProtocolType::ND, IPPROTO_ND },
    { ProtocolType::Iclfxbm, IPPROTO_ICLFXBM },
    { ProtocolType::Pim, IPPROTO_PIM },
    { ProtocolType::Pgm, IPPROTO_PGM },
    { ProtocolType::L2TP, IPPROTO_L2TP },
    { ProtocolType::SCTP, IPPROTO_SCTP },
    { ProtocolType::Raw, IPPROTO_RAW }
};

Socket::Socket() : _handle(INVALID_SOCKET)
{
}

Socket::Socket(AddressFamily family, SocketType type, ProtocolType protocol)
{
    auto af = _socketAddressFamilyMap[family];
    auto ty = _socketTypeMap[type];
    auto pr = _socketProtocolTypeMap[protocol];

    _handle = socket(af, ty, pr);
    if(_handle == INVALID_SOCKET)
        throw SocketException(WSAGetLastError(), "failed to create socket");
}

Socket::Socket(uintptr_t handle) {
    _handle = handle;
}

Socket::~Socket()
{
    if(_handle != INVALID_SOCKET)
        closesocket(_handle);
}

Socket::Socket(Socket&& socket) {
    _handle = socket._handle;
    socket._handle = INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& socket) {
    _handle = socket._handle;
    socket._handle = INVALID_SOCKET;
    return *this;
}

uintptr_t Socket::handle() const {
    return _handle;
}

bool Socket::valid() const {
    return _handle != INVALID_SOCKET;
}

void Socket::SetNonBlocking(bool value)
{
    if(_handle == INVALID_SOCKET)
        throw SocketException(-1, "invalid socket");

    ULONG nonBlocking = value ? 1 : 0;
    if(ioctlsocket(_handle, FIONBIO, &nonBlocking) == SOCKET_ERROR)
        throw SocketException(WSAGetLastError(), "failed to set socket blocking option");
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
	    if(setsockopt(_handle, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == SOCKET_ERROR)
            throw SocketException(WSAGetLastError(), "failed to set socket option: reuse address");
    }
    
    if(::bind(_handle, (LPSOCKADDR)&addr, sizeof(sockaddr_in)) == SOCKET_ERROR)
        throw SocketException(WSAGetLastError(), "failed to bind");
}

void Socket::Listen()
{
    if(listen(_handle, SOMAXCONN) == SOCKET_ERROR)
        throw SocketException(WSAGetLastError(), "failed to listen");
}

void Socket::Connect(int port, const char* address)
{
    sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
    inet_pton(AF_INET, address, &addr.sin_addr);

	if(connect(_handle, (sockaddr*)&addr, sizeof(sockaddr_in)) == SOCKET_ERROR)
        throw SocketException(WSAGetLastError(), "failed to connect");
}

Socket Socket::Accept()
{
    if(_handle == INVALID_SOCKET)
        throw SocketException(-1, "invalid socket");
    
    sockaddr_in addr{};
	int len = sizeof(addr);

	SOCKET ret = accept(_handle, (sockaddr*)&addr, &len);
	if(ret == INVALID_SOCKET)
        throw SocketException(WSAGetLastError(), "failed to accept connection");

    return Socket((uintptr_t)ret);
}

void Socket::Close()
{
    if(_handle != INVALID_SOCKET) {
        closesocket(_handle);
        _handle = INVALID_SOCKET;
    }
}

int Socket::Recv(char *buffer, size_t length)
{
    if(_handle == INVALID_SOCKET)
        throw SocketException(-1, "invalid socket");
    
    int ret = recv(_handle, buffer, (int)length, 0);
    if(ret == SOCKET_ERROR)
    {
        auto code = WSAGetLastError();
        if(code == WSAEWOULDBLOCK)
            return -1;
        else
            throw SocketException(code, "receive operation failed");
    }

    return ret;
}

int Socket::Recv(vector<char>& buffer) {
    return Recv(buffer.data(), buffer.size());
}

int Socket::Send(const char *buffer, size_t length)
{
    if(_handle == INVALID_SOCKET)
        throw SocketException(-1, "invalid socket");
    
	int ret = send(_handle, buffer, (int)length, 0);
    if(ret == SOCKET_ERROR)
    {
        auto code = WSAGetLastError();
        if(code == WSAEWOULDBLOCK)
            return -1;
        else
            throw SocketException(code, "send operation failed");
    }

    return ret;
}

int Socket::Send(const vector<char>& buffer) {
    return Send(buffer.data(), buffer.size());
}

int Socket::Poll(SocketPollMode mode, milliseconds timeout)
{
    if(_handle == INVALID_SOCKET)
        throw SocketException(-1, "invalid socket");

    timeval* pTimeout = nullptr;
    timeval tv;

    if(timeout.count() >= 0)
    {
	    long long secs = duration_cast<seconds>(timeout).count();
        long long usecs = duration_cast<microseconds>(timeout).count() - secs * micro::den;
	    tv = { (long)secs, (long)usecs };
        pTimeout = &tv;
    }

    fd_set fdSet = { 1, _handle };
    int ret = -1;

	switch(mode)
	{
	case SocketPollMode::Accept:
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
        throw SocketException(WSAGetLastError(), "select operation failed");

	return ret;

}

string Socket::GetHostIP(const string &host)
{
    addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));

	hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

	addrinfo *result;

	if(getaddrinfo(host.c_str(), 0, &hints, &result) == 0)
	{
		for(addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			if(ptr->ai_family == AF_INET
			&& ptr->ai_socktype == SOCK_STREAM
			&& ptr->ai_protocol == IPPROTO_TCP)
			{
                char addressBuffer[INET6_ADDRSTRLEN];
				
                return inet_ntop(
                    AF_INET,
                    (sockaddr_in*)ptr->ai_addr,
                    addressBuffer,
                    sizeof(addressBuffer));
			}
		}
	}

	return "";
}
