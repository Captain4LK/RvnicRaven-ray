/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "../../src/RvnicRaven.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define READ_ARG(I) \
   ((++(I))<argc?argv[(I)]:NULL)

#define MIN(a,b) ((a)<(b)?(a):(b))
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void print_help(char **argv);

static void version_old(RvR_rw *rw);
static void version_0(RvR_rw *rw);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   const char *path_out = NULL;
   const char *path_in = NULL;
   int old = 0;

   for(int i = 1;i<argc;i++)
   {
      if(strcmp(argv[i],"--help")==0||
         strcmp(argv[i],"-help")==0||
         strcmp(argv[i],"-h")==0||
         strcmp(argv[i],"?")==0)
      { print_help(argv); return 0; }
      else if(strcmp(argv[i],"-fout")==0)
         path_out = READ_ARG(i);
      else if(strcmp(argv[i],"-fin")==0)
         path_in = READ_ARG(i);
      else if(strcmp(argv[i],"-old")==0)
         old = 1;
   }
   
   if(path_in==NULL)
   {
      RvR_log("input map path not specified, try %s -h for more info\n",argv[0]);
      return 0;
   }
   if(path_out==NULL)
   {
      RvR_log("output map path not specified, try %s -h for more info\n",argv[0]);
      return 0;
   }

   RvR_rw rw_file = {0};
   RvR_rw_init_path(&rw_file,path_in,"rb");
   int size_out = 0;
   uint8_t endian = 0;
   uint8_t *mem_map = NULL;
   mem_map = RvR_decompress(&rw_file,&size_out,&endian);

   RvR_rw rw_map = {0};
   RvR_rw_init_const_mem(&rw_map,mem_map,size_out);
   RvR_rw_endian(&rw_map,endian);
   RvR_rw_close(&rw_file);

   if(old)
   {
      version_old(&rw_map);
      RvR_rw_close(&rw_map);
      RvR_free(mem_map);
      RvR_ray_map_save(path_out);

      return 0;
   }

   //Read version
   uint16_t version = RvR_rw_read_u16(&rw_map);
   switch(version)
   {
   case 0: version_0(&rw_map); break;
   default: RvR_log("Invalid version\n"); break;
   }
   
   RvR_rw_close(&rw_map);
   RvR_free(mem_map);

   RvR_ray_map_save(path_out);

   return 0;
}

static void print_help(char **argv)
{
   RvR_log("%s usage:\n"
          "%s -fin filename -fout filename ]\n"
          "   -fin        input map path\n"
          "   -fout       output map path\n"
          "   -old        flag input as old format\n",
         argv[0],argv[0]);
}

static void version_old(RvR_rw *rw)
{
   //dimensions
   uint16_t width = RvR_rw_read_u16(rw);
   uint16_t height = RvR_rw_read_u16(rw);

   RvR_ray_map_create(width,height);
   RvR_ray_map *map = RvR_ray_map_get();

   //sprite count
   map->sprite_count = RvR_rw_read_u32(rw);
   map->sprites = RvR_malloc(sizeof(*map->sprites)*map->sprite_count);

   //floor color
   RvR_rw_read_u8(rw);

   //sky texture
   map->sky_tex = RvR_rw_read_u16(rw);

   //texture, floor and ceiling
   int32_t tile_count = map->width*map->height;
   map->wall_ftex = RvR_malloc(tile_count*sizeof(*map->wall_ftex));
   map->wall_ctex = RvR_malloc(tile_count*sizeof(*map->wall_ctex));
   map->floor_tex = RvR_malloc(tile_count*sizeof(*map->floor_tex));
   map->ceil_tex = RvR_malloc(tile_count*sizeof(*map->ceil_tex));
   map->floor = RvR_malloc(tile_count*sizeof(*map->floor));
   map->ceiling = RvR_malloc(tile_count*sizeof(*map->ceiling));

   for(int32_t i = 0;i<tile_count;i++) map->wall_ftex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) map->wall_ctex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) map->floor_tex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) map->ceil_tex[i] = RvR_rw_read_u16(rw);
   for(int32_t i = 0;i<tile_count;i++) map->floor[i] = RvR_rw_read_i8(rw)*128;
   for(int32_t i = 0;i<tile_count;i++) map->ceiling[i] = RvR_rw_read_i8(rw)*128;

   //sprites
   for(unsigned i = 0;i<map->sprite_count;i++)
   {
      map->sprites[i].texture = RvR_rw_read_u16(rw);
      map->sprites[i].pos.x = RvR_rw_read_i32(rw);
      map->sprites[i].pos.y = RvR_rw_read_i32(rw);
      map->sprites[i].pos.z = RvR_rw_read_i32(rw);
      map->sprites[i].direction = RvR_rw_read_i32(rw);
      map->sprites[i].extra0 = RvR_rw_read_i32(rw);
      map->sprites[i].extra1 = RvR_rw_read_i32(rw);
      map->sprites[i].extra2 = RvR_rw_read_i32(rw);
      RvR_rw_read_i32(rw);
      map->sprites[i].flags = 0;
   }
}

static void version_0(RvR_rw *rw)
{
   //TODO: add once new map format is added
}
//-------------------------------------
