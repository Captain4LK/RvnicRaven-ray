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
#include "sprite.h"
#include "ai.h"
#include "player.h"
#include "config.h"
//-------------------------------------

//#defines
#define GRAVITY 16
#define MAX_VERTICAL_SPEED 256
#define JUMP_SPEED 128
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Player player;

static int shearing = 0;
static RvR_fix22 last_vertical_speed = 1;
static int on_ground = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void player_update()
{
   player.key_needed = 0;

   //Input
   int x,y;
   RvR_core_mouse_relative_pos(&x,&y);
   RvR_vec2 direction = RvR_vec2_rot(player.entity->direction);
   direction.x/=8;
   direction.y/=8;
   int step = 1;
   RvR_vec3 move_offset = {0};

   //Forward/Backward movement
   if(RvR_core_key_down(config_move_forward))
   {
      move_offset.x+=step*direction.x;
      move_offset.y+=step*direction.y;
   }
   else if(RvR_core_key_down(config_move_backward))
   {
      move_offset.x-=step*direction.x;
      move_offset.y-=step*direction.y;
   }

   //Strafing
   if(RvR_core_key_down(config_strafe_left))
   {
      move_offset.x-=direction.y;
      move_offset.y+=direction.x;
   }
   else if(RvR_core_key_down(config_strafe_right))
   {
      move_offset.x+=direction.y;
      move_offset.y-=direction.x;
   }
   
   //Mouse look: x-axis
   if(x!=0)
      player.entity->direction+=(x*config_mouse_sensitivity)/128;

   //Shearing (fake looking up/down)
   //Drift back to 0
   if(!shearing&&player.shear!=0)
      player.shear = (player.shear>0)?(RvR_max(0,player.shear-config_camera_shear_step)):(RvR_min(0,player.shear+config_camera_shear_step));
   //Enable freelook
   if(RvR_core_key_pressed(config_enable_freelook))
      shearing = !shearing;
   //Mouse look: y-axis
   if(y!=0&&shearing)
      player.shear = RvR_max(RvR_min(player.shear-(y*config_mouse_sensitivity_vertical)/128,config_camera_max_shear),-config_camera_max_shear);

   //Only for testing --> flying basically
   player.vertical_speed-=GRAVITY;
   if(RvR_core_key_down(RVR_KEY_PGDN))
      player.vertical_speed = -step*100;
   else if(RvR_core_key_down(RVR_KEY_PGUP))
      player.vertical_speed = step*100;

   //Jumping (hacky but works)
   if(RvR_core_key_down(config_jump)&&on_ground)
      player.vertical_speed = JUMP_SPEED;

   //Cap player speed
   player.vertical_speed = RvR_max(-MAX_VERTICAL_SPEED,RvR_min(player.vertical_speed,MAX_VERTICAL_SPEED));
   move_offset.z = player.vertical_speed;
   //-------------------------------------

   //Update raycasting values
   //needed by collision
   RvR_ray_set_position(player.entity->pos);
   RvR_ray_set_angle(player.entity->direction);
   RvR_ray_set_shear(player.shear);
   //-------------------------------------

   //Collision
   RvR_fix22 floor_height = 0;
   RvR_fix22 ceiling_height = 0;
   RvR_ray_move_with_collision(move_offset,1,1,&floor_height,&ceiling_height);
   player.entity->pos = RvR_ray_get_position();
   on_ground = 0;

   //Reset verticall speed if ceiling was hit
   if(player.entity->pos.z+CAMERA_COLL_HEIGHT_ABOVE==ceiling_height)
      player.vertical_speed = 0;

   //Enable jumping if on ground
   if(player.entity->pos.z-CAMERA_COLL_HEIGHT_BELOW==floor_height)
   {
      on_ground = 1;
      player.vertical_speed = 0;
   }

   last_vertical_speed = player.vertical_speed;
   //-------------------------------------

   //Update raycasting values again
   //might be changed by collision
   RvR_ray_set_position(player.entity->pos);
   RvR_ray_set_angle(player.entity->direction);
   RvR_ray_set_shear(player.shear);
   //-------------------------------------
}

int32_t player_weapon_ammo(int32_t weapon)
{
   switch(weapon)
   {
   case 0: return -1;
   case 1: return player.ammo_bull; //Machine gun
   case 2: return player.ammo_bull; //Shotgun
   case 3: return player.ammo_rckt; //Rocket launcher
   case 4: return player.ammo_cell; //Plasma gun
   case 5: return player.ammo_cell; //Solution
   }

   return 0;
}

void player_weapon_rotate(int direction)
{
   int32_t weapon_start = player.weapon;
   int dir = direction>0?1:-1;

   for(;;)
   {
      player.weapon = (player.weapon+dir)%6;
      if(player.weapon==weapon_start)
         break;

      if(player_weapon_ammo(player.weapon)!=0)
         break;
   }
}

void player_weapon_switch(int32_t weapon)
{
   if(weapon<0||weapon>5)
      return;

   int32_t ammo = player_weapon_ammo(weapon);
   if(ammo==0)
      return;

   player.weapon = weapon;
}
//-------------------------------------
