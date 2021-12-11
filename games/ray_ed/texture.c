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
#include "texture.h"
#include "map.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Texture_list texture_list = {0};
Texture_list_used texture_list_used = {0};
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void texture_list_create()
{
   if(texture_list.data==NULL)
   {
      texture_list.data_size = 16;
      texture_list.data = RvR_malloc(sizeof(*texture_list.data)*texture_list.data_size);
   }
   texture_list.data_used = 0;

   for(unsigned i = 0;i<UINT16_MAX;i++)
   {
      char tmp[16];
      snprintf(tmp,16,"TEX%05d",i);
      if(!RvR_lump_exists(tmp))
         continue;

      RvR_texture_load(i);
      RvR_texture_lock(i);
      texture_list.data[texture_list.data_used++] = i;
      if(texture_list.data_used==texture_list.data_size)
      {
         texture_list.data_size+=16;
         texture_list.data = RvR_realloc(texture_list.data,sizeof(*texture_list.data)*texture_list.data_size);
      }
   }
}

void texture_list_used_create()
{
   for(unsigned i = 0;i<map->width*map->height;i++)
   {
      texture_list_used_add(map->floor_tex[i]);
      texture_list_used_add(map->ceil_tex[i]);
      texture_list_used_add(map->wall_ftex[i]);
      texture_list_used_add(map->wall_ctex[i]);
   }
}

void texture_list_used_add(uint16_t tex)
{
   if(texture_list_used.data==NULL)
   {
      texture_list_used.data_used = 0;
      texture_list_used.data_size = 16;
      texture_list_used.data = RvR_malloc(sizeof(*texture_list_used.data)*texture_list_used.data_size);
   }

   for(unsigned i = 0;i<texture_list_used.data_used;i++)
   {
      if(texture_list_used.data[i].tex==tex)
      {
         texture_list_used.data[i].count++;
         return;
      }
   }

   texture_list_used.data[texture_list_used.data_used].tex = tex;
   texture_list_used.data[texture_list_used.data_used].count = 1;
   texture_list_used.data_used++;

   if(texture_list_used.data_used==texture_list_used.data_size)
   {
      texture_list_used.data_size+=16;
      texture_list_used.data = RvR_realloc(texture_list_used.data,sizeof(*texture_list_used.data)*texture_list_used.data_size);
   }
}
//-------------------------------------
