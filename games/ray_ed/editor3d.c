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
#include "editor3d.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void mouse_world_pos(int mx, int my, int16_t *x, int16_t *y, int *location);
//-------------------------------------

//Function implementations

void editor3d_update()
{
   camera_update();

   //Get real world tile position of mouse
   int mx,my;
   int16_t wx, wy;
   int location = 0;
   RvR_core_mouse_pos(&mx,&my);
   mouse_world_pos(mx,my,&wx,&wy,&location);

   if(RvR_core_key_pressed(RVR_KEY_PGUP))
   {
      if(location==0||location==2)
         RvR_ray_map_floor_height_set(wx,wy,RvR_ray_map_floor_height_at(wx,wy)+128);
      if(location==1||location==3)
         RvR_ray_map_ceiling_height_set(wx,wy,RvR_ray_map_ceiling_height_at(wx,wy)+128);
   }
   else if(RvR_core_key_pressed(RVR_KEY_PGDN))
   {
      if(location==0||location==2)
         RvR_ray_map_floor_height_set(wx,wy,RvR_ray_map_floor_height_at(wx,wy)-128);
      if(location==1||location==3)
         RvR_ray_map_ceiling_height_set(wx,wy,RvR_ray_map_ceiling_height_at(wx,wy)-128);
   }
}

void editor3d_draw()
{
   RvR_ray_draw();

   //Draw cursor
   int mx,my;
   RvR_core_mouse_pos(&mx,&my);
   RvR_draw_horizontal_line(mx-4,mx-1,my,color_magenta);
   RvR_draw_horizontal_line(mx+1,mx+4,my,color_magenta);
   RvR_draw_vertical_line(mx,my-1,my-4,color_magenta);
   RvR_draw_vertical_line(mx,my+1,my+4,color_magenta);
}

static void mouse_world_pos(int mx, int my, int16_t *x, int16_t *y, int *location)
{
   if(my<0||my>=RVR_YRES||mx<0||mx>=RVR_XRES)
      return;

   RvR_vec2 dir0 = RvR_vec2_rot(RvR_ray_get_angle()-(RVR_RAY_HORIZONTAL_FOV/2));
   RvR_vec2 dir1 = RvR_vec2_rot(RvR_ray_get_angle()+(RVR_RAY_HORIZONTAL_FOV/2));
   RvR_fix22 ray_start_floor_height = RvR_ray_map_floor_height_at(RvR_div_round_down(RvR_ray_get_position().x,1024),RvR_div_round_down(RvR_ray_get_position().y,1024))-1*RvR_ray_get_position().z;
   RvR_fix22 ray_start_ceil_height = RvR_ray_map_ceiling_height_at(RvR_div_round_down(RvR_ray_get_position().x,1024),RvR_div_round_down(RvR_ray_get_position().y,1024))-1*RvR_ray_get_position().z;
   int32_t ray_middle_row = (RVR_YRES/2)+RvR_ray_get_shear();
   RvR_fix22 cos = RvR_non_zero(RvR_fix22_cos(RVR_RAY_HORIZONTAL_FOV/2));
   dir0.x = (dir0.x*1024)/cos;
   dir0.y = (dir0.y*1024)/cos;
   dir1.x = (dir1.x*1024)/cos;
   dir1.y = (dir1.y*1024)/cos;

   RvR_fix22 dx = dir1.x-dir0.x;
   RvR_fix22 dy = dir1.y-dir0.y;

   RvR_ray_hit_result hits[RVR_RAY_MAX_STEPS] = {0};
   uint16_t hit_count = 0;

   RvR_ray r;
   r.start = (RvR_vec2) {RvR_ray_get_position().x,RvR_ray_get_position().y};

   RvR_fix22 current_dx = 0;
   RvR_fix22 current_dy = 0;
   current_dx+=dx*mx;
   current_dy+=dy*mx;

   r.direction.x = dir0.x+(current_dx/RVR_XRES);
   r.direction.y = dir0.y+(current_dy/RVR_XRES);
   
   RvR_ray_cast_multi_hit(r,hits,&hit_count);

   //last written Y position, can never go backwards
   RvR_fix22 f_pos_y = RVR_YRES;
   RvR_fix22 c_pos_y = -1;

   //world coordinates (relative to camera height though)
   RvR_fix22 f_z1_world = ray_start_floor_height;
   RvR_fix22 c_z1_world = ray_start_ceil_height;

   int start = 0;
   int end = 0;
   const int direction_modx[4] = {0,1,0,-1};
   const int direction_mody[4] = {1,0,-1,0};

   //we'll be simulatenously drawing the floor and the ceiling now  
   for(RvR_fix22 j = 0;j<RVR_RAY_MAX_STEPS;++j)
   {                    //^ = add extra iteration for horizon plane
      int8_t drawing_horizon = j==(RVR_RAY_MAX_STEPS-1);

      RvR_ray_hit_result hit;
      RvR_fix22 distance = 1;

      RvR_fix22 f_wall_height = 0, c_wall_height = 0;
      RvR_fix22 f_z2_world = 0,    c_z2_world = 0;
      RvR_fix22 f_z1_screen = 0,   c_z1_screen = 0;
      RvR_fix22 f_z2_screen = 0,   c_z2_screen = 0;

      if(!drawing_horizon)
      {
         hit = hits[j];
         distance = RvR_non_zero(hit.distance); 
         //p.hit = hit;

         f_wall_height = RvR_ray_map_floor_height_at(hit.square.x,hit.square.y);
         f_z2_world = f_wall_height-RvR_ray_get_position().z;
         f_z1_screen = ray_middle_row-RvR_ray_perspective_scale_vertical((f_z1_world*RVR_YRES)/1024,distance);
         f_z2_screen = ray_middle_row-RvR_ray_perspective_scale_vertical((f_z2_world*RVR_YRES)/1024,distance);

         c_wall_height = RvR_ray_map_ceiling_height_at(hit.square.x,hit.square.y);
         c_z2_world = c_wall_height-RvR_ray_get_position().z;
         c_z1_screen = ray_middle_row-RvR_ray_perspective_scale_vertical((c_z1_world*RVR_YRES)/1024,distance);
         c_z2_screen = ray_middle_row-RvR_ray_perspective_scale_vertical((c_z2_world*RVR_YRES)/1024,distance);
      }
      else
      {
         f_z1_screen = ray_middle_row;
         c_z1_screen = ray_middle_row+1;
         hit.square.x = -1;
         hit.square.y = -1;
         hit.direction = 0;
      }

      RvR_fix22 limit;

      //floor until wall
      limit = RvR_clamp(f_z1_screen,c_pos_y+1,RVR_YRES);
      start = limit;
      end = f_pos_y-1;
      if(my>=start&&my<=end)
      {
         *x = hit.square.x+direction_modx[hit.direction];
         *y = hit.square.y+direction_mody[hit.direction];
         *location = 0;

         return;
      }

      if(f_pos_y>limit)
         f_pos_y = limit;

      //ceiling until wall
      limit = RvR_clamp(c_z1_screen,-1,f_pos_y-1);
      start = c_pos_y+1;
      end = limit;
      if(my>=start&&my<=end)
      {
         *x = hit.square.x+direction_modx[hit.direction];
         *y = hit.square.y+direction_mody[hit.direction];
         *location = 1;

         return;
      }

      if(c_pos_y<limit)
         c_pos_y = limit;

      if(!drawing_horizon) //don't draw walls for horizon plane
      {
         //floor wall
         if(f_pos_y>0&&f_z1_world!=f_z2_world)  //still pixels left?
         {
            limit = RvR_clamp(f_z2_screen,c_pos_y+1,RVR_YRES);
            start = limit;
            end = f_pos_y-1;
            if(my>=start&&my<=end)
            {
               *x = hit.square.x;
               *y = hit.square.y;
               *location = 2;

               return;
            }

            if(f_pos_y>limit)
               f_pos_y = limit;

            f_z1_world = f_z2_world; //for the next iteration
         }               //^ purposfully allow outside screen bounds here

         //draw ceiling wall
         if(c_pos_y<RVR_YRES-1&&c_z1_world!=c_z2_world) //pixels left?
         {
            limit = RvR_clamp(c_z2_screen,-1,f_pos_y-1);
            start = c_pos_y+1;
            end = limit;
            if(my>=start&&my<=end)
            {
               *x = hit.square.x;
               *y = hit.square.y;
               *location = 3;

               return;
            }

            if(c_pos_y<limit)
               c_pos_y = limit;

            c_z1_world = c_z2_world; //for the next iteration
         }              //^ puposfully allow outside screen bounds here 
      }
   }
}
//-------------------------------------
