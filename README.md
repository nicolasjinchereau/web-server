A simple web server built with awaitable sockets using C++20 coroutines for Windows and Mac OS

#### Capabilities:

This server supports basic GET requests with ranges, the bare minimum for serving simple web pages and streaming media.

#### Architecture:

The previous version of this server used a fixed number of worker threads, and a state-machine to schedule the processing of requests. The resulting implementation was confusing and inefficient.
https://github.com/nicolasjinchereau/web-server/tree/9cd6d3619e2b39f2f7231c46bedaa2cdcc7c8796

The new version is built on top of awaitable socket operations, which are implemented using C++20 coroutines. The previous state-machine implementation has been refactored into resumable functions, and the resulting implementation is 100 lines shorter, and much easier to read.

#### Build
* VS2019+ with C++17 language standard and /await command line option
* XCode 11.3.1+ with C++17 language dialect and -fcoroutines-ts C++ flag
