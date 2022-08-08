/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "state.h"
#include "turn.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static State state;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void state_set(State s)
{
   state = s;
   switch(state)
   {
   case STATE_TURN: turn_init(); break;
   }
}

void state_update(void)
{
   switch(state)
   {
   case STATE_TURN: turn_update(); break;
   }
}

void state_draw(void)
{
   switch(state)
   {
   case STATE_TURN: turn_draw(); break;
   }
}
//-------------------------------------
