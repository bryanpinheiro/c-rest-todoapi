#!/bin/bash

# Check if the build folder exists before removing it
if [ -d "build" ]; then
    rm -rf build
fi

# Create a new build folder
mkdir build

# Compile the C project and move the executable to the build folder
clang -o build/main main.c -I/usr/local/include -L/usr/local/lib -lmicrohttpd
