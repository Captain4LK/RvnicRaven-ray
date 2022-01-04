#!/bin/sh

i686-w64-mingw32-gcc -c src/RvR_all.c -std=c99 -Wall -Wextra -Wshadow -pedantic -O3 -flto
