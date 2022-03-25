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
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include "../../external/cute_path.h"
#include "../../external/cute_files.h"
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "config.h"
#include "map.h"
#include "editor.h"
#include "texture.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
RvR_ray_map *map = NULL;
Map_sprite *map_sprites = NULL;

static struct
{
   char (*data)[128];
   unsigned data_used;
   unsigned data_size;
}path_list = {0};
static Map_list map_list = {0};
static char map_path[128];

static Map_sprite *map_sprite_pool = NULL;
//-------------------------------------

//Function prototypes
static void map_list_add(const char *path);
//-------------------------------------

//Function implementations

void map_load(const char *path)
{
   //Free old sprites
   while(map_sprites!=NULL)
      map_sprite_free(map_sprites);

   map_set_path(path);
   RvR_ray_map_load_path(map_path);
   
   map = RvR_ray_map_get();
   printf("Map dimensions: %ux%u\n",map->width,map->height);

   //Load sprites
   printf("%d Sprites\n",RvR_ray_map_sprite_count());
   for(int i = 0;i<RvR_ray_map_sprite_count();i++)
   {
      RvR_ray_map_sprite *s = RvR_ray_map_sprite_get(i);
      Map_sprite *ms = map_sprite_new();

      ms->texture = s->texture;
      ms->extra0 = s->extra0;
      ms->extra1 = s->extra1;
      ms->extra2 = s->extra2;
      ms->flags = s->flags;
      ms->direction = s->direction;
      ms->pos = s->pos;

      map_sprite_add(ms);
   }

   camera.pos.x = map->width*512+512;
   camera.pos.y = map->height*512+512;
   camera.pos.z = INT16_MIN;

   editor_undo_reset();
}

void map_new(uint16_t width, uint16_t height)
{
   //Free old sprites
   while(map_sprites!=NULL)
      map_sprite_free(map_sprites);

   snprintf(map_path,127,"map%"PRIu64".map",(uint64_t)time(NULL));
   RvR_ray_map_create(width,height);
   map = RvR_ray_map_get();
   //printf("%d\n",texture_sky);
   map->sky_tex = texture_sky;
   for(int i = 0;i<map->width*map->height;i++)
      map->ceil_tex[i] = texture_sky;
   printf("Map dimensions: %ux%u\n",map->width,map->height);

   camera.pos.x = map->width*512+512;
   camera.pos.y = map->height*512+512;
   camera.pos.z = INT16_MIN;

   editor_undo_reset();
}

void map_save()
{
   //Save sprites
   //count
   int sprite_count = 0;
   Map_sprite *s = map_sprites;
   while(s!=NULL)
   {
      s = s->next;
      sprite_count++;
   }
   map->sprite_count = sprite_count;
   map->sprites = RvR_realloc(map->sprites,sizeof(*map->sprites)*map->sprite_count);
   s = map_sprites;
   for(int i = 0;s!=NULL;i++)
   {
      map->sprites[i].texture = s->texture;
      map->sprites[i].pos = s->pos;
      map->sprites[i].direction = s->direction;
      map->sprites[i].extra0 = s->extra0;
      map->sprites[i].extra1 = s->extra1;
      map->sprites[i].extra2 = s->extra2;
      map->sprites[i].flags = s->flags;
      s = s->next;
   }

   puts(map_path);
   RvR_ray_map_save(map_path);
}

void map_set_path(const char *path)
{
   strncpy(map_path,path,128);
   map_path[127] = '\0';
}

const char *map_path_get()
{
   return map_path;
}

void map_path_add(const char *path)
{
   if(path_list.data==NULL)
   {
      path_list.data_used = 0;
      path_list.data_size = 1;
      path_list.data = RvR_malloc(sizeof(*path_list.data)*path_list.data_size);
   }

   for(unsigned i = 0;i<path_list.data_used;i++)
      if(strcmp(path,path_list.data[i])==0)
         return;

   strncpy(path_list.data[path_list.data_used],path,127);
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

   for(unsigned i = 0;i<map_list.data_used;i++)
      if(strcmp(path,map_list.data[i])==0)
         return;

   strncpy(map_list.data[map_list.data_used],path,127);
   map_list.data_used++;

   if(map_list.data_used>=map_list.data_size)
   {
      map_list.data_size+=1;
      map_list.data = RvR_realloc(map_list.data,sizeof(*map_list.data)*map_list.data_size);
   }
}

int map_tile_comp(uint16_t ftex, uint16_t ctex, RvR_fix22 fheight, RvR_fix22 cheight, int x, int y)
{
   if(!RvR_ray_map_inbounds(x,y))
      return 0;

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

void map_sky_tex_set(uint16_t tex)
{
   map->sky_tex = tex;
}

Map_list *map_list_get()
{
   char path[128];
   map_list.data_used = 0;

   for(unsigned i = 0;i<path_list.data_used;i++)
   {
      cf_dir_t dir = {0};
      //RvR_log_line("map_list_get ","opening directory %s\n",path_list.data[i]);
      cf_dir_open(&dir,path_list.data[i]);

      while(dir.has_next)
      {
         cf_file_t file;
         cf_read_file(&dir,&file);
         //RvR_log_line("map_list_get ","found file %s\n",file.path);
         if(cf_match_ext(&file,".map"))
         {
            snprintf(path,128,"%s%s",path_list.data[i],file.name);
            map_list_add(path);
         }

         cf_dir_next(&dir);
      }

      cf_dir_close(&dir);
   }

   return &map_list;
}

Map_sprite *map_sprite_new()
{
   if(map_sprite_pool==NULL)
   {
      Map_sprite *ms = RvR_malloc(sizeof(*ms)*256);
      memset(ms,0,sizeof(*ms)*256);

      for(int i = 0;i<255;i++)
         ms[i].next = &ms[i+1];
      map_sprite_pool = &ms[0];
   }

   Map_sprite *ms = map_sprite_pool;
   map_sprite_pool = ms->next;
   ms->next = NULL;
   ms->prev_next = NULL;

   return ms;
}

void map_sprite_add(Map_sprite *sp)
{
   sp->prev_next = &map_sprites;
   if(map_sprites!=NULL)
      map_sprites->prev_next = &sp->next;

   sp->next = map_sprites;
   map_sprites = sp;
}

void map_sprite_free(Map_sprite *sp)
{
   if(sp==NULL)
      return;

   *sp->prev_next = sp->next;
   if(sp->next!=NULL)
      sp->next->prev_next = sp->prev_next;

   sp->next = map_sprite_pool;
   map_sprite_pool = sp;
}
//-------------------------------------
