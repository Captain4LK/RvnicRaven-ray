#!/bin/bash
 
riscv32-unknown-elf-gcc "$@" src/*.c lib/start.s -o src.out -mabi=ilp32 -march=rv32im -mlittle-endian -nostdlib -Wl,-T,lib/link.ld -I lib -O3 -Wall -Wextra
