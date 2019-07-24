/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <deque>
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <cassert>
#include <net/sockets/Socket.h>
#include <net/http/Http.h>
#include <system/Dispatcher.h>
#include <system/Turnstyle.h>

class HttpServer
{
    using milliseconds = std::chrono::milliseconds;

    static constexpr size_t BufferSize = 8192;
    static constexpr int RequestWakePort = 32190;
    static constexpr int SendWakePort = 32191;
    static constexpr char* LoopbackAddress = "127.0.0.1";
    static constexpr milliseconds SessionTimeout = milliseconds(5000);
    static constexpr milliseconds MaxTimeSlice = milliseconds(20);

    std::string defaultPage = "index.html";
    std::string httpdocs;
    int port = 0;
    std::atomic<bool> run = false;
    Socket listenSocket;
    std::deque<Socket> clientSockets;
    std::vector<std::thread> requestThreads;
    Turnstyle turnstyle;
    std::mutex mut;

    void RequestDispatchEntryPoint();

    Task<void> ListenForConnections();
    Task<void> GetRequests();
    Task<void> AcceptRequests(Socket socket);
    Task<void> SendError(Socket& socket, HttpStatus status, bool keepAlive);
    Task<void> SendFile(Socket& socket, HttpResponse response, std::ifstream fin, size_t contentLength);

    void EnqueueClient(Socket socket);
    Socket GetNextClient();
    static int GetRangeInfo(std::vector<Http::ContentRange>& ranges, size_t fileSize, size_t* rangeStart, size_t* rangeEnd);


public:
    HttpServer();
    ~HttpServer();
    
    ///<summary>set 'threadCount' to zero to use std::thread::hardware_concurrency()</summary>
    void Start(int port, const std::string& docsPath, size_t threadCount = 0);
    
    void Stop();
};
