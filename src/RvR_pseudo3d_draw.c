/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdlib.h>
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
static RvR_p3d_sprite *p3d_sprite_array = NULL;
static int32_t p3d_sprite_array_size = 0;
static int32_t p3d_sprite_pool = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

int32_t RvR_p3d_sprite_new()
{
   if(p3d_sprite_pool==-1)
   {
      p3d_sprite_array_size+=128;
      p3d_sprite_array = RvR_realloc(p3d_sprite_array,sizeof(*p3d_sprite_array)*p3d_sprite_array_size);
      for(int i = p3d_sprite_array_size-128;i<p3d_sprite_array_size-1;i++)
         p3d_sprite_array[i].next = i+1;
      p3d_sprite_array[p3d_sprite_array_size-1].next = -1;
   }


}

void RvR_p3d_sprite_free(uint32_t sprite)
{
}
//-------------------------------------
