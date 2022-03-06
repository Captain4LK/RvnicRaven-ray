/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs

typedef struct
{
   //Word coordinates
   RvR_fix22 w0_x;
   RvR_fix22 w1_x;
   RvR_fix22 w0_depth;
   RvR_fix22 w1_depth;

   //Drawing "coordinates"
   RvR_fix22 d0_x;
   RvR_fix22 d1_x;
   RvR_fix22 d0_depth;
   RvR_fix22 d1_depth;

   int next;
}port_potvis_wall;

RvR_stack_type(int16_t,port_stack_i16);
RvR_stack_type(port_potvis_wall,port_stack_potwall);
//-------------------------------------

//Variables
static port_stack_potwall port_potwall = {0};
//-------------------------------------

//Function prototypes
RvR_stack_function_prototype(int16_t,port_stack_i16,static);
RvR_stack_function_prototype(port_potvis_wall,port_stack_potwall,static);
//-------------------------------------

//Function implementations

void RvR_port_draw_2d()
{
}

void RvR_port_draw()
{
   RvR_port_map *map = RvR_port_map_get();

   //Look through all sectors and build potential visible set
   static port_stack_i16 to_visit = {0};
   static port_stack_i16 potvis_start = {0};
   static port_stack_i16 potvis_end = {0};
   uint32_t visited[(RVR_PORT_MAX_SECTORS+31)/32] = {0};

   port_stack_i16_clear(&to_visit);
   port_stack_i16_clear(&potvis_start);
   port_stack_i16_clear(&potvis_end);
   port_stack_potwall_clear(&port_potwall);
   port_stack_i16_push(&to_visit,RvR_port_get_sector());

   //TODO: cache?
   RvR_fix22 cos = RvR_fix22_cos(RvR_port_get_angle());
   RvR_fix22 sin = RvR_fix22_sin(RvR_port_get_angle());
   RvR_fix22 tan = RvR_fix22_tan(RvR_port_get_fov()/2);
   RvR_fix22 cos_fov = (cos*tan)/1024;
   RvR_fix22 sin_fov = (sin*tan)/1024;

   while(!port_stack_i16_empty(&to_visit))
   {
      int16_t sector = port_stack_i16_pop(&to_visit);

      visited[sector/32]|=1<<(sector&31);

      int potvis_start_used = potvis_start.data_used;

      RvR_port_wall *wall0 = &map->walls[map->sectors[sector].first_wall];
      RvR_port_wall *wall1 = NULL;
      RvR_vec2 to_point0 = {0};
      RvR_vec2 to_point1 = {0};
      for(int i = 0;i<map->sectors[sector].num_walls;i++,wall0++)
      {
         wall1 = &map->walls[wall0->p2];
         int32_t x0 = wall0->x-RvR_port_get_position().x;
         int32_t y0 = wall0->y-RvR_port_get_position().y;
         int32_t x1 = wall1->x-RvR_port_get_position().x;
         int32_t y1 = wall1->y-RvR_port_get_position().y;
         port_potvis_wall potwall = {0};

         //Rotate to camera space
         //x is depth
         //y is x [sic] coordinate
         if(i==0||(wall0-1)->p2!=i)
         {
            to_point0.x = (x0*cos_fov+y0*sin_fov)/1024; 
            to_point0.y = (x0*sin-y0*cos)/1024; 
         }
         else
         {
            to_point0 = to_point1;
         }
         to_point1.x = (x1*cos_fov+y1*sin_fov)/1024; 
         to_point1.y = (x1*sin-y1*cos)/1024; 
         
         //Wall fully behind camera 
         if(to_point0.x<128&&to_point1.x<128)
            goto skip;

         //Wall not facing camera (determined by winding order of sector walls)
         if(to_point0.x*to_point1.y-to_point1.x*to_point0.y>=0)
            goto skip;

         //Check if in fov
         //Left point in fov
         if(to_point0.x>to_point0.y)
         {
            //Wall completely out of sight
            if(-to_point0.x>to_point0.y)
               goto skip;

            potwall.d0_x = RvR_min(RVR_XRES/2+(to_point0.y*(RVR_XRES/2))/to_point0.x,RVR_XRES-1);
            potwall.d0_depth = to_point0.x;
         }
         //Left point to the left of fov
         else
         {
            //Wall completely out of sight
            if(to_point1.x<to_point1.y)
               goto skip;

            potwall.d0_x = 0;

         }

         //Right point in fov
         if(to_point1.x>-to_point1.y)
         {
            //Wall completely out of sight
            if(to_point1.x<to_point1.y)
               goto skip;

            potwall.d1_x = RvR_min(RVR_XRES/2+(to_point1.y*(RVR_XRES/2))/to_point1.x,RVR_XRES-1);
            potwall.d1_depth = to_point1.x;
         }
         //Right point to the right of fov
         else
         {
            //Wall completely out of sight
            if(-to_point0.x>to_point0.y)
               goto skip;

            potwall.d1_x = RVR_XRES-1;
         }

         //Near clip wall
         //if(potwall.d0_depth<128||potwall.d1_depth<128)
            //goto skip;

         RvR_draw_line(to_point0.x/128+RVR_XRES/2,to_point0.y/128+RVR_YRES/2,to_point1.x/128+RVR_XRES/2,to_point1.y/128+RVR_YRES/2,16);

         int16_t portal = wall0->portal;
         if(portal>=0&&!(visited[portal/32]&(1<<(portal&31))))
            port_stack_i16_push(&to_visit,wall0->portal);

         //Well, lets swap x and y, I'm sure this won't cause any problems later...
         potwall.w0_x = to_point0.y;
         potwall.w1_x = to_point1.y;
         potwall.w0_depth = to_point0.x;
         potwall.w1_depth = to_point1.x;
skip:
         continue;
      }
   }
}

RvR_stack_function(int16_t,port_stack_i16,16,16,static);
RvR_stack_function(port_potvis_wall,port_stack_potwall,16,16,static);
//-------------------------------------
