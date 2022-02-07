#!/bin/sh

gcc -o anarch_mapconv main.c ../../RvR_all.o -std=c99 -Wall -Wextra -Wshadow -pedantic -O3 -flto=auto -s -lSDL2 -lm
