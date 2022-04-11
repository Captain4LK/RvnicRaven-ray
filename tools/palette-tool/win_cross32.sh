#!/bin/sh

i686-w64-mingw32-gcc -o palette-tool palette-tool.c ../../RvR_all.o -std=c99 -Wall -Wextra -Wshadow -pedantic -O3 -s -flto=auto -lSDL2 -lmingw32 -lSDL2main -static-libgcc
