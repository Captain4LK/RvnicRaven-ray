/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _SPRITE_H_

#define _SPRITE_H_

typedef enum
{
   SPRITE_TREE,
   SPRITE_LAMP,
   SPRITE_RUIN,
   SPRITE_TERMINAL,
   SPRITE_BARREL,
   SPRITE_ITEM_KEY,
   SPRITE_ITEM_BULLET,
   SPRITE_ITEM_ROCKET,
   SPRITE_ITEM_CELL,
   SPRITE_ITEM_HEALTH,
   SPRITE_KNIFE,
   SPRITE_MACHINE_GUN,
   SPRITE_SHOTGUN,
   SPRITE_ROCKET_LAUNCHER,
   SPRITE_PLASMA_GUN,
   SPRITE_SOLUTION,
   SPRITE_MAX,
}Sprite;

int32_t sprite_rot(Sprite sp, uint8_t rot);

#endif
