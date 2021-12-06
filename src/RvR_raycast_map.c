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
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
RvR_ray_map ray_map = {0};
RvR_ray_map_cache ray_map_cache = {0};
//-------------------------------------

//Function prototypes
static void ray_map_free_cache();
static void ray_map_free_level();
//-------------------------------------

//Function implementations

void RvR_ray_map_create(uint16_t width, uint16_t height)
{
   ray_map_free_cache();

   ray_map_cache.width = width;
   ray_map_cache.height = height;
   ray_map_cache.wall_ftex = RvR_malloc(ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.wall_ftex));
   ray_map_cache.wall_ctex = RvR_malloc(ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.wall_ctex));
   ray_map_cache.floor_tex = RvR_malloc(ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.floor_tex));
   ray_map_cache.ceil_tex = RvR_malloc(ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.ceil_tex));
   ray_map_cache.floor = RvR_malloc(ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.floor));
   ray_map_cache.ceiling = RvR_malloc(ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.ceiling));
   memset(ray_map_cache.wall_ftex,0,ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.wall_ftex));
   memset(ray_map_cache.wall_ctex,0,ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.wall_ctex));
   memset(ray_map_cache.floor_tex,0,ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.floor_tex));
   memset(ray_map_cache.ceil_tex,0,ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.ceil_tex));
   memset(ray_map_cache.floor,0,ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.floor));
   memset(ray_map_cache.ceiling,0,ray_map_cache.width*ray_map_cache.height*sizeof(*ray_map_cache.ceiling));

   ray_map_cache.floor_color = 23;
   ray_map_cache.sky_tex = 0;
   ray_map_cache.sprite_count = 0;

   for(int y = 1;y<ray_map_cache.height-1;y++)
   {
      for(int x = 1;x<ray_map_cache.width-1;x++)
      {
         ray_map_cache.floor[y*ray_map_cache.width+x] = 0;
         ray_map_cache.ceiling[y*ray_map_cache.width+x] = 127;
      }
   }

   //TODO: don't call this here, make user call this
   //If I don't reset the map myself, it is possible to cache a map by loading it, but not reseting
   RvR_ray_map_reset_full();
}

static void ray_map_free_cache()
{
   if(ray_map_cache.wall_ftex!=NULL)
      RvR_free(ray_map_cache.wall_ftex);
   if(ray_map_cache.wall_ctex!=NULL)
      RvR_free(ray_map_cache.wall_ctex);
   if(ray_map_cache.floor_tex!=NULL)
      RvR_free(ray_map_cache.floor_tex);
   if(ray_map_cache.ceil_tex!=NULL)
      RvR_free(ray_map_cache.ceil_tex);
   if(ray_map_cache.floor!=NULL)
      RvR_free(ray_map_cache.floor);
   if(ray_map_cache.ceiling!=NULL)
      RvR_free(ray_map_cache.ceiling);
   if(ray_map_cache.sprites!=NULL)
      RvR_free(ray_map_cache.sprites);

   ray_map_cache.wall_ftex = NULL;
   ray_map_cache.wall_ctex = NULL;
   ray_map_cache.floor_tex = NULL;
   ray_map_cache.ceil_tex = NULL;
   ray_map_cache.floor = NULL;
   ray_map_cache.ceiling = NULL;
   ray_map_cache.width = 0;
   ray_map_cache.height = 0;
   ray_map_cache.sprite_count= 0;
}

static void ray_map_free_level()
{
   if(ray_map.wall_ftex!=NULL)
      RvR_free(ray_map.wall_ftex);
   if(ray_map.wall_ctex!=NULL)
      RvR_free(ray_map.wall_ctex);
   if(ray_map.floor_tex!=NULL)
      RvR_free(ray_map.floor_tex);
   if(ray_map.ceil_tex!=NULL)
      RvR_free(ray_map.ceil_tex);
   if(ray_map.floor!=NULL)
      RvR_free(ray_map.floor);
   if(ray_map.ceiling!=NULL)
      RvR_free(ray_map.ceiling);

   ray_map.wall_ftex = NULL;
   ray_map.wall_ctex = NULL;
   ray_map.floor_tex = NULL;
   ray_map.ceil_tex= NULL;
   ray_map.floor = NULL;
   ray_map.ceiling = NULL;
   ray_map.width = 0;
   ray_map.height = 0;
}

void RvR_ray_map_reset()
{
   if(ray_map.width!=ray_map_cache.width||ray_map.height!=ray_map_cache.height|| //Wrong dimensions
      ray_map.floor==NULL||ray_map.ceiling==NULL||ray_map.wall_ftex==NULL||ray_map.wall_ctex==NULL) //Not allocated yet)
   {
      ray_map_free_level();

      ray_map.width = ray_map_cache.width;
      ray_map.height = ray_map_cache.height;
      ray_map.wall_ftex = RvR_malloc(ray_map.width*ray_map.height*sizeof(*ray_map.wall_ftex));
      ray_map.wall_ctex = RvR_malloc(ray_map.width*ray_map.height*sizeof(*ray_map.wall_ctex));
      ray_map.floor_tex = RvR_malloc(ray_map.width*ray_map.height*sizeof(*ray_map.floor_tex));
      ray_map.ceil_tex = RvR_malloc(ray_map.width*ray_map.height*sizeof(*ray_map.ceil_tex));
      ray_map.floor = RvR_malloc(ray_map.width*ray_map.height*sizeof(*ray_map.floor));
      ray_map.ceiling = RvR_malloc(ray_map.width*ray_map.height*sizeof(*ray_map.ceiling));

      memset(ray_map.wall_ftex,0,ray_map.width*ray_map.height*sizeof(*ray_map.wall_ftex));
      memset(ray_map.wall_ctex,0,ray_map.width*ray_map.height*sizeof(*ray_map.wall_ctex));
      memset(ray_map.floor_tex,0,ray_map.width*ray_map.height*sizeof(*ray_map.floor_tex));
      memset(ray_map.ceil_tex,0,ray_map.width*ray_map.height*sizeof(*ray_map.ceil_tex));
      memset(ray_map.floor,0,ray_map.width*ray_map.height*sizeof(*ray_map.floor));
      memset(ray_map.ceiling,0,ray_map.width*ray_map.height*sizeof(*ray_map.ceiling));
   }

   //Misc
   ray_map.floor_color = ray_map_cache.floor_color;
   ray_map.sky_tex = ray_map_cache.sky_tex;

   memcpy(ray_map.wall_ftex,ray_map_cache.wall_ftex,sizeof(*ray_map_cache.wall_ftex)*ray_map_cache.width*ray_map_cache.height);
   memcpy(ray_map.wall_ctex,ray_map_cache.wall_ctex,sizeof(*ray_map_cache.wall_ctex)*ray_map_cache.width*ray_map_cache.height);
   memcpy(ray_map.floor_tex,ray_map_cache.floor_tex,sizeof(*ray_map_cache.floor_tex)*ray_map_cache.width*ray_map_cache.height);
   memcpy(ray_map.ceil_tex,ray_map_cache.ceil_tex,sizeof(*ray_map_cache.ceil_tex)*ray_map_cache.width*ray_map_cache.height);
   for(int i = 0;i<ray_map.width*ray_map.height;i++)
   {
      ray_map.floor[i] = (int16_t)ray_map_cache.floor[i]*128;
      ray_map.ceiling[i] = (int16_t)ray_map_cache.ceiling[i]*128;
   }

   //Textures
   RvR_texture_load_begin();
   RvR_texture_load(0); //Texture 0 is always loaded, since it's used as a fallback texture
   RvR_texture_load(ray_map.sky_tex); //Sky texture is always used
   for(int i = 0;i<ray_map.width*ray_map.height;i++)
   {
      RvR_texture_load(ray_map.wall_ftex[i]);
      RvR_texture_load(ray_map.wall_ctex[i]);
      RvR_texture_load(ray_map.floor_tex[i]);
      RvR_texture_load(ray_map.ceil_tex[i]);
   }
   RvR_texture_load_end();

   //TODO: do we want to do this here?
   RvR_pak_flush();
}

int RvR_ray_map_sprite_count()
{
   return ray_map_cache.sprite_count;
}

RvR_ray_map_sprite *RvR_ray_map_sprite_get(unsigned index)
{
   if(index>=ray_map_cache.sprite_count)
      return NULL;
   return &ray_map_cache.sprites[index];
}

void RvR_ray_map_reset_full()
{
   RvR_ray_map_reset();
}

void RvR_ray_map_load_path(const char *path)
{
   int size = 0;
   uint8_t endian;
   uint8_t *mem = RvR_decompress_path(path,&size,&endian);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw,mem,size);
   RvR_rw_endian(&rw,endian);
   RvR_ray_map_load_rw(&rw);
   RvR_free(mem);
}

void RvR_ray_map_load(uint16_t id)
{
   char tmp[64];
   sprintf(tmp,"MAP%05d",id);

   unsigned size_in;
   int32_t size_out;
   uint8_t *mem_pak, *mem_decomp;
   uint8_t endian;
   mem_pak = RvR_lump_get(tmp,&size_in);
   mem_decomp = RvR_mem_decompress(mem_pak,size_in,&size_out,&endian);

   RvR_free(mem_pak);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw,mem_decomp,size_out);
   RvR_rw_endian(&rw,endian);
   RvR_ray_map_load_rw(&rw);
   RvR_rw_close(&rw);

   RvR_free(mem_decomp);
}

void RvR_ray_map_load_rw(RvR_rw *rw)
{
   //Read level width and height
   ray_map_cache.width = RvR_rw_read_u16(rw);
   ray_map_cache.height = RvR_rw_read_u16(rw);

   //Read sprite count
   ray_map_cache.sprite_count = RvR_rw_read_u32(rw);
   ray_map_cache.sprites = RvR_malloc(sizeof(*ray_map_cache.sprites)*ray_map_cache.sprite_count);

   //Read floor color
   ray_map_cache.floor_color = RvR_rw_read_u8(rw);

   //Read sky texture
   ray_map_cache.sky_tex = RvR_rw_read_u16(rw);

   //Read texture, floor and ceiling
   int32_t tile_count = ray_map_cache.width*ray_map_cache.height;
   ray_map_cache.wall_ftex = RvR_malloc(tile_count*sizeof(*ray_map_cache.wall_ftex));
   ray_map_cache.wall_ctex = RvR_malloc(tile_count*sizeof(*ray_map_cache.wall_ctex));
   ray_map_cache.floor_tex = RvR_malloc(tile_count*sizeof(*ray_map_cache.floor_tex));
   ray_map_cache.ceil_tex = RvR_malloc(tile_count*sizeof(*ray_map_cache.ceil_tex));
   ray_map_cache.floor = RvR_malloc(tile_count*sizeof(*ray_map_cache.floor));
   ray_map_cache.ceiling = RvR_malloc(tile_count*sizeof(*ray_map_cache.ceiling));

   for(int32_t i = 0;i<tile_count;i++) ray_map_cache.wall_ftex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) ray_map_cache.wall_ctex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) ray_map_cache.floor_tex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) ray_map_cache.ceil_tex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) ray_map_cache.floor[i] = RvR_rw_read_i8(rw);
   for(int32_t i = 0;i<tile_count;i++) ray_map_cache.ceiling[i] = RvR_rw_read_i8(rw);

   //Read sprites
   for(unsigned i = 0;i<ray_map_cache.sprite_count;i++)
   {
      ray_map_cache.sprites[i].type = RvR_rw_read_u16(rw);
      ray_map_cache.sprites[i].pos.x = RvR_rw_read_i32(rw);
      ray_map_cache.sprites[i].pos.y = RvR_rw_read_i32(rw);
      ray_map_cache.sprites[i].pos.z = RvR_rw_read_i32(rw);
      ray_map_cache.sprites[i].direction = RvR_rw_read_i32(rw);
      ray_map_cache.sprites[i].extra0 = RvR_rw_read_i32(rw);
      ray_map_cache.sprites[i].extra1 = RvR_rw_read_i32(rw);
      ray_map_cache.sprites[i].extra2 = RvR_rw_read_i32(rw);
      ray_map_cache.sprites[i].extra3 = RvR_rw_read_i32(rw);
   }

   //Update actual map
   RvR_ray_map_reset_full();
}

void RvR_ray_map_save(const char *path)
{
   //Calculate needed memory
   int size = 0;
   size+=2; //ray_map.width
   size+=2; //ray_map.height
   size+=1; //ray_map.floor_color
   size+=2; //ray_map.sky_tex
   size+=ray_map_cache.width*ray_map_cache.height*2; //ray_map.wall_ftex
   size+=ray_map_cache.width*ray_map_cache.height*2; //ray_map.wall_ctex
   size+=ray_map_cache.width*ray_map_cache.height*2; //ray_map.floor_tex
   size+=ray_map_cache.width*ray_map_cache.height*2; //ray_map.ceil_tex
   size+=ray_map_cache.width*ray_map_cache.height; //ray_map.floor
   size+=ray_map_cache.width*ray_map_cache.height; //ray_map.ceiling

   uint8_t *mem = RvR_malloc(size);
   int pos = 0;

   //Write width and height
   *(uint16_t *)(mem+pos) = ray_map_cache.width; pos+=2;
   *(uint16_t *)(mem+pos) = ray_map_cache.height; pos+=2;

   //Write sprite count
   *(uint32_t *)(mem+pos) = ray_map_cache.sprite_count; pos+=4;

   //Write floor color
   *(uint8_t *)(mem+pos) = ray_map_cache.floor_color; pos+=1;

   //Write sky texture
   *(uint16_t *)(mem+pos) = ray_map_cache.sky_tex; pos+=1;

   //Write texture, floor and ceiling
   memcpy(mem+pos,ray_map_cache.wall_ftex,ray_map_cache.width*ray_map_cache.height*2); pos+=ray_map_cache.width*ray_map_cache.height*2;
   memcpy(mem+pos,ray_map_cache.wall_ctex,ray_map_cache.width*ray_map_cache.height*2); pos+=ray_map_cache.width*ray_map_cache.height*2;
   memcpy(mem+pos,ray_map_cache.floor_tex,ray_map_cache.width*ray_map_cache.height*2); pos+=ray_map_cache.width*ray_map_cache.height*2;
   memcpy(mem+pos,ray_map_cache.ceil_tex,ray_map_cache.width*ray_map_cache.height*2); pos+=ray_map_cache.width*ray_map_cache.height*2;
   memcpy(mem+pos,ray_map_cache.floor,ray_map_cache.width*ray_map_cache.height); pos+=ray_map_cache.width*ray_map_cache.height;
   memcpy(mem+pos,ray_map_cache.ceiling,ray_map_cache.width*ray_map_cache.height); pos+=ray_map_cache.width*ray_map_cache.height;

   //Write sprites
   for(unsigned i = 0;i<ray_map_cache.sprite_count;i++)
   {
      *(uint16_t *)(mem+pos) = ray_map_cache.sprites[i].type; pos+=2;
      *(int32_t *)(mem+pos) = ray_map_cache.sprites[i].pos.x; pos+=4;
      *(int32_t *)(mem+pos) = ray_map_cache.sprites[i].pos.y; pos+=4;
      *(int32_t *)(mem+pos) = ray_map_cache.sprites[i].pos.z; pos+=4;
      *(int32_t *)(mem+pos) = ray_map_cache.sprites[i].extra0; pos+=4;
      *(int32_t *)(mem+pos) = ray_map_cache.sprites[i].extra1; pos+=4;
      *(int32_t *)(mem+pos) = ray_map_cache.sprites[i].extra2; pos+=4;
      *(int32_t *)(mem+pos) = ray_map_cache.sprites[i].extra3; pos+=4;
   }

   //Compress and write to disk
   FILE *f = fopen(path,"wb");
   RvR_mem_compress(mem,size,f);
   fclose(f);

   //Free temp buffer
   RvR_free(mem);
}

int RvR_ray_map_inbounds(int16_t x, int16_t y)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      return 1;
   return 0;
}

uint16_t RvR_ray_map_sky_tex()
{
   return ray_map.sky_tex;
}

uint16_t RvR_ray_map_wall_ftex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      return ray_map.wall_ftex[y*ray_map.width+x]; 

   return 0;
}

uint16_t RvR_ray_map_wall_ctex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      return ray_map.wall_ctex[y*ray_map.width+x]; 

   return 0;
}

uint16_t RvR_ray_map_floor_tex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      return ray_map.floor_tex[y*ray_map.width+x]; 

   return 0;
}

uint16_t RvR_ray_map_ceil_tex_at(int16_t x, int16_t y)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      return ray_map.ceil_tex[y*ray_map.width+x]; 

   return ray_map.sky_tex;
}

RvR_fix22 RvR_ray_map_floor_height_at(int16_t x, int16_t y)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      return ray_map.floor[y*ray_map.width+x];

   return 0;
}

RvR_fix22 RvR_ray_map_ceiling_height_at(int16_t x, int16_t y)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      return ray_map.ceiling[y*ray_map.width+x]; 

   return 127*128;
}

void RvR_ray_map_wall_ftex_set(int16_t x, int16_t y, uint16_t tex)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      ray_map.wall_ftex[y*ray_map.width+x] = tex;
}

void RvR_ray_map_wall_ctex_set(int16_t x, int16_t y, uint16_t tex)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      ray_map.wall_ctex[y*ray_map.width+x] = tex;
}

void RvR_ray_map_floor_tex_set(int16_t x, int16_t y, uint16_t tex)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      ray_map.floor_tex[y*ray_map.width+x] = tex;
}

void RvR_ray_map_ceil_tex_set(int16_t x, int16_t y, uint16_t tex)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      ray_map.ceil_tex[y*ray_map.width+x] = tex;
}

void RvR_ray_map_floor_height_set(int16_t x, int16_t y, RvR_fix22 height)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      ray_map.floor[y*ray_map.width+x] = height;
}

void RvR_ray_map_ceiling_height_set(int16_t x, int16_t y, RvR_fix22 height)
{
   if(x>=0&&x<ray_map.width&&y>=0&&y<ray_map.height)
      ray_map.ceiling[y*ray_map.width+x] = height;
}

uint16_t RvR_ray_map_wall_ftex_at_us(int16_t x, int16_t y)
{
   return ray_map.wall_ftex[y*ray_map.width+x]; 
}

uint16_t RvR_ray_map_wall_ctex_at_us(int16_t x, int16_t y)
{
   return ray_map.wall_ctex[y*ray_map.width+x]; 
}

uint16_t RvR_ray_map_floor_tex_at_us(int16_t x, int16_t y)
{
   return ray_map.floor_tex[y*ray_map.width+x]; 
}

uint16_t RvR_ray_map_ceil_tex_at_us(int16_t x, int16_t y)
{
   return ray_map.ceil_tex[y*ray_map.width+x]; 
}

RvR_fix22 RvR_ray_map_floor_height_at_us(int16_t x, int16_t y)
{
   return ray_map.floor[y*ray_map.width+x];
}

RvR_fix22 RvR_ray_map_ceiling_height_at_us(int16_t x, int16_t y)
{
   return ray_map.ceiling[(y)*ray_map.width+x]; 
}

RvR_ray_map *RvR_ray_map_get()
{
   return &ray_map;
}

RvR_ray_map_cache *RvR_ray_map_cache_get()
{
   return &ray_map_cache;
}
//-------------------------------------
