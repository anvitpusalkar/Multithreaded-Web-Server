#!/bin/bash

# Compilation
make all

# Start the server
./server > /dev/null 2>&1 & SERVER_PID=$!

sleep 2

# Run the client
./client

# Stop the server
kill $SERVER_PID
