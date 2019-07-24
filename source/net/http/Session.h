/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdlib>
#include <chrono>
#include <string>
#include <vector>
#include <fstream>
#include <utility>
#include <net/sockets/Socket.h>

enum class SessionState
{
    Request,
    Response,
    Done
};

class Session
{
public:
    using time_point = std::chrono::steady_clock::time_point;

    Socket socket;
    SessionState state = SessionState::Request;
    time_point timeout = time_point::min();
    bool keepAlive = true;

    std::ifstream fin;
    size_t contentLength = 0;
    size_t bufferOffset = 0;
    std::vector<char> buffer;

    Session() = delete;
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    Session::Session(Socket&& socket) : socket(std::move(socket)) {}
};

class Session2
{
public:
    using time_point = std::chrono::steady_clock::time_point;

    Socket socket;
    time_point timeout = time_point::min();
    bool keepAlive = true;

    std::ifstream fin;
    size_t contentLength = 0;
    size_t bufferOffset = 0;
    std::vector<char> buffer;

    Session2() = delete;
    Session2(const Session2&) = delete;
    Session2& operator=(const Session2&) = delete;

    Session2(Socket&& socket) : socket(std::move(socket)) {}
};
