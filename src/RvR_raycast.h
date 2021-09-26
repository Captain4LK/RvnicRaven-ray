/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_RAYCAST_H_

#define _RVR_RAYCAST_H_

typedef struct
{
   RvR_vec2 start;
   RvR_vec2 direction;
}RvR_ray;

typedef struct
{
   RvR_vec2 position;
   RvR_vec2 square;
   RvR_fix22 distance;
   uint8_t direction;
   uint16_t wall_tex;
   uint16_t floor_tex;
   uint16_t ceil_tex;
   RvR_fix22 fheight;
   RvR_fix22 cheight;
   RvR_fix22 texture_coord;
}RvR_hit_result;

typedef void (*RvR_column_function) (RvR_hit_result *hits, uint16_t hit_count, uint16_t x, RvR_ray ray);

void RvR_cast_ray_multi_hit(RvR_ray ray, RvR_hit_result *hit_results, uint16_t *hit_results_len);
void RvR_cast_rays_multi_hit(RvR_column_function column);
RvR_fix22 RvR_raycast_perspective_scale_vertical(RvR_fix22 org_size, RvR_fix22 distance);
RvR_fix22 RvR_raycast_perspective_scale_vertical_inverse(RvR_fix22 org_size, RvR_fix22 sc_size);
RvR_fix22 RvR_raycast_perspective_scale_horizontal(RvR_fix22 org_size, RvR_fix22 distance);
void      RvR_raycast_move_with_collision(RvR_vec3 offset, int8_t compute_height, int8_t force);

void      RvR_raycast_set_angle(RvR_fix22 angle);
RvR_fix22 RvR_raycast_get_angle();
void      RvR_raycast_set_shear(int16_t shear);
int16_t  RvR_raycast_get_shear();
void      RvR_raycast_set_position(RvR_vec3 position);
RvR_vec3  RvR_raycast_get_position();

#endif
