/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <iostream>
#include <queue>
#include <experimental/coroutine>
#include <thread>
#include <functional>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <vector>
#include <atomic>
#include <chrono>
#include <string>
#include <net/sockets/Socket.h>
#include <net/sockets/SocketController.h>
#include <net/http/Http.h>
#include <net/http/HttpServer.h>
#include <system/FileSystemUtility.h>

using namespace std;
using namespace std::experimental;
using namespace std::chrono;

HttpServer server;

void startup(void* p, intmax_t n)
{
    // host at http://127.0.0.1:80/ using document root /bin/httpdocs
    auto httpdocs = FileSystemUtility::GetCurrentWorkingDir() + "/httpdocs";
    server.Start(80, httpdocs);
}

int main()
{
    Dispatcher::current().InvokeAsync(startup);
    Dispatcher::current().Run();
    return 0;
}
