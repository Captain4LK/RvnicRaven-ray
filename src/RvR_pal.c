/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
//-------------------------------------

//#defines
#define READ(v,m,p,l,t) \
   do\
   {\
      RvR_error_check((p)+sizeof(t)<=(l),0x200);\
      (v) = (*((t *)((m)+(p))));\
      (p)+=sizeof(t);\
   }while(0)
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t shade_table[64][256] = {0};
static RvR_color *palette = NULL;
//-------------------------------------

//Function prototypes
static void pal_calculate_colormap();
//-------------------------------------

//Function implementations

void RvR_palette_load(uint16_t id)
{
   unsigned size_in = 0;
   unsigned pos = 0;
   uint8_t *mem_pak = NULL;
   char tmp[64] = "";

   //Allocate palette if it isn't yet
   if(palette==NULL)
   {
      palette = RvR_malloc(sizeof(*palette)*256);
      RvR_error_check(palette!=NULL,0x001);
   }

   snprintf(tmp,64,"PAL%05d",id);
   mem_pak = RvR_lump_get(tmp,RVR_LUMP_PAL,&size_in);
   if(mem_pak==NULL)
      RvR_error_check(0,0x000);
   for(unsigned i = 0;i<256;i++)
   {
      READ(palette[i].r,mem_pak,pos,size_in,uint8_t);
      READ(palette[i].g,mem_pak,pos,size_in,uint8_t);
      READ(palette[i].b,mem_pak,pos,size_in,uint8_t);
      palette[i].a = 255;
   }
   RvR_free(mem_pak);

   pal_calculate_colormap();

   return;

RvR_err:

   if(mem_pak!=NULL)
      RvR_free(mem_pak);

   RvR_log("RvR error %s\n",RvR_error_get_string());
}

//Taken from: https://quakewiki.org/wiki/Quake_palette (public domain), original note:
//The following C code will generate a colormap.lmp that is nearly (but not exactly) identical to the Quake colormap. Written by metlslime who placed it in the public domain.
static void pal_calculate_colormap()
{
   for(int x = 0;x<256;x++)
   {
      for(int y = 0;y<64;y++)
      {
         if(x<256)
         {
            int r = RvR_min(255,((int)palette[x].r*(63-y)+16)>>5);
            int g = RvR_min(255,((int)palette[x].g*(63-y)+16)>>5);
            int b = RvR_min(255,((int)palette[x].b*(63-y)+16)>>5);
            int best_index = -1;
            int best_dist = 0;

            for(int i = 0;i<256;i++)
            {
               int dist = 0;
               dist+=abs(r-palette[i].r)*abs(r-palette[i].r);
               dist+=abs(g-palette[i].g)*abs(g-palette[i].g);
               dist+=abs(b-palette[i].b)*abs(b-palette[i].b);

               if(best_index==-1||dist<best_dist)
               {
                  best_index = i;
                  best_dist = dist;
               }
            }

            shade_table[y][x] = (uint8_t)best_index;
         }
         else
         {
            shade_table[y][x] = x;
         }
      }
   }
}

RvR_color *RvR_palette()
{
   return palette;
}

uint8_t *RvR_shade_table(uint8_t light)
{
   return shade_table[light];
}

#undef READ
//-------------------------------------
