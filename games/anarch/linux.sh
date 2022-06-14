#!/bin/sh

gcc -o ../../bin/Anarch *.c ../../RvR_all.o -std=c99 -Wall -Wextra -Wshadow -pedantic -lSDL2 -lSDL2_mixer -O3 -flto=auto -s
