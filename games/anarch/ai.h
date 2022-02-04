/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _AI_H_

#define _AI_H_

#include "ai_type.h"

typedef enum
{
   AI_STATE_NULL,
   AI_STATE_PLAYER_SHOTGUN_READY,
   AI_STATE_PLAYER_SHOTGUN_LOAD0,
   AI_STATE_PLAYER_SHOTGUN_LOAD1,
   AI_STATE_PLAYER_SHOTGUN_LOAD2,
   AI_STATE_PLAYER_SHOTGUN_LOAD3,
   AI_STATE_PLAYER_SHOTGUN_LOAD4,
   AI_STATE_PLAYER_SHOTGUN_LOAD5,
   AI_STATE_PLAYER_SHOTGUN_LOAD6,
   AI_STATE_PLAYER_SHOTGUN_LOAD7,
   AI_STATE_PLAYER_SHOTGUN_LOAD8,
   AI_STATE_TREE,
   AI_STATE_LAMP,
   AI_STATE_RUIN,
   AI_STATE_TERMINAL,
   AI_STATE_ELEVATOR_RISE,
   AI_STATE_ELEVATOR_LOWER,
   AI_STATE_ELEVATOR_STILLR,
   AI_STATE_ELEVATOR_STILLL,
   AI_STATE_DOOR,
   AI_STATE_ITEM_KEY,
   AI_STATE_ITEM_BULLET,
   AI_STATE_ITEM_ROCKET,
   AI_STATE_ITEM_CELL,
   AI_STATE_ITEM_HEALTH,
   AI_STATE_MAX,
}AI_statenum;

typedef enum
{
   AI_TYPE_PLAYER,
   AI_TYPE_TREE,
   AI_TYPE_LAMP,
   AI_TYPE_RUIN,
   AI_TYPE_TERMINAL,
   AI_TYPE_ELEVATOR,
   AI_TYPE_DOOR,
   AI_TYPE_ITEM_KEY,
   AI_TYPE_ITEM_BULLET,
   AI_TYPE_ITEM_ROCKET,
   AI_TYPE_ITEM_CELL,
   AI_TYPE_ITEM_HEALTH,
   AI_TYPE_MAX,
}AI_type;

typedef AI_statenum (*ai_func)(AI_ent *);

typedef struct
{
   AI_statenum next;
   ai_func action;
   uint32_t ticks;
   Sprite sprite;
}AI_state;

typedef struct {
  AI_statenum state_idle;
  AI_statenum state_move;
  AI_statenum state_attack;
  AI_statenum state_pain;
  AI_statenum state_death;
  uint8_t attack_delay;
  RvR_fix22 speed;
  int32_t health;
  RvR_fix22 pain_chance;
}AI_info;

typedef struct
{
   uint32_t generation;
   AI_ent *index;
}AI_index;

typedef struct
{
   const AI_state *state;
  AI_type type;
  AI_index target;
  AI_index parent;
  uint32_t tick_end;
}AI;

struct AI_ent
{
   uint32_t generation;

   RvR_vec3 pos;
   RvR_fix22 direction;
   RvR_vec2 direction_vec;
   int32_t health;
   int32_t extra0;
   int32_t extra1;
   int32_t extra2;
   int32_t extra3;

   //Monsters
   uint8_t attack_delay;
   uint8_t move_dir;
   int16_t move;
   RvR_vec3 move_dir_vec;

   //bit 0: attacked last tick
   //bit 1: ranged attack
   uint32_t flags;

   Sprite sprite;
   AI ai;
   Collider *collider;
   AI_ent *next;
   AI_ent **prev_next;
};

AI_type ai_type_from_tex(uint16_t tex);
void ai_init(AI_ent *e, AI_type type);
void ai_run(AI_ent *e);
void ai_damage(AI_ent *to, AI_index *source);

AI_index ai_index_get(AI_ent *e);
AI_ent *ai_index_try(AI_index in);

AI_ent *ai_ent_new();
void    ai_ent_add(AI_ent *e);
void    ai_ent_free(AI_ent *e);
void    ai_ent_clear();
AI_ent *ai_ents();

#endif
