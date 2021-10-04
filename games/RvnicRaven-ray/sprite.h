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
   SPRITE_TREE0,
   SPRITE_TREE1,
   SPRITE_TREE2,
   SPRITE_ROCK0,
   SPRITE_ROCK1,
   SPRITE_CORPSE0,
   SPRITE_CORPSE1,
   SPRITE_CORPSE2,
   SPRITE_CORPSE3,
   SPRITE_CORPSE4,
   SPRITE_CORPSE5,
   SPRITE_CORPSE6,
   SPRITE_CORPSE7,
   SPRITE_GARGOYLE,
   SPRITE_SHOTGUN_READY,
   SPRITE_SHOTGUN_LOAD0,
   SPRITE_SHOTGUN_LOAD1,
   SPRITE_SHOTGUN_LOAD2,
   SPRITE_SHOTGUN_LOAD3,
   SPRITE_SHOTGUN_LOAD4,
   SPRITE_SHOTGUN_LOAD5,
   SPRITE_SHOTGUN_LOAD6,
   SPRITE_SHOTGUN_LOAD7,
   SPRITE_SHOTGUN_LOAD8,
   SPRITE_MAX,
}Sprite;

uint16_t sprite_rot(Sprite sp, uint8_t rot);
void sprite_load_sprite(Sprite sp);

#endif
