#!/bin/bash
curl https://ftp.gnu.org/gnu/inetutils/inetutils-2.0.tar.xz > inetutils-2.0.tar.xz
tar -xf inetutils-2.0.tar.xz
cd inetutils-2.0
echo $((pwd))
./configure
make
cp ping/ping ../inet_ping
