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
   uint8_t *mem_pak = NULL;
   RvR_rw rw = {0};

   //Allocate palette if it isn't yet
   if(palette==NULL)
   {
      palette = RvR_malloc(sizeof(*palette)*256);
      RvR_error_check(palette!=NULL,0x001);
   }

   //Format lump name
   //Palettes must be named in this exact way (e.g. PAL00000)
   char tmp[64];
   snprintf(tmp,64,"PAL%05d",id);

   //Read palette from lump and create rw stream
   if((mem_pak = RvR_lump_get(tmp,&size_in))==NULL)
      RvR_error_check(0,0x000);
   RvR_rw_init_mem(&rw,mem_pak,size_in);

   //Read palette and perform post processing
   for(unsigned i = 0;i<256;i++)
   {
      palette[i].r = RvR_rw_read_u8(&rw);
      palette[i].g = RvR_rw_read_u8(&rw);
      palette[i].b = RvR_rw_read_u8(&rw);
      palette[i].a = 255;
   }
   pal_calculate_colormap();

   //Cleanup
   RvR_rw_close(&rw);
   RvR_free(mem_pak);

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
//-------------------------------------
