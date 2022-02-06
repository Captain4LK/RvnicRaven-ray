/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _COLLISION_H_

#define _COLLISION_H_

#include "ai_type.h"

typedef struct Collider
{
   RvR_vec3 pos;
   RvR_fix22 height;
   RvR_fix22 radius;

   //bit 0: shootable
   uint32_t flags;
   AI_ent *ent;

   int removed;

   struct Collider *next;
   struct Collider **prev_next;
}Collider;

void collision_move(Collider *col_cur, RvR_vec3 offset, RvR_fix22 *floor_height, RvR_fix22 *ceiling_height, RvR_vec3 *moved, int on_ground);

Collider *collision_new();
void collision_add(Collider *c);
void collision_remove(Collider *c);
void collision_free(Collider *c);
void collision_clear();

#endif
