/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
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
   {21264}, //SPRITE_TREE
   {21258}, //SPRITE_LAMP
   {21261}, //SPRITE_RUIN
   {21263}, //SPRITE_TERMINAL
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
