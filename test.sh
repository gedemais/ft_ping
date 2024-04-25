#!/bin/bash
make re
./ft_ping google.com | diff -u a b -- <(ping google.com)
