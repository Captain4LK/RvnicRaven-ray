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
RvR_stack_type(int16_t,port_stack_i16);
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
RvR_stack_function_prototype(int16_t,port_stack_i16,static);
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
   port_stack_i16_push(&to_visit,RvR_port_get_sector());

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
         int32_t x0 = wall0->x-RvR_port_get_position().x;
         int32_t y0 = wall0->y-RvR_port_get_position().y;
         int32_t x1 = wall1->x-RvR_port_get_position().x;
         int32_t y1 = wall1->y-RvR_port_get_position().y;

         int16_t portal = wall0->portal;
         if(portal>=0&&!(visited[portal/32]&(1<<(portal&31))))
         {
            //TODO: increase/decrease value/bounds check (-1024,1024)
            if((x0*y1-x1*y0)>-1024)
               port_stack_i16_push(&to_visit,wall0->portal);
         }

         if(i==0||(wall0-1)->p2!=i)
         {
            //TODO: cache
            RvR_fix22 cos = RvR_fix22_cos(RvR_ray_get_angle());
            RvR_fix22 sin = RvR_fix22_sin(RvR_ray_get_angle());

            to_point0.x = (x0*cos+y0*sin)/1024; 
            to_point0.y = (x0*sin-y0*cos)/1024; 
         }
         else
         {
            to_point0 = to_point1;
         }

         //TODO: cache
         RvR_fix22 cos = RvR_fix22_cos(RvR_ray_get_angle());
         RvR_fix22 sin = RvR_fix22_sin(RvR_ray_get_angle());

         to_point1.x = (x1*cos+y1*sin)/1024; 
         to_point1.y = (x1*sin-y1*cos)/1024; 
         
         if(to_point0.y<2048||to_point1.y<2048)
            goto add_wall;

         //Wedge product, again
         if((to_point0.x*to_point1.y)/1024-(to_point1.x*to_point0.y)/1024>=0)
            goto add_wall;


add_wall:
      }


      /*RvR_port_wall *wall0 = &map->walls[map->sectors[sector].first_wall];
      RvR_port_wall *wall1 = NULL;
      for(int i = map->sectors[sector].first_wall;i<map->sectors[sector].first_wall+map->sectors[sector].num_walls;i++,wall0++)
      {
         wall1 = &map->walls[wall0->p2];

         int32_t x0 = wall0->x-RvR_port_get_position().x;
         int32_t y0 = wall0->y-RvR_port_get_position().y;
         int32_t x1 = wall1->x-RvR_port_get_position().x;
         int32_t y1 = wall1->y-RvR_port_get_position().y;

         //Check if wall is portal
         if(wall0->portal>=0&&!(visited[wall0->portal/32]&(1<<(wall0->portal&31))))
         {
            //Check if wall is facing player
            //The wall is facing the player if the wedge product (explained in RvR_portal_sector.c) is greater (why? it should be smaller...) than 0
            //Additionally some tolerance is added to prevent some funky behaviour
            if((x0*y1-x1*y0)>-1024)
               port_stack_i16_push(&to_visit,wall0->portal);
         }

         //Rotate points to viewspace,
         //cache when possible
      }*/
   }
}

RvR_stack_function(int16_t,port_stack_i16,16,16,static);
//-------------------------------------
