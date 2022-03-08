/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdlib.h>
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

   //Corresponding sector/wall
   int16_t wall;
   int16_t sector;

   int16_t next;
}port_potvis_wall;

typedef struct
{
   int16_t start;
   int16_t end;
}port_potvis_element;

RvR_stack_type(int16_t,port_stack_i16);
RvR_stack_type(port_potvis_wall,port_stack_potwall);
RvR_stack_type(port_potvis_element,port_stack_potvis);
//-------------------------------------

//Variables
static port_stack_potwall port_potwall = {0};
static port_stack_potvis port_potvis = {0};
//-------------------------------------

//Function prototypes
RvR_stack_function_prototype(int16_t,port_stack_i16,static);
RvR_stack_function_prototype(port_potvis_wall,port_stack_potwall,static);
RvR_stack_function_prototype(port_potvis_element,port_stack_potvis,static);

static int port_comp_wall(const void *a, const void *b);
//-------------------------------------

//Function implementations

void RvR_port_draw_2d()
{
}

void RvR_port_draw()
{
   RvR_port_map *map = RvR_port_map_get();

   //-------------------------------------
   //Look through all sectors and build potential visible set
   static port_stack_i16 to_visit = {0};
   static port_stack_i16 potwall_sort = {0};
   uint32_t visited[(RVR_PORT_MAX_SECTORS+31)/32] = {0};

   //TODO: cache?
   RvR_fix22 cos = RvR_fix22_cos(RvR_port_get_angle());
   RvR_fix22 sin = RvR_fix22_sin(RvR_port_get_angle());
   RvR_fix22 tan = RvR_fix22_tan(RvR_port_get_fov()/2);
   RvR_fix22 cos_fov = (cos*tan)/1024;
   RvR_fix22 sin_fov = (sin*tan)/1024;

   port_stack_i16_clear(&to_visit);
   port_stack_i16_clear(&potwall_sort);
   port_stack_potwall_clear(&port_potwall);
   port_stack_potvis_clear(&port_potvis);

   port_stack_i16_push(&to_visit,RvR_port_get_sector());
   while(!port_stack_i16_empty(&to_visit))
   {
      int16_t sector = port_stack_i16_pop(&to_visit);

      visited[sector/32]|=1<<(sector&31);

      int potwall_start_used = port_potwall.data_used;
      int potwall_first = port_potwall.data_used;

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

            potwall.d1_x = RvR_min(RVR_XRES/2+(to_point0.y*(RVR_XRES/2))/to_point0.x,RVR_XRES-1);
            potwall.d1_depth = to_point0.x;
         }
         //Left point to the left of fov
         else
         {
            //Wall completely out of sight
            if(to_point1.x<to_point1.y)
               goto skip;

            potwall.d1_x = RVR_XRES-1;
            RvR_fix22 wall_dx = (to_point1.y-to_point0.y)+(to_point0.x-to_point1.x);
            if(wall_dx==0)
               goto skip;
            potwall.d1_depth = to_point0.x+((to_point1.x-to_point0.x)*(to_point0.x-to_point0.y))/wall_dx;
         }

         //Right point in fov
         if(to_point1.x>-to_point1.y)
         {
            //Wall completely out of sight
            if(to_point1.x<to_point1.y)
               goto skip;

            potwall.d0_x = RvR_min(RVR_XRES/2+(to_point1.y*(RVR_XRES/2))/to_point1.x,RVR_XRES-1)-1;
            potwall.d0_depth = to_point1.x;
         }
         //Right point to the right of fov
         else
         {
            //Wall completely out of sight
            if(-to_point0.x>to_point0.y)
               goto skip;

            potwall.d0_x = 0;
            RvR_fix22 wall_dx = (to_point0.x-to_point1.x)+(to_point0.y-to_point1.y);
            if(wall_dx==0)
               goto skip;
            potwall.d0_depth = to_point0.x+((to_point1.x-to_point0.x)*(to_point0.x+to_point0.y))/wall_dx;
         }

         //Near clip wall
         if(potwall.d0_depth<128||potwall.d1_depth<128||potwall.d0_x>=potwall.d1_x)
            goto skip;

         RvR_draw_line(to_point0.x/128+RVR_XRES/2,to_point0.y/128+RVR_YRES/2,to_point1.x/128+RVR_XRES/2,to_point1.y/128+RVR_YRES/2,16);

         int16_t portal = wall0->portal;
         if(portal>=0&&!(visited[portal/32]&(1<<(portal&31))))
            port_stack_i16_push(&to_visit,wall0->portal);

         //Well, lets swap x and y, I'm sure this won't cause any problems later...
         potwall.w0_x = to_point0.y;
         potwall.w1_x = to_point1.y;
         potwall.w0_depth = to_point0.x;
         potwall.w1_depth = to_point1.x;
         potwall.wall = i+map->sectors[sector].first_wall;
         potwall.sector = sector;
         potwall.next = port_potwall.data_used+1;
         port_stack_potwall_push(&port_potwall,potwall);
         port_stack_i16_push(&potwall_sort,potwall_sort.data_used);
skip:

         if(wall0->p2<i+map->sectors[sector].first_wall&&potwall_first<port_potwall.data_used)
         {
            port_potwall.data[port_potwall.data_used-1].next = potwall_first;
            potwall_first = port_potwall.data_used;
         }
      }
      
      int port_potvis_used = port_potvis.data_used;
      for(int i = potwall_start_used;i<port_potwall.data_used;i++)
      {
         port_potvis_wall *wl = &port_potwall.data[i];
         if(map->walls[wl->wall].p2!=port_potwall.data[wl->next].wall||wl->d1_x>=port_potwall.data[wl->next].d0_x)
         {
            port_stack_potvis_push(&port_potvis,(port_potvis_element){.start = wl->next});
            wl->next = -1;
         }
      }

      for(int i = port_potvis_used;i<port_potvis.data_used;i++)
      {
         //Find last wall in potvis set
         int j = 0;
         for(j = port_potvis.data[i].start;port_potwall.data[j].next>=0;j = port_potwall.data[j].next);
         port_potvis.data[i].end = j;
      }
   }
   //-------------------------------------

   //Sort and draw potvis walls
   int16_t start[RVR_XRES] = {0};
   int16_t end[RVR_XRES];
   for(int i = 0;i<RVR_XRES;i++) end[i] = RVR_YRES;
   qsort(potwall_sort.data,potwall_sort.data_used,sizeof(*potwall_sort.data),port_comp_wall);
   for(int i = 0;i<potwall_sort.data_used;i++)
   {
      port_potvis_wall *wall = &port_potwall.data[potwall_sort.data[i]];
      int height0 = (RVR_YRES*1024)/wall->d0_depth;
      int height1 = (RVR_YRES*1024)/wall->d1_depth;
      RvR_fix22 width = (wall->d1_x-wall->d0_x);
      RvR_fix22 step_y = ((RVR_YRES/2-height1)*1024-(RVR_YRES/2-height0)*1024)/width;
      RvR_fix22 step_h = (height1*1024*2-height0*1024*2)/width;
      RvR_fix22 y = (RVR_YRES/2-height0)*1024;
      RvR_fix22 h = height0*1024*2;

      for(int x = wall->d0_x;x<wall->d1_x;x++)
      {
         if(map->walls[wall->wall].portal<0)
         {
            RvR_draw_vertical_line(x,y/1024,(y+h)/1024,i+2);
            RvR_draw_vertical_line(x,0,y/1024,wall->sector+128);
            RvR_draw_vertical_line(x,(y+h)/1024,RVR_YRES-1,wall->sector+129);
         }
         else
         {
         }
         y+=step_y;
         h+=step_h;
      }
   }
   //-------------------------------------
}

static int port_comp_wall(const void *a, const void *b)
{
   RvR_port_map *map = RvR_port_map_get();
   port_potvis_wall *wa = &port_potwall.data[*((int16_t *)a)];
   port_potvis_wall *wb = &port_potwall.data[*((int16_t *)b)];

   if(wa->d0_x>=wb->d1_x)
      return 0;
   if(wa->d1_x<wb->d0_x)
      return 0;

   RvR_fix22 x00 = map->walls[wa->wall].x;
   RvR_fix22 y00 = map->walls[wa->wall].y;
   RvR_fix22 x01 = map->walls[map->walls[wa->wall].p2].x;
   RvR_fix22 y01 = map->walls[map->walls[wa->wall].p2].y;
   RvR_fix22 x10 = map->walls[wb->wall].x;
   RvR_fix22 y10 = map->walls[wb->wall].y;
   RvR_fix22 x11 = map->walls[map->walls[wb->wall].p2].x;
   RvR_fix22 y11 = map->walls[map->walls[wb->wall].p2].y;

   RvR_fix22 dx = x01-x00;
   RvR_fix22 dy = y01-y00;
   RvR_fix22 t0 = ((x10-x00)*dy-(y10-y00)*dx)/1024;
   RvR_fix22 t1 = ((x11-x00)*dy-(y11-y00)*dx)/1024;
   if(t0==0)
   {
      if(t1==0)
         return 0;
      t0 = t1;
   }
   else if(t1==0)
   {
      t1 = t0;
   }

   if(RvR_sign_equal(t0,t1))
   {
      t1 = ((RvR_port_get_position().x-x00)*dy-(RvR_port_get_position().y-y00)*dx)/1024;
      return RvR_sign_equal(t0,t1)?-1:1;
   }

   dx = x11-x10;
   dy = y11-y10;
   t0 = ((x00-x10)*dy-(y00-y10)*dx)/1024;
   t1 = ((x01-x10)*dy-(y01-y10)*dx)/1024;
   if(t0==0)
   {
      if(t1==0)
         return 0;
      t0 = t1;
   }
   else if(t1==0)
   {
      t1 = t0;
   }

   if(RvR_sign_equal(t0,t1))
   {
      t1 = ((RvR_port_get_position().x-x10)*dy-(RvR_port_get_position().y-y10)*dx)/1024;
      return (!RvR_sign_equal(t0,t1))?-1:1;
   }

   return 0;
}

RvR_stack_function(int16_t,port_stack_i16,16,16,static);
RvR_stack_function(port_potvis_wall,port_stack_potwall,16,16,static);
RvR_stack_function(port_potvis_element,port_stack_potvis,16,16,static);
//-------------------------------------
