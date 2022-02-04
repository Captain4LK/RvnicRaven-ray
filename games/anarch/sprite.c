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
uint16_t sprite_table[SPRITE_MAX][8] =
{
   {21264,21264,21264,21264,21264,21264,21264,21264}, //SPRITE_TREE
   {21258,21258,21258,21258,21258,21258,21258,21258}, //SPRITE_LAMP
   {21261,21261,21261,21261,21261,21261,21261,21261}, //SPRITE_RUIN
   {21263,21263,21263,21263,21263,21263,21263,21263}, //SPRITE_TERMINAL
   {21254,21254,21254,21254,21254,21254,21254,21254}, //SPRITE_ITEM_KEY
   {21253,21253,21253,21253,21253,21253,21253,21253}, //SPRITE_ITEM_BULLET
   {21260,21260,21260,21260,21260,21260,21260,21260}, //SPRITE_ITEM_ROCKET
   {21259,21259,21259,21259,21259,21259,21259,21259}, //SPRITE_ITEM_CELL
   {21257,21257,21257,21257,21257,21257,21257,21257}, //SPRITE_ITEM_HEALTH
   {21285,21285,21285,21285,21285,21285,21285,21285}, //SPRITE_KNIFE
   {21286,21286,21286,21286,21286,21286,21286,21286}, //SPRITE_MACHINE_GUN
   {21289,21289,21289,21289,21289,21289,21289,21289}, //SPRITE_SHOTGUN
   {21288,21288,21288,21288,21288,21288,21288,21288}, //SPRITE_ROCKET_LAUNCHER
   {21287,21287,21287,21287,21287,21287,21287,21287}, //SPRITE_PLASMA_GUN
   {21290,21290,21290,21290,21290,21290,21290,21290}, //SPRITE_SOLUTION
};
//-------------------------------------

//Function implementations

uint16_t sprite_rot(Sprite sp, uint8_t rot)
{
   return sprite_table[sp][rot&7];
}
//-------------------------------------
