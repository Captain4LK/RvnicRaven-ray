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
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void map_load(uint16_t id)
{
   RvR_ray_map_load(id);
   
   map = RvR_ray_map_get();
   printf("Map dimensions: %ux%u\n",map->width,map->height);

   camera.pos.x = map->width*512+512;
   camera.pos.y = map->height*512+512;
   camera.pos.z = INT16_MIN;
}

void map_new(uint16_t width, uint16_t height)
{
   RvR_ray_map_create(width,height);
   map = RvR_ray_map_get();
   printf("Map dimensions: %ux%u\n",map->width,map->height);

   camera.pos.x = map->width*512+512;
   camera.pos.y = map->height*512+512;
   camera.pos.z = INT16_MIN;
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
