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
#include "../../src/RvnicRaven.h"
//-------------------------------------

//#defines
#include "sprite.h"
#include "collision.h"
#include "ai.h"
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Collider *collider_pool = NULL;
static Collider *colliders = NULL;
//-------------------------------------

//Function prototypes
static void collision_intersects(Collider *a, Collider *b, RvR_fix22 *depth, RvR_vec2 *normal);
//-------------------------------------

//Function implementations
void collision_move(Collider *col_cur, RvR_vec3 offset, RvR_fix22 *floor_height, RvR_fix22 *ceiling_height, RvR_vec3 *moved, int on_ground)
{
   if(col_cur->removed)
      return;

   int8_t moves_in_plane = offset.x!=0||offset.y!=0;

   if(floor_height!=NULL)
      *floor_height = INT32_MIN;
   if(ceiling_height!=NULL)
      *ceiling_height = INT32_MAX;

   Collider *col = colliders;
   Collider cur = {0};
   cur.pos.x = col_cur->pos.x+offset.x;
   cur.pos.y = col_cur->pos.y+offset.y;
   cur.pos.z = col_cur->pos.z+offset.z;
   cur.radius = 256;
   cur.height = 768;
   while(col!=NULL)
   {
      if(col==col_cur)
         goto next;
      if(col->removed)
         goto next;
      if(col->pos.z>(cur.pos.z)+cur.height)
         goto next;
      if(col->pos.z+col->height<(cur.pos.z))
         goto next;

      RvR_fix22 depth;
      RvR_vec2 normal;

      collision_intersects(&cur,col,&depth,&normal);
      if(depth>0)
      {
         //Compare collision resolution posiblilities
         RvR_fix22 depth_z;
         if(cur.pos.z>col->pos.z)
            depth_z = (col->pos.z+col->height)-(cur.pos.z);
         else
            depth_z = -((cur.pos.z+cur.height)-col->pos.z);

         if(RvR_abs(depth_z)>depth)
         {
            cur.pos.x-=(normal.x*depth)/1024;
            cur.pos.y-=(normal.y*depth)/1024;
         }
         else
         {
            cur.pos.z+=depth_z;
            if(floor_height!=NULL&&depth_z>0&&col->pos.z+col->height>*floor_height)
               *floor_height = col->pos.z+col->height;
            else if(ceiling_height!=NULL&&depth_z<0&&col->pos.z<*ceiling_height)
               *ceiling_height = col->pos.z;
         }
      }

next:
      col = col->next;
   }

   offset.x = cur.pos.x-col_cur->pos.x;
   offset.y = cur.pos.y-col_cur->pos.y;
   offset.z = cur.pos.z-col_cur->pos.z;
   RvR_vec3 old_pos = cur.pos;
   old_pos.x-=offset.x;
   old_pos.y-=offset.y;
   old_pos.z-=offset.z;

   int16_t x_square_new, y_square_new;

   RvR_vec2 corner; // BBox corner in the movement direction
   RvR_vec2 corner_new;

   int16_t x_dir = offset.x>0?1:-1;
   int16_t y_dir = offset.y>0?1:-1;

   corner.x = col_cur->pos.x+x_dir*CAMERA_COLL_RADIUS;
   corner.y = col_cur->pos.y+y_dir*CAMERA_COLL_RADIUS;

   int16_t x_square = RvR_div_round_down(corner.x,1024);
   int16_t y_square = RvR_div_round_down(corner.y,1024);

   corner_new.x = corner.x+offset.x;
   corner_new.y = corner.y+offset.y;

   x_square_new = RvR_div_round_down(corner_new.x,1024);
   y_square_new = RvR_div_round_down(corner_new.y,1024);

   RvR_fix22 bottom_limit = -RvR_fix22_infinity;
   RvR_fix22 top_limit = RvR_fix22_infinity;

   RvR_fix22 curr_ceil_height = RvR_fix22_infinity;

   bottom_limit = col_cur->pos.z;
   if(on_ground)
      bottom_limit = col_cur->pos.z+CAMERA_COLL_STEP_HEIGHT;

   top_limit = col_cur->pos.z+col_cur->height;

   curr_ceil_height = RvR_ray_map_ceiling_height_at(x_square,y_square);

   // checks a single square for collision against the camera
#define collCheck(dir,s1,s2)\
   RvR_fix22 height = RvR_ray_map_floor_height_at(s1,s2);\
   if(height>bottom_limit||\
   curr_ceil_height-height<\
   col_cur->height)\
      dir##_collides = 1;\
   else\
   {\
      RvR_fix22 height2 = RvR_ray_map_ceiling_height_at(s1,s2);\
      if((height2<top_limit)||((height2-height)<\
      (col_cur->height)))\
         dir##_collides = 1;\
   }

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
         {\
            collCheck(dir,dir##_square_new,dir2##_square2)\
         }\
         else\
         {\
            collCheck(dir,dir2##_square2,dir##_square_new)\
         }\
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

         new_pos.x = RvR_max(square_pos.x+CAMERA_COLL_RADIUS+1,RvR_min(square_pos.x+1024-CAMERA_COLL_RADIUS-1,col_cur->pos.x));
         new_pos.y = RvR_max(square_pos.y+CAMERA_COLL_RADIUS+1,RvR_min(square_pos.y+1024-CAMERA_COLL_RADIUS-1,col_cur->pos.y));

         corner_new.x = corner.x+(new_pos.x-col_cur->pos.x);
         corner_new.y = corner.y+(new_pos.y-col_cur->pos.y);
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

   col_cur->pos.x = corner_new.x-x_dir*CAMERA_COLL_RADIUS;
   col_cur->pos.y = corner_new.y-y_dir*CAMERA_COLL_RADIUS;  

   col_cur->pos.z+=offset.z;

   int16_t x_square1 = RvR_div_round_down(col_cur->pos.x-CAMERA_COLL_RADIUS,1024);

   int16_t x_square2 = RvR_div_round_down(col_cur->pos.x+CAMERA_COLL_RADIUS,1024);

   int16_t y_square1 = RvR_div_round_down(col_cur->pos.y-CAMERA_COLL_RADIUS,1024);

   int16_t y_square2 = RvR_div_round_down(col_cur->pos.y+CAMERA_COLL_RADIUS,1024);

   bottom_limit = RvR_ray_map_floor_height_at(x_square1,y_square1);
   top_limit = RvR_ray_map_ceiling_height_at(x_square1,y_square1);

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

   if(floor_height!=NULL&&*floor_height<bottom_limit)
      *floor_height = bottom_limit;
   if(ceiling_height!=NULL&&*ceiling_height>top_limit)
      *ceiling_height = top_limit;

   col_cur->pos.z = RvR_clamp(col_cur->pos.z,bottom_limit,top_limit-col_cur->height);

#undef checkSquares
#undef collCheckOrtho
#undef collHandle
#undef collCheck

   moved->x = (col_cur->pos.x-old_pos.x)<<6;
   moved->y = (col_cur->pos.y-old_pos.y)<<6;
   moved->z = (col_cur->pos.z-old_pos.z)<<6;
}

static void collision_intersects(Collider *a, Collider *b, RvR_fix22 *depth, RvR_vec2 *normal)
{
   normal->x = 0;
   normal->y = 0;
   *depth = 0;

   if(a->removed||b->removed)
      return;

   RvR_fix22 dx = b->pos.x-a->pos.x;
   RvR_fix22 dy = b->pos.y-a->pos.y;

   //Early out, overflow protection
   if(RvR_abs(dx)>a->radius+b->radius)
      return;
   if(RvR_abs(dy)>a->radius+b->radius)
      return;

   RvR_fix22 depth2 = (dx*dx+dy*dy)/1024;
   RvR_fix22 r = a->radius+b->radius;
   if(depth2<r)
   {
      *depth = RvR_fix22_sqrt(depth2);
      if(*depth==0)
      {
         normal->x = 0;
         normal->y = 1024;
      }
      else
      {
         normal->x = (dx*1024)/(*depth);
         normal->y = (dy*1024)/(*depth);
      }
      *depth = r-*depth;
   }
}

Collider *collision_new()
{
   if(collider_pool==NULL)
   {
      Collider *nc = RvR_malloc(sizeof(*nc)*256);
      memset(nc,0,sizeof(*nc)*256);
      
      for(int i = 0;i<256-1;i++)
         nc[i].next = &nc[i+1];
      collider_pool = nc;
   }

   Collider *c = collider_pool;
   collider_pool = c->next;
   c->next = NULL;
   c->prev_next = NULL;

   memset(c,0,sizeof(*c));

   return c;
}

void collision_add(Collider *c)
{
   c->prev_next = &colliders;
   if(colliders!=NULL)
      colliders->prev_next = &c->next;
   c->next = colliders;
   colliders = c;
}

void collision_remove(Collider *c)
{
   c->removed = 1;
}

void collision_free(Collider *c)
{
   if(c==NULL)
      return;

   *c->prev_next = c->next;
   if(c->next!=NULL)
      c->next->prev_next = c->prev_next;

   c->next = collider_pool;
   collider_pool = c;
}

void collision_clear()
{
   Collider *c = colliders;
   while(c!=NULL)
   {
      Collider *next = c->next;
      c->ent = NULL;
      collision_free(c);
      c = next;
   }
   colliders = NULL;

   Collider *pool = collider_pool;
   while(pool!=NULL)
   {
      Collider *next = pool->next;
      memset(pool,0,sizeof(*pool));
      pool = next;
   }
}

void collision_post()
{
   Collider *c = colliders;
   while(c!=NULL)
   {
      Collider *next = c->next;
      if(c->removed)
         collision_free(c);
      c = next;
   }
}
//-------------------------------------
