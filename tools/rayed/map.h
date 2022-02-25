/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _MAP_H_

#define _MAP_H_

typedef struct
{
   char (*data)[128];
   unsigned data_used;
   unsigned data_size;
}Map_list;

typedef struct Map_sprite
{
   uint16_t type;
   RvR_vec3 pos;
   RvR_fix22 direction;
   int32_t extra0;
   int32_t extra1;
   int32_t extra2;
   int32_t extra3;

   struct Map_sprite *next;
   struct Map_sprite **prev_next;
}Map_sprite;

extern RvR_ray_map *map;
extern Map_sprite *map_sprites;

void map_load(const char *path);
void map_new(uint16_t width, uint16_t height);
void map_save();
void map_set_path(const char *path);
const char *map_path_get();
int map_tile_comp(uint16_t ftex, uint16_t ctex, RvR_fix22 fheight, RvR_fix22 cheight, int x, int y);

void map_sky_tex_set(uint16_t tex);

void map_path_add(const char *path);
Map_list *map_list_get();

Map_sprite *map_sprite_new();
void map_sprite_add(Map_sprite *sp);
void map_sprite_free(Map_sprite *sp);

#endif
