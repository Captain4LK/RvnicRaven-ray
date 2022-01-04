#!/bin/sh

emcc -c src/RvR_all.c -std=c99 -Wall -Wextra -Wshadow -pedantic -O3 -flto -USE_SDL=2
