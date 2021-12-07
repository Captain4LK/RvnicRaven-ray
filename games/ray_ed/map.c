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
#include <string.h>

#include "../../external/cute_path.h"
#include "../../external/cute_files.h"
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "map.h"
#include "editor.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
RvR_ray_map *map = NULL;

static struct
{
   char (*data)[64];
   unsigned data_used;
   unsigned data_size;
}path_list = {0};
static Map_list map_list = {0};
static char map_name[9];
//-------------------------------------

//Function prototypes
static void map_list_add(const char *path);
//-------------------------------------

//Function implementations

void map_load(uint16_t id)
{
   snprintf(map_name,9,"MAP%05d",id);
   RvR_ray_map_load(id);
   
   map = RvR_ray_map_get();
   printf("Map dimensions: %ux%u\n",map->width,map->height);

   camera.pos.x = map->width*512+512;
   camera.pos.y = map->height*512+512;
   camera.pos.z = INT16_MIN;
}

void map_new(uint16_t width, uint16_t height)
{
   snprintf(map_name,9,"MAP%05d",UINT16_MAX);
   RvR_ray_map_create(width,height);
   map = RvR_ray_map_get();
   printf("Map dimensions: %ux%u\n",map->width,map->height);

   camera.pos.x = map->width*512+512;
   camera.pos.y = map->height*512+512;
   camera.pos.z = INT16_MIN;
}

void map_save()
{
   const char *path = RvR_lump_get_path("PAL00000");
   char path_new[512] = {0};
   path_pop(path,path_new,NULL);

   strcat(path_new,"/");
   strcat(path_new,map_name);
   strcat(path_new,".rvr");
   puts(path_new);

   RvR_ray_map_cache *map_cache = RvR_ray_map_cache_get();
   map_cache->floor_color = map->floor_color;
   map_cache->sky_tex = map->sky_tex;
   memcpy(map_cache->wall_ftex,map->wall_ftex,sizeof(*map->wall_ftex)*map->width*map->height);
   memcpy(map_cache->wall_ctex,map->wall_ctex,sizeof(*map->wall_ctex)*map->width*map->height);
   memcpy(map_cache->floor_tex,map->floor_tex,sizeof(*map->floor_tex)*map->width*map->height);
   memcpy(map_cache->ceil_tex,map->ceil_tex,sizeof(*map->ceil_tex)*map->width*map->height);
   for(int i = 0;i<map->width*map->height;i++)
   {
      map_cache->floor[i] = map->floor[i]/128;
      map_cache->ceiling[i] = map->ceiling[i]/128;
   }

   RvR_ray_map_save(path_new);
}

void map_set_name(const char *name)
{
   strncpy(map_name,name,8);
   map_name[8] = '\0';
}

void map_path_add(const char *path)
{
   if(path_list.data==NULL)
   {
      path_list.data_used = 0;
      path_list.data_size = 1;
      path_list.data = RvR_malloc(sizeof(*path_list.data)*path_list.data_size);
   }

   for(int i = 0;i<path_list.data_used;i++)
      if(strcmp(path,path_list.data[i])==0)
         return;

   strncpy(path_list.data[path_list.data_used],path,64);
   path_list.data_used++;

   if(path_list.data_used>=path_list.data_size)
   {
      path_list.data_size+=1;
      path_list.data = RvR_realloc(path_list.data,sizeof(*path_list.data)*path_list.data_size);
   }
}

static void map_list_add(const char *path)
{
   if(map_list.data==NULL)
   {
      map_list.data_used = 0;
      map_list.data_size = 1;
      map_list.data = RvR_malloc(sizeof(*map_list.data)*map_list.data_size);
   }

   for(int i = 0;i<map_list.data_used;i++)
      if(strcmp(path,map_list.data[i])==0)
         return;

   strncpy(map_list.data[map_list.data_used],path,64);
   map_list.data_used++;

   if(map_list.data_used>=map_list.data_size)
   {
      map_list.data_size+=1;
      map_list.data = RvR_realloc(map_list.data,sizeof(*map_list.data)*map_list.data_size);
   }
}

int map_tile_comp(uint16_t ftex, uint16_t ctex, RvR_fix22 fheight, RvR_fix22 cheight, int x, int y)
{
   if(ftex!=RvR_ray_map_floor_tex_at(x,y))
      return 0;

   if(ctex!=RvR_ray_map_ceil_tex_at(x,y))
      return 0;

   if(fheight!=RvR_ray_map_floor_height_at(x,y))
      return 0;

   if(cheight!=RvR_ray_map_ceiling_height_at(x,y))
      return 0;

   return 1;
}
//-------------------------------------
