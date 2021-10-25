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
#include "player.h"
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

static AI_statenum elevator_rise(AI_ent *e);
static AI_statenum elevator_lower(AI_ent *e);
static AI_statenum door(AI_ent *e);

static AI_statenum item_key(AI_ent *e);
static AI_statenum item_bullet(AI_ent *e);
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
  { .next = AI_STATE_RUIN, .action = NULL, .ticks = 0, .sprite = SPRITE_RUIN},   //STATE_RUIN
  { .next = AI_STATE_TERMINAL, .action = NULL, .ticks = 0, .sprite = SPRITE_TERMINAL},   //STATE_TERMINAL
  { .next = AI_STATE_ELEVATOR_RISE, .action = elevator_rise, .ticks = 0, .sprite = SPRITE_MAX},   //STATE_ELEVATOR_RISE
  { .next = AI_STATE_ELEVATOR_LOWER, .action = elevator_lower, .ticks = 0, .sprite = SPRITE_MAX},   //STATE_ELEVATOR_LOWER
  { .next = AI_STATE_ELEVATOR_LOWER, .action = NULL, .ticks = 30, .sprite = SPRITE_MAX},   //STATE_ELEVATOR_STILLR
  { .next = AI_STATE_ELEVATOR_RISE, .action = NULL, .ticks = 30, .sprite = SPRITE_MAX},   //STATE_ELEVATOR_STILLL
  { .next = AI_STATE_DOOR, .action = door, .ticks = 0, .sprite = SPRITE_MAX},   //STATE_DOOR
  { .next = AI_STATE_ITEM_KEY, .action = item_key, .ticks = 0, .sprite = SPRITE_ITEM_KEY},   //STATE_ITEM_KEY
  { .next = AI_STATE_ITEM_BULLET, .action = item_bullet, .ticks = 0, .sprite = SPRITE_ITEM_BULLET},   //STATE_ITEM_BULLET
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
  //AI_TYPE_RUIN
  {
    .state_idle = AI_STATE_RUIN,
    .state_move = AI_STATE_RUIN,
    .state_attack = AI_STATE_RUIN,
    .state_death = AI_STATE_RUIN,
  },
  //AI_TYPE_TERMINAL
  {
    .state_idle = AI_STATE_TERMINAL,
    .state_move = AI_STATE_TERMINAL,
    .state_attack = AI_STATE_TERMINAL,
    .state_death = AI_STATE_TERMINAL,
  },
  //AI_TYPE_ELEVATOR
  {
    .state_idle = AI_STATE_ELEVATOR_STILLR,
    .state_move = AI_STATE_ELEVATOR_STILLR,
    .state_attack = AI_STATE_ELEVATOR_STILLR,
    .state_death = AI_STATE_ELEVATOR_STILLR,
  },
  //AI_TYPE_DOOR
  {
    .state_idle = AI_STATE_DOOR,
    .state_move = AI_STATE_DOOR,
    .state_attack = AI_STATE_DOOR,
    .state_death = AI_STATE_DOOR,
  },
  //AI_TYPE_ITEM_KEY
  {
    .state_idle = AI_STATE_ITEM_KEY,
    .state_move = AI_STATE_ITEM_KEY,
    .state_attack = AI_STATE_ITEM_KEY,
    .state_death = AI_STATE_ITEM_KEY,
  },
  //AI_TYPE_ITEM_BULLET
  {
    .state_idle = AI_STATE_ITEM_BULLET,
    .state_move = AI_STATE_ITEM_BULLET,
    .state_attack = AI_STATE_ITEM_BULLET,
    .state_death = AI_STATE_ITEM_BULLET,
  },
};

static AI_ent *ai_ent_pool = NULL;
static AI_ent *ents = NULL;
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
   n->prev_next = NULL;

   return n;
}

void ai_ent_add(AI_ent *e)
{
   e->prev_next = &ents;
   if(ents!=NULL)
      ents->prev_next = &e->next;
   e->next = ents;
   ents = e;
}

void ai_ent_free(AI_ent *e)
{
   if(e==NULL)
      return;

   *e->prev_next = e->next;
   if(e->next!=NULL)
      e->next->prev_next = e->prev_next;
   //if(e->prev!=NULL)
      //e->prev->next = e->next;
   //if(e->next!=NULL)
      //e->next->prev = e->prev;

   e->generation++;
   e->next = ai_ent_pool;
   ai_ent_pool = e;
}

void ai_ent_clear()
{
   AI_ent *e = ents;
   while(e!=NULL)
   {
      AI_ent *next = e->next;
      ai_ent_free(e);
      e = next;
   }
   ents = NULL;
}

AI_ent *ai_ents()
{
   return ents;
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
      case AI_TYPE_RUIN:
         sprite_load_sprite(SPRITE_RUIN);
         break;
      case AI_TYPE_TERMINAL:
         sprite_load_sprite(SPRITE_TERMINAL);
         break;
      case AI_TYPE_ELEVATOR:
         break;
      case AI_TYPE_DOOR:
         break;
      case AI_TYPE_ITEM_KEY:
         sprite_load_sprite(SPRITE_ITEM_KEY);
         break;
      case AI_TYPE_ITEM_BULLET:
         sprite_load_sprite(SPRITE_ITEM_BULLET);
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

static AI_statenum elevator_rise(AI_ent *e)
{
   *e = *e;

   RvR_fix22 z = RvR_ray_map_floor_height_at(e->pos.x/1024,e->pos.y/1024);
   if(z>=e->extra1)
      return AI_STATE_ELEVATOR_STILLR;
   z = RvR_min(e->extra1,z+48);
   RvR_ray_map_floor_height_set(e->pos.x/1024,e->pos.y/1024,z);

   return AI_STATE_NULL;
}

static AI_statenum elevator_lower(AI_ent *e)
{
   *e = *e;

   RvR_fix22 z = RvR_ray_map_floor_height_at(e->pos.x/1024,e->pos.y/1024);
   if(z<=e->extra0)
      return AI_STATE_ELEVATOR_STILLL;

   z = RvR_max(e->extra0,z-48);
   RvR_ray_map_floor_height_set(e->pos.x/1024,e->pos.y/1024,z);
   
   return AI_STATE_NULL;
}

static AI_statenum door(AI_ent *e)
{
   //Door lowers if the manhatten distance between the door and the player is smaller than 2048
   RvR_fix22 dist = RvR_abs(player.entity->pos.x-e->pos.x)+RvR_abs(player.entity->pos.y-e->pos.y);
   RvR_fix22 z = RvR_ray_map_floor_height_at(e->pos.x/1024,e->pos.y/1024);


   int locked = (e->extra2&player.key)!=e->extra2;

   if(dist<2048&&!locked)
   {
      z = RvR_max(e->extra0,z-48);
      RvR_ray_map_floor_height_set(e->pos.x/1024,e->pos.y/1024,z);
   }
   else
   {
      if(dist<2048)
         player.key_needed|=e->extra2;
      z = RvR_min(e->extra1,z+48);
      RvR_ray_map_floor_height_set(e->pos.x/1024,e->pos.y/1024,z);
   }

   return AI_STATE_NULL;
}

static AI_statenum item_key(AI_ent *e)
{
   RvR_fix22 dist = RvR_abs(player.entity->pos.x-e->pos.x)+RvR_abs(player.entity->pos.y-e->pos.y);
   if(dist<512)
   {
      player.key|=e->extra0;
      ai_ent_free(e);
      return AI_STATE_NULL;
   }

   return AI_STATE_NULL;
}

static AI_statenum item_bullet(AI_ent *e)
{
   RvR_fix22 dist = RvR_abs(player.entity->pos.x-e->pos.x)+RvR_abs(player.entity->pos.y-e->pos.y);
   if(dist<512)
   {
      player.ammo_bull =  RvR_min(player.ammo_bull+10,200);
      ai_ent_free(e);
      return AI_STATE_NULL;
   }

   return AI_STATE_NULL;
}
//-------------------------------------
