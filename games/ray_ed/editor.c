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
static void move_with_collision(RvR_vec3 offset, int8_t compute_height, int8_t compute_plane, RvR_fix22 *floor_height, RvR_fix22 *ceiling_height);
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
      camera.direction-=16;
   else if(RvR_core_key_down(RVR_KEY_RIGHT))
      camera.direction+=16;
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
   move_with_collision(move_offset,1,1,&floor_height,&ceiling_height);

   //Reset vertical speed if floor was hit
   if(camera.pos.z-CAMERA_COLL_HEIGHT_BELOW==floor_height)
      camera.vertical_speed = 0;
   //-------------------------------------

   //Update raycasting values again
   //might be changed by collision
   RvR_ray_set_position(camera.pos);
   RvR_ray_set_angle(camera.direction);
   RvR_ray_set_shear(camera.shear);
   //-------------------------------------
}

static void move_with_collision(RvR_vec3 offset, int8_t compute_height, int8_t compute_plane, RvR_fix22 *floor_height, RvR_fix22 *ceiling_height)
{
   int8_t moves_in_plane = offset.x!=0||offset.y!=0;

   if(compute_plane)
   {
      int16_t x_square_new, y_square_new;

      RvR_vec2 corner; // BBox corner in the movement direction
      RvR_vec2 corner_new;

      int16_t x_dir = offset.x>0?1:-1;
      int16_t y_dir = offset.y>0?1:-1;

      corner.x = camera.pos.x+x_dir*CAMERA_COLL_RADIUS;
      corner.y = camera.pos.y+y_dir*CAMERA_COLL_RADIUS;

      int16_t x_square = RvR_div_round_down(corner.x,1024);
      int16_t y_square = RvR_div_round_down(corner.y,1024);

      corner_new.x = corner.x+offset.x;
      corner_new.y = corner.y+offset.y;

      x_square_new = RvR_div_round_down(corner_new.x,1024);
      y_square_new = RvR_div_round_down(corner_new.y,1024);

      RvR_fix22 bottom_limit = -RvR_fix22_infinity;
      RvR_fix22 top_limit = RvR_fix22_infinity;

      RvR_fix22 curr_ceil_height = RvR_fix22_infinity;

      if(compute_height)
      {
         bottom_limit = camera.pos.z-CAMERA_COLL_HEIGHT_BELOW+CAMERA_COLL_STEP_HEIGHT;

         top_limit = camera.pos.z+CAMERA_COLL_HEIGHT_ABOVE;

         curr_ceil_height = RvR_ray_map_ceiling_height_at(x_square,y_square);
      }

      // checks a single square for collision against the camera
#define collCheck(dir,s1,s2)\
      if(compute_height)\
      {\
         RvR_fix22 height = RvR_ray_map_floor_height_at(s1,s2);\
         if(height>bottom_limit||\
         curr_ceil_height-height<\
         CAMERA_COLL_HEIGHT_BELOW+CAMERA_COLL_HEIGHT_ABOVE)\
            dir##_collides = 1;\
         else\
         {\
            RvR_fix22 height2 = RvR_ray_map_ceiling_height_at(s1,s2);\
            if((height2<top_limit)||((height2-height)<\
            (CAMERA_COLL_HEIGHT_ABOVE+CAMERA_COLL_HEIGHT_BELOW)))\
               dir##_collides = 1;\
         }\
      }\
      else\
         dir##_collides = RvR_ray_map_floor_height_at(s1,s2)>CAMERA_COLL_STEP_HEIGHT;

      // check collision against non-diagonal square
#define collCheckOrtho(dir,dir2,s1,s2,x)\
      if (dir##_square_new != dir##_square)\
      {\
         collCheck(dir,s1,s2)\
      }\
      if(!dir##_collides)\
      { /* now also check for coll on the neighbouring square */ \
         int16_t dir2##_square2 = RvR_div_round_down(corner.dir2-dir2##_dir *\
         CAMERA_COLL_RADIUS*2,1024);\
         if(dir2##_square2!=dir2##_square)\
         {\
            if(x)\
               collCheck(dir,dir##_square_new,dir2##_square2)\
            else\
               collCheck(dir,dir2##_square2,dir##_square_new)\
         }\
      }

      int8_t x_collides = 0;
      collCheckOrtho(x,y,x_square_new,y_square,1)

      int8_t y_collides = 0;
      collCheckOrtho(y,x,x_square,y_square_new,0)

      if(x_collides||y_collides)
      {
         if(moves_in_plane)
         {
            #define collHandle(dir)\
            if (dir##_collides)\
               corner_new.dir = (dir##_square)*1024+\
               1024/2+dir##_dir*(1024/2)-\
               dir##_dir;\

            collHandle(x)
            collHandle(y)

            #undef collHandle
         }
         else
         {
            /* Player collides without moving in the plane; this can happen e.g. on
            elevators due to vertical only movement. This code can get executed
            when force == 1. */

            RvR_vec2 square_pos;
            RvR_vec2 new_pos;

            square_pos.x = x_square*1024;
            square_pos.y = y_square*1024;

            new_pos.x = RvR_max(square_pos.x+CAMERA_COLL_RADIUS+1,RvR_min(square_pos.x+1024-CAMERA_COLL_RADIUS-1,camera.pos.x));
            new_pos.y = RvR_max(square_pos.y+CAMERA_COLL_RADIUS+1,RvR_min(square_pos.y+1024-CAMERA_COLL_RADIUS-1,camera.pos.y));

            corner_new.x = corner.x+(new_pos.x-camera.pos.x);
            corner_new.y = corner.y+(new_pos.y-camera.pos.y);
         }
      }
      else 
      {
         /* If no non-diagonal collision is detected, a diagonal/corner collision
         can still happen, check it here. */

         if(x_square!=x_square_new&&y_square!=y_square_new)
         {
            int8_t xy_collides = 0;
            collCheck(xy,x_square_new,y_square_new)

            if (xy_collides)
            {
               corner_new = corner;
            }
         }
      }

#undef collCheck

      camera.pos.x = corner_new.x-x_dir*CAMERA_COLL_RADIUS;
      camera.pos.y = corner_new.y-y_dir*CAMERA_COLL_RADIUS;  
   }

   if(compute_height&&(moves_in_plane||(offset.z!=0)||1))
   {
      camera.pos.z+=offset.z;

      int16_t x_square1 = RvR_div_round_down(camera.pos.x-CAMERA_COLL_RADIUS,1024);

      int16_t x_square2 = RvR_div_round_down(camera.pos.x+CAMERA_COLL_RADIUS,1024);

      int16_t y_square1 = RvR_div_round_down(camera.pos.y-CAMERA_COLL_RADIUS,1024);

      int16_t y_square2 = RvR_div_round_down(camera.pos.y+CAMERA_COLL_RADIUS,1024);

      RvR_fix22 bottom_limit = RvR_ray_map_floor_height_at(x_square1,y_square1);
      RvR_fix22 top_limit = RvR_ray_map_ceiling_height_at(x_square1,y_square1);

      RvR_fix22 height;

#define checkSquares(s1,s2)\
      {\
         height = RvR_ray_map_floor_height_at(x_square##s1,y_square##s2);\
         bottom_limit = RvR_max(bottom_limit,height);\
         height = RvR_ray_map_ceiling_height_at(x_square##s1,y_square##s2);\
         top_limit = RvR_min(top_limit,height);\
      }

      if(x_square2!=x_square1)
         checkSquares(2,1)

      if(y_square2!=y_square1)
         checkSquares(1,2)

      if(x_square2!=x_square1&&y_square2!=y_square1)
         checkSquares(2,2)

      if(floor_height!=NULL)
         *floor_height = bottom_limit;
      if(ceiling_height!=NULL)
         *ceiling_height = top_limit;

      camera.pos.z = RvR_clamp(camera.pos.z,bottom_limit+CAMERA_COLL_HEIGHT_BELOW,top_limit-CAMERA_COLL_HEIGHT_ABOVE);

#undef checkSquares
   }
}
//-------------------------------------
