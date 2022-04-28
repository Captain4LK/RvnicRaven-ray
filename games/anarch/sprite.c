/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "collision.h"
#include "ai.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Function prototypes
//-------------------------------------

//Variables
int32_t sprite_table[SPRITE_MAX][8] =
{
   {1040,1040,1040,1040,1040,1040,1040,1040}, //SPRITE_TREE
   {1034,1034,1034,1034,1034,1034,1034,1034}, //SPRITE_LAMP
   {1037,1037,1037,1037,1037,1037,1037,1037}, //SPRITE_RUIN
   {1039,1039,1039,1039,1039,1039,1039,1039}, //SPRITE_TERMINAL
   {1028,1028,1028,1028,1028,1028,1028,1028}, //SPRITE_BARREL
   {1030,1030,1030,1030,1030,1030,1030,1030}, //SPRITE_ITEM_KEY
   {1029,1029,1029,1029,1029,1029,1029,1029}, //SPRITE_ITEM_BULLET
   {1036,1036,1036,1036,1036,1036,1036,1036}, //SPRITE_ITEM_ROCKET
   {1035,1035,1035,1035,1035,1035,1035,1035}, //SPRITE_ITEM_CELL
   {1033,1033,1033,1033,1033,1033,1033,1033}, //SPRITE_ITEM_HEALTH
   {1061,1061,1061,1061,1061,1061,1061,1061}, //SPRITE_KNIFE
   {1062,1062,1062,1062,1062,1062,1062,1062}, //SPRITE_MACHINE_GUN
   {1065,1065,1065,1065,1065,1065,1065,1065}, //SPRITE_SHOTGUN
   {1064,1064,1064,1064,1064,1064,1064,1064}, //SPRITE_ROCKET_LAUNCHER
   {1063,1063,1063,1063,1063,1063,1063,1063}, //SPRITE_PLASMA_GUN
   {1066,1066,1066,1066,1066,1066,1066,1066}, //SPRITE_SOLUTION
};
//-------------------------------------

//Function implementations

int32_t sprite_rot(Sprite sp, uint8_t rot)
{
   return sprite_table[sp][rot&7];
}
//-------------------------------------
