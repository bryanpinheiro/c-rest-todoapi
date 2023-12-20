#!/bin/bash

# Exit on error
set -e

# Set up variables for source files and output executable
SOURCE_FILES="src/main.c src/todoroutes.c"  # Adjust file paths as needed
OUTPUT_EXECUTABLE="build/main"

# Check if the build folder exists before removing it
if [ -d "build" ]; then
    rm -rf build
fi

# Create a new build folder
mkdir build

# Compile the C project and move the executable to the build folder
clang -o "$OUTPUT_EXECUTABLE" $SOURCE_FILES -lmicrohttpd -I/usr/local/include -L/usr/local/lib

# Print a success message
echo "Build successful! Executable: $OUTPUT_EXECUTABLE"
