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
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int rand_seed = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

unsigned RvR_rand()
{
   static uint32_t w32_rtable[RVR_RNG_TABLE_SIZE] = {0};
   static int initialized = 0;

   if(!initialized)
   {
      uint32_t inc = 0xacd2d391, mul = 0xf34f9201, x = 0;
      for(int i = 0;i<RVR_RNG_TABLE_SIZE;i++)
      {     
         w32_rtable[i] = x;
         x = (x+inc)*mul;
      }
      initialized = 1;
   }

   rand_seed = (rand_seed+1)%RVR_RNG_TABLE_SIZE;
   return w32_rtable[rand_seed];
}

int RvR_rand_get_state()
{
   return rand_seed;
}

void RvR_rand_set_state(int state)
{
   rand_seed = state%RVR_RNG_TABLE_SIZE;
}
//-------------------------------------
