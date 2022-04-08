/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "config.h"
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
uint16_t texture_sky = 0;
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
   texture_sky = 0;

   for(unsigned i = 0;i<UINT16_MAX;i++)
   {
      char tmp[16];
      snprintf(tmp,16,"TEX%05d",i);
      if(!RvR_lump_exists(tmp))
         continue;
      //puts(tmp);

      //RvR_texture_load(i);
      //RvR_texture_lock(i);
      RvR_texture *tex = RvR_texture_get(i);
      //printf("%d %d\n",tex->width,tex->height);

      if(tex->width==1<<8&&tex->height==1<<7)
         texture_sky = i;

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
   for(unsigned i = 0;i<TEXTURE_MRU_SIZE;i++)
   {
      //Place back to front
      if(texture_list_used.data[i]==tex)
      {  
         if(i!=texture_list_used.data_last)
         {
            unsigned index = texture_list_used_wrap(i);
            while(index!=texture_list_used.data_last)
            {
               texture_list_used.data[texture_list_used_wrap(index)] = texture_list_used.data[texture_list_used_wrap(index+1)];
               index = texture_list_used_wrap(index+1);
            }
            texture_list_used.data[texture_list_used.data_last] = tex;
         }

         return;
      }
   }

   texture_list_used.data_last = texture_list_used_wrap(texture_list_used.data_last+1);
   texture_list_used.data[texture_list_used.data_last] = tex;
}

int texture_valid(uint16_t tex)
{
   RvR_texture *texture = RvR_texture_get(tex);
   if(texture==NULL)
      return 0;

   if(texture->width==1<<RvR_log2(texture->width)&&texture->height==1<<RvR_log2(texture->height))
      return 1;

   return 0;
}
//-------------------------------------
