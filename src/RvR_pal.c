/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
static uint8_t trans_table[256][256] = {0};
static RvR_color *palette = NULL;
//-------------------------------------

//Function prototypes
static void pal_calculate_colormap();

static uint8_t pal_find_closest(int r, int g, int b);
//-------------------------------------

//Function implementations

void RvR_palette_load(uint16_t id)
{
   unsigned size_in = 0;
   uint8_t *mem_pak = NULL;
   RvR_rw rw = {0};

   //Allocate palette if it isn't yet
   if(palette==NULL)
      palette = RvR_malloc(sizeof(*palette)*256);

   //Format lump name
   //Palettes must be named in this exact way (e.g. PAL00000)
   char tmp[64];
   snprintf(tmp,64,"PAL%05d",id);

   //Read palette from lump and create rw stream
   mem_pak = RvR_lump_get(tmp,&size_in);
   RvR_rw_init_mem(&rw,mem_pak,size_in,size_in);

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
}

static void pal_calculate_colormap()
{
   //Distance fading
   for(int x = 0;x<256;x++)
   {
      for(int y = 0;y<64;y++)
      {
         if(x==0)
         {
            shade_table[y][x] = 0;
            continue;
         }

         int r = RvR_max(0,RvR_min(255,((int)palette[x].r*(63-y))/63));
         int g = RvR_max(0,RvR_min(255,((int)palette[x].g*(63-y))/63));
         int b = RvR_max(0,RvR_min(255,((int)palette[x].b*(63-y))/63));

         shade_table[y][x] = pal_find_closest(r,g,b);
      }
   }

   //Transparancy
   for(int i = 0;i<256;i++)
   {
      for(int j = 0;j<256;j++)
      {
         //Special case: entry 0 is transparent
         if(i==0)
         {
            trans_table[i][j] = j;
            continue;
         }
         else if(j==0)
         {
            trans_table[i][j] = i;
            continue;
         }

         RvR_fix22 t = 2048/3;
         uint8_t r = RvR_max(0,RvR_min(255,(t*palette[i].r+(1024-t)*palette[j].r)/1024));
         uint8_t g = RvR_max(0,RvR_min(255,(t*palette[i].g+(1024-t)*palette[j].g)/1024));
         uint8_t b = RvR_max(0,RvR_min(255,(t*palette[i].b+(1024-t)*palette[j].b)/1024));

         trans_table[i][j] = pal_find_closest(r,g,b);
      }
   }
}

static uint8_t pal_find_closest(int r, int g, int b)
{
   int32_t best_dist = INT32_MAX;
   uint8_t best_index = 0;

   for(int i = 0;i<256;i++)
   {
      int32_t dist = 0;
      dist+=(r-palette[i].r)*(r-palette[i].r);
      dist+=(g-palette[i].g)*(g-palette[i].g);
      dist+=(b-palette[i].b)*(b-palette[i].b);
      
      if(dist<best_dist)
      {
         best_index = i;
         best_dist = dist;
      }
   }

   return best_index;
}

const RvR_color *RvR_palette()
{
   return palette;
}

uint8_t *RvR_shade_table(uint8_t light)
{
   return shade_table[light];
}

uint8_t RvR_blend(uint8_t c0, uint8_t c1)
{
   return trans_table[c0][c1];
}
//-------------------------------------
