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
   SPRITE_ITEM_KEY,
   SPRITE_ITEM_BULLET,
   SPRITE_MAX,
}Sprite;

uint16_t sprite_rot(Sprite sp, uint8_t rot);
void sprite_load_sprite(Sprite sp);

#endif
