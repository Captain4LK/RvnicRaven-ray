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
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "color.h"
#include "map.h"
#include "editor.h"
#include "editor2d.h"
#include "editor3d.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Camera camera = {0};

static int editor_mode = 0;
//-------------------------------------

//Function prototypes
static void camera_update();
//-------------------------------------

//Function implementations

void editor_update()
{
   if(editor_mode==0)
      editor2d_update();
   else
      editor3d_update();

   if(RvR_core_key_pressed(RVR_KEY_ENTER))
      editor_mode = !editor_mode;

   camera_update();
}

void editor_draw()
{
   if(editor_mode==0)
      editor2d_draw();
   else
      editor3d_draw();
}

static void camera_update()
{
   RvR_vec2 direction = RvR_vec2_rot(camera.direction);
   direction.x/=8;
   direction.y/=8;
   int step = 1;
   RvR_vec3 move_offset = {0};

   //Forward/Backward movement
   if(RvR_core_key_down(RVR_KEY_UP))
   {
      move_offset.x+=step*direction.x;
      move_offset.y+=step*direction.y;
   }
   else if(RvR_core_key_down(RVR_KEY_DOWN))
   {
      move_offset.x-=step*direction.x;
      move_offset.y-=step*direction.y;
   }

   //Rotation
   if(RvR_core_key_down(RVR_KEY_LEFT))
      camera.direction+=16;
   else if(RvR_core_key_down(RVR_KEY_RIGHT))
      camera.direction-=16;
   camera.direction&=1023;

   //Gravity
   camera.vertical_speed-=16;
   camera.vertical_speed = RvR_max(-256,RvR_min(camera.vertical_speed,256));
   move_offset.z = camera.vertical_speed;

   //Update raycasting values
   //needed by collision
   RvR_ray_set_position(camera.pos);
   RvR_ray_set_angle(camera.direction);
   RvR_ray_set_shear(camera.shear);
   //-------------------------------------

   //Collision
   RvR_fix22 floor_height = 0;
   RvR_fix22 ceiling_height = 0;
   RvR_ray_move_with_collision(move_offset,1,1,&floor_height,&ceiling_height);
   camera.pos = RvR_ray_get_position();
   /*on_ground = 0;

   //Reset verticall speed if ceiling was hit
   if(player.entity->pos.z+CAMERA_COLL_HEIGHT_ABOVE==ceiling_height)
      player.vertical_speed = 0;

   //Enable jumping if on ground
   if(player.entity->pos.z-CAMERA_COLL_HEIGHT_BELOW==floor_height)
   {
      on_ground = 1;
      player.vertical_speed = 0;
   }

   last_vertical_speed = player.vertical_speed;*/
   //-------------------------------------

   //Update raycasting values again
   //might be changed by collision
   RvR_ray_set_position(camera.pos);
   RvR_ray_set_angle(camera.direction);
   RvR_ray_set_shear(camera.shear);
   //-------------------------------------
}
//-------------------------------------
