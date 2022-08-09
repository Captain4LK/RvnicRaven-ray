#!/bin/sh

gcc -o ../../bin/Baller *.c ../../RvR_all.o -std=c99 -Wall -Wextra -Wshadow -pedantic -lSDL2 -lSDL2_mixer -lm -Og -g
