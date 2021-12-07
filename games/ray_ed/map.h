/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _MAP_H_

#define _MAP_H_

typedef struct
{
   char (*data)[64];
   unsigned data_used;
   unsigned data_size;
}Map_list;

extern RvR_ray_map *map;

void map_load(uint16_t id);
void map_new(uint16_t width, uint16_t height);
void map_save();
void map_set_name(const char *name);
int map_tile_comp(uint16_t ftex, uint16_t ctex, RvR_fix22 fheight, RvR_fix22 cheight, int x, int y);
void map_path_add(const char *path);

#endif
