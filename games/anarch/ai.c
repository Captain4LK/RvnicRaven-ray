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
#include <string.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "game.h"
#include "sprite.h"
#include "ai.h"
#include "sound.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Function prototypes
static AI_statenum set_state(AI_ent *e, AI_statenum nstate);
static AI_statenum run_state(AI_ent *e);

static AI_statenum mutant_idle(AI_ent *e);
static AI_statenum shotgun(AI_ent *e);
static AI_statenum shotgun_open(AI_ent *e);
static AI_statenum shotgun_load(AI_ent *e);
static AI_statenum shotgun_close(AI_ent *e);
//-------------------------------------

//Variables

//This array will contain the possible states of every possible ai type.
//Having this in a central place allows for easy tweaking of AI behaviour.
static const AI_state _ai_state[AI_STATE_MAX] = {
  { .next = AI_STATE_NULL, .action = NULL, .ticks = 0},                                                    //STATE_NULL
  { .next = AI_STATE_PLAYER_SHOTGUN_READY, .action = shotgun, .ticks = 0, .sprite = SPRITE_TREE}, //STATE_SHOTGUN_READY
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD1, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD0
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD2, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD1
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD3, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD2
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD4, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD3
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD5, .action = shotgun_open, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD4
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD6, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD5
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD7, .action = shotgun_load, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD6
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD8, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD7
  { .next = AI_STATE_PLAYER_SHOTGUN_READY, .action = shotgun_close, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD8
  { .next = AI_STATE_TREE, .action = NULL, .ticks = 0, .sprite = SPRITE_TREE},   //STATE_TREE
  { .next = AI_STATE_LAMP, .action = NULL, .ticks = 0, .sprite = SPRITE_LAMP},   //STATE_LAMP
};

static const AI_info _ai_entinfo[AI_TYPE_MAX] = {
  //AI_TYPE_PLAYER
  {
    .state_idle = AI_STATE_PLAYER_SHOTGUN_READY,
    .state_move = AI_STATE_NULL,
    .state_attack = AI_STATE_NULL,
    .state_death = AI_STATE_NULL,
  },
  //AI_TYPE_TREE
  {
    .state_idle = AI_STATE_TREE,
    .state_move = AI_STATE_TREE,
    .state_attack = AI_STATE_TREE,
    .state_death = AI_STATE_TREE,
  },
  //AI_TYPE_LAMP
  {
    .state_idle = AI_STATE_LAMP,
    .state_move = AI_STATE_LAMP,
    .state_attack = AI_STATE_LAMP,
    .state_death = AI_STATE_LAMP,
  },
};

AI_ent *ai_ent_pool = NULL;
//-------------------------------------

//Function implementations

void ai_init(AI_ent *e, AI_type type)
{
   e->ai.type = type;
   e->ai.state = &_ai_state[_ai_entinfo[e->ai.type].state_idle];
   e->ai.tick_end = game_tick+e->ai.state->ticks;
}

void ai_run(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;

   AI_statenum next;
   if(game_tick>=e->ai.tick_end)
      next = set_state(e,e->ai.state->next);
   else
      next = run_state(e);

   while(next!=AI_STATE_NULL)
      next = set_state(e,next);
}

void ai_damage(AI_ent *to, AI_index *source)
{
   //Go after attacker
   AI_ent *target = ai_index_try(to->ai.target);
   if(target==NULL&&ai_index_try(*source)!=NULL)
   {
      to->ai.target = *source;
      set_state(to,AI_STATE_NULL);
   }
}

AI_index ai_index_get(AI_ent *e)
{
   return (AI_index){.generation = e->generation, .index = e};
}

AI_ent *ai_index_try(AI_index in)
{
   if(in.index==NULL)
      return NULL;

   if(in.generation==in.index->generation)
      return in.index;
   return NULL;
}

AI_ent *ai_ent_new()
{
   if(ai_ent_pool==NULL)
   {
      AI_ent *ne = RvR_malloc(sizeof(*ne)*256);
      memset(ne,0,sizeof(*ne)*256);
      
      for(int i = 0;i<256-1;i++)
         ne[i].next = &ne[i+1];
      ai_ent_pool = &ne[0];
   }

   AI_ent *n = ai_ent_pool;
   ai_ent_pool = n->next;
   n->next = NULL;

   return n;
}

void ai_ent_free(AI_ent *e)
{
   if(e==NULL)
      return;

   ai_ent_free(e->next);

   e->generation++;
   e->next = ai_ent_pool;
   ai_ent_pool = e;
}

void sprite_load(AI_type t)
{
   switch(t)
   {
      case AI_TYPE_PLAYER:
         sprite_load_sprite(SPRITE_TREE);
         break;
      case AI_TYPE_TREE:
         sprite_load_sprite(SPRITE_TREE);
         break;
      case AI_TYPE_LAMP:
         sprite_load_sprite(SPRITE_LAMP);
         break;
      case AI_TYPE_MAX:
      break;
   }
}

static AI_statenum set_state(AI_ent *e, AI_statenum nstate)
{
   e->ai.state = &_ai_state[nstate];
   e->ai.tick_end = game_tick+e->ai.state->ticks;

   if(e->ai.state->action!=NULL)
      return e->ai.state->action(e);
   return AI_STATE_NULL;
}

static AI_statenum run_state(AI_ent *e)
{
   if(e->ai.state->action!=NULL)
      return e->ai.state->action(e);
   return AI_STATE_NULL;
}

static AI_statenum mutant_idle(AI_ent *e)
{
   *e = *e;

   return AI_STATE_NULL;
}

static AI_statenum shotgun(AI_ent *e)
{
   *e = *e;

   if(RvR_core_mouse_down(RVR_BUTTON_LEFT))
   {
      sound_play(SOUND_DSG,1);
      return AI_STATE_PLAYER_SHOTGUN_LOAD0;
   }
   return AI_STATE_NULL;
}

static AI_statenum shotgun_open(AI_ent *e)
{
   *e = *e;

   sound_play(SOUND_DSG_OPEN,1);

   return AI_STATE_NULL;
}

static AI_statenum shotgun_load(AI_ent *e)
{
   *e = *e;

   sound_play(SOUND_DSG_LOAD,1);

   return AI_STATE_NULL;
}

static AI_statenum shotgun_close(AI_ent *e)
{
   *e = *e;

   sound_play(SOUND_DSG_CLOSE,1);

   return AI_STATE_NULL;
}
//-------------------------------------
