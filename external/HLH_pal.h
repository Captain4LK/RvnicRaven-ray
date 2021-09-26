/*
Corax palette file format

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _HLH_PAL_H_

#define _HLH_PAL_H_

typedef struct
{
   uint8_t r;
   uint8_t g;
   uint8_t b;
}HLH_pal_color;

typedef struct
{
   HLH_pal_color colors[256];
   uint8_t shades[64][256]; //Trying to improve cache hits by putting all colors of the same shade next to each other
}HLH_palette;

HLH_palette *HLH_pal_load_from_file(const char *path);
HLH_palette *HLH_pal_load_from_mem(const void *mem, unsigned size);
int          HLH_pal_save(const char *path, const HLH_palette *pal);
void         HLH_pal_calculate_colormap(HLH_palette *pal);
void         HLH_pal_free(HLH_palette *pal);
const char  *HLH_pal_get_error();

#endif

#ifdef HLH_PAL_IMPLEMENTATION
#ifndef HLH_PAL_IMPLEMENTATION_ONCE
#define HLH_PAL_IMPLEMENTATION_ONCE

#ifndef HLH_PAL_NUM_FULLBRIGHT
#define HLH_PAL_NUM_FULLBRIGHT 32
#endif

#ifndef HLH_PAL_MALLOC
#define HLH_PAL_MALLOC malloc
#endif

#ifndef HLH_PAL_FREE
#define HLH_PAL_FREE free
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint8_t HLH_pal_read_u8(const uint8_t *buffer, unsigned *pos);
static uint32_t HLH_pal_read_u32(const uint8_t *buffer, unsigned *pos);

#ifdef HLH_PAL_ERROR
static const char *HLH_pal_error = NULL;
#define HLH_SET_ERROR(a) HLH_pal_error = (a)
#else
#ifdef _MSC_VER
#define HLH_SET_ERROR(v) (void)(v)
#else
#define HLH_SET_ERROR(v) (void)sizeof(v)
#endif
#endif

#define MAGIC_NUM ('H' + ('L' << 8) + ('H' << 16) + ('P' << 24))
#define MIN(a,b) \
   ((a)>(b)?(b):(a))

HLH_palette *HLH_pal_load_from_file(const char *path)
{
   FILE *f = fopen(path,"rb");
   if(!f)
   {
      HLH_SET_ERROR("HLH_pal_load_from_file: FILE pointer is NULL");
      return NULL;
   }
   unsigned size = 0;
   fseek(f,0,SEEK_END);
   size = ftell(f);
   fseek(f,0,SEEK_SET);
   uint8_t *buffer = HLH_PAL_MALLOC(size+1);
   fread(buffer,size,1,f);
   buffer[size] = 0;
   fclose(f);

   HLH_palette *p = HLH_pal_load_from_mem(buffer,size);
   HLH_PAL_FREE(buffer);

   return p;
}

HLH_palette *HLH_pal_load_from_mem(const void *mem, unsigned size)
{
   unsigned pos = 0;
   if(!mem)
   {
      HLH_SET_ERROR("HLH_pal_load_from_mem: mem is NULL");
      return NULL;
   }

   //Check magic num
   if(HLH_pal_read_u32(mem,&pos)!=MAGIC_NUM)
   {
      HLH_SET_ERROR("HLH_pal_load_from_mem: magic number incorrect");
      return NULL;
   }

   //Read colors
   HLH_palette *p = HLH_PAL_MALLOC(sizeof(*p));
   for(int i = 0;i<256;i++)
   {
      p->colors[i].r = HLH_pal_read_u8(mem,&pos); 
      p->colors[i].g = HLH_pal_read_u8(mem,&pos); 
      p->colors[i].b = HLH_pal_read_u8(mem,&pos); 
   }

   //Calculate colormap
   HLH_pal_calculate_colormap(p);

   return p;
}

int HLH_pal_save(const char *path, const HLH_palette *pal)
{
   FILE *f = fopen(path,"wb");
   if(!f)
   {
      HLH_SET_ERROR("HLH_pal_save: failed to open file");
      return 0;
   }
   int num = MAGIC_NUM;
   fwrite(&num,4,1,f);
   for(int i = 0;i<256;i++)
   {
      fwrite(&pal->colors[i].r,1,1,f);
      fwrite(&pal->colors[i].g,1,1,f);
      fwrite(&pal->colors[i].b,1,1,f);
   }
   fclose(f);
   
   return 1;
}

void HLH_pal_calculate_colormap(HLH_palette *pal)
{
   for(int x = 0;x<256;x++)
   {
      for(int y = 0;y<64;y++)
      {
         if(x<256-HLH_PAL_NUM_FULLBRIGHT)
         {
            int r,g,b;
            r = MIN(255,((int)pal->colors[x].r*(63-y)+16)>>5);
            g = MIN(255,((int)pal->colors[x].g*(63-y)+16)>>5);
            b = MIN(255,((int)pal->colors[x].b*(63-y)+16)>>5);

            int best_index = -1;
            int best_dist = 0;
            for(int i = 0;i<256;i++)
            {
               int dist = 0;
               dist+=abs(r-pal->colors[i].r)*abs(r-pal->colors[i].r);
               dist+=abs(g-pal->colors[i].g)*abs(g-pal->colors[i].g);
               dist+=abs(b-pal->colors[i].b)*abs(b-pal->colors[i].b);

               if(best_index==-1||dist<best_dist)
               {
                  best_index = i;
                  best_dist = dist;
               }
            }
            pal->shades[y][x] = (uint8_t)best_index;
         }
         else
         {
            pal->shades[y][x] = x;
         }
      }
   }
}

void HLH_pal_free(HLH_palette *pal)
{
   HLH_PAL_FREE(pal);
}

const char *HLH_pal_get_error()
{
#ifdef HLH_PAL_ERROR
   return HLH_pal_error;
#else
   return NULL;
#endif
}

static uint8_t HLH_pal_read_u8(const uint8_t *buffer, unsigned *pos)
{
   uint8_t ret = *((uint8_t *)&buffer[*pos]);
   (*pos)++;
   return ret;
}

static uint32_t HLH_pal_read_u32(const uint8_t *buffer, unsigned *pos)
{
   uint32_t ret = *((uint32_t *)&buffer[*pos]);
   (*pos)+=4;
   return ret;
}

#undef HLH_SET_ERROR
#undef MAGIC_NUM

#endif
#endif
