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
#define TEX_AND ((1<<RVR_RAY_TEXTURE)-1)
#define TEX_MUL (1<<RVR_RAY_TEXTURE)
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

typedef struct
{
   int min;
   int max;
   int16_t sector;
   int16_t pos;
   uint16_t start[RVR_XRES+2];
   uint16_t end[RVR_XRES+2];
}port_plane;

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

static int port_planes_used;
static port_plane port_planes[256];
static RvR_fix22 port_span_start[RVR_YRES];

static RvR_vec2 port_cam_dir0;
static RvR_vec2 port_cam_dir1;
static RvR_fix22 port_fov_factor;
//-------------------------------------

//Function prototypes
RvR_stack_function_prototype(int16_t,port_stack_i16,static);
RvR_stack_function_prototype(port_potwall_element,port_stack_potwall,static);
RvR_stack_function_prototype(port_potvis_element,port_stack_potvis,static);

static void port_potvis_build();

static void port_wall_draw(int wall_num);

static int port_potvis_order(int16_t va, int16_t vb);
static int port_wall_order(int16_t a, int16_t b);

static void port_plane_add(int16_t sector, int16_t pos, int x, int y0, int y1);
static void port_span_draw_tex(int x0, int x1, int y, RvR_fix22 height, const RvR_texture *texture);
//-------------------------------------

//Function implementations

void RvR_port_draw_2d()
{
}

void RvR_port_draw()
{
   //Clear planes
   port_planes_used = 0;

   //Clear occlusion arrays
   memset(port_ymin,0,sizeof(port_ymin));
   for(int i = 0;i<RVR_XRES;i++) port_ymax[i] = RVR_YRES-1;

   //Initialize necessary variables
   RvR_port_map *map = RvR_port_map_get();
   port_fov_factor = RvR_fix22_tan(RvR_port_get_fov()/2);
   port_middle_row = (RVR_YRES/2)+RvR_port_get_shear();
   port_cam_dir0 = RvR_vec2_rot(RvR_port_get_angle()-RvR_port_get_fov()/2);
   port_cam_dir1 = RvR_vec2_rot(RvR_port_get_angle()+RvR_port_get_fov()/2);
   RvR_fix22 cos = RvR_non_zero(RvR_fix22_cos(RvR_port_get_fov()/2));
   port_cam_dir0.x = (port_cam_dir0.x*1024)/cos;
   port_cam_dir0.y = (port_cam_dir0.y*1024)/cos;
   port_cam_dir1.x = (port_cam_dir1.x*1024)/cos;
   port_cam_dir1.y = (port_cam_dir1.y*1024)/cos;

   port_potvis_build();

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
   //-------------------------------------

   //Render floor planes
   for(int i = 0;i<port_planes_used;i++)
   {
      port_plane *pl = &port_planes[i];

      if(pl->min>pl->max)
         continue;

      //This would be parallax textures:
      //Sky texture is rendered differently (vertical collumns instead of horizontal ones)
      /*if(pl->tex==RvR_ray_map_sky_tex())
      {
         RvR_fix22 angle_step = (TEX_SKYW_MUL*1024)/RVR_XRES;
         RvR_fix22 tex_step = (1024*TEX_SKYH_MUL-1)/RVR_YRES;

         RvR_texture *texture = RvR_texture_get(RvR_ray_map_sky_tex());
         RvR_fix22 angle = (RvR_ray_get_angle())*1024;
         angle+=(pl->min-1)*angle_step;

         for(int x = pl->min;x<pl->max+1;x++)
         {
            //Sky is rendered fullbright, no lut needed
            uint8_t * restrict pix = &RvR_core_framebuffer()[(pl->start[x])*RVR_XRES+x-1];
            const uint8_t * restrict tex = &texture->data[((angle>>10)&TEX_SKYW_AND)*TEX_SKYH_MUL];
            const uint8_t * restrict col = RvR_shade_table(32);

            //Slip in two parts: above and below horizon
            int middle = RvR_max(0,RvR_min(RVR_YRES,ray_middle_row+RVR_YRES/32));
            int tex_start = pl->start[x];
            int tex_end = middle;
            if(tex_end>pl->end[x])
               tex_end = pl->end[x];
            if(tex_start>tex_end)
               tex_end = tex_start;
            if(tex_start>middle)
               tex_end = tex_start-1;
            int solid_end = pl->end[x];
            RvR_fix22 texture_coord = (RVR_YRES-middle+pl->start[x])*tex_step;

            for(int y = tex_start;y<tex_end+1;y++)
            {
               *pix = tex[texture_coord>>10];
               texture_coord+=tex_step;
               pix+=RVR_XRES;
            }
            RvR_fix22 tex_coord = (RVR_YRES)*tex_step-1;
            texture_coord = RvR_min(tex_coord,tex_coord-tex_step*(tex_end-middle));
            for(int y = tex_end+1;y<solid_end+1;y++)
            {
               *pix = col[tex[(texture_coord>>10)&TEX_SKYH_AND]];
               texture_coord-=tex_step;
               pix+=RVR_XRES;
            }

            angle+=angle_step;
         }
         continue;
      }*/

      //Convert plane to horizontal spans
      RvR_texture *texture = RvR_texture_get(pl->pos?map->sectors[pl->sector].floor_tex:map->sectors[pl->sector].ceiling_tex);
      for(int x = pl->min;x<pl->max+2;x++)
      {
         RvR_fix22 s0 = pl->start[x-1];
         RvR_fix22 s1 = pl->start[x];
         RvR_fix22 e0 = pl->end[x-1];
         RvR_fix22 e1 = pl->end[x];

         //End spans top
         for(;s0<s1&&s0<=e0;s0++)
         {
            port_span_draw_tex(port_span_start[s0],x-1,s0,pl->pos?map->sectors[pl->sector].floor_height:map->sectors[pl->sector].ceiling_height,texture);
         }

         //End spans bottom
         for(;e0>e1&&e0>=s0;e0--)
         {
            port_span_draw_tex(port_span_start[e0],x-1,e0,pl->pos?map->sectors[pl->sector].floor_height:map->sectors[pl->sector].ceiling_height,texture);
         }

         //Start spans top
         for(;s1<s0&&s1<=e1;s1++)
            port_span_start[s1] = x-1;

         //Start spans bottom
         for(;e1>e0&&e1>=s1;e1--)
            port_span_start[e1] = x-1;
      }
   }
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
         if(y_to>wy)
         {
            port_plane_add(wall->sector,0,x,wy,y_to);
            wy = y_to;
            pix = &RvR_core_framebuffer()[wy*RVR_XRES+x];
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
         if(y_to>wy)
            port_plane_add(wall->sector,1,x,wy,y_to);

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
         if(y_to>wy)
         {
            port_plane_add(wall->sector,0,x,wy,y_to);
            wy = y_to;
            pix = &RvR_core_framebuffer()[wy*RVR_XRES+x];
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
         if(y_to>wy)
            port_plane_add(wall->sector,1,x,wy,y_to);

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

static void port_plane_add(int16_t sector, int16_t pos, int x, int y0, int y1)
{
   port_plane *cur = NULL;
   x+=1;

   //TODO: hash + linked list?
   int i;
   for(i = 0;i<port_planes_used;i++)
   {
      //ray_planes need to have be in the same sector...
      if(sector!=port_planes[i].sector)
         continue;
      //... and the same pos (floor/ceiling) to be valid for concatination
      if(pos!=port_planes[i].pos)
         continue;

      //Additionally the spans collumn needs to be either empty...
      if(port_planes[i].start[x]!=UINT16_MAX)
      {
         //... or directly adjacent to each other vertically, in that case
         //Concat planes vertically
         if(port_planes[i].start[x]-1==y1)
         {
            port_planes[i].start[x] = y0;
            return;
         }
         if(port_planes[i].end[x]+1==y0)
         {
            port_planes[i].end[x] = y1;
            return;
         }

         continue;
      }

      break;
   }

   if(i==port_planes_used)
   {
      if(port_planes_used==256)
      {
         //TODO
      }

      cur = &port_planes[port_planes_used++];
      cur->min = RVR_XRES;
      cur->max = -1;
      cur->sector = sector;
      cur->pos = pos;

      //Since this is an unsigned int, we can use memset to set all values to 65535 (0xffff)
      memset(cur->start,255,sizeof(cur->start));
   }
   else
   {
      cur = &port_planes[i];
   }

   if(x<cur->min)
      cur->min = x;
   if(x>cur->max)
      cur->max = x;

   cur->end[x] = y1;
   cur->start[x] = y0;
}

static void port_span_draw_tex(int x0, int x1, int y, RvR_fix22 height, const RvR_texture *texture)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   //Calculate the depth of the row to be rendered
   RvR_fix22 cam_height_screen_size = RvR_abs((RvR_port_get_position().z-height)*RVR_YRES)/1024;
   RvR_fix22 depth = RvR_port_perspective_scale_vertical_inverse(cam_height_screen_size,RvR_abs(y-port_middle_row));

   //Calculate texture mapping step size, starting coordinates and wrapping values
   RvR_fix22 step_x = (depth*(port_cam_dir1.x-port_cam_dir0.x))/RVR_XRES;
   RvR_fix22 step_y = (depth*(port_cam_dir1.y-port_cam_dir0.y))/RVR_XRES;
   RvR_fix22 tx = ((RvR_port_get_position().x)&1023)*1024+(depth*port_cam_dir0.x);
   RvR_fix22 ty = ((RvR_port_get_position().y)&1023)*1024+(depth*port_cam_dir0.y);
   tx+=x0*step_x;
   ty+=x0*step_y;
   RvR_fix22 x_and = (1<<RvR_log2(texture->width))-1;
   RvR_fix22 y_and = (1<<RvR_log2(texture->height))-1;
   RvR_fix22 y_log2 = RvR_log2(texture->height);

   //const and restrict don't seem to influence the generated assembly in this case
   uint8_t * restrict pix = &RvR_core_framebuffer()[y*RVR_XRES+x0];
   const uint8_t * restrict col = RvR_shade_table(RvR_min(63,(depth>>8)));
   const uint8_t * restrict tex = texture->data;

#if RVR_UNROLL

   int count = x1-x0;
   int n = (count+7)/8;
   switch(count%8)
   {
   case 0: do {
           *pix = col[tex[(((tx>>14)&x_and)<<y_log2)+((ty>>14)&y_and)]]; tx+=step_x; ty+=step_y; pix++; //fallthrough
   case 7: *pix = col[tex[(((tx>>14)&x_and)<<y_log2)+((ty>>14)&y_and)]]; tx+=step_x; ty+=step_y; pix++; //fallthrough
   case 6: *pix = col[tex[(((tx>>14)&x_and)<<y_log2)+((ty>>14)&y_and)]]; tx+=step_x; ty+=step_y; pix++; //fallthrough
   case 5: *pix = col[tex[(((tx>>14)&x_and)<<y_log2)+((ty>>14)&y_and)]]; tx+=step_x; ty+=step_y; pix++; //fallthrough
   case 4: *pix = col[tex[(((tx>>14)&x_and)<<y_log2)+((ty>>14)&y_and)]]; tx+=step_x; ty+=step_y; pix++; //fallthrough
   case 3: *pix = col[tex[(((tx>>14)&x_and)<<y_log2)+((ty>>14)&y_and)]]; tx+=step_x; ty+=step_y; pix++; //fallthrough
   case 2: *pix = col[tex[(((tx>>14)&x_and)<<y_log2)+((ty>>14)&y_and)]]; tx+=step_x; ty+=step_y; pix++; //fallthrough
   case 1: *pix = col[tex[(((tx>>14)&x_and)<<y_log2)+((ty>>14)&y_and)]]; tx+=step_x; ty+=step_y; pix++; //fallthrough
           }while(--n>0);
   }

#else

   for(int x = x0;x<x1;x++)
   {
      *pix = col[tex[(((tx>>14)&x_and)<<y_log2)+((ty>>14)&y_and)]];
      tx+=step_x;
      ty+=step_y;
      pix++;
   }

#endif
}

static void port_potvis_build()
{
   RvR_port_map *map = RvR_port_map_get();
   static port_stack_i16 to_visit = {0};
   uint32_t visited[(RVR_PORT_MAX_SECTORS+31)/32] = {0};

   RvR_fix22 cos = RvR_fix22_cos(RvR_port_get_angle());
   RvR_fix22 sin = RvR_fix22_sin(RvR_port_get_angle());
   RvR_fix22 cos_fov = (cos*port_fov_factor)/1024;
   RvR_fix22 sin_fov = (sin*port_fov_factor)/1024;

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
}

RvR_stack_function(int16_t,port_stack_i16,16,16,static);
RvR_stack_function(port_potwall_element,port_stack_potwall,16,16,static);
RvR_stack_function(port_potvis_element,port_stack_potvis,16,16,static);

#undef TEX_AND
#undef TEX_MUL
//-------------------------------------
