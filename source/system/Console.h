/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <mutex>
#include <string>
#include <iomanip>
#include <system/format.h>

class Socket;

class Console
{
    static std::mutex mut;
public:

    template<typename... Args>
    static void WriteLine(Socket& socket, const char* fmt, Args&& ... args)
    {
        std::stringstream ss;
        ss << std::setfill(' ') << std::setw(6) << (uint64_t)socket.handle() << ": " << fmt;
        writeln(mut, ss.str().c_str(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void WriteLine(const char* fmt, Args&& ... args)
    {
        std::stringstream ss;
        ss << std::string(6, '-') << ": " << fmt;
        writeln(mut, ss.str().c_str(), std::forward<Args>(args)...);
    }
};
