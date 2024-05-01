#!/bin/bash

# Check if inetutils-2.0 directory does not exist
if [ ! -d "inetutils-2.0" ]; then
    # Download inetutils-2.0.tar.xz
    curl https://ftp.gnu.org/gnu/inetutils/inetutils-2.0.tar.xz > inetutils-2.0.tar.xz
    # Extract the downloaded file
    tar -xf inetutils-2.0.tar.xz
fi

# Move into inetutils-2.0 directory
cd inetutils-2.0

# Check if ping/ping file does not exist
if [ ! -f "ping/ping" ]; then
    # Configure, compile, and install inetutils-2.0
    ./configure
    make
    # Copy ping/ping to parent directory
fi

if [ ! -f "../ping" ]; then
	cp ping/ping ../ping
fi
