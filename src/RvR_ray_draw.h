/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_RAYCAST_DRAW_H_

#define _RVR_RAYCAST_DRAW_H_

void RvR_ray_draw_sprite(RvR_vec3 pos, uint16_t tex);
void RvR_ray_draw(RvR_vec3 cpos, RvR_fix22 cangle, int16_t cshear);

#endif
