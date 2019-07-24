/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once

#ifndef _WIN32
	#include <fcntl.h>
	//#include <errno.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <sys/ioctl.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/socket.h>
	#include <poll.h>
	#define SOCKET int
    #define S_EWOULDBLOCK             EWOULDBLOCK
    #define S_EINPROGRESS             EINPROGRESS
    #define S_EALREADY                EALREADY
    #define S_ENOTSOCK                ENOTSOCK
    #define S_EDESTADDRREQ            EDESTADDRREQ
    #define S_EMSGSIZE                EMSGSIZE
    #define S_EPROTOTYPE              EPROTOTYPE
    #define S_ENOPROTOOPT             ENOPROTOOPT
    #define S_EPROTONOSUPPORT         EPROTONOSUPPORT
    #define S_ESOCKTNOSUPPORT         ESOCKTNOSUPPORT
    #define S_EOPNOTSUPP              EOPNOTSUPP
    #define S_EPFNOSUPPORT            EPFNOSUPPORT
    #define S_EAFNOSUPPORT            EAFNOSUPPORT
    #define S_EADDRINUSE              EADDRINUSE
    #define S_EADDRNOTAVAIL           EADDRNOTAVAIL
    #define S_ENETDOWN                ENETDOWN
    #define S_ENETUNREACH             ENETUNREACH
    #define S_ENETRESET               ENETRESET
    #define S_ECONNABORTED            ECONNABORTED
    #define S_ECONNRESET              ECONNRESET
    #define S_ENOBUFS                 ENOBUFS
    #define S_EISCONN                 EISCONN
    #define S_ENOTCONN                ENOTCONN
    #define S_ESHUTDOWN               ESHUTDOWN
    #define S_ETOOMANYREFS            ETOOMANYREFS
    #define S_ETIMEDOUT               ETIMEDOUT
    #define S_ECONNREFUSED            ECONNREFUSED
    #define S_ELOOP                   ELOOP
    #define S_ENAMETOOLONG            ENAMETOOLONG
    #define S_EHOSTDOWN               EHOSTDOWN
    #define S_EHOSTUNREACH            EHOSTUNREACH
    #define S_ENOTEMPTY               ENOTEMPTY
    #define S_EPROCLIM                EPROCLIM
    #define S_EUSERS                  EUSERS
    #define S_EDQUOT                  EDQUOT
    #define S_ESTALE                  ESTALE
    #define S_EREMOTE                 EREMOTE
#else
	#pragma comment(lib, "ws2_32.lib")
	#include <WinSock2.h>
	#include <Ws2tcpip.h>
	#include <Windows.h>
    #include <stdexcept>

	#if defined(errno)
		#undef errno
	#endif

	#define errno (WSAGetLastError())
	#define close closesocket
	#define ioctl ioctlsocket
	#define poll WSAPoll
    #define S_EWOULDBLOCK             WSAEWOULDBLOCK
    #define S_EINPROGRESS             WSAEINPROGRESS
    #define S_EALREADY                WSAEALREADY
    #define S_ENOTSOCK                WSAENOTSOCK
    #define S_EDESTADDRREQ            WSAEDESTADDRREQ
    #define S_EMSGSIZE                WSAEMSGSIZE
    #define S_EPROTOTYPE              WSAEPROTOTYPE
    #define S_ENOPROTOOPT             WSAENOPROTOOPT
    #define S_EPROTONOSUPPORT         WSAEPROTONOSUPPORT
    #define S_ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
    #define S_EOPNOTSUPP              WSAEOPNOTSUPP
    #define S_EPFNOSUPPORT            WSAEPFNOSUPPORT
    #define S_EAFNOSUPPORT            WSAEAFNOSUPPORT
    #define S_EADDRINUSE              WSAEADDRINUSE
    #define S_EADDRNOTAVAIL           WSAEADDRNOTAVAIL
    #define S_ENETDOWN                WSAENETDOWN
    #define S_ENETUNREACH             WSAENETUNREACH
    #define S_ENETRESET               WSAENETRESET
    #define S_ECONNABORTED            WSAECONNABORTED
    #define S_ECONNRESET              WSAECONNRESET
    #define S_ENOBUFS                 WSAENOBUFS
    #define S_EISCONN                 WSAEISCONN
    #define S_ENOTCONN                WSAENOTCONN
    #define S_ESHUTDOWN               WSAESHUTDOWN
    #define S_ETOOMANYREFS            WSAETOOMANYREFS
    #define S_ETIMEDOUT               WSAETIMEDOUT
    #define S_ECONNREFUSED            WSAECONNREFUSED
    #define S_ELOOP                   WSAELOOP
    #define S_ENAMETOOLONG            WSAENAMETOOLONG
    #define S_EHOSTDOWN               WSAEHOSTDOWN
    #define S_EHOSTUNREACH            WSAEHOSTUNREACH
    #define S_ENOTEMPTY               WSAENOTEMPTY
    #define S_EPROCLIM                WSAEPROCLIM
    #define S_EUSERS                  WSAEUSERS
    #define S_EDQUOT                  WSAEDQUOT
    #define S_ESTALE                  WSAESTALE
    #define S_EREMOTE                 WSAEREMOTE
#endif
