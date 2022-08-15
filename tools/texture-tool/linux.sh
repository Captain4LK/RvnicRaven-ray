#!/bin/sh

gcc -o texture-tool texture-tool.c ../../RvR_all.o -std=c99 -Wall -Wextra -Wshadow -pedantic -Og -g -lSDL2 -lm
