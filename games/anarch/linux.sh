#!/bin/sh

gcc -o ../../bin/Anarch *.c ../../RvR_all.o -std=c99 -Wall -Wextra -Wshadow -pedantic -O3 -flto=auto -s -lSDL2 -lSDL2_mixer
