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
//-------------------------------------

//Variables
RvR_ray_map ray_map = {0};
//-------------------------------------

//Function prototypes
static void ray_map_free();
//-------------------------------------

//Function implementations

void RvR_ray_map_create(uint16_t width, uint16_t height)
{
   if(width==0)
      width = 1;
   if(height==0)
      height = 1;

   ray_map_free();

   ray_map.width = width;
   ray_map.height = height;
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

   ray_map.sky_tex = 0;
   ray_map.sprite_count = 0;

   for(int y = 0;y<ray_map.height;y++)
   {
      for(int x = 0;x<ray_map.width;x++)
      {
         ray_map.floor[y*ray_map.width+x] = 0;
         ray_map.ceiling[y*ray_map.width+x] = 32*128;
      }
   }
}

static void ray_map_free()
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

int RvR_ray_map_sprite_count()
{
   return ray_map.sprite_count;
}

RvR_ray_map_sprite *RvR_ray_map_sprite_get(unsigned index)
{
   if(index>=ray_map.sprite_count)
      return NULL;
   return &ray_map.sprites[index];
}

void RvR_ray_map_load_path(const char *path)
{
   int32_t size = 0;
   uint8_t endian;
   uint8_t *mem = RvR_decompress_path(path,&size,&endian);

   RvR_rw rw;
   RvR_rw_init_const_mem(&rw,mem,size);
   RvR_rw_endian(&rw,endian);
   RvR_ray_map_load_rw(&rw);

   RvR_rw_close(&rw);
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
   //Read and check version
   uint16_t version = RvR_rw_read_u16(rw);
   if(version!=0)
   {
      RvR_log_line("RvR_ray_map_load_rw","Invalid version '%d', expected version '0'\n",version);
      return;
   }

   //Read sky texture
   ray_map.sky_tex = RvR_rw_read_u16(rw);

   //Read level width and height
   ray_map.width = RvR_rw_read_u16(rw);
   ray_map.height = RvR_rw_read_u16(rw);

   //Read sprite count
   ray_map.sprite_count = RvR_rw_read_u32(rw);
   ray_map.sprites = RvR_malloc(sizeof(*ray_map.sprites)*ray_map.sprite_count);

   //Read texture, floor and ceiling
   int32_t tile_count = ray_map.width*ray_map.height;
   ray_map.wall_ftex = RvR_malloc(tile_count*sizeof(*ray_map.wall_ftex));
   ray_map.wall_ctex = RvR_malloc(tile_count*sizeof(*ray_map.wall_ctex));
   ray_map.floor_tex = RvR_malloc(tile_count*sizeof(*ray_map.floor_tex));
   ray_map.ceil_tex = RvR_malloc(tile_count*sizeof(*ray_map.ceil_tex));
   ray_map.floor = RvR_malloc(tile_count*sizeof(*ray_map.floor));
   ray_map.ceiling = RvR_malloc(tile_count*sizeof(*ray_map.ceiling));

   for(int32_t i = 0;i<tile_count;i++) ray_map.floor[i] = RvR_rw_read_i32(rw);
   for(int32_t i = 0;i<tile_count;i++) ray_map.ceiling[i] = RvR_rw_read_i32(rw);
   for(int32_t i = 0;i<tile_count;i++) ray_map.floor_tex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) ray_map.ceil_tex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) ray_map.wall_ftex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) ray_map.wall_ctex[i] = RvR_rw_read_u16(rw);

   //Read sprites
   for(unsigned i = 0;i<ray_map.sprite_count;i++)
   {
      ray_map.sprites[i].pos.x = RvR_rw_read_i32(rw);
      ray_map.sprites[i].pos.y = RvR_rw_read_i32(rw);
      ray_map.sprites[i].pos.z = RvR_rw_read_i32(rw);
      ray_map.sprites[i].direction = RvR_rw_read_i32(rw);
      ray_map.sprites[i].texture = RvR_rw_read_u16(rw);
      ray_map.sprites[i].flags = RvR_rw_read_u32(rw);
      ray_map.sprites[i].extra0 = RvR_rw_read_i32(rw);
      ray_map.sprites[i].extra1 = RvR_rw_read_i32(rw);
      ray_map.sprites[i].extra2 = RvR_rw_read_i32(rw);
   }
}

void RvR_ray_map_save(const char *path)
{
   //Calculate needed memory
   int size = 0;
   size+=2; //version
   size+=2; //ray_map.sky_tex
   size+=2; //ray_map.width
   size+=2; //ray_map.height
   size+=4; //ray_map.sprite_count
   size+=ray_map.width*ray_map.height*4; //ray_map.floor
   size+=ray_map.width*ray_map.height*4; //ray_map.ceiling
   size+=ray_map.width*ray_map.height*2; //ray_map.floor_tex
   size+=ray_map.width*ray_map.height*2; //ray_map.ceil_tex
   size+=ray_map.width*ray_map.height*2; //ray_map.wall_ftex
   size+=ray_map.width*ray_map.height*2; //ray_map.wall_ctex
   size+=ray_map.sprite_count*sizeof(*ray_map.sprites);

   uint8_t *mem = RvR_malloc(size);
   RvR_rw rw = {0};
   RvR_rw_init_mem(&rw,mem,size,size);

   //version
   RvR_rw_write_u16(&rw,0);

   //sky texture
   RvR_rw_write_u16(&rw,ray_map.sky_tex);

   //width and height
   RvR_rw_write_u16(&rw,ray_map.width);
   RvR_rw_write_u16(&rw,ray_map.height);

   //sprite count
   RvR_rw_write_u32(&rw,ray_map.sprite_count);

   //floor height
   for(int i = 0;i<ray_map.width*ray_map.height;i++)
      RvR_rw_write_i32(&rw,ray_map.floor[i]);

   //ceiling height
   for(int i = 0;i<ray_map.width*ray_map.height;i++)
      RvR_rw_write_i32(&rw,ray_map.ceiling[i]);

   //floor texture
   for(int i = 0;i<ray_map.width*ray_map.height;i++)
      RvR_rw_write_u16(&rw,ray_map.floor_tex[i]);

   //ceiling texture
   for(int i = 0;i<ray_map.width*ray_map.height;i++)
      RvR_rw_write_u16(&rw,ray_map.ceil_tex[i]);

   //wall ftex
   for(int i = 0;i<ray_map.width*ray_map.height;i++)
      RvR_rw_write_u16(&rw,ray_map.wall_ftex[i]);

   //wall ctex
   for(int i = 0;i<ray_map.width*ray_map.height;i++)
      RvR_rw_write_u16(&rw,ray_map.wall_ctex[i]);

   //sprites
   for(unsigned i = 0;i<ray_map.sprite_count;i++)
   {
      RvR_rw_write_i32(&rw,ray_map.sprites[i].pos.x);
      RvR_rw_write_i32(&rw,ray_map.sprites[i].pos.y);
      RvR_rw_write_i32(&rw,ray_map.sprites[i].pos.z);
      RvR_rw_write_i32(&rw,ray_map.sprites[i].direction);
      RvR_rw_write_u16(&rw,ray_map.sprites[i].texture);
      RvR_rw_write_u32(&rw,ray_map.sprites[i].flags);
      RvR_rw_write_i32(&rw,ray_map.sprites[i].extra0);
      RvR_rw_write_i32(&rw,ray_map.sprites[i].extra1);
      RvR_rw_write_i32(&rw,ray_map.sprites[i].extra2);
   }

   //Compress and write to disk
   FILE *f = fopen(path,"wb");
   RvR_mem_compress(mem,size,f);
   fclose(f);

   //Free temp buffer
   RvR_rw_close(&rw);
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

   return 127*128;
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
//-------------------------------------
