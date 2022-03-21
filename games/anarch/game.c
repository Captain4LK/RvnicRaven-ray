/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "collision.h"
#include "ai.h"
#include "player.h"
#include "message.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
uint32_t game_tick = 0;

static int map_current = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void game_map_load(uint16_t id)
{
   message_reset();

   map_current = id;

   game_tick = 0;
   int player_health = 100;
   if(player.entity!=NULL)
      player_health = player.entity->health;

   //Free entities
   ai_ent_clear();
   collision_clear();

   RvR_ray_map_load(id);

   for(int i = 0;i<RvR_ray_map_sprite_count();i++)
   {
      RvR_ray_map_sprite *s = RvR_ray_map_sprite_get(i);

      AI_ent *e = ai_ent_new();
      ai_ent_add(e);
      e->pos = s->pos;
      e->direction = s->direction;
      e->direction_vec = RvR_vec2_rot(s->direction);
      e->extra0 = s->extra0;
      e->extra1 = s->extra1;
      e->extra2 = s->extra2;
      e->extra3 = s->extra3;
      ai_init(e,ai_type_from_tex(s->type));

      if(ai_type_from_tex(s->type)==AI_TYPE_PLAYER)
      {
         player.entity = e;
         player.entity->health = player_health;
         player.key = 0;
      }
   }
}

void game_map_reset()
{
   player_init_new();
   player.entity = NULL;
   game_map_load(map_current);
}

void game_update()
{
   game_tick++;

   if(RvR_core_key_pressed(RVR_KEY_M))
      RvR_malloc_report();

   //AI
   AI_ent *e = ai_ents();
   while(e!=NULL)
   {
      AI_ent *next = e->next;
      ai_run(e);
      e = next;
   }

   if(player.entity->health>0)
      player_update();

   RvR_ray_draw_begin();

   RvR_texture *player_sprite = 0;
   e = ai_ents();
   while(e!=NULL)
   {
      if(e->ai.state==NULL)
         goto next;

      if(e->ai.type==AI_TYPE_PLAYER)
      {
         player_sprite = RvR_texture_get(sprite_rot(e->ai.state->sprite,0));
      }
      else
      {
         RvR_vec3 pos = e->pos;
         if(e!=player.entity&&e->ai.state->sprite!=SPRITE_MAX)
         {
            RvR_fix22 rot = (RvR_fix22_atan2(player.entity->pos.x-pos.x,player.entity->pos.y-pos.y));
            rot = (rot+64-e->direction)&1023;

            RvR_ray_draw_sprite(pos,sprite_rot(e->ai.state->sprite,rot/128));
         }
      }

next:
      e = e->next;
   }

   collision_post();

   //Graphics

   RvR_ray_draw_map();
   RvR_ray_draw_end();
   RvR_draw_texture(player_sprite,(RVR_XRES-player_sprite->width)/2,RVR_YRES-player_sprite->height);
   //RvR_ray_draw_debug(8);

   RvR_draw_rectangle_fill(0,RVR_YRES-24,RVR_XRES,24,2);

   char tmp[16];

   //Draw health
   snprintf(tmp,16,"%3d",player.entity->health);
   RvR_draw_rectangle_fill(94,RVR_YRES-22,30,12,1);
   RvR_draw_string(96,RVR_YRES-20,2,tmp,176);
   RvR_draw_string(95,RVR_YRES-8,1,"Health",7);

   //Draw ammo
   int32_t ammo = player_weapon_ammo(player.weapon);
   snprintf(tmp,16,"%3d",ammo==-1?0:ammo);
   RvR_draw_rectangle_fill(RVR_XRES-98-30,RVR_YRES-22,30,12,1);
   RvR_draw_string(RVR_XRES-96-30,RVR_YRES-20,2,tmp,144);
   RvR_draw_string(RVR_XRES-92-30,RVR_YRES-8,1,"Ammo",7);

   //Draw ammo
   snprintf(tmp,16,"BULL %3d/200",player.ammo_bull);
   RvR_draw_string(RVR_XRES-13*5+3,RVR_YRES-20,1,tmp,7);
   snprintf(tmp,16,"RCKT %3d/100",player.ammo_rckt);
   RvR_draw_string(RVR_XRES-13*5+3,RVR_YRES-20+6,1,tmp,7);
   snprintf(tmp,16,"CELL %3d/150",player.ammo_cell);
   RvR_draw_string(RVR_XRES-13*5+3,RVR_YRES-20+12,1,tmp,7);

   //Draw keys
   RvR_draw_string(96+30+23,RVR_YRES-14,1,"Keys",7);
   tmp[1] = '\0';
   tmp[0] = '1'; RvR_draw_string(96+30+16,RVR_YRES-20,1,tmp,(player.key_needed&1)?176:(player.key&1)?7:4);
   tmp[0] = '2'; RvR_draw_string(96+30+16,RVR_YRES-20+6,1,tmp,(player.key_needed&2)?176:(player.key&2)?7:4);
   tmp[0] = '3'; RvR_draw_string(96+30+16,RVR_YRES-20+12,1,tmp,(player.key_needed&4)?176:(player.key&4)?7:4);
   tmp[0] = '4'; RvR_draw_string(RVR_XRES-96-30-23,RVR_YRES-20,1,tmp,(player.key_needed&8)?176:(player.key&8)?7:4);
   tmp[0] = '5'; RvR_draw_string(RVR_XRES-96-30-23,RVR_YRES-20+6,1,tmp,(player.key_needed&16)?176:(player.key&16)?7:4);
   tmp[0] = '6'; RvR_draw_string(RVR_XRES-96-30-23,RVR_YRES-20+12,1,tmp,(player.key_needed&32)?176:(player.key&32)?7:4);

   //Draw messages
   message_draw(7);
}
//-------------------------------------
