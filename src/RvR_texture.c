/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
typedef struct
{
   int32_t last_access;
   uint16_t tex;
}Texture_cache_entry;
//-------------------------------------

//Variables
static RvR_texture **textures = NULL;
static int16_t *textures_cache = NULL;

static int32_t texture_last_access = 1;
static struct
{
   Texture_cache_entry *cache;
   unsigned cache_used;
}texture_cache = {.cache = NULL, .cache_used = 0};
//-------------------------------------

//Function prototypes
static void texture_load(uint16_t id);
//-------------------------------------

//Function implementations

RvR_texture *RvR_texture_get(uint16_t id)
{
   if(textures==NULL||textures[id]==NULL)
      texture_load(id);

   int cache_id = textures_cache[id];
   if(cache_id==-1)
      return textures[id];

   texture_cache.cache[cache_id].last_access = texture_last_access++;
   if(texture_last_access==INT32_MAX)
   {
#if RVR_TEXTURE_DEBUG
      RvR_log("normalizing cache\n");
#endif

      texture_last_access = 0;
      for(int i = 0;i<texture_cache.cache_used;i++)
      {
         texture_cache.cache[i].last_access>>=16;
         if(texture_cache.cache[i].last_access>texture_last_access)
            texture_last_access = texture_cache.cache[i].last_access;
      }
   }

   return textures[id];
}

static void texture_load(uint16_t id)
{
   if(texture_cache.cache==NULL)
   {
      texture_cache.cache_used = 0;
      texture_cache.cache = RvR_malloc(sizeof(*texture_cache.cache)*RVR_TEXTURE_MAX);
      memset(texture_cache.cache,0,sizeof(*texture_cache.cache)*RVR_TEXTURE_MAX);
   }
   if(textures==NULL)
   {
      textures = RvR_malloc(sizeof(*textures)*(UINT16_MAX+1));
      textures_cache = RvR_malloc(sizeof(*textures_cache)*(UINT16_MAX+1));
      memset(textures,0,sizeof(*textures)*(UINT16_MAX+1));
      memset(textures_cache,0,sizeof(*textures_cache)*(UINT16_MAX+1));
   }

   int index_new = texture_cache.cache_used;

   //Cache full --> delete 'oldest' texture
   if(texture_cache.cache_used==RVR_TEXTURE_MAX)
   {
      //Find 'oldest' texture
      int32_t tex_old = texture_last_access;
      int tex_old_index = 0;
      for(int i = 0;i<texture_cache.cache_used;i++)
      {
         if(texture_cache.cache[i].last_access<tex_old)
         {
            tex_old_index = i;
            tex_old = texture_cache.cache[i].last_access;
         }
      }

#if RVR_TEXTURE_DEBUG
      RvR_log("unloading texture %d\n",texture_cache.cache[tex_old_index].tex);
#endif
      
      //Delete texture
      int tex_index = texture_cache.cache[tex_old_index].tex;
      if(textures[tex_index]!=NULL)
         RvR_free(textures[tex_index]);
      textures[tex_index] = NULL;
      

      index_new = tex_old_index;
   }
   else
   {
      texture_cache.cache_used++;
   }

#if RVR_TEXTURE_DEBUG
   RvR_log("loading texture %d\n",id);
#endif

   //Format lump name
   //Textures must be named in this exact way (e.g. TEX00000)
   char tmp[64];
   sprintf(tmp,"TEX%05d",id);

   unsigned size_in;
   int32_t size_out;
   uint8_t *mem_pak, *mem_decomp;
   uint8_t endian;
   mem_pak = RvR_lump_get(tmp,&size_in);
   RvR_rw rw_decomp;
   RvR_rw_init_const_mem(&rw_decomp,mem_pak,size_in);
   mem_decomp = RvR_decompress(&rw_decomp,&size_out,&endian);
   RvR_rw_close(&rw_decomp);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw,mem_decomp,size_out);
   RvR_rw_endian(&rw,endian);

   int32_t width = RvR_rw_read_i32(&rw);
   int32_t height = RvR_rw_read_i32(&rw);
   textures[id] = RvR_malloc(sizeof(*textures[id])+sizeof(*textures[id]->data)*width*height);
   textures[id]->width = width;
   textures[id]->height = height;
   for(int i = 0;i<textures[id]->width*textures[id]->height;i++)
      textures[id]->data[i] = RvR_rw_read_u8(&rw);
   textures_cache[id] = index_new;
   texture_cache.cache[index_new].tex = id;

   RvR_rw_close(&rw);

   RvR_free(mem_pak);
   RvR_free(mem_decomp);
}

void RvR_texture_create(uint16_t id, int width, int height)
{
   if(textures==NULL)
   {
      textures = RvR_malloc(sizeof(*textures)*(UINT16_MAX+1));
      textures_cache = RvR_malloc(sizeof(*textures_cache)*(UINT16_MAX+1));
      memset(textures,0,sizeof(*textures)*(UINT16_MAX+1));
      memset(textures_cache,0,sizeof(*textures_cache)*(UINT16_MAX+1));
   }

   if(textures[id]!=NULL)
      return;

   textures[id] = RvR_malloc(sizeof(*textures[id])+sizeof(*textures[id]->data)*width*height);
   textures[id]->width = width;
   textures[id]->height = height;
   textures_cache[id] = -1;
}

void RvR_texture_create_free(uint16_t id)
{
   RvR_free(textures[id]);
   textures[id] = NULL;
}
//-------------------------------------
