/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _AI_H_

#define _AI_H_

typedef struct AI_ent AI_ent;

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
   AI_STATE_MAX,
}AI_statenum;

typedef enum
{
   AI_TYPE_PLAYER,
   AI_TYPE_TREE,
   AI_TYPE_LAMP,
   AI_TYPE_RUIN,
   AI_TYPE_TERMINAL,
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
  AI_statenum state_death;
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
  uint32_t tick_end;
}AI;

struct AI_ent
{
   uint32_t generation;
   RvR_vec3 pos;
   RvR_fix22 direction;
   Sprite sprite;
   AI ai;
   AI_ent *next;
};

void ai_init(AI_ent *e, AI_type type);
void ai_run(AI_ent *e);
void ai_damage(AI_ent *to, AI_index *source);

AI_index ai_index_get(AI_ent *e);
AI_ent *ai_index_try(AI_index in);

AI_ent *ai_ent_new();
void    ai_ent_free(AI_ent *e);

void sprite_load(AI_type t);

#endif
