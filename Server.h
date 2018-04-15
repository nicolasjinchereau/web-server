/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <cassert>
#include <deque>
#include "Spinlock.h"
#include "ring_queue.h"
#include "Session.h"
#include "Socket.h"

class Server
{
    using milliseconds = std::chrono::milliseconds;

    static constexpr size_t BufferSize = 16384;
    static constexpr int RequestWakePort = 32190;
    static constexpr int SendWakePort = 32191;
    static constexpr char* LoopbackAddress = "127.0.0.1";
    static constexpr milliseconds SessionTimeout = milliseconds(5000);
    static constexpr milliseconds MaxTimeSlice = milliseconds(20);

    std::string defaultPage = "index.html";
    std::string httpdocs;
	int port = 0;
	bool run = false;
    Spinlock lock;

	std::thread listenThread;
    Socket listenSocket;

    std::thread idleThread;
    Socket idleThreadWakeSockets[2];

    std::vector<std::thread> activeThreads;
    std::condition_variable_any activeThreadWakeCond;

    std::vector<std::shared_ptr<Session>> idleSessions;
    ring_queue<std::shared_ptr<Session>> activeSessions;
    
    void ListenThreadRunLoop();
    void IdleThreadRunLoop();
    void ActiveThreadRunLoop();
    
    void WakeIdleThread();
    void WakeActiveThreads();
    
    bool ReceiveRequest(const std::shared_ptr<Session>& session);
    bool SendResponse(const std::shared_ptr<Session>& session, milliseconds timeSliceLength);

    static void Log(const Session* session, const std::string& message);
public:
	Server();
	~Server();
	
    ///<summary>If 'threadCount' is zero, std::thread::hardware_concurrency() will be used.</summary>
    void Start(int port, const std::string& docsPath, size_t threadCount = 0);

    void Stop();
};
