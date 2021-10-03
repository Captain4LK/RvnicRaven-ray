/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdlib.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "RvR_core.h"
#include "RvR_math.h"
#include "RvR_texture.h"
#include "sprite.h"
#include "ai.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Function prototypes
//-------------------------------------

//Variables
uint16_t sprite_table[SPRITE_MAX][8] =
{
   {0x5300}, //SPRITE_TREE0
   {0x5301}, //SPRITE_TREE1
   {0x5302}, //SPRITE_TREE2
   {0x5303}, //SPRITE_ROCK0
   {0x5304}, //SPRITE_ROCK1
   {0x5305}, //SPRITE_CORPSE0
   {0x5306}, //SPRITE_CORPSE1
   {0x5307}, //SPRITE_CORPSE2
   {0x5308}, //SPRITE_CORPSE3
   {0x5309}, //SPRITE_CORPSE4
   {0x530A}, //SPRITE_CORPSE5
   {0x530B}, //SPRITE_CORPSE6
   {0x530C}, //SPRITE_CORPSE7
   {0x530D}, //SPRITE_GARGOYLE
   {0x5700}, //SPRITE_SHOTGUN_READY
   {0x5701}, //SPRITE_SHOTGUN_LOAD0
   {0x5702}, //SPRITE_SHOTGUN_LOAD1
   {0x5703}, //SPRITE_SHOTGUN_LOAD2
   {0x5704}, //SPRITE_SHOTGUN_LOAD3
   {0x5705}, //SPRITE_SHOTGUN_LOAD4
   {0x5706}, //SPRITE_SHOTGUN_LOAD5
   {0x5707}, //SPRITE_SHOTGUN_LOAD6
   {0x5708}, //SPRITE_SHOTGUN_LOAD7
   {0x5709}, //SPRITE_SHOTGUN_LOAD8
};
//-------------------------------------

//Function implementations

uint16_t sprite_rot(Sprite sp, uint8_t rot)
{
   return sprite_table[sp][rot&7];
}

void sprite_load_sprite(Sprite sp)
{
   for(int i = 0;i<8;i++)
   {
      RvR_texture_load(sprite_rot(sp,i));
   }
}
//-------------------------------------
