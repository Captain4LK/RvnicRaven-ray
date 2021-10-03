/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "RvR_core.h"
#include "RvR_math.h"
#include "RvR_ray.h"
#include "sprite.h"
#include "ai.h"
#include "player.h"
//-------------------------------------

//#defines
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
   //Input
   int x,y;
   RvR_core_mouse_relative_pos(&x,&y);
   RvR_vec2 direction = RvR_vec2_rot(player.entity->direction);
   direction.x/=8;
   direction.y/=8;
   int step = 1;
   RvR_vec3 move_offset = {0};

   //Forward/Backward movement
   if(RvR_core_key_down(RvR_config_move_forward))
   {
      move_offset.x+=step*direction.x;
      move_offset.y+=step*direction.y;
   }
   else if(RvR_core_key_down(RvR_config_move_backward))
   {
      move_offset.x-=step*direction.x;
      move_offset.y-=step*direction.y;
   }

   //Strafing
   if(RvR_core_key_down(RvR_config_strafe_left))
   {
      move_offset.x-=direction.y;
      move_offset.y+=direction.x;
   }
   else if(RvR_core_key_down(RvR_config_strafe_right))
   {
      move_offset.x+=direction.y;
      move_offset.y-=direction.x;
   }
   
   //Mouse look: x-axis
   if(x!=0)
      player.entity->direction+=(x*RvR_config_mouse_sensitivity)/128;

   //Shearing (fake looking up/down)
   //Drift back to 0
   if(!shearing&&player.shear!=0)
      player.shear = (player.shear>0)?(RvR_max(0,player.shear-RvR_config_camera_shear_step)):(RvR_min(0,player.shear+RvR_config_camera_shear_step));
   //Enable freelook
   if(RvR_core_key_pressed(RvR_config_enable_freelook))
      shearing = !shearing;
   //Mouse look: y-axis
   if(y!=0&&shearing)
      player.shear = RvR_max(RvR_min(player.shear-(y*RvR_config_mouse_sensitivity_vertical)/128,RvR_config_camera_max_shear),-RvR_config_camera_max_shear);

   //Only for testing --> flying basically
   player.vertical_speed-=GRAVITY;
   if(RvR_core_key_down(RVR_KEY_PGDN))
      player.vertical_speed = -step*100;
   else if(RvR_core_key_down(RVR_KEY_PGUP))
      player.vertical_speed = step*100;

   //Jumping (hacky but works)
   if(RvR_core_key_down(RvR_config_jump)&&player.vertical_speed<=0&&last_vertical_speed==0&&on_ground)
      player.vertical_speed = JUMP_SPEED;

   //Cap player speed
   player.vertical_speed = RvR_max(-MAX_VERTICAL_SPEED,RvR_min(player.vertical_speed,MAX_VERTICAL_SPEED));
   move_offset.z = player.vertical_speed;
   //-------------------------------------

   //Collision
   RvR_ray_set_position(player.entity->pos);
   RvR_fix22 last_z = player.entity->pos.z;
   RvR_ray_move_with_collision(move_offset,1,1);
   player.entity->pos = RvR_ray_get_position();
   on_ground = 0;
   if(last_z==player.entity->pos.z)
   {
      if(player.entity->pos.z>last_z+move_offset.z)
         on_ground = 1;
      player.vertical_speed = 0;
   }
   last_vertical_speed = player.vertical_speed;
   //-------------------------------------
}
//-------------------------------------
