A basic web server created for educational purposes.

#### Capabilities:

This server supports basic GET requests with ranges and keep-alive connections - the bare minimum for serving simple web pages and streaming media.

#### Architecture:

* 1 thread accepts incoming connections
* 1 thread waits on blocked connections
* n worker threads process requests

The architecture is similar to an event driven approach, but maintains a state machine for each connection - which will eventually be replaced with a fiber. The worker threads pull from a queue of active connections and update the connection's state for a fixed amount of time. If the update completes without anything blocking, the connection is placed back into the active-queue. Otherwise, it's placed into an idle-queue to be poll'ed. This server is far from optimal, but it works, and performs reasonably. I may do some benchmarking one day when I have time.

#### Build
* VS2017+ (or any C++17 compiler for Windows)
