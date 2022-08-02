/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "color.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
uint8_t color_white;
uint8_t color_black;
//-------------------------------------

//Function prototypes
static uint8_t find_closest(const RvR_color *pal, RvR_color color);
//-------------------------------------

//Function implementations

void color_init(void)
{
   const RvR_color *pal = RvR_palette();

   color_white = find_closest(pal,(RvR_color){.r = 255, .g = 255, .b = 255});
   color_black = find_closest(pal,(RvR_color){.r = 0, .g = 0, .b = 0});
}

static uint8_t find_closest(const RvR_color *pal, RvR_color color)
{
   int dist_min = INT_MAX;
   int index_min = 0;

   for(int i = 0;i<256;i++)
   {
      int dr = pal[i].r-color.r;
      int dg = pal[i].g-color.g;
      int db = pal[i].b-color.b;
      int dist = dr*dr+dg*dg+db*db;
      if(dist<dist_min)
      {
         dist_min = dist;
         index_min = i;
      }
   }

   return index_min;
}
//-------------------------------------
