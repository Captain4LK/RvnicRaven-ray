#!/bin/sh

gcc -o rayed *.c ../../RvR_all.o -std=c99 -Wall -Wextra -Wshadow -pedantic -O3 -flto=auto -s -lSDL2 -lm
