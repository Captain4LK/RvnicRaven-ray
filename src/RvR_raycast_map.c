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
#include "RvR_core.h"
#include "RvR_math.h"
#include "RvR_malloc.h"
#include "RvR_pak.h"
#include "RvR_texture.h"
#include "RvR_compress.h"
#include "RvR_ray_map.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint16_t *level_wall_tex = NULL;
static uint16_t *level_floor_tex = NULL;
static uint16_t *level_ceil_tex = NULL;
static int8_t *level_floor = NULL;
static int8_t *level_ceiling = NULL;
static uint16_t level_width = 0;
static uint16_t level_height = 0;
static uint8_t level_floor_color = 23;

static struct
{
   uint16_t *level_wall_tex;
   uint16_t *level_floor_tex;
   uint16_t *level_ceil_tex;
   int8_t *level_floor;
   int8_t *level_ceiling;
   uint16_t level_width;
   uint16_t level_height;
   uint8_t level_floor_color;

   RvR_ray_map_sprite *level_sprites;
   uint32_t level_sprite_count;
}map_cache = {0};
//-------------------------------------

//Function prototypes
static void map_free_editor();
static void map_free_level();
//-------------------------------------

//Function implementations

void RvR_ray_map_create(uint16_t width, uint16_t height)
{
   map_free_editor();
   map_free_level();

   map_cache.level_width = width;
   map_cache.level_height = height;
   map_cache.level_wall_tex = RvR_malloc(map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_wall_tex));
   map_cache.level_floor_tex = RvR_malloc(map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_floor_tex));
   map_cache.level_ceil_tex = RvR_malloc(map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_ceil_tex));
   map_cache.level_floor = RvR_malloc(map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_floor));
   map_cache.level_ceiling = RvR_malloc(map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_ceiling));
   memset(map_cache.level_wall_tex,0,map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_wall_tex));
   memset(map_cache.level_floor_tex,0,map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_floor_tex));
   memset(map_cache.level_ceil_tex,0,map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_ceil_tex));
   memset(map_cache.level_floor,0,map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_floor));
   memset(map_cache.level_ceiling,0,map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_ceiling));

   map_cache.level_floor_color = 23;

   for(int y = 1;y<map_cache.level_height-1;y++)
   {
      for(int x = 1;x<map_cache.level_width-1;x++)
      {
         map_cache.level_floor[y*map_cache.level_width+x] = 0;
         map_cache.level_ceiling[y*map_cache.level_width+x] = 127;
      }
   }

   //TODO: don't call this here, make user call this
   //If I don't reset the map myself, it is possible to cache a map by loading it, but not reseting
   RvR_ray_map_reset_full();
}

static void map_free_editor()
{
   if(map_cache.level_wall_tex!=NULL)
      RvR_free(map_cache.level_wall_tex);
   if(map_cache.level_floor_tex!=NULL)
      RvR_free(map_cache.level_floor_tex);
   if(map_cache.level_ceil_tex!=NULL)
      RvR_free(map_cache.level_ceil_tex);
   if(map_cache.level_floor!=NULL)
      RvR_free(map_cache.level_floor);
   if(map_cache.level_ceiling!=NULL)
      RvR_free(map_cache.level_ceiling);
   if(map_cache.level_sprites!=NULL)
      RvR_free(map_cache.level_sprites);
   map_cache.level_wall_tex = NULL;
   map_cache.level_floor = NULL;
   map_cache.level_ceiling = NULL;
   map_cache.level_width = 0;
   map_cache.level_height = 0;
   map_cache.level_sprite_count= 0;
}

static void map_free_level()
{
   if(level_wall_tex!=NULL)
      RvR_free(level_wall_tex);
   if(level_floor_tex!=NULL)
      RvR_free(level_floor_tex);
   if(level_ceil_tex!=NULL)
      RvR_free(level_ceil_tex);
   if(level_floor!=NULL)
      RvR_free(level_floor);
   if(level_ceiling!=NULL)
      RvR_free(level_ceiling);
   level_wall_tex = NULL;
   level_floor = NULL;
   level_ceiling = NULL;
   level_width = 0;
   level_height = 0;
}

void RvR_ray_map_reset()
{
   if(level_width!=map_cache.level_width||level_height!=map_cache.level_height|| //Wrong dimensions
      level_floor==NULL||level_ceiling==NULL||level_wall_tex==NULL //Not allocated yet
      ) 
   {
      map_free_level();

      level_width = map_cache.level_width;
      level_height = map_cache.level_height;
      level_wall_tex = RvR_malloc(level_width*level_height*sizeof(*level_wall_tex));
      level_floor_tex = RvR_malloc(level_width*level_height*sizeof(*level_floor_tex));
      level_ceil_tex = RvR_malloc(level_width*level_height*sizeof(*level_ceil_tex));
      level_floor = RvR_malloc(level_width*level_height*sizeof(*level_floor));
      level_ceiling = RvR_malloc(level_width*level_height*sizeof(*level_ceiling));
      memset(level_wall_tex,0,level_width*level_height*sizeof(*level_wall_tex));
      memset(level_floor_tex,0,level_width*level_height*sizeof(*level_floor_tex));
      memset(level_ceil_tex,0,level_width*level_height*sizeof(*level_ceil_tex));
      memset(level_floor,0,level_width*level_height*sizeof(*level_floor));
      memset(level_ceiling,0,level_width*level_height*sizeof(*level_ceiling));
   }

   //Misc
   level_floor_color = map_cache.level_floor_color;

   memcpy(level_wall_tex,map_cache.level_wall_tex,sizeof(*map_cache.level_wall_tex)*map_cache.level_width*map_cache.level_height);
   memcpy(level_floor_tex,map_cache.level_floor_tex,sizeof(*map_cache.level_floor_tex)*map_cache.level_width*map_cache.level_height);
   memcpy(level_ceil_tex,map_cache.level_ceil_tex,sizeof(*map_cache.level_ceil_tex)*map_cache.level_width*map_cache.level_height);
   memcpy(level_floor,map_cache.level_floor,sizeof(*map_cache.level_floor)*map_cache.level_width*map_cache.level_height);
   memcpy(level_ceiling,map_cache.level_ceiling,sizeof(*map_cache.level_ceiling)*map_cache.level_width*map_cache.level_height);

   //Textures
   RvR_texture_load_begin();
   RvR_texture_load(0); //Texture 0 is always loaded, since it's used as a fallback texture
   RvR_texture_load(0x5300); //Sprite test texture
   for(int i = 0;i<level_width*level_height;i++)
   {
      RvR_texture_load(level_wall_tex[i]);
      RvR_texture_load(level_floor_tex[i]);
      RvR_texture_load(level_ceil_tex[i]);
   }
   RvR_texture_load_end();

   //TODO: do we want to do this here?
   RvR_pak_flush();
}

int RvR_ray_map_sprite_count()
{
   return map_cache.level_sprite_count;
}

RvR_ray_map_sprite *RvR_ray_map_sprite_get(unsigned index)
{
   if(index>=map_cache.level_sprite_count)
      return NULL;
   return &map_cache.level_sprites[index];
}

void RvR_ray_map_reset_full()
{
   RvR_ray_map_reset();
}

void RvR_ray_map_load_path(const char *path)
{
   int size = 0;
   uint8_t *mem = RvR_decompress_path(path,&size);

   RvR_ray_map_load_mem(mem,size);
   RvR_free(mem);
}

void RvR_ray_map_load(uint16_t id)
{
   char tmp[64];
   sprintf(tmp,"MAP%05d",id);

   unsigned size_in;
   int32_t size_out;
   uint8_t *mem_pak, *mem_decomp;
   mem_pak = RvR_lump_get(tmp,RVR_LUMP_MAP,&size_in);
   mem_decomp = RvR_mem_decompress(mem_pak,size_in,&size_out);

   RvR_free(mem_pak);
   RvR_ray_map_load_mem(mem_decomp,size_out);
   RvR_free(mem_decomp);
}

void RvR_ray_map_load_mem(uint8_t *mem, int len)
{
   int pos = 0;

   //Read level width and height
   map_cache.level_width = *(uint16_t *)(mem+pos); pos+=2;
   map_cache.level_height = *(uint16_t *)(mem+pos); pos+=2;

   //Read sprite count
   map_cache.level_sprite_count = *(uint32_t *)(mem+pos); pos+=4;
   map_cache.level_sprites = RvR_malloc(sizeof(*map_cache.level_sprites)*map_cache.level_sprite_count);

   //Read floor color
   map_cache.level_floor_color = *(uint8_t *)(mem+pos); pos+=1;

   //Read texture, floor and ceiling
   map_cache.level_wall_tex = RvR_malloc(map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_wall_tex));
   map_cache.level_floor_tex = RvR_malloc(map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_floor_tex));
   map_cache.level_ceil_tex = RvR_malloc(map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_ceil_tex));
   map_cache.level_floor = RvR_malloc(map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_floor));
   map_cache.level_ceiling = RvR_malloc(map_cache.level_width*map_cache.level_height*sizeof(*map_cache.level_ceiling));
   memcpy(map_cache.level_wall_tex,mem+pos,map_cache.level_width*map_cache.level_height*2); pos+=map_cache.level_width*map_cache.level_height*2;
   memcpy(map_cache.level_floor_tex,mem+pos,map_cache.level_width*map_cache.level_height*2); pos+=map_cache.level_width*map_cache.level_height*2;
   memcpy(map_cache.level_ceil_tex,mem+pos,map_cache.level_width*map_cache.level_height*2); pos+=map_cache.level_width*map_cache.level_height*2;
   memcpy(map_cache.level_floor,mem+pos,map_cache.level_width*map_cache.level_height); pos+=map_cache.level_width*map_cache.level_height;
   memcpy(map_cache.level_ceiling,mem+pos,map_cache.level_width*map_cache.level_height); pos+=map_cache.level_width*map_cache.level_height;

   //Read sprites
   for(unsigned i = 0;i<map_cache.level_sprite_count;i++)
   {
      map_cache.level_sprites[i].type = *(uint16_t *)(mem+pos); pos+=2;
      map_cache.level_sprites[i].pos.x = *(uint32_t *)(mem+pos); pos+=4;
      map_cache.level_sprites[i].pos.y = *(uint32_t *)(mem+pos); pos+=4;
      map_cache.level_sprites[i].pos.z = *(uint32_t *)(mem+pos); pos+=4;
   }

   //Update actual map
   RvR_ray_map_reset_full();
}

void RvR_ray_map_save(const char *path)
{
   //Calculate needed memory
   int size = 0;
   size+=2; //level_width
   size+=2; //level_height
   size+=1; //level_floor_color
   size+=map_cache.level_width*map_cache.level_height*2; //level_wall_tex
   size+=map_cache.level_width*map_cache.level_height*2; //level_floor_tex
   size+=map_cache.level_width*map_cache.level_height*2; //level_ceil_tex
   size+=map_cache.level_width*map_cache.level_height; //level_floor
   size+=map_cache.level_width*map_cache.level_height; //level_ceiling

   uint8_t *mem = RvR_malloc(size);
   int pos = 0;

   //Write width and height
   *(uint16_t *)(mem+pos) = map_cache.level_width; pos+=2;
   *(uint16_t *)(mem+pos) = map_cache.level_height; pos+=2;

   //Write sprite count
   *(uint32_t *)(mem+pos) = map_cache.level_sprite_count; pos+=4;

   //Write floor color
   *(uint8_t *)(mem+pos) = map_cache.level_floor_color; pos+=1;

   //Write texture, floor and ceiling
   memcpy(mem+pos,map_cache.level_wall_tex,map_cache.level_width*map_cache.level_height*2); pos+=map_cache.level_width*map_cache.level_height*2;
   memcpy(mem+pos,map_cache.level_floor_tex,map_cache.level_width*map_cache.level_height*2); pos+=map_cache.level_width*map_cache.level_height*2;
   memcpy(mem+pos,map_cache.level_ceil_tex,map_cache.level_width*map_cache.level_height*2); pos+=map_cache.level_width*map_cache.level_height*2;
   memcpy(mem+pos,map_cache.level_floor,map_cache.level_width*map_cache.level_height); pos+=map_cache.level_width*map_cache.level_height;
   memcpy(mem+pos,map_cache.level_ceiling,map_cache.level_width*map_cache.level_height); pos+=map_cache.level_width*map_cache.level_height;

   //Write sprites
   for(unsigned i = 0;i<map_cache.level_sprite_count;i++)
   {
      *(uint16_t *)(mem+pos) = map_cache.level_sprites[i].type; pos+=2;
      *(int32_t *)(mem+pos) = map_cache.level_sprites[i].pos.x; pos+=4;
      *(int32_t *)(mem+pos) = map_cache.level_sprites[i].pos.y; pos+=4;
      *(int32_t *)(mem+pos) = map_cache.level_sprites[i].pos.z; pos+=4;
   }

   //Compress and write to disk
   FILE *f = fopen(path,"wb");
   RvR_mem_compress(mem,size,f);
   fclose(f);

   //Free temp buffer
   RvR_free(mem);
}

uint16_t RvR_ray_map_wall_tex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<level_width&&y>=0&&y<level_height)
      return level_wall_tex[y*level_width+x]; 

   return 0;
}

uint16_t RvR_ray_map_floor_tex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<level_width&&y>=0&&y<level_height)
      return level_floor_tex[y*level_width+x]; 

   return 0;
}

uint16_t RvR_ray_map_ceil_tex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<level_width&&y>=0&&y<level_height)
      return level_ceil_tex[y*level_width+x]; 

   return 0x43;
}

RvR_fix22 RvR_ray_map_floor_height_at(int16_t x, int16_t y)
{
   if(x>=0&&x<level_width&&y>=0&&y<level_height)
      return (level_floor[y*level_width+x]*1024)/8;

   return 0;
}

RvR_fix22 RvR_ray_map_ceiling_height_at(int16_t x, int16_t y)
{
   int v = 127;

   if(x>=0&&x<level_width&&y>=0&&y<level_height)
      v = level_ceiling[(y)*level_width+x]; 

   return (v*1024)/8;
}
//-------------------------------------
