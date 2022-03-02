#!/bin/sh

gcc -o ../../bin/Portal *.c ../../RvR_all.o -std=c99 -Wall -Wextra -Wshadow -pedantic -Og -g -lSDL2 -lSDL2_mixer
