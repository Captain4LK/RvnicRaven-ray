/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <string.h>
#include <SLK/SLK.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR_error.h"
#include "RvR_math.h"
#include "RvR_map.h"
#include "RvR_raycast.h"
#include "RvR_raycast_draw.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static RvR_fix22 cam_angle = 0;
static RvR_vec3 cam_position = {0};
static int16_t cam_shear = 0;
//-------------------------------------

//Function prototypes
static RvR_fix22 fov_correction_factor(RvR_fix22 fov);
//-------------------------------------

//Function implementations

void RvR_cast_ray_multi_hit(RvR_ray ray, RvR_hit_result *hit_results, uint16_t *hit_results_len)
{
   RvR_vec2 current_pos = ray.start;
   RvR_vec2 current_square;

   current_square.x = RvR_div_round_down(ray.start.x,RvR_fix22_one);
   current_square.y = RvR_div_round_down(ray.start.y,RvR_fix22_one);

   *hit_results_len = 0;

   //DDA variables
   RvR_vec2 next_side_dist; //dist. from start to the next side in given axis
   RvR_vec2 delta;
   RvR_vec2 step; //-1 or 1 for each axis
   int8_t step_horizontal = 0; //whether the last step was hor. or vert.

   next_side_dist.x = 0;
   next_side_dist.y = 0;

   RvR_fix22 dir_vec_length_norm = RvR_len2(ray.direction)*RvR_fix22_one;

   delta.x = RvR_abs(dir_vec_length_norm/RvR_non_zero(ray.direction.x));
   delta.y = RvR_abs(dir_vec_length_norm/RvR_non_zero(ray.direction.y));

   //Init DDA
   if(ray.direction.x<0)
   {
      step.x = -1;
      next_side_dist.x = (RvR_wrap(ray.start.x,RvR_fix22_one)*delta.x)/RvR_fix22_one;
   }
   else
   {
      step.x = 1;
      next_side_dist.x = (RvR_wrap(RvR_fix22_one-ray.start.x,RvR_fix22_one)*delta.x)/RvR_fix22_one;
   }

   if(ray.direction.y<0)
   {
      step.y = -1;
      next_side_dist.y = (RvR_wrap(ray.start.y,RvR_fix22_one)*delta.y)/RvR_fix22_one;
   }
   else
   {
      step.y = 1;
      next_side_dist.y = (RvR_wrap(RvR_fix22_one-ray.start.y,RvR_fix22_one)*delta.y)/RvR_fix22_one;
   }

   //DDA loop
#define RECIP_SCALE 65536

   RvR_fix22 ray_dir_x_recip = RECIP_SCALE/RvR_non_zero(ray.direction.x);
   RvR_fix22 ray_dir_y_recip = RECIP_SCALE/RvR_non_zero(ray.direction.y);
   //^ we precompute reciprocals to avoid divisions in the loop

   int first = 1;
   for(unsigned i = 0;i<MAX_STEPS;i++)
   {
      if(!first)//current_type!=square_type)
      {
         //collision
         RvR_hit_result h;

         h.position = current_pos;
         h.square   = current_square;
         if(step_horizontal)
         {
            h.position.x = current_square.x*RvR_fix22_one;
            h.direction = 3;

            if(step.x==-1)
            {
               h.direction = 1;
               h.position.x+=RvR_fix22_one;
            }

            RvR_fix22 diff = h.position.x-ray.start.x;

            //avoid division by multiplying with reciprocal
            h.position.y = ray.start.y+(ray.direction.y*diff*ray_dir_x_recip)/RECIP_SCALE;

            /* Here we compute the fish eye corrected distance (perpendicular to
            the projection plane) as the Euclidean distance (of hit from camera
            position) divided by the length of the ray direction vector. This can
            be computed without actually computing Euclidean distances as a
            hypothenuse A (distance) divided by hypothenuse B (length) is equal to
            leg A (distance along principal axis) divided by leg B (length along
            the same principal axis). */

            #define CORRECT(dir1,dir2)\
            RvR_fix22 tmp = diff/4;        /* 4 to prevent overflow */ \
            /* prevent a bug with small dists */ \
            h.distance = ((tmp/8)!=0)?((tmp*RvR_fix22_one*ray_dir_ ## dir1 ## _recip)/(RECIP_SCALE/4)):\
            RvR_abs(h.position.dir2-ray.start.dir2);

            CORRECT(x,y)
         }
         else
         {
            h.position.y = current_square.y*RvR_fix22_one;
            h.direction = 2;

            if(step.y==-1)
            {
               h.direction = 0;
               h.position.y+=RvR_fix22_one;
            }

            RvR_fix22 diff = h.position.y-ray.start.y;

            h.position.x = ray.start.x+(ray.direction.x*diff*ray_dir_y_recip)/RECIP_SCALE;

            CORRECT(y,x) //same as above but for different axis

#undef CORRECT
         }

         h.wall_tex = RvR_map_wall_tex_at(current_square.x,current_square.y);

         switch(h.direction)
         {
         case 0:
            h.texture_coord = RvR_wrap(-1*h.position.x,RvR_fix22_one);
            h.fheight = RvR_map_floor_height_at(current_square.x,current_square.y+1);
            h.cheight = RvR_map_ceiling_height_at(current_square.x,current_square.y+1);
            h.floor_tex = RvR_map_floor_tex_at(h.square.x,h.square.y+1);
            h.ceil_tex = RvR_map_ceil_tex_at(h.square.x,h.square.y+1);
            break;
         case 1:
            h.texture_coord = RvR_wrap(h.position.y,RvR_fix22_one);
            h.fheight = RvR_map_floor_height_at(current_square.x+1,current_square.y);
            h.cheight = RvR_map_ceiling_height_at(current_square.x+1,current_square.y);
            h.floor_tex = RvR_map_floor_tex_at(h.square.x+1,h.square.y);
            h.ceil_tex = RvR_map_ceil_tex_at(h.square.x+1,h.square.y);
            break;
         case 2:
            h.texture_coord = RvR_wrap(h.position.x,RvR_fix22_one);
            h.fheight = RvR_map_floor_height_at(current_square.x,current_square.y-1);
            h.cheight = RvR_map_ceiling_height_at(current_square.x,current_square.y-1);
            h.floor_tex = RvR_map_floor_tex_at(h.square.x,h.square.y-1);
            h.ceil_tex = RvR_map_ceil_tex_at(h.square.x,h.square.y-1);
            break;
         case 3:
            h.texture_coord = RvR_wrap(-1*h.position.y,RvR_fix22_one);
            h.fheight = RvR_map_floor_height_at(current_square.x-1,current_square.y);
            h.cheight = RvR_map_ceiling_height_at(current_square.x-1,current_square.y);
            h.floor_tex = RvR_map_floor_tex_at(h.square.x-1,h.square.y);
            h.ceil_tex = RvR_map_ceil_tex_at(h.square.x-1,h.square.y);
            break;
         default:
            h.texture_coord = 0;
            break;
         }

         hit_results[*hit_results_len] = h;
         *hit_results_len+=1;

         if(*hit_results_len>=MAX_HITS)
            break;
      }

      // DDA step
      if(next_side_dist.x<next_side_dist.y)
      {
         next_side_dist.x+=delta.x;
         current_square.x+=step.x;
         step_horizontal = 1;
      }
      else
      {
         next_side_dist.y+=delta.y;
         current_square.y+=step.y;
         step_horizontal = 0;
      }
      first = 0;
   }
}

void RvR_cast_rays_multi_hit(RvR_column_function column)
{
   RvR_vec2 dir0 = RvR_vec2_rot(cam_angle-(HORIZONTAL_FOV/2));
   RvR_vec2 dir1 = RvR_vec2_rot(cam_angle+(HORIZONTAL_FOV/2));

   RvR_fix22 cos = RvR_non_zero(RvR_fix22_cos(HORIZONTAL_FOV/2));

   dir0.x = (dir0.x*RvR_fix22_one)/cos;
   dir0.y = (dir0.y*RvR_fix22_one)/cos;

   dir1.x = (dir1.x*RvR_fix22_one)/cos;
   dir1.y = (dir1.y*RvR_fix22_one)/cos;

   RvR_fix22 dx = dir1.x-dir0.x;
   RvR_fix22 dy = dir1.y-dir0.y;

   RvR_hit_result hits[MAX_HITS] = {0};
   uint16_t hit_count = 0;

   RvR_ray r;
   r.start = (RvR_vec2) {cam_position.x,cam_position.y};

   RvR_fix22 current_dx = 0;
   RvR_fix22 current_dy = 0;

   for(int16_t i = 0;i<XRES;i++)
   {
       /* Here by linearly interpolating the direction vector its length changes,
       which in result achieves correcting the fish eye effect (computing
       perpendicular distance). */

      r.direction.x = dir0.x+(current_dx/XRES);
      r.direction.y = dir0.y+(current_dy/XRES);
      
      RvR_cast_ray_multi_hit(r,hits,&hit_count);

      column(hits,hit_count,i,r);

      current_dx+=dx;
      current_dy+=dy;
   }
}

RvR_fix22 RvR_raycast_perspective_scale_vertical(RvR_fix22 org_size, RvR_fix22 distance)
{
   static RvR_fix22 correction_factor = 0;
   if(correction_factor==0)
      correction_factor = fov_correction_factor(VERTICAL_FOV);

   return distance!=0?((org_size*RvR_fix22_one)/RvR_non_zero((correction_factor*distance)/RvR_fix22_one)):0;
}

RvR_fix22 RvR_raycast_perspective_scale_vertical_inverse(RvR_fix22 org_size, RvR_fix22 sc_size)
{
   static RvR_fix22 correction_factor = 0;
   if(correction_factor==0)
      correction_factor = fov_correction_factor(VERTICAL_FOV);

   return sc_size!=0?((org_size*RvR_fix22_one)/RvR_non_zero((correction_factor*sc_size)/RvR_fix22_one)):RvR_INFINITY;
}

RvR_fix22 RvR_raycast_perspective_scale_horizontal(RvR_fix22 org_size, RvR_fix22 distance)
{
   static RvR_fix22 correction_factor = 0;
   if(correction_factor==0)
      correction_factor = fov_correction_factor(HORIZONTAL_FOV);

   return distance!=0?((org_size*RvR_fix22_one)/RvR_non_zero((correction_factor*distance)/RvR_fix22_one)):0;
}

void RvR_raycast_move_with_collision(RvR_vec3 offset, int8_t compute_height, int8_t force)
{
   int8_t moves_in_plane = offset.x!=0||offset.y!=0;

   if(moves_in_plane||force)
   {
      int16_t x_square_new, y_square_new;

      RvR_vec2 corner; // BBox corner in the movement direction
      RvR_vec2 corner_new;

      int16_t x_dir = offset.x>0?1:-1;
      int16_t y_dir = offset.y>0?1:-1;

      corner.x = cam_position.x+x_dir*CAMERA_COLL_RADIUS;
      corner.y = cam_position.y+y_dir*CAMERA_COLL_RADIUS;

      int16_t x_square = RvR_div_round_down(corner.x,RvR_fix22_one);
      int16_t y_square = RvR_div_round_down(corner.y,RvR_fix22_one);

      corner_new.x = corner.x+offset.x;
      corner_new.y = corner.y+offset.y;

      x_square_new = RvR_div_round_down(corner_new.x,RvR_fix22_one);
      y_square_new = RvR_div_round_down(corner_new.y,RvR_fix22_one);

      RvR_fix22 bottom_limit = -RvR_INFINITY;
      RvR_fix22 top_limit = RvR_INFINITY;

      RvR_fix22 curr_ceil_height = RvR_INFINITY;

      if(compute_height)
      {
         bottom_limit = cam_position.z-CAMERA_COLL_HEIGHT_BELOW+CAMERA_COLL_STEP_HEIGHT;

         top_limit = cam_position.z+CAMERA_COLL_HEIGHT_ABOVE;

         curr_ceil_height = RvR_map_ceiling_height_at(x_square,y_square);
      }

      // checks a single square for collision against the camera
#define collCheck(dir,s1,s2)\
      if(compute_height)\
      {\
         RvR_fix22 height = RvR_map_floor_height_at(s1,s2);\
         if(height>bottom_limit||\
         curr_ceil_height-height<\
         CAMERA_COLL_HEIGHT_BELOW+CAMERA_COLL_HEIGHT_ABOVE)\
            dir##_collides = 1;\
         else\
         {\
            RvR_fix22 height2 = RvR_map_ceiling_height_at(s1,s2);\
            if((height2<top_limit)||((height2-height)<\
            (CAMERA_COLL_HEIGHT_ABOVE+CAMERA_COLL_HEIGHT_BELOW)))\
               dir##_collides = 1;\
         }\
      }\
      else\
         dir##_collides = RvR_map_floor_height_at(s1,s2)>CAMERA_COLL_STEP_HEIGHT;

      // check collision against non-diagonal square
#define collCheckOrtho(dir,dir2,s1,s2,x)\
      if (dir##_square_new != dir##_square)\
      {\
         collCheck(dir,s1,s2)\
      }\
      if(!dir##_collides)\
      { /* now also check for coll on the neighbouring square */ \
         int16_t dir2##_square2 = RvR_div_round_down(corner.dir2-dir2##_dir *\
         CAMERA_COLL_RADIUS*2,RvR_fix22_one);\
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
               corner_new.dir = (dir##_square)*RvR_fix22_one+\
               RvR_fix22_one/2+dir##_dir*(RvR_fix22_one/2)-\
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

            square_pos.x = x_square*RvR_fix22_one;
            square_pos.y = y_square*RvR_fix22_one;

            new_pos.x = RvR_max(square_pos.x+CAMERA_COLL_RADIUS+1,RvR_min(square_pos.x+RvR_fix22_one-CAMERA_COLL_RADIUS-1,cam_position.x));
            new_pos.y = RvR_max(square_pos.y+CAMERA_COLL_RADIUS+1,RvR_min(square_pos.y+RvR_fix22_one-CAMERA_COLL_RADIUS-1,cam_position.y));

            corner_new.x = corner.x+(new_pos.x-cam_position.x);
            corner_new.y = corner.y+(new_pos.y-cam_position.y);
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

      cam_position.x = corner_new.x-x_dir*CAMERA_COLL_RADIUS;
      cam_position.y = corner_new.y-y_dir*CAMERA_COLL_RADIUS;  
   }

   if(compute_height&&(moves_in_plane||(offset.z!=0)||force))
   {
      cam_position.z+=offset.z;

      int16_t x_square1 = RvR_div_round_down(cam_position.x-CAMERA_COLL_RADIUS,RvR_fix22_one);

      int16_t x_square2 = RvR_div_round_down(cam_position.x+CAMERA_COLL_RADIUS,RvR_fix22_one);

      int16_t y_square1 = RvR_div_round_down(cam_position.y-CAMERA_COLL_RADIUS,RvR_fix22_one);

      int16_t y_square2 = RvR_div_round_down(cam_position.y+CAMERA_COLL_RADIUS,RvR_fix22_one);

      RvR_fix22 bottom_limit = RvR_map_floor_height_at(x_square1,y_square1);
      RvR_fix22 top_limit = RvR_map_ceiling_height_at(x_square1,y_square1);

      RvR_fix22 height;

#define checkSquares(s1,s2)\
      {\
         height = RvR_map_floor_height_at(x_square##s1,y_square##s2);\
         bottom_limit = RvR_max(bottom_limit,height);\
         height = RvR_map_ceiling_height_at(x_square##s1,y_square##s2);\
         top_limit = RvR_min(top_limit,height);\
      }

      if(x_square2!=x_square1)
         checkSquares(2,1)

      if(y_square2!=y_square1)
         checkSquares(1,2)

      if(x_square2!=x_square1&&y_square2!=y_square1)
         checkSquares(2,2)

      cam_position.z = RvR_clamp(cam_position.z,bottom_limit+CAMERA_COLL_HEIGHT_BELOW,top_limit-CAMERA_COLL_HEIGHT_ABOVE);

#undef checkSquares
   }
}

void RvR_raycast_set_angle(RvR_fix22 angle)
{
   cam_angle = angle;
}

RvR_fix22 RvR_raycast_get_angle()
{
   return cam_angle;
}

void RvR_raycast_set_shear(int16_t shear)
{
   cam_shear = shear;
}

int16_t RvR_raycast_get_shear()
{
   return cam_shear;
}

void RvR_raycast_set_position(RvR_vec3 position)
{
   cam_position = position;
}

RvR_vec3 RvR_raycast_get_position()
{
   return cam_position;
}

static RvR_fix22 fov_correction_factor(RvR_fix22 fov)
{
   uint16_t table[9] = {1,208,408,692,1024,1540,2304,5376,30000};

   fov = RvR_min(RvR_fix22_one/2-1,fov);

   uint8_t index = fov/64;
   uint32_t t = ((fov-index*64)*RvR_fix22_one)/64; 
   uint32_t v1 = table[index];
   uint32_t v2 = table[index+1];

   return v1+((v2-v1)*t)/RvR_fix22_one;
}
//-------------------------------------
