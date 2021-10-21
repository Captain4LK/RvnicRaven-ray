/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "game.h"
#include "sprite.h"
#include "ai.h"
#include "player.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
uint32_t game_tick = 0;

static AI_ent *ai_ents = NULL;
//-------------------------------------

//Function prototypes
static AI_type u16_to_type(uint16_t type);
//-------------------------------------

//Function implementations

void set_difficulty(int difficulty)
{

}

void set_episode(int episode)
{

}

void game_map_load()
{
   //Free entities
   ai_ent_free(ai_ents);
   ai_ents = NULL;

   RvR_ray_map_load(0);

   for(int i = 0;i<RvR_ray_map_sprite_count();i++)
   {
      RvR_ray_map_sprite *s = RvR_ray_map_sprite_get(i);

      AI_ent *e = ai_ent_new();
      e->next = ai_ents;
      ai_ents = e;
      ai_init(e,u16_to_type(s->type));
      sprite_load(u16_to_type(s->type));
      e->pos = s->pos;
      if(u16_to_type(s->type)==AI_TYPE_PLAYER)
         player.entity = e;
   }

   printf("%d %d\n",player.entity->pos.x,player.entity->pos.y);
}

void game_update()
{
   RvR_texture *player_sprite = 0;
   game_tick++;

   player_update();
   if(RvR_core_key_pressed(RVR_KEY_M))
      RvR_malloc_report();

   //AI
   AI_ent *e = ai_ents;
   while(e!=NULL)
   {
      ai_run(e);
      e = e->next;
   }

   e = ai_ents;
   while(e!=NULL)
   {
      if(e->ai.type==AI_TYPE_PLAYER)
      {
         player_sprite = RvR_texture_get(sprite_rot(e->ai.state->sprite,0));
      }
      else
      {
         //TODO: calculate rotation
         RvR_ray_draw_sprite(e->pos,sprite_rot(e->ai.state->sprite,0));
      }
      e = e->next;
   }

   //Graphics
   RvR_ray_draw();
   RvR_draw_texture(player_sprite,(RVR_XRES-player_sprite->width)/2,RVR_YRES-player_sprite->height);
}

static AI_type u16_to_type(uint16_t type)
{
   switch(type)
   {
   case 0: return AI_TYPE_TREE0;
   case 1: return AI_TYPE_TREE1;
   case 2: return AI_TYPE_TREE2;
   case 3: return AI_TYPE_ROCK0;
   case 4: return AI_TYPE_ROCK1;
   case 5: return AI_TYPE_CORPSE0;
   case 6: return AI_TYPE_CORPSE1;
   case 7: return AI_TYPE_CORPSE2;
   case 8: return AI_TYPE_CORPSE3;
   case 9: return AI_TYPE_CORPSE4;
   case 10: return AI_TYPE_CORPSE5;
   case 11: return AI_TYPE_CORPSE6;
   case 12: return AI_TYPE_CORPSE7;
   case 13: return AI_TYPE_GARGOYLE;
   case 14: return AI_TYPE_PLAYER;
   }

   return AI_TYPE_MAX;
}
//-------------------------------------
