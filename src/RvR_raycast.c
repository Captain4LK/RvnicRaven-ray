/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
//-------------------------------------

//Variables
static RvR_fix22 ray_cam_angle = 0;
static RvR_vec3 ray_cam_position = {0};
static int16_t ray_cam_shear = 0;
static RvR_fix22 ray_cam_fov = 256;
//-------------------------------------

//Function prototypes
static RvR_fix22 ray_fov_correction_factor(RvR_fix22 fov);
//-------------------------------------

//Function implementations

//raycastlib by drummyfish: https://gitlab.com/drummyfish/raycastlib
//Original header:
//raycastlib (RCL) - Small C header-only raycasting library for embedded and
//low performance computers, such as Arduino. Only uses integer math and stdint
//standard library.

void RvR_ray_cast_multi_hit(RvR_ray ray, RvR_ray_hit_result *hit_results, uint16_t *hit_results_len)
{
   *hit_results_len = 0;

   RvR_vec2 current_pos = ray.start;
   RvR_vec2 current_square;

   current_square.x = ray.start.x/1024;
   current_square.y = ray.start.y/1024;

   RvR_vec2 delta;
   delta.x = RvR_abs((1024*1024)/RvR_non_zero(ray.direction.x));
   delta.y = RvR_abs((1024*1024)/RvR_non_zero(ray.direction.y));

   RvR_vec2 step;
   RvR_vec2 side_dist;
   int side;

   if(ray.direction.x<0)
   {
      step.x = -1;
      side_dist.x = ((ray.start.x-current_square.x*1024)*delta.x)/1024;
   }
   else
   {
      step.x = 1;
      side_dist.x = ((current_square.x*1024+1024-ray.start.x)*delta.x)/1024;
   }

   if(ray.direction.y<0)
   {
      step.y = -1;
      side_dist.y = ((ray.start.y-current_square.y*1024)*delta.y)/1024;
   }
   else
   {
      step.y = 1;
      side_dist.y = ((current_square.y*1024+1024-ray.start.y)*delta.y)/1024;
   }

   for(unsigned i = 0;i<RVR_RAY_MAX_STEPS;i++)
   {
      // DDA step
      if(side_dist.x<side_dist.y)
      {
         side_dist.x+=delta.x;
         current_square.x+=step.x;
         side = 0;
      }
      else
      {
         side_dist.y+=delta.y;
         current_square.y+=step.y;
         side = 1;
      }

      RvR_ray_hit_result h;
      h.position = current_pos;
      h.square   = current_square;

      if(!side)
      {
         h.distance = (side_dist.x-delta.x);
         h.position.y = ray.start.y+(h.distance*ray.direction.y)/1024;
         h.position.x = current_square.x*1024;
         h.direction = 3;
         if(step.x==-1)
         {
            h.direction = 1;
            h.position.x+=1024;
         }
      }
      else
      {
         h.distance = (side_dist.y-delta.y);
         h.position.x = ray.start.x+(h.distance*ray.direction.x)/1024;
         h.position.y = current_square.y*1024;
         h.direction = 2;
         if(step.y==-1)
         {
            h.direction = 0;
            h.position.y+=1024;
         }
      }
      
      if(RvR_ray_map_inbounds(current_square.x,current_square.y))
      {
         h.wall_ftex = RvR_ray_map_wall_ftex_at_us(current_square.x,current_square.y);
         h.wall_ctex = RvR_ray_map_wall_ctex_at_us(current_square.x,current_square.y);
      }
      else
      {
         h.wall_ftex = RvR_ray_map_sky_tex();
         h.wall_ctex = RvR_ray_map_sky_tex();
      }

      h.fheight = 0;
      h.cheight = (127*1024)/8;
      h.floor_tex = RvR_ray_map_sky_tex();
      h.ceil_tex = RvR_ray_map_sky_tex();

      switch(h.direction)
      {
      case 0:
         h.texture_coord = (h.position.x)&1023;
         if(RvR_ray_map_inbounds(current_square.x,current_square.y+1))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(current_square.x,current_square.y+1);
            h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x,current_square.y+1);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x,h.square.y+1);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x,h.square.y+1);
         }
         break;
      case 1:
         h.texture_coord = (-h.position.y)&1023;
         if(RvR_ray_map_inbounds(current_square.x+1,current_square.y))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(current_square.x+1,current_square.y);
            h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x+1,current_square.y);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x+1,h.square.y);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x+1,h.square.y);
         }
         break;
      case 2:
         h.texture_coord = (-h.position.x)&1023;
         if(RvR_ray_map_inbounds(current_square.x,current_square.y-1))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(current_square.x,current_square.y-1);
            h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x,current_square.y-1);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x,h.square.y-1);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x,h.square.y-1);
         }
         break;
      case 3:
         h.texture_coord = (h.position.y)&1023;
         if(RvR_ray_map_inbounds(current_square.x-1,current_square.y))
         {
            h.fheight = RvR_ray_map_floor_height_at_us(current_square.x-1,current_square.y);
            h.cheight = RvR_ray_map_ceiling_height_at_us(current_square.x-1,current_square.y);
            h.floor_tex = RvR_ray_map_floor_tex_at_us(h.square.x-1,h.square.y);
            h.ceil_tex = RvR_ray_map_ceil_tex_at_us(h.square.x-1,h.square.y);
         }
         break;
      default:
         h.texture_coord = 0;
         break;
      }

      hit_results[*hit_results_len] = h;
      (*hit_results_len)++;

      if(*hit_results_len>=RVR_RAY_MAX_STEPS)
         break;
   }
}

void RvR_rays_cast_multi_hit(RvR_ray_column_function column)
{
   RvR_vec2 dir0 = RvR_vec2_rot(ray_cam_angle-(RVR_RAY_HORIZONTAL_FOV/2));
   RvR_vec2 dir1 = RvR_vec2_rot(ray_cam_angle+(RVR_RAY_HORIZONTAL_FOV/2));
   RvR_fix22 cos = RvR_non_zero(RvR_fix22_cos(RVR_RAY_HORIZONTAL_FOV/2));
   dir0.x = (dir0.x*1024)/cos;
   dir0.y = (dir0.y*1024)/cos;
   dir1.x = (dir1.x*1024)/cos;
   dir1.y = (dir1.y*1024)/cos;

   RvR_fix22 dx = dir1.x-dir0.x;
   RvR_fix22 dy = dir1.y-dir0.y;
   RvR_fix22 current_dx = 0;
   RvR_fix22 current_dy = 0;

   RvR_ray_hit_result hits[RVR_RAY_MAX_STEPS] = {0};
   uint16_t hit_count = 0;

   for(int16_t i = 0;i<RVR_XRES;i++)
   {
      //Here by linearly interpolating the direction vector its length changes,
      //which in result achieves correcting the fish eye effect (computing
      //perpendicular distance).
      RvR_ray r;
      r.start = (RvR_vec2) {ray_cam_position.x,ray_cam_position.y};
      r.direction.x = dir0.x+(current_dx/RVR_XRES);
      r.direction.y = dir0.y+(current_dy/RVR_XRES);
      
      RvR_ray_cast_multi_hit(r,hits,&hit_count);

      column(hits,i,r);

      current_dx+=dx;
      current_dy+=dy;
   }
}

RvR_fix22 RvR_ray_perspective_scale_vertical(RvR_fix22 org_size, RvR_fix22 distance)
{
   static RvR_fix22 correction_factor = 0;
   if(correction_factor==0)
      correction_factor = ray_fov_correction_factor(RVR_RAY_VERTICAL_FOV);

   if(distance==0)
      return 0;
   return ((org_size*1024)/RvR_non_zero((correction_factor*distance)/1024));
}

RvR_fix22 RvR_ray_perspective_scale_vertical_inverse(RvR_fix22 org_size, RvR_fix22 sc_size)
{
   static RvR_fix22 correction_factor = 0;
   if(correction_factor==0)
      correction_factor = ray_fov_correction_factor(RVR_RAY_VERTICAL_FOV);

   if(sc_size==0)
      return RvR_fix22_infinity;
   return ((org_size*1024)/RvR_non_zero((correction_factor*sc_size)/1024));
}

RvR_fix22 RvR_ray_perspective_scale_horizontal(RvR_fix22 org_size, RvR_fix22 distance)
{
   static RvR_fix22 correction_factor = 0;
   if(correction_factor==0)
      correction_factor = ray_fov_correction_factor(RVR_RAY_HORIZONTAL_FOV);

   if(distance==0)
      return 0;
   return ((org_size*1024)/RvR_non_zero((correction_factor*distance)/1024));
}

void RvR_ray_set_angle(RvR_fix22 angle)
{
   ray_cam_angle = angle;
}

RvR_fix22 RvR_ray_get_angle()
{
   return ray_cam_angle;
}

void RvR_ray_set_shear(int16_t shear)
{
   ray_cam_shear = shear;
}

int16_t RvR_ray_get_shear()
{
   return ray_cam_shear;
}

void RvR_ray_set_position(RvR_vec3 position)
{
   ray_cam_position = position;
}

RvR_vec3 RvR_ray_get_position()
{
   return ray_cam_position;
}

void RvR_ray_set_fov(RvR_fix22 fov)
{
   ray_cam_fov = fov;
}

RvR_fix22 RvR_ray_get_fov()
{
   return ray_cam_fov;
}

static RvR_fix22 ray_fov_correction_factor(RvR_fix22 fov)
{
   return RvR_fix22_tan(fov/2);
}
//-------------------------------------
