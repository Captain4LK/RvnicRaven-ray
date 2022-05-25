/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/*
//Taken from: https://github.com/mattiasgustavsson/libs

rnd.h - v1.0 - Pseudo-random number generators for C/C++.

------------------------------------------------------------------------------

This software is available under 2 licenses - you may choose the one you like.
Based on public domain implementation - original licenses can be found next to
the relevant implementation sections of this file.

------------------------------------------------------------------------------

ALTERNATIVE A - MIT License

Copyright (c) 2016 Mattias Gustavsson

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

------------------------------------------------------------------------------

ALTERNATIVE B - Public Domain (www.unlicense.org)

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static uint64_t rand_internal_murmur3_avalanche64(uint64_t h);
static uint32_t rand_internal_murmur3_avalanche32(uint32_t h);
//-------------------------------------

//Function implementations

void RvR_rand_pcg_seed(RvR_rand_pcg *pcg, uint32_t seed)
{
   uint64_t value = (((uint64_t)seed)<<1)|1;
   value = rand_internal_murmur3_avalanche64(value);
   (*pcg)[0] = 0;
   (*pcg)[1] = (value<<1)|1;
   RvR_rand_pcg_next(pcg);
   (*pcg)[0]+=rand_internal_murmur3_avalanche64(value);
   RvR_rand_pcg_next(pcg);
}

uint32_t RvR_rand_pcg_next(RvR_rand_pcg *pcg)
{
   uint64_t oldstate = (*pcg)[0];
   (*pcg)[0] = oldstate*UINT64_C(0x5851f42d4c957f2d)+(*pcg)[1];
   uint32_t xorshifted = (uint32_t)(((oldstate>>18)^oldstate)>>27);
   uint32_t rot = (uint32_t)(oldstate>>59);

   return (xorshifted>>rot)|(xorshifted<<((-(int32_t)rot)&31));
}

int32_t RvR_rand_pcg_next_range(RvR_rand_pcg *pcg, int32_t min, int32_t max)
{
   uint32_t range = RvR_abs((max-min))+1;
   return min+(RvR_rand_pcg_next(pcg)%range);
}

void RvR_rand_well_seed(RvR_rand_well *well, uint32_t seed)
{
   uint32_t value = rand_internal_murmur3_avalanche32((seed<<1)|1);
   (*well)[16] = 0;
   (*well)[0] = value^0xf68a9fc1U;
   for(int i = 1;i<16;i++) 
      (*well)[i] = (0x6c078965U*((*well)[i-1]^((*well)[i-1]>>30))+i);
}

uint32_t RvR_rand_well_next(RvR_rand_well *well)
{
   uint32_t a = (*well)[(*well)[16]];
   uint32_t c = (*well)[((*well)[16]+13)&15];
   uint32_t b = a^c^(a<<16)^(c<<15);
   c = (*well)[(*well[16]+9)&15];
   c^=(c>>11);
   a = (*well)[(*well)[16]] = b^c;
   uint32_t d = a^((a<<5)&0xda442d24);
   (*well)[16] = ((*well)[16]+15)&15;
   a = (*well)[(*well)[16]];
   (*well)[(*well)[16]] = a^b^d^(a<<2)^(b<<18)^(c<<28);

   return (*well)[(*well)[16]];
}

int32_t RvR_rand_well_next_range(RvR_rand_well *well, int32_t min, int32_t max)
{
   uint32_t range = RvR_abs((max-min))+1;
   return min+(RvR_rand_well_next(well)%range);
}

void RvR_rand_game_seed(RvR_rand_game *game, uint32_t seed)
{
   uint32_t value = rand_internal_murmur3_avalanche32((seed<<1)|1);
   (*game)[0] = value;
   (*game)[1] = value^0x49616e42U;
}

uint32_t RvR_rand_game_next(RvR_rand_game *game)
{
   (*game)[0] = ((*game)[0]<<16)+((*game)[0]>>16);
   (*game)[0]+=(*game)[1];
   (*game)[1]+=(*game)[0];

   return (*game)[0];
}

int32_t RvR_rand_game_next_range(RvR_rand_game *game, int32_t min, int32_t max)
{
   uint32_t range = RvR_abs((max-min))+1;
   return min+(RvR_rand_game_next(game)%range);
}

void RvR_rand_xor_seed(RvR_rand_xor *xor, uint64_t seed)
{
   uint64_t value = rand_internal_murmur3_avalanche64((seed<<1)|1);
   (*xor)[0] = value;
   value = rand_internal_murmur3_avalanche64(value);
   (*xor)[1] = value;
}

uint64_t RvR_rand_xor_next(RvR_rand_xor *xor)
{
   uint64_t x = (*xor)[0];
   uint64_t const y = (*xor)[1];
   (*xor)[0] = y;
   x^=x<<23;
   x^=x>>17;
   x^=y^(y>>26);
   (*xor)[1] = x;

   return x+y;
}

int32_t RvR_rand_xor_next_range(RvR_rand_xor *xor, int32_t min, int32_t max)
{
   uint32_t range = RvR_abs((max-min))+1;
   return min+(RvR_rand_xor_next(xor)%range);
}

static uint64_t rand_internal_murmur3_avalanche64(uint64_t h)
{
   h^=h>>33;
   h*=0xff51afd7ed558ccd;
   h^=h>>33;
   h*=0xc4ceb9fe1a85ec53;
   h^=h>>33;

   return h;
}

static uint32_t rand_internal_murmur3_avalanche32(uint32_t h)
{
   h^=h>>16;
   h*=0x85ebca6b;
   h^=h>>13;
   h*=0xc2b2ae35;
   h^=h>>16;

   return h;
}
//-------------------------------------
