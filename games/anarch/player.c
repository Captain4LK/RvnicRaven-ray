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
#include "config.h"
#include "sprite.h"
#include "collision.h"
#include "ai.h"
#include "player.h"
#include "game.h"
//-------------------------------------

//#defines
#define GRAVITY 16
#define MAX_VERTICAL_SPEED 256
#define JUMP_SPEED 128

#define CAMERA_SHEAR_MAX_PIXELS ((CAMERA_SHEAR_MAX*RVR_YRES)/1024)
#define CAMERA_SHEAR_STEP_FRAME ((RVR_YRES*CAMERA_SHEAR_SPEED)/(RVR_FPS*4))
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Player player;

static int shearing = 0;
static RvR_fix22 last_vertical_speed = 1;
static int on_ground = 0;
static uint32_t move_start_tick = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void player_init_new()
{
   player.ammo_bull = 0;
   player.ammo_cell = 0;
   player.ammo_rckt = 0;
}

void player_update()
{
   player.key_needed = 0;

   //Input
   int x,y;
   RvR_core_mouse_relative_pos(&x,&y);
   RvR_vec2 direction = RvR_vec2_rot(player.entity->direction);

   direction.x+=direction.x/2;
   direction.y+=direction.y/2;
   int step = 1;
   RvR_vec3 move_offset = {0};
   int accel = 0;

   //Forward/Backward movement
   if(RvR_core_key_down(RVR_KEY_W))
   {
      player.vel.x+=step*direction.x;
      player.vel.y+=step*direction.y;
      accel = 1;
   }
   else if(RvR_core_key_down(RVR_KEY_S))
   {
      player.vel.x-=step*direction.x;
      player.vel.y-=step*direction.y;
      accel = 1;
   }

   //Strafing
   if(RvR_core_key_down(RVR_KEY_A))
   {
      player.vel.x+=direction.y;
      player.vel.y-=direction.x;
      accel = 1;
   }
   else if(RvR_core_key_down(RVR_KEY_D))
   {
      player.vel.x-=direction.y;
      player.vel.y+=direction.x;
      accel = 1;
   }

   RvR_fix22 vel_len = RvR_fix22_sqrt((player.vel.x*player.vel.x+player.vel.y*player.vel.y)/1024);
   player.vel_mag = vel_len;
   if(vel_len>196*64)
   {
      player.vel.x = (player.vel.x*196*64)/vel_len;
      player.vel.y = (player.vel.y*196*64)/vel_len;
      player.vel_mag  =196*64;
   }

   move_offset = player.vel;
   move_offset.x>>=6;
   move_offset.y>>=6;
   move_offset.x = (move_offset.x*860)/1024;
   move_offset.y = (move_offset.y*860)/1024;

   if(RvR_core_key_pressed(RVR_KEY_B))
      player.entity->pos.x = 1024;
   
   //Mouse look: x-axis
   if(x!=0)
      player.entity->direction+=(x*128)/128;

   //Shearing (fake looking up/down)
   //Drift back to 0
   if(!shearing&&player.shear!=0)
      player.shear = (player.shear>0)?(RvR_max(0,player.shear-CAMERA_SHEAR_STEP_FRAME)):(RvR_min(0,player.shear+CAMERA_SHEAR_STEP_FRAME));
   //Enable freelook
   if(RvR_core_key_pressed(RVR_KEY_F))
      shearing = !shearing;
   //Mouse look: y-axis
   if(y!=0&&shearing)
      player.shear = RvR_max(RvR_min(player.shear-(y*128)/128,CAMERA_SHEAR_MAX_PIXELS),-CAMERA_SHEAR_MAX_PIXELS);

   //Only for testing --> flying basically
   player.vertical_speed-=GRAVITY;
   //if(RvR_core_key_down(RVR_KEY_PGDN))
      //player.vertical_speed = -step*100;
   //else if(RvR_core_key_down(RVR_KEY_PGUP))
      //player.vertical_speed = step*100;

   //Jumping (hacky but works)
   if(RvR_core_key_pressed(RVR_KEY_SPACE)&&on_ground)
   {
      //sound_play(SOUND_PLAYER_JUMP,ai_index_get(player.entity),255);
      player.vertical_speed = JUMP_SPEED;
   }

   //Weapon switching
   if(RvR_core_key_pressed(RVR_KEY_1))
      player_weapon_switch(0);
   if(RvR_core_key_pressed(RVR_KEY_2))
      player_weapon_switch(1);
   if(RvR_core_key_pressed(RVR_KEY_3))
      player_weapon_switch(2);
   if(RvR_core_key_pressed(RVR_KEY_4))
      player_weapon_switch(3);
   player_weapon_rotate(RvR_core_mouse_wheel_scroll());

   if(player_weapon_ammo(player.weapon)==0)
      player_weapon_rotate(-1);

   //Cap player speed
   player.vertical_speed = RvR_max(-MAX_VERTICAL_SPEED,RvR_min(player.vertical_speed,MAX_VERTICAL_SPEED));
   move_offset.z = player.vertical_speed;
   //-------------------------------------

   //Collision
   RvR_fix22 floor_height = 0;
   RvR_fix22 ceiling_height = 0;
   collision_move(player.entity->collider,move_offset,&floor_height,&ceiling_height,&player.vel,on_ground);
   player.entity->pos = player.entity->collider->pos;
   //player.entity->pos = RvR_ray_get_position();
   on_ground = 0;

   //Reset verticall speed if ceiling was hit
   if(player.entity->pos.z+player.entity->collider->height>=ceiling_height)
      player.vertical_speed = 0;

   //Enable jumping if on ground
   if(player.entity->pos.z==floor_height)
   {
      on_ground = 1;
      player.vertical_speed = 0;
   }

   last_vertical_speed = player.vertical_speed;
   //-------------------------------------

   //Update raycasting values again
   //might be changed by collision
   RvR_vec3 rpos = player.entity->pos;
   rpos.z+=CAMERA_COLL_HEIGHT_BELOW;
   RvR_ray_set_position(rpos);
   RvR_ray_set_angle(player.entity->direction);
   RvR_ray_set_shear(player.shear);

   if(accel)
   {
      RvR_vec3 pos = RvR_ray_get_position();
      pos.z+=RvR_fix22_sin((4096*(game_tick-move_start_tick))/30)/48;
      RvR_ray_set_position(pos);
   }
   else
   {
      move_start_tick = game_tick;
   }
   //-------------------------------------

   /*direction.x/=8;
   direction.y/=8;
   int step = 1;
   RvR_vec3 move_offset = {0};*/

   //Forward/Backward movement
   /*if(RvR_core_key_down(config_move_forward))
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
      move_offset.x+=direction.y;
      move_offset.y-=direction.x;
   }
   else if(RvR_core_key_down(config_strafe_right))
   {
      move_offset.x-=direction.y;
      move_offset.y+=direction.x;
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
      player.shear = RvR_max(RvR_min(player.shear-(y*config_mouse_sensitivity_vertical)/128,CAMERA_SHEAR_MAX_PIXELS),-CAMERA_SHEAR_MAX_PIXELS);

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
   //-------------------------------------*/
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
