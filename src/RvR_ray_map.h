/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_MAP_H_

#define _RVR_MAP_H_

typedef struct
{
   uint16_t type;
   RvR_vec3 pos;
}RvR_ray_map_sprite;

void RvR_ray_map_create(uint16_t width, uint16_t height);
void RvR_ray_map_reset();
void RvR_ray_map_reset_full();
void RvR_ray_map_load_path(const char *path);
void RvR_ray_map_load(uint16_t id);
void RvR_ray_map_load_mem(uint8_t *mem, int len);
void RvR_ray_map_save(const char *path);
int RvR_ray_map_sprite_count();
RvR_ray_map_sprite *RvR_ray_map_sprite_get(unsigned index);

uint16_t RvR_ray_map_wall_tex_at(int16_t x, int16_t y);
uint16_t RvR_ray_map_floor_tex_at(int16_t x, int16_t y);
uint16_t RvR_ray_map_ceil_tex_at(int16_t x, int16_t y);
RvR_fix22 RvR_ray_map_floor_height_at(int16_t x, int16_t y);
RvR_fix22 RvR_ray_map_ceiling_height_at(int16_t x, int16_t y);

#endif
