#!/bin/sh

gcc -o ray_mapconv main.c ../../RvR_all.o -std=c99 -Wall -Wextra -Wshadow -pedantic -Og -g -lSDL2 -lm
