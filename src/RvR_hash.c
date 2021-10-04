/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
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
#define FNV_64_PRIME ((uint64_t)0x100000001b3ULL)
#define FNV_32_PRIME ((uint32_t)0x01000193)
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

uint64_t RvR_fnv64a(const char *str)
{
   uint64_t hval = 0xcbf29ce484222325ULL;
   unsigned char *s = (unsigned char *)str;
   while(*s)
   {
      hval^=(uint64_t)*s++;
      hval*=FNV_64_PRIME;
   }

   return hval;
}

uint32_t RvR_fnv32a(const char *str)
{
   uint32_t hval = 0x811c9dc5;
   unsigned char *s = (unsigned char *)str;
   while (*s) 
   {
      hval^=(uint32_t)*s++;
      hval *= FNV_32_PRIME;
   }

   return hval;
}
//-------------------------------------
