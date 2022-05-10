#!/bin/sh

gcc -o ../../bin/VM *.c ../../RvR_all.o -std=c99 -Wall -Wextra -Wshadow -pedantic -lSDL2 -lSDL2_mixer -Og -g
