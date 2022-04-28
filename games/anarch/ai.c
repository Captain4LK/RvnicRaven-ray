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
#include <string.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "game.h"
#include "sprite.h"
#include "collision.h"
#include "ai.h"
#include "player.h"
#include "sound.h"
#include "message.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Function prototypes
static void mutant_idle(AI_ent *e);
static void shotgun(AI_ent *e);
static void shotgun_open(AI_ent *e);
static void shotgun_load(AI_ent *e);
static void shotgun_close(AI_ent *e);

static void elevator_rise(AI_ent *e);
static void elevator_lower(AI_ent *e);
static void door(AI_ent *e);

static void item_key(AI_ent *e);
static void item_bullet(AI_ent *e);
static void item_rocket(AI_ent *e);
static void item_cell(AI_ent *e);
static void item_health(AI_ent *e);

static void add_collider(AI_ent *e, RvR_vec3 pos, RvR_fix22 radius, RvR_fix22 height, uint32_t flags);
//-------------------------------------

//Variables

//This array will contain the possible states of every possible ai type.
//Having this in a central place allows for easy tweaking of AI behaviour.
static const AI_state ai_state[AI_STATE_MAX] = {
  { .next = AI_STATE_NULL, .action = NULL, .ticks = 0},                                                    //STATE_NULL
  { .next = AI_STATE_PLAYER_SHOTGUN_READY, .action = shotgun, .ticks = 1, .sprite = SPRITE_KNIFE}, //STATE_SHOTGUN_READY
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD1, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD0
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD2, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD1
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD3, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD2
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD4, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD3
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD5, .action = shotgun_open, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD4
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD6, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD5
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD7, .action = shotgun_load, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD6
  { .next = AI_STATE_PLAYER_SHOTGUN_LOAD8, .action = NULL, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD7
  { .next = AI_STATE_PLAYER_SHOTGUN_READY, .action = shotgun_close, .ticks = 4, .sprite = SPRITE_TREE},   //STATE_SHOTGUN_LOAD8
  { .next = AI_STATE_TREE, .action = NULL, .ticks = 1, .sprite = SPRITE_TREE},   //STATE_TREE
  { .next = AI_STATE_LAMP, .action = NULL, .ticks = 1, .sprite = SPRITE_LAMP},   //STATE_LAMP
  { .next = AI_STATE_RUIN, .action = NULL, .ticks = 1, .sprite = SPRITE_RUIN},   //STATE_RUIN
  { .next = AI_STATE_TERMINAL, .action = NULL, .ticks = 1, .sprite = SPRITE_TERMINAL},   //STATE_TERMINAL
  { .next = AI_STATE_BLOCKER, .action = NULL, .ticks = 1, .sprite = SPRITE_MAX},   //STATE_BLOCKER
  { .next = AI_STATE_BARREL, .action = NULL, .ticks = 1, .sprite = SPRITE_BARREL},   //STATE_BARREL
  { .next = AI_STATE_ELEVATOR_RISE, .action = elevator_rise, .ticks = 1, .sprite = SPRITE_MAX},   //STATE_ELEVATOR_RISE
  { .next = AI_STATE_ELEVATOR_LOWER, .action = elevator_lower, .ticks = 1, .sprite = SPRITE_MAX},   //STATE_ELEVATOR_LOWER
  { .next = AI_STATE_ELEVATOR_LOWER, .action = NULL, .ticks = 30, .sprite = SPRITE_MAX},   //STATE_ELEVATOR_STILLR
  { .next = AI_STATE_ELEVATOR_RISE, .action = NULL, .ticks = 30, .sprite = SPRITE_MAX},   //STATE_ELEVATOR_STILLL
  { .next = AI_STATE_DOOR, .action = door, .ticks = 1, .sprite = SPRITE_MAX},   //STATE_DOOR
  { .next = AI_STATE_ITEM_KEY, .action = item_key, .ticks = 1, .sprite = SPRITE_ITEM_KEY},   //STATE_ITEM_KEY
  { .next = AI_STATE_ITEM_BULLET, .action = item_bullet, .ticks = 1, .sprite = SPRITE_ITEM_BULLET},   //STATE_ITEM_BULLET
  { .next = AI_STATE_ITEM_ROCKET, .action = item_rocket, .ticks = 1, .sprite = SPRITE_ITEM_ROCKET},   //STATE_ITEM_ROCKET
  { .next = AI_STATE_ITEM_CELL, .action = item_cell, .ticks = 1, .sprite = SPRITE_ITEM_CELL},   //STATE_ITEM_CELL
  { .next = AI_STATE_ITEM_HEALTH, .action = item_health, .ticks = 1, .sprite = SPRITE_ITEM_HEALTH},   //STATE_ITEM_HEALTH
};

static const AI_info ai_entinfo[AI_TYPE_MAX] = {
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
  //AI_TYPE_BLOCKER
  {
    .state_idle = AI_STATE_BLOCKER,
    .state_move = AI_STATE_BLOCKER,
    .state_attack = AI_STATE_BLOCKER,
    .state_death = AI_STATE_BLOCKER,
  },
  //AI_TYPE_BARREL
  {
    .state_idle = AI_STATE_BARREL,
    .state_move = AI_STATE_BARREL,
    .state_attack = AI_STATE_BARREL,
    .state_death = AI_STATE_BARREL,
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
  //AI_TYPE_ITEM_ROCKET
  {
    .state_idle = AI_STATE_ITEM_ROCKET,
    .state_move = AI_STATE_ITEM_ROCKET,
    .state_attack = AI_STATE_ITEM_ROCKET,
    .state_death = AI_STATE_ITEM_ROCKET,
  },
  //AI_TYPE_ITEM_CELL
  {
    .state_idle = AI_STATE_ITEM_CELL,
    .state_move = AI_STATE_ITEM_CELL,
    .state_attack = AI_STATE_ITEM_CELL,
    .state_death = AI_STATE_ITEM_CELL,
  },
  //AI_TYPE_ITEM_HEALTH
  {
    .state_idle = AI_STATE_ITEM_HEALTH,
    .state_move = AI_STATE_ITEM_HEALTH,
    .state_attack = AI_STATE_ITEM_HEALTH,
    .state_death = AI_STATE_ITEM_HEALTH,
  },
};

static AI_ent *ai_ent_pool = NULL;
static AI_ent *ents = NULL;
//-------------------------------------

//Function implementations

AI_type ai_type_from_tex(uint16_t tex)
{
   switch(tex)
   {
   case 1040: return AI_TYPE_TREE;
   case 1034: return AI_TYPE_LAMP;
   case 1037: return AI_TYPE_RUIN;
   case 1039: return AI_TYPE_TERMINAL;

   case 1028: return AI_TYPE_BARREL;
   case 1029: return AI_TYPE_ITEM_BULLET;
   case 1030: return AI_TYPE_ITEM_KEY;
   case 1036: return AI_TYPE_ITEM_ROCKET;
   case 1035: return AI_TYPE_ITEM_CELL;
   case 1033: return AI_TYPE_ITEM_HEALTH;

   case 2048: return AI_TYPE_PLAYER;
   case 2049: return AI_TYPE_DOOR;
   case 2050: return AI_TYPE_ELEVATOR;
   case 2052: return AI_TYPE_BLOCKER;
   }

   return AI_TYPE_TREE;
}

void ai_init(AI_ent *e, AI_type type)
{
   e->ai.type = type;
   e->ai.state = &ai_state[ai_entinfo[e->ai.type].state_idle];
   e->attack_delay = ai_entinfo[e->ai.type].attack_delay;
   e->move_dir = 8;
   e->move = 0;
   e->ai.ticks = e->ai.state->ticks;
   e->health = ai_entinfo[e->ai.type].health;

   //Type specifc attributes, colliders
   switch(e->ai.type)
   {
   case AI_TYPE_PLAYER: add_collider(e,e->pos,384,1024,1); break;
   case AI_TYPE_TREE: add_collider(e,e->pos,384,1024,1); break;
   case AI_TYPE_LAMP: add_collider(e,e->pos,384,1024,1); break;
   case AI_TYPE_RUIN: add_collider(e,e->pos,384,1024,1); break;
   case AI_TYPE_BLOCKER: add_collider(e,e->pos,384,1024,1); break;
   default: break;
   }
}

void ai_run(AI_ent *e)
{
   if(e->ai.state==NULL)
   {
      if(e->collider!=NULL)
         collision_remove(e->collider);
      e->collider = NULL;
      ai_ent_free(e);
      return;
   }

   e->ai.ticks--;
   if(e->ai.ticks<=0)
      ai_set_state(e,e->ai.state->next);
}

void ai_damage(AI_ent *to, AI_index *source, int damage)
{
   if(to==NULL||to->ai.state==NULL)
      return;

   if(to->collider==NULL)
      return;

   //Not shootable
   if(!(to->collider->flags&1))
      return;

   //Go after attacker
   AI_ent *target = ai_index_try(to->ai.target);

   to->health-=damage;
   if(to->health<=0)
   {
      if(ai_entinfo[to->ai.type].state_death!=AI_STATE_NULL)
      {
         ai_set_state(to,ai_entinfo[to->ai.type].state_death);
      }

      return;
   }

   //if(to->ai.type==AI_TYPE_PLAYER)
      //sound_play(SOUND_PLAYER_DAMAGE,ai_index_get(to),255);

   if(target==NULL&&ai_index_try(*source)!=NULL&&ai_index_try(*source)->collider!=NULL&&to!=player.entity)
   {
      int rnd = RvR_rand()%3;
      //Sound rnd_sounds[] = {SOUND_ALIEN_ALERT0,SOUND_ALIEN_ALERT1,SOUND_ALIEN_ALERT2};
      //sound_play(rnd_sounds[rnd],ai_index_get(to),255);

      to->ai.target = *source;
      if(ai_entinfo[to->ai.type].state_move!=AI_STATE_NULL)
         ai_set_state(to,ai_entinfo[to->ai.type].state_move);

      return;
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

   uint32_t gen = n->generation;
   memset(n,0,sizeof(*n));
   n->generation = gen;

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

void ai_ent_remove(AI_ent *e)
{
   if(e==NULL)
      return;

   e->generation++;
   e->ai.state = NULL;
}

void ai_ent_free(AI_ent *e)
{
   if(e==NULL)
      return;

   *e->prev_next = e->next;
   if(e->next!=NULL)
      e->next->prev_next = e->prev_next;

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

   //Reset pool
   AI_ent *pool = ai_ent_pool;
   while(pool!=NULL)
   {
      AI_ent *next = pool->next;
      memset(pool,0,sizeof(*pool));
      pool = next;
   }
}

AI_ent *ai_ents()
{
   return ents;
}

void ai_set_state(AI_ent *e, AI_statenum nstate)
{
   if(e==NULL||e->ai.state==NULL)
      return;

   do
   {
      if(nstate==AI_STATE_NULL)
      {
         if(e->collider!=NULL)
            collision_remove(e->collider);
         ai_ent_remove(e);

         return;
      }

      e->ai.state = &ai_state[nstate];
      e->ai.ticks = e->ai.state->ticks;

      if(e->ai.state->action!=NULL)
         e->ai.state->action(e);

      if(e->ai.state==NULL)
         return;

      nstate = e->ai.state->next;
   }while(e->ai.ticks<=0);
}

static void mutant_idle(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;
}

static void shotgun(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;
   /**e = *e;

   if(RvR_core_mouse_down(RVR_BUTTON_LEFT))
   {
      sound_play(SOUND_DSG,1);
      return AI_STATE_PLAYER_SHOTGUN_LOAD0;
   }
   return AI_STATE_NULL;*/
}

static void shotgun_open(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;
   /**e = *e;

   sound_play(SOUND_DSG_OPEN,1);

   return AI_STATE_NULL;*/
}

static void shotgun_load(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;
   /**e = *e;

   sound_play(SOUND_DSG_LOAD,1);

   return AI_STATE_NULL;*/
}

static void shotgun_close(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;
   /**e = *e;

   sound_play(SOUND_DSG_CLOSE,1);

   return AI_STATE_NULL;*/
}

static void elevator_rise(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;

   RvR_fix22 z = RvR_ray_map_floor_height_at(e->pos.x/1024,e->pos.y/1024);
   if(z>=e->extra1)
      ai_set_state(e,AI_STATE_ELEVATOR_STILLR);
   z = RvR_min(e->extra1,z+48);
   RvR_ray_map_floor_height_set(e->pos.x/1024,e->pos.y/1024,z);
}

static void elevator_lower(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;

   RvR_fix22 z = RvR_ray_map_floor_height_at(e->pos.x/1024,e->pos.y/1024);
   if(z<=e->extra0)
      ai_set_state(e,AI_STATE_ELEVATOR_STILLL);

   z = RvR_max(e->extra0,z-48);
   RvR_ray_map_floor_height_set(e->pos.x/1024,e->pos.y/1024,z);
}

static void door(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;

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

   //return AI_STATE_NULL;*/
}

static void item_key(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;

   RvR_fix22 dist = RvR_abs(player.entity->pos.x-e->pos.x)+RvR_abs(player.entity->pos.y-e->pos.y);
   if(dist<512)
   {
      player.key|=e->extra0;
      //sound_play(SOUND_PICKUP0,ai_index_get(player.entity),255);
      ai_set_state(e,AI_STATE_NULL);
      message_queue("Picked up key");
      return;
   }
}

static void item_bullet(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;

   RvR_fix22 dist = RvR_abs(player.entity->pos.x-e->pos.x)+RvR_abs(player.entity->pos.y-e->pos.y);
   if(dist<512&&player.ammo_bull<150)
   {
      player.ammo_bull =  RvR_min(player.ammo_bull+10,150);
      //sound_play(SOUND_PICKUP0,ai_index_get(player.entity),255);
      ai_set_state(e,AI_STATE_NULL);
      message_queue("Picked up bullets");
      return;
   }
}

static void item_rocket(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;

   RvR_fix22 dist = RvR_abs(player.entity->pos.x-e->pos.x)+RvR_abs(player.entity->pos.y-e->pos.y);
   if(dist<512&&player.ammo_rckt<100)
   {
      player.ammo_rckt =  RvR_min(player.ammo_rckt+5,100);
      //sound_play(SOUND_PICKUP0,ai_index_get(player.entity),255);
      ai_set_state(e,AI_STATE_NULL);
      message_queue("Picked up rockets");
      return;
   }
}

static void item_cell(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;

   RvR_fix22 dist = RvR_abs(player.entity->pos.x-e->pos.x)+RvR_abs(player.entity->pos.y-e->pos.y);
   if(dist<512&&player.ammo_cell<150)
   {
      player.ammo_cell =  RvR_min(player.ammo_cell+8,150);
      //sound_play(SOUND_PICKUP0,ai_index_get(player.entity),255);
      ai_set_state(e,AI_STATE_NULL);
      message_queue("Picked up cells");
      return;
   }
}

static void item_health(AI_ent *e)
{
   if(e->ai.state==NULL)
      return;

   RvR_fix22 dist = RvR_abs(player.entity->pos.x-e->pos.x)+RvR_abs(player.entity->pos.y-e->pos.y);
   if(dist<512&&player.entity->health<125)
   {
      player.entity->health =  RvR_min(player.entity->health+20,125);
      //sound_play(SOUND_PICKUP0,ai_index_get(player.entity),255);
      ai_set_state(e,AI_STATE_NULL);
      message_queue("Picked up health");
      return;
   }
}

static void add_collider(AI_ent *e, RvR_vec3 pos, RvR_fix22 radius, RvR_fix22 height, uint32_t flags)
{
   e->collider = collision_new();
   e->collider->pos = pos;
   e->collider->radius = radius;
   e->collider->height = height;
   e->collider->flags = flags;
   e->collider->ent = e;
   collision_add(e->collider);
}
//-------------------------------------
