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
}port_potwall_element;

typedef struct
{
   int16_t start;
   int16_t end;
}port_potvis_element;

RvR_stack_type(int16_t,port_stack_i16);
RvR_stack_type(port_potwall_element,port_stack_potwall);
RvR_stack_type(port_potvis_element,port_stack_potvis);
//-------------------------------------

//Variables
static port_stack_potwall port_potwall = {0};
static port_stack_potvis port_potvis = {0};
static int32_t port_middle_row = 0;

static int_fast16_t port_ymin[RVR_XRES];
static int_fast16_t port_ymax[RVR_XRES];
//-------------------------------------

//Function prototypes
RvR_stack_function_prototype(int16_t,port_stack_i16,static);
RvR_stack_function_prototype(port_potwall_element,port_stack_potwall,static);
RvR_stack_function_prototype(port_potvis_element,port_stack_potvis,static);

static void port_wall_draw(int wall_num);

static int port_potvis_order(int16_t va, int16_t vb);
static int port_wall_order(int16_t a, int16_t b);
//-------------------------------------

//Function implementations

void RvR_port_draw_2d()
{
}

void RvR_port_draw()
{
   RvR_port_map *map = RvR_port_map_get();
   port_middle_row = (RVR_YRES/2)+RvR_port_get_shear();

   //Clear occlusion arrays
   memset(port_ymin,0,sizeof(port_ymin));
   for(int i = 0;i<RVR_XRES;i++) port_ymax[i] = RVR_YRES-1;

   //-------------------------------------
   //Look through all sectors and build potential visible set
   static port_stack_i16 to_visit = {0};
   uint32_t visited[(RVR_PORT_MAX_SECTORS+31)/32] = {0};

   //TODO: cache?
   RvR_fix22 cos = RvR_fix22_cos(RvR_port_get_angle());
   RvR_fix22 sin = RvR_fix22_sin(RvR_port_get_angle());
   RvR_fix22 tan = RvR_fix22_tan(RvR_port_get_fov()/2);
   RvR_fix22 cos_fov = (cos*tan)/1024;
   RvR_fix22 sin_fov = (sin*tan)/1024;

   port_stack_i16_clear(&to_visit);
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
         port_potwall_element potwall = {0};

         //Rotate to camera space
         //y is depth
         //x is x coordinate
         if(i==0||(wall0-1)->p2!=i+map->sectors[sector].first_wall)
         {
            to_point0.x = (-x0*sin+y0*cos)/1024; 
            to_point0.y = (x0*cos_fov+y0*sin_fov)/1024; 
         }
         else
         {
            to_point0 = to_point1;
         }
         to_point1.x = (-x1*sin+y1*cos)/1024; 
         to_point1.y = (x1*cos_fov+y1*sin_fov)/1024; 
         
         //Wall fully behind camera 
         if(to_point0.y<-128&&to_point1.y<-128)
            goto skip;

         //Wall not facing camera (determined by winding order of sector walls)
         //Wall points MUST be ordered left to right when the wall is
         //facing the camera
         if((to_point0.x*to_point1.y-to_point1.x*to_point0.y)/65536>0)
            goto skip;

         //Here we can treat everything as if we have a 90 degree
         //fov, since the rotation to camera space transforms it to
         //that
         //Check if in fov
         //Left point in fov
         if(to_point0.x>=-to_point0.y)
         {
            //Wall completely out of sight
            if(to_point0.x>to_point0.y)
               goto skip;

            potwall.d0_x = RvR_min(RVR_XRES/2+(to_point0.x*(RVR_XRES/2))/to_point0.y,RVR_XRES-1);
            potwall.d0_depth = to_point0.y;
         }
         //Left point to the left of fov
         else
         {
            //Wall completely out of sight
            if(to_point1.x<-to_point1.y)
               goto skip;

            potwall.d0_x = 0;

            //Basically just this equation: (0,0)+n(-1,1) = (to_point0.x,to_point0.y)+m(to_point1.x-to_point0.x,to_point1.y-to_point0.y), reordered to n = ...
            //This is here to circumvent a multiplication overflow while also minimizing the resulting error
            //TODO: the first version might be universally better, making the if statement useless
            RvR_fix22 dx0 = to_point1.x-to_point0.x;
            RvR_fix22 dx1 = to_point0.y+to_point0.x;
            if(RvR_abs(dx0)>RvR_abs(dx1))
               potwall.d0_depth = (dx1*((dx0*1024)/RvR_non_zero(to_point1.y-to_point0.y+to_point1.x-to_point0.x)))/1024-to_point0.x;
            else
               potwall.d0_depth = (dx0*((dx1*1024)/RvR_non_zero(to_point1.y-to_point0.y+to_point1.x-to_point0.x)))/1024-to_point0.x;
         }

         //Right point in fov
         if(to_point1.x<=to_point1.y)
         {
            //Wall completely out of sight
            if(to_point1.x<-to_point1.y)
               goto skip;

            potwall.d1_x = RvR_min(RVR_XRES/2+(to_point1.x*(RVR_XRES/2))/to_point1.y-1,RVR_XRES-1);
            potwall.d1_depth = to_point1.y;
         }
         //Right point to the right of fov
         else
         {
            //Wall completely out of sight
            if(to_point0.x>to_point0.y)
               goto skip;

            potwall.d1_x = RVR_XRES-1;

            //Basically just this equation: (0,0)+n(1,1) = (to_point0.x,to_point0.y)+m(to_point1.x-to_point0.x,to_point1.y-to_point0.y), reordered to n = ...
            //This is here to circumvent a multiplication overflow while also minimizing the resulting error
            //TODO: the first version might be universally better, making the if statement useless
            RvR_fix22 dx0 = to_point1.x-to_point0.x;
            RvR_fix22 dx1 = to_point0.y-to_point0.x;
            if(RvR_abs(dx0)>RvR_abs(dx1))
               potwall.d1_depth = to_point0.x-(dx1*((dx0*1024)/RvR_non_zero(to_point1.y-to_point0.y-to_point1.x+to_point0.x)))/1024;
            else
               potwall.d1_depth = to_point0.x-(dx0*((dx1*1024)/RvR_non_zero(to_point1.y-to_point0.y-to_point1.x+to_point0.x)))/1024;
         }

         //Near clip wall
         //Special case, near clipped portals still need to be processed
         if(potwall.d0_depth<16||potwall.d1_depth<16)
            goto skip_near;

         if(potwall.d0_x>potwall.d1_x)
            goto skip;

         potwall.w0_x = to_point0.x;
         potwall.w1_x = to_point1.x;
         potwall.w0_depth = to_point0.y;
         potwall.w1_depth = to_point1.y;
         potwall.wall = i+map->sectors[sector].first_wall;
         potwall.sector = sector;
         potwall.next = port_potwall.data_used+1;
         port_stack_potwall_push(&port_potwall,potwall);

skip_near:

         //Wrapped in block to avoid compiler warning
         //Could also move the portal declaration somewhere else, though
         {
         int16_t portal = wall0->portal;
         if(portal>=0&&!(visited[portal/32]&(1<<(portal&31))))
            port_stack_i16_push(&to_visit,wall0->portal);
         }

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
         port_potwall_element *wl = &port_potwall.data[i];

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

   if(RvR_core_key_pressed(RVR_KEY_P))
   {
      for(int i = 0;i<port_potvis.data_used;i++)
      {
         printf("Potvis %d: ",i);
         for(int j = port_potvis.data[i].start;j>=0;j = port_potwall.data[j].next)
            printf("%d ",port_potwall.data[j].wall);
         puts("");
      }
   }

   //Sort and draw potvis walls
   while(!port_stack_potvis_empty(&port_potvis))
   {
      uint8_t certain[port_potvis.data_used];
      memset(certain,0,sizeof(*certain)*port_potvis.data_used);
      int16_t near = 0;
      certain[near] = 1;
      int done = 0;

      do
      {
         done = 1;
         for(int i = 0;i<port_potvis.data_used;i++)
         {
            if(certain[i])
               continue;

            int order = port_potvis_order(i,near);
            if(order<0)
               continue;

            certain[i] = 1;
            if(order)
            {
               done = 0;
               near = i;
               certain[near] = 1;
            }
         }
      }while(!done);

      for(int i = port_potvis.data[near].start;i>=0;i = port_potwall.data[i].next)
         port_wall_draw(i);

      port_potvis.data_used--;
      port_potvis.data[near] = port_potvis.data[port_potvis.data_used];
   }
   /*for(int i = 0;i<potwall_sort.data_used;i++)
   {
      port_potvis_wall *wall = &port_potwall.data[potwall_sort.data[i]];
      RvR_draw_line(wall->w0_depth/128+RVR_XRES/2,wall->w0_x/128+RVR_YRES/2,wall->w1_depth/128+RVR_XRES/2,wall->w1_x/128+RVR_YRES/2,16);
   }*/
   //-------------------------------------
}

static void port_wall_draw(int wall_num)
{
   RvR_port_map *map = RvR_port_map_get();
   port_potwall_element *wall = &port_potwall.data[wall_num];
   RvR_fix22 width = (wall->d1_x-wall->d0_x)+1;
   int16_t portal = map->walls[wall->wall].portal;

   //Normal wall
   if(portal<0)
   {
      //Depth
      RvR_fix22 step_depth = (wall->d1_depth-wall->d1_depth)/width;
      RvR_fix22 depth = wall->d0_depth;

      //Ceiling
      RvR_fix22 cy0 = (RVR_YRES*(map->sectors[wall->sector].ceiling_height-RvR_port_get_position().z)*1024)/wall->d0_depth;
      RvR_fix22 cy1 = (RVR_YRES*(map->sectors[wall->sector].ceiling_height-RvR_port_get_position().z)*1024)/wall->d1_depth;
      cy0 = port_middle_row*1024-cy0;
      cy1 = port_middle_row*1024-cy1;
      RvR_fix22 step_cy = (cy1-cy0)/width;
      RvR_fix22 cy = cy0;

      //Floor
      RvR_fix22 fy0 = (RVR_YRES*(map->sectors[wall->sector].floor_height-RvR_port_get_position().z)*1024)/wall->d0_depth;
      RvR_fix22 fy1 = (RVR_YRES*(map->sectors[wall->sector].floor_height-RvR_port_get_position().z)*1024)/wall->d1_depth;
      fy0 = port_middle_row*1024-fy0;
      fy1 = port_middle_row*1024-fy1;
      RvR_fix22 step_fy = (fy1-fy0)/width;
      RvR_fix22 fy = fy0;

      for(int x = wall->d0_x;x<=wall->d1_x;x++)
      {
         int wy = port_ymin[x];
         uint8_t * restrict pix = &RvR_core_framebuffer()[port_ymin[x]*RVR_XRES+x];

         //Ceiling
         int y_to = RvR_min(cy/1024,port_ymax[x]);
         for(;wy<y_to;wy++)
         {
            *pix = map->sectors[wall->sector].ceiling_tex;
            pix+=RVR_XRES;
         }

         if(RvR_core_key_down(RVR_KEY_SPACE))
            RvR_core_render_present();

         //Wall
         y_to = RvR_min(fy/1024-1,port_ymax[x]);
         for(;wy<y_to;wy++)
         {
            *pix = 4;
            pix+=RVR_XRES;
         }

         if(RvR_core_key_down(RVR_KEY_SPACE))
            RvR_core_render_present();

         //Floor
         y_to = RvR_min(RVR_YRES-1,port_ymax[x]);
         for(;wy<y_to;wy++)
         {
            *pix = map->sectors[wall->sector].floor_tex;
            pix+=RVR_XRES;
         }

         if(RvR_core_key_down(RVR_KEY_SPACE))
            RvR_core_render_present();

         port_ymin[x] = RVR_YRES;
         port_ymax[x] = 0;

         cy+=step_cy;
         fy+=step_fy;
         depth+=step_depth;
      }
   }
   //Portal
   else
   {
      //Current sector ceiling
      RvR_fix22 cy0 = (RVR_YRES*(map->sectors[wall->sector].ceiling_height-RvR_port_get_position().z)*1024)/wall->d0_depth;
      RvR_fix22 cy1 = (RVR_YRES*(map->sectors[wall->sector].ceiling_height-RvR_port_get_position().z)*1024)/wall->d1_depth;
      cy0 = port_middle_row*1024-cy0;
      cy1 = port_middle_row*1024-cy1;
      RvR_fix22 step_cy = (cy1-cy0)/width;
      RvR_fix22 cy = cy0;

      //Portal sector ceiling
      RvR_fix22 cph0 = RvR_max(0,(RVR_YRES*(map->sectors[wall->sector].ceiling_height-map->sectors[portal].ceiling_height)*1024)/wall->d0_depth);
      RvR_fix22 cph1 = RvR_max(0,(RVR_YRES*(map->sectors[wall->sector].ceiling_height-map->sectors[portal].ceiling_height)*1024)/wall->d1_depth);
      RvR_fix22 step_cph = (cph1-cph0)/width;
      RvR_fix22 cph = cph0;

      //Current sector floor
      RvR_fix22 fy0 = (RVR_YRES*(map->sectors[wall->sector].floor_height-RvR_port_get_position().z)*1024)/wall->d0_depth;
      RvR_fix22 fy1 = (RVR_YRES*(map->sectors[wall->sector].floor_height-RvR_port_get_position().z)*1024)/wall->d1_depth;
      fy0 = port_middle_row*1024-fy0;
      fy1 = port_middle_row*1024-fy1;
      RvR_fix22 step_fy = (fy1-fy0)/width;
      RvR_fix22 fy = fy0;

      //Portal sector floor
      RvR_fix22 fph0 = RvR_max(0,(RVR_YRES*(map->sectors[portal].floor_height-map->sectors[wall->sector].floor_height)*1024)/wall->d0_depth);
      RvR_fix22 fph1 = RvR_max(0,(RVR_YRES*(map->sectors[portal].floor_height-map->sectors[wall->sector].floor_height)*1024)/wall->d1_depth);
      RvR_fix22 step_fph = (fph1-fph0)/width;
      RvR_fix22 fph = fph0;

      for(int x = wall->d0_x;x<=wall->d1_x;x++)
      {
         int wy = port_ymin[x];
         uint8_t * restrict pix = &RvR_core_framebuffer()[wy*RVR_XRES+x];

         //Draw ceiling until ceiling wall
         int y_to = RvR_min(cy/1024,port_ymax[x]);
         for(;wy<y_to;wy++)
         {
            *pix = map->sectors[wall->sector].ceiling_tex;
            pix+=RVR_XRES;
         }

         if(RvR_core_key_down(RVR_KEY_SPACE))
            RvR_core_render_present();

         //Draw ceiling wall
         y_to = RvR_min((cy+cph)/1024,port_ymax[x]);
         for(;wy<y_to;wy++)
         {
            *pix = 4;
            pix+=RVR_XRES;
         }

         port_ymin[x] = wy;
         wy = RvR_max(wy,(fy-fph)/1024);
         pix = &RvR_core_framebuffer()[wy*RVR_XRES+x];

         if(RvR_core_key_down(RVR_KEY_SPACE))
            RvR_core_render_present();

         //Draw floor wall
         y_to = RvR_min(fy/1024-1,port_ymax[x]);
         for(;wy<y_to;wy++)
         {
            *pix = 4;
            pix+=RVR_XRES;
         }

         if(RvR_core_key_down(RVR_KEY_SPACE))
            RvR_core_render_present();

         //Draw floor
         y_to = RvR_min(RVR_YRES-1,port_ymax[x]);
         for(;wy<y_to;wy++)
         {
            *pix = map->sectors[wall->sector].floor_tex;
            pix+=RVR_XRES;
         }

         if(RvR_core_key_down(RVR_KEY_SPACE))
            RvR_core_render_present();

         port_ymax[x] = RvR_min((fy-fph)/1024,port_ymax[x]);

         cy+=step_cy;
         cph+=step_cph;
         fy+=step_fy;
         fph+=step_fph;
      }
   }
}

//Returns:
//-1 potvis don't obstruct each other
//0 potvis a is obstructed by potvis b --> potvis b first
//1 potvis b is obstructed by potvis a --> potvis a first
static int port_potvis_order(int16_t va, int16_t vb)
{
   port_potvis_element *a = &port_potvis.data[va];
   port_potvis_element *b = &port_potvis.data[vb];

   RvR_fix22 xaf = port_potwall.data[a->start].d0_x;
   RvR_fix22 xal = port_potwall.data[a->end].d1_x;
   RvR_fix22 xbf = port_potwall.data[b->start].d0_x;
   RvR_fix22 xbl = port_potwall.data[b->end].d1_x;

   //No overlap
   if(xaf>=xbl||xbf>=xal)
      return -1;

   //potvis a starts in potvis b
   if(xaf>=xbf)
   {
      int i;
      for(i = b->start;port_potwall.data[i].d1_x<xaf;i = port_potwall.data[i].next);
      return port_wall_order(a->start,i);
   }

   //potvis a starts to the left of potvis b
   int i;
   for(i = a->start;port_potwall.data[i].d1_x<xbf;i = port_potwall.data[i].next);
   return port_wall_order(i,b->start);
}

//Returns:
//-1 walls don't obstruct each other
//0 wall a is obstructed by wall b --> wall b first
//1 wall b is obstructed by wall a --> wall a first
static int port_wall_order(int16_t a, int16_t b)
{
   RvR_port_map *map = RvR_port_map_get();
   port_potwall_element *wa = &port_potwall.data[a];
   port_potwall_element *wb = &port_potwall.data[b];

   RvR_fix22 x00 = map->walls[wa->wall].x;
   RvR_fix22 y00 = map->walls[wa->wall].y;
   RvR_fix22 x01 = map->walls[map->walls[wa->wall].p2].x;
   RvR_fix22 y01 = map->walls[map->walls[wa->wall].p2].y;
   RvR_fix22 x10 = map->walls[wb->wall].x;
   RvR_fix22 y10 = map->walls[wb->wall].y;
   RvR_fix22 x11 = map->walls[map->walls[wb->wall].p2].x;
   RvR_fix22 y11 = map->walls[map->walls[wb->wall].p2].y;

   //(x00/y00) is origin, all calculations centered arround it
   //t0 is relation of (b,p0) to wall a
   //t1 is relation of (b,p1) to wall a
   //See RvR_port_sector_inside for more in depth explanation
   RvR_fix22 x = x01-x00;
   RvR_fix22 y = y01-y00;
   RvR_fix22 t0 = ((x10-x00)*y-(y10-y00)*x)/1024;
   RvR_fix22 t1 = ((x11-x00)*y-(y11-y00)*x)/1024;

   //walls on the same line (identicall or adjacent)
   if(t0==0&&t1==0)
      return -1;

   //(b,p0) on extension of wall a (shared corner, etc)
   //Set t0 = t1 to trigger RvR_sign_equal check (and for the return !RvR_sign_equal to be correct)
   if(t0==0)
      t0 = t1;

   //(b,p1) on extension of wall a
   //Set t0 = t1 to trigger RvR_sign_equal check
   if(t1==0)
      t1 = t0;

   //Wall either completely to the left or to the right of other wall
   if(RvR_sign_equal(t0,t1))
   {
      //Compare with player position relative to wall a
      //if wall b and the player share the same relation, wall a needs to be drawn first
      t1 = ((RvR_port_get_position().x-x00)*y-(RvR_port_get_position().y-y00)*x)/1024;
      return !RvR_sign_equal(t0,t1);
   }

   //Extension of wall a intersects with wall b
   //--> check wall b instead
   //(x10/y10) is origin, all calculations centered arround it
   x = x11-x10;
   y = y11-y10;
   t0 = ((x00-x10)*y-(y00-y10)*x)/1024;
   t1 = ((x01-x10)*y-(y01-y10)*x)/1024;

   //(a,p0) on extension of wall b
   if(t0==0)
      t0 = t1;

   //(a,p1) on extension of wall b
   if(t1==0)
      t1 = t0;

   //Wall either completely to the left or to the right of other wall
   if(RvR_sign_equal(t0,t1))
   {
      //Compare with player position relative to wall b
      //if wall a and the player share the same relation, wall b needs to be drawn first
      t1 = ((RvR_port_get_position().x-x10)*y-(RvR_port_get_position().y-y10)*x)/1024;
      return RvR_sign_equal(t0,t1);
   }

   //Invalid case (walls are intersecting), expect rendering glitches
   return -1;
}

RvR_stack_function(int16_t,port_stack_i16,16,16,static);
RvR_stack_function(port_potwall_element,port_stack_potwall,16,16,static);
RvR_stack_function(port_potvis_element,port_stack_potvis,16,16,static);
//-------------------------------------
