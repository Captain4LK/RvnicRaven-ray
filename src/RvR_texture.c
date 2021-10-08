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
#define PERMANENT INT8_MAX

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
static RvR_texture **textures = NULL;
static int8_t *textures_timeout = NULL;
//-------------------------------------

//Function prototypes
static RvR_texture *texture_load(const uint8_t *mem, unsigned len);
//-------------------------------------

//Function implementations

RvR_texture *RvR_texture_get(uint16_t id)
{
   return textures[id];
}

void RvR_texture_load_begin()
{
   if(textures!=NULL&&textures_timeout!=NULL)
   {
      for(int i = 0;i<UINT16_MAX;i++)
         if(textures_timeout[i]!=PERMANENT)
            textures_timeout[i]--;
   }
}

void RvR_texture_load_end()
{
   if(textures!=NULL&&textures_timeout!=NULL)
   {
      for(int i = 0;i<UINT16_MAX;i++)
      {
         if(textures_timeout[i]<=0)
         {
            textures_timeout[i] = 0;
            if(textures[i]!=NULL)
            {
               RvR_free(textures[i]->data);
               RvR_free(textures[i]);
            }
            textures[i] = NULL;
         }
      }
   }
}

void RvR_texture_load(uint16_t id)
{
   if(textures==NULL)
   {
      textures = RvR_malloc(sizeof(*textures)*UINT16_MAX);
      textures_timeout = RvR_malloc(sizeof(*textures_timeout)*UINT16_MAX);
      memset(textures,0,sizeof(*textures)*UINT16_MAX);
      memset(textures_timeout,0,sizeof(*textures_timeout)*UINT16_MAX);
   }

   textures_timeout[id] = RVR_TEXTURE_TIMEOUT;
   if(textures[id]!=NULL)
      return;

   char tmp[64];
   sprintf(tmp,"TEX%05d",id);

   unsigned size_in;
   int32_t size_out;
   uint8_t *mem_pak, *mem_decomp;
   mem_pak = RvR_lump_get(tmp,RVR_LUMP_TEX,&size_in);
   mem_decomp = RvR_mem_decompress(mem_pak,size_in,&size_out);
   textures[id] = texture_load(mem_decomp,size_out);

   RvR_free(mem_pak);
   RvR_free(mem_decomp);
}

void RvR_font_load(uint16_t id)
{
   RvR_texture_load(id);
   textures_timeout[id] = PERMANENT;

   if(RvR_texture_get(id))
      RvR_draw_set_font(RvR_texture_get(id));
   else
      RvR_draw_set_font(RvR_texture_get(0));
}

void RvR_font_unload(uint16_t id)
{
   if(textures[id]!=NULL)
      textures_timeout[id] = 0;
}

static RvR_texture *texture_load(const uint8_t *mem, unsigned len)
{
   RvR_texture *p = NULL;
   int32_t width = 0;
   int32_t height = 0;
   unsigned pos = 0;

   READ(width,mem,pos,len,int32_t);
   READ(height,mem,pos,len,int32_t);

   p = RvR_malloc(sizeof(*p));
   p->data = RvR_malloc(sizeof(*p->data)*width*height);
   p->width = width;
   p->height = height;
   RvR_error_check(p!=NULL,0x001);
   RvR_error_check(p->data!=NULL,0x001);

   for(int i = 0;i<width*height;i++)
      READ(p->data[i],mem,pos,len,uint8_t);

   return p;

RvR_err:

   RvR_log("RvR error %s\n",RvR_error_get_string());

   return NULL;
}

#undef READ
#undef PERMANENT
//-------------------------------------
