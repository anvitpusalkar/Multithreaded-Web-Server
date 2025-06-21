This project implements a multi-threaded C server for evaluating mathematical expressions and a corresponding multi-threaded C clientthat concurrently sends expressions read from a file to the server.

Features
Server (server.c):

Multi-threaded Architecture: Utilizes a fixed-size thread pool (NUM_WORKER_THREADS = 5) for concurrent client connection handling.

Producer-Consumer Pattern: A main thread accepts connections and enqueues client sockets into a thread-safe queue. Worker threads dequeue and process requests.

Expression Evaluation: Integrates the tinyexpr C library for parsing and evaluating mathematical expressions.

Network Communication: Implements standard TCP socket programming.

Graceful Shutdown: Ensures proper termination of all worker threads before server exit.

Client (client.c):

Multi-threaded Architecture: Employs a client-side thread pool (NUM_CLIENT_THREADS = 5) to send multiple expressions concurrently.

Producer-Consumer Pattern: A main thread reads expressions from expressions.txt and enqueues them. Client worker threads dequeue, connect to the server, send expressions, and receive results.

File-based Input: Reads mathematical expressions line-by-line from expressions.txt.

Concurrent Request Generation: Designed to generate simultaneous requests for server load testing.

Build System (Makefile):

Automates compilation of client and server executables.

Includes a clean target for removing generated files.

Orchestration Script (run_test.sh):

Automates the build process via make.

Starts the server in the background.

Starts the client in the foreground.

Manages background server process termination upon client completion.

Redirects server output to suppress it, focusing terminal output on the client.

Prerequisites
To build and run this project, you will need:

GCC (GNU Compiler Collection): C compiler.

make: Build automation tool.

tinyexpr library: tinyexpr.c and tinyexpr.h must be present in the project directory.

WSL (Windows Subsystem for Linux) or a Linux environment: Required for POSIX-specific networking and threading APIs.

Setup Instructions
Clone or Download: Place all project files (server.c, client.c, tinyexpr.c, tinyexpr.h, Makefile, run_test.sh) into a single directory.

Make run_test.sh Executable:

chmod +x run_test.sh

Create expressions.txt:
Create a file named expressions.txt in the project directory. Each line must contain one mathematical expression.
Example expressions.txt content:

1 + 1
2 * 3.5
(5 - 2) * 4
sin(0) + cos(0)
log10(100)
(10 + 20) / 5

For performance testing, it is recommended to use a large number of expressions (e.g., 50,000 to 100,000).

How to Run
Open your WSL Debian terminal (or a Linux terminal).

Navigate to your project directory.

Execute the test script:

./run_test.sh

Expected Output:
The script compiles programs, starts the server in the background, and runs the client. Terminal output will primarily display messages from the client.

Project Structure
.
├── Makefile
├── server.c
├── client.c
├── tinyexpr.c
├── tinyexpr.h
├── expressions.txt
└── run_test.sh
