This project implements a multi-threaded C server for evaluating mathematical expressions and a corresponding multi-threaded C clientthat concurrently sends expressions read from a file to the server.

Features

Server (server.c):

Multi-threaded Architecture: Utilizes a fixed-size thread pool (WORKER_THREADS = 5) for concurrent client connection handling.

Producer-Consumer Pattern: A main thread accepts connections and enqueues client sockets into a thread-safe queue. Worker threads dequeue and process requests.

Expression Evaluation: Integrates the tinyexpr C library for parsing and evaluating mathematical expressions.

Network Communication: Implements standard TCP socket programming.

Graceful Shutdown: Ensures proper termination of all worker threads before server exit.

Client (client.c):

Multi-threaded Architecture: Employs a client-side thread pool (CLIENT_THREADS = 5) to send multiple expressions concurrently.

Producer-Consumer Pattern: A main thread reads expressions from expressions.txt and enqueues them. Client worker threads dequeue, connect to the server, send expressions, and receive results.

File-based Input: Reads mathematical expressions line-by-line from expressions.txt.

Concurrent Request Generation: Designed to generate simultaneous requests for server load testing.

Build System (Makefile):

Automates compilation of client and server executables.

Includes a clean target for removing generated files.

Orchestration Script (test.sh):

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

Make test.sh Executable:

chmod +x test.sh

How to Run
Open your WSL Debian terminal (or a Linux terminal).

Navigate to your project directory.

Execute the test script:

./test.sh

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
└── test.sh
