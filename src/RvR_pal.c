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
#include "RvR_core.h"
#include "RvR_malloc.h"
#include "RvR_pak.h"
#include "RvR_math.h"
#include "RvR_pal.h"
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
uint8_t RvR_shade_table[64][256] = {0};
RvR_color *RvR_palette = NULL;
//-------------------------------------

//Function prototypes
static void HLH_pal_calculate_colormap();
//-------------------------------------

//Function implementations

void RvR_palette_load(uint16_t id)
{
   unsigned size_in = 0;
   unsigned pos = 0;
   uint8_t *mem_pak = NULL;
   char tmp[64] = "";

   sprintf(tmp,"PAL%05d",id);
   mem_pak = RvR_lump_get(tmp,RVR_LUMP_PAL,&size_in);

   if(RvR_palette==NULL)
   {
      RvR_palette = RvR_malloc(sizeof(*RvR_palette)*256);
   }
   RvR_error_check(RvR_palette!=NULL,0x001);

   for(unsigned i = 0;i<256;i++)
   {
      READ(RvR_palette[i].r,mem_pak,pos,size_in,uint8_t);
      READ(RvR_palette[i].g,mem_pak,pos,size_in,uint8_t);
      READ(RvR_palette[i].b,mem_pak,pos,size_in,uint8_t);
      RvR_palette[i].a = 255;
   }

   HLH_pal_calculate_colormap();

   RvR_free(mem_pak);

   return;

RvR_err:

   if(mem_pak!=NULL)
      RvR_free(mem_pak);

   RvR_log("RvR error %s\n",RvR_error_get_string());
}

static void HLH_pal_calculate_colormap()
{
   for(int x = 0;x<256;x++)
   {
      for(int y = 0;y<64;y++)
      {
         if(x<256)
         {
            int r,g,b;
            r = RvR_min(255,((int)RvR_palette[x].r*(63-y)+16)>>5);
            g = RvR_min(255,((int)RvR_palette[x].g*(63-y)+16)>>5);
            b = RvR_min(255,((int)RvR_palette[x].b*(63-y)+16)>>5);

            int best_index = -1;
            int best_dist = 0;
            for(int i = 0;i<256;i++)
            {
               int dist = 0;
               dist+=abs(r-RvR_palette[i].r)*abs(r-RvR_palette[i].r);
               dist+=abs(g-RvR_palette[i].g)*abs(g-RvR_palette[i].g);
               dist+=abs(b-RvR_palette[i].b)*abs(b-RvR_palette[i].b);

               if(best_index==-1||dist<best_dist)
               {
                  best_index = i;
                  best_dist = dist;
               }
            }
            RvR_shade_table[y][x] = (uint8_t)best_index;
         }
         else
         {
            RvR_shade_table[y][x] = x;
         }
      }
   }
}
//-------------------------------------
