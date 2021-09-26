/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SLK/SLK.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR_error.h"
#include "RvR_math.h"
#include "RvR_malloc.h"
#include "RvR_pal.h"
#include "RvR_map.h"
#include "RvR_texture.h"
#include "RvR_raycast.h"
#include "RvR_raycast_draw.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   RvR_vec2       position;  ///< On-screen position.
   RvR_vec2       tex_coords; /**< Normalized (0 to RCL_UNITS_PER_SQUARE - 1)
                                texture coordinates. */
   RvR_fix22      depth;     ///< Corrected depth.
   int8_t         is_horizon; ///< If the pixel belongs to horizon segment.
   RvR_hit_result hit;       ///< Corresponding ray hit.
}pixel_info;

typedef struct
{
   int min;
   int max;
   RvR_fix22 height;
   uint16_t tex;
   uint16_t bottom[XRES+2];
   uint16_t top[XRES+2];
}Plane;

typedef struct
{
   RvR_vec2 s_pos;
   uint16_t s_depth; //Depth can be at max 1024*8, so we don't need to use the full 32bits, making faster sorting possible (radix)
   uint16_t tex;
}Sprite;
//-------------------------------------

//Variables
static SLK_Pal_sprite *target;
static RvR_fix22 depth_buffer[MAX_STEPS*2][XRES][4];

static RvR_fix22 start_floor_height = 0;
static RvR_fix22 start_ceil_height = 0;
static int32_t middle_row = 0;

static int planes_used;
static Plane planes[256];
static RvR_fix22 span_start[YRES];
static RvR_vec2 cam_dir0;
static RvR_vec2 cam_dir1;

struct
{
   Sprite *data; //Pointer to data0 or data1
   Sprite * restrict data0;
   Sprite * restrict data1;
   uint32_t data_used;
   uint32_t data_size;
}sprite_stack = {0};
//-------------------------------------

//Function prototypes
static void plane_add(RvR_fix22 height, uint16_t tex, int x, int y0, int y1);
static void span_horizontal_draw(int x0, int x1, int y, RvR_fix22 height, uint16_t texture);
static int16_t draw_wall(RvR_fix22 y_current, RvR_fix22 y_from, RvR_fix22 y_to, RvR_fix22 limit0, RvR_fix22 limit1, RvR_fix22 height, int16_t increment, pixel_info *pixel_info);
static int16_t draw_horizontal_column(RvR_fix22 y_current, RvR_fix22 y_to, RvR_fix22 limit0, RvR_fix22 limit1, int16_t increment, pixel_info *pixel_info);
static void draw_column(RvR_hit_result *hits, uint16_t hit_count, uint16_t x, RvR_ray ray);
static pixel_info map_to_screen(RvR_vec3 world_position);

static void sprite_stack_push(Sprite s);
static int sort(const void *a, const void *b);
//-------------------------------------

//Function implementations

void RvR_raycast_draw_sprite(RvR_vec3 pos, uint16_t tex)
{
   Sprite s = {0};
   pixel_info p = map_to_screen(pos);

   //Clip sprite
   //The more clipping we do here, the faster the sprite sorting will be
   if(p.depth<0||p.depth>8*RvR_fix22_one)
      return;
   //TODO: find a better way to do this, which requires
   //less tolerance (needed here to handle really close sprites)
   if(p.position.x<-2*XRES||p.position.x>4*XRES)
      return;

   //Only store the absolute minimum of information needed
   //to keep struct small and make sorting faster (alternative: sort array of pointers instead of structs)
   s.s_pos = p.position;
   s.s_depth = p.depth;
   s.tex = tex;

   sprite_stack_push(s);
}

void RvR_raycast_draw(RvR_vec3 cpos, RvR_fix22 cangle, int16_t cshear)
{
   //Update drawing target pointer
   //in case its location has changed (e.g. resize)
   target = SLK_draw_pal_get_target();

   //Clear depth buffer
   //but only to the depth needed for
   //max drawing distance
   for(int i = 0;i<XRES;i++)
   {
      depth_buffer[0][i][0] = 0;
      depth_buffer[0][i][2] = YRES;
   }
   for(int i = 1;i<16;i++)
      memcpy(depth_buffer[i],depth_buffer[0],sizeof(depth_buffer[0]));

   //Clear planes
   planes_used = 0;

   //Render walls and fill plane data
   RvR_raycast_set_position(cpos);
   RvR_raycast_set_angle(cangle);
   RvR_raycast_set_shear(cshear);

   middle_row = (YRES/2)+cshear;
   start_floor_height = RvR_map_floor_height_at(RvR_div_round_down(cpos.x,RvR_fix22_one),RvR_div_round_down(cpos.y,RvR_fix22_one))-1*cpos.z;
   start_ceil_height = RvR_map_ceiling_height_at(RvR_div_round_down(cpos.x,RvR_fix22_one),RvR_div_round_down(cpos.y,RvR_fix22_one))-1*cpos.z;

   RvR_cast_rays_multi_hit(draw_column);
   //-------------------------------------
   
   //Render floor planes
   cam_dir0 = RvR_vec2_rot(RvR_raycast_get_angle()-(HORIZONTAL_FOV/2));
   cam_dir1 = RvR_vec2_rot(RvR_raycast_get_angle()+(HORIZONTAL_FOV/2));
   RvR_fix22 cos = RvR_non_zero(RvR_fix22_cos(HORIZONTAL_FOV/2));
   cam_dir0.x = (cam_dir0.x*RvR_fix22_one)/cos;
   cam_dir0.y = (cam_dir0.y*RvR_fix22_one)/cos;
   cam_dir1.x = (cam_dir1.x*RvR_fix22_one)/cos;
   cam_dir1.y = (cam_dir1.y*RvR_fix22_one)/cos;

   for(int i = 0;i<planes_used;i++)
   {
      Plane *pl = &planes[i];

      if(pl->min>pl->max)
         continue;

      //Sky texture is rendered differently (vertical collumns instead of horizontal ones)
      if(pl->tex==0x43)
      {
         SLK_Pal_sprite *texture = RvR_texture_get(0x43);
         RvR_fix22 angle = (RvR_raycast_get_angle())*RvR_fix22_one;
         angle+=(pl->min-1)*ANGLE_STEP;

         for(int x = pl->min;x<pl->max+1;x++)
         {
            //Sky is rendered fullbright, no lut needed
            RvR_fix22 texture_coord = pl->top[x]*SKY_TEX_STEP;
            uint8_t * restrict pix = &target->data[(pl->top[x])*XRES+x-1];
            const uint8_t * restrict tex = &texture->data[((angle>>10)&255)*texture->height];

            for(int y = pl->top[x];y<pl->bottom[x]+1;y++)
            {
               *pix = tex[texture_coord>>10];
               texture_coord+=SKY_TEX_STEP;
               pix+=XRES;
            }

            angle+=ANGLE_STEP;
         }
         continue;
      }

      //Convert plane to horizontal spans
      for(int x = pl->min;x<pl->max+2;x++)
      {
         RvR_fix22 t0 = pl->top[x-1];
         RvR_fix22 t1 = pl->top[x];
         RvR_fix22 b0 = pl->bottom[x-1];
         RvR_fix22 b1 = pl->bottom[x];

         //End spans top
         for(;t0<t1&&t0<=b0;t0++)
            span_horizontal_draw(span_start[t0],x-1,t0,pl->height,pl->tex);

         //End spans bottom
         for(;b0>b1&&b0>=t0;b0--)
            span_horizontal_draw(span_start[b0],x-1,b0,pl->height,pl->tex);

         //Start spans top
         for(;t1<t0&&t1<=b1;t1++)
            span_start[t1] = x-1;

         //Start spans bottom
         for(;b1>b0&&b1>=t1;b1--)
            span_start[b1] = x-1;
      }
   }
   //-------------------------------------

   //Sort sprites
   qsort(sprite_stack.data,sprite_stack.data_used,sizeof(*sprite_stack.data),sort);
   //TODO: radix sort

   //Sprite rendering
   for(unsigned i = 0;i<sprite_stack.data_used;i++)
   {
      Sprite sp = sprite_stack.data[i];
      RvR_fix22 depth = sp.s_depth;
      SLK_Pal_sprite *texture = RvR_texture_get(sp.tex);
      int size_vertical = RvR_raycast_perspective_scale_vertical(texture->height*3,depth);
      int size_horizontal = RvR_raycast_perspective_scale_horizontal(texture->width*2,depth);
      int sx = 0;
      int sy = 0;
      int ex = size_horizontal;
      int ey = size_vertical;
      int x = sp.s_pos.x-size_horizontal/2;
      int y = sp.s_pos.y-size_vertical;
      int clip_depth = (depth)/DEPTH_BUFFER_PRECISION;
      RvR_fix22 u_step = (texture->width*RvR_fix22_one-1)/RvR_non_zero(size_horizontal);
      RvR_fix22 u = 0;
      RvR_fix22 v_step = (texture->height*RvR_fix22_one-1)/RvR_non_zero(size_vertical);
      RvR_fix22 v_start = 0;

      //Clip coordinates to screen
      if(x<0)
         sx = -x;
      if(y<0)
         sy = -y;
      if(x+ex>XRES)
         ex = size_horizontal+(XRES-x-ex);
      if(y+ey>YRES)
         ey = size_vertical+(YRES-y-ey);
      x = x<0?0:x;
      y = y<0?0:y;

      u = sx*u_step;
      v_start = sy*v_step;

      //Draw
      const uint8_t * restrict col = RvR_shade_table[RvR_min(63,10+22+(depth>>8))];
      uint8_t * restrict dst = NULL;
      const uint8_t * restrict tex = NULL;

      for(int x1 = sx;x1<ex;x1++,x++)
      {
         u+=u_step;

         //Clip against walls
         int ey1 = ey;
         int ys = y;
         for(int i = 0;i<clip_depth+1;i++)
         {
            if(depth>depth_buffer[i][x][3]&&y+(ey1-sy)>depth_buffer[i][x][2])
               ey1 = depth_buffer[i][x][2]-y+sy;
            if(depth>depth_buffer[i][x][1]&&ys<depth_buffer[i][x][0])
            {
               int diff = ys-depth_buffer[i][x][0];
               ys = depth_buffer[i][x][0];
               ey1+=diff;
            }
         }

         tex = &texture->data[texture->height*(u>>10)];
         dst = &target->data[ys*XRES+x];
         RvR_fix22 v = v_start+(ys-y)*v_step;
         for(int y1 = sy;y1<ey1;y1++,dst+=XRES)
         {
            uint8_t index = tex[v>>10];
            *dst = index?col[index]:*dst;
            v+=v_step;
         }
      }
   }

   sprite_stack.data_used = 0;
}

static int16_t draw_wall(RvR_fix22 y_current, RvR_fix22 y_from, RvR_fix22 y_to, RvR_fix22 limit0, RvR_fix22 limit1, RvR_fix22 height, int16_t increment, pixel_info *pixel_info)
{
   int16_t limit = RvR_clamp(y_to,limit0,limit1);

   //Sky texture is handled differently and instead added as a plane
   if(pixel_info->hit.wall_tex==0x43)
   {
      if(increment==-1)
      {
         RvR_fix22 f_start = limit;
         RvR_fix22 f_end = y_current+increment;
         if(f_end<f_start)
            return limit;
         plane_add(pixel_info->is_horizon?-RvR_INFINITY:pixel_info->hit.fheight,pixel_info->hit.floor_tex,pixel_info->position.x,f_start,f_end);
      }
      else if(increment==1)
      {
         RvR_fix22 c_start = y_current+increment;
         RvR_fix22 c_end = limit;
         if(c_end<c_start)
            return limit;
         plane_add(pixel_info->is_horizon?RvR_INFINITY:pixel_info->hit.cheight,pixel_info->hit.ceil_tex,pixel_info->position.x,c_start,c_end);
      }

      return limit;
   }

   SLK_Pal_sprite *texture = RvR_texture_get(pixel_info->hit.wall_tex);
   height = RvR_abs(height);
   RvR_fix22 wall_length = RvR_non_zero(RvR_abs(y_to-y_from-1));
   RvR_fix22 wall_position = RvR_abs(y_from-y_current)-increment;
   RvR_fix22 height_scaled = height*RvR_fix22_one;
   RvR_fix22 coord_step_scaled = (height_scaled/wall_length);
   RvR_fix22 texture_coord_scaled = 0;

   if(increment==-1)
   {
      texture_coord_scaled = height_scaled-(wall_position*coord_step_scaled);
      coord_step_scaled*=-1;
   }
   else if(increment==1)
   {
      texture_coord_scaled = RvR_zero_clamp(wall_position*coord_step_scaled);
   }

   uint8_t * restrict pix = &target->data[(y_current+increment)*XRES+pixel_info->position.x];
   const uint8_t * restrict col = RvR_shade_table[RvR_min(63,(pixel_info->hit.direction&1)*10+22+(pixel_info->depth>>8))];
   const uint8_t * restrict tex = &texture->data[(pixel_info->tex_coords.x>>4)*texture->height];

   if(increment==-1)
   {
      if(texture->height==128)
      {
         for(RvR_fix22 i = y_current+increment;i>=limit;i--)
         {
            texture_coord_scaled+=coord_step_scaled;
            *pix = col[tex[(texture_coord_scaled&2097151)>>14]];
            pix-=XRES;
         }
      }
      else
      {
         for(RvR_fix22 i = y_current+increment;i>=limit;i--)
         {
            texture_coord_scaled+=coord_step_scaled;
            *pix = col[tex[(texture_coord_scaled&1048575)>>14]];
            pix-=XRES;
         }
      }
   }
   else if(increment==1)
   {
      if(texture->height==128)
      {
         for(RvR_fix22 i = y_current+increment;i<=limit;i++)
         {
            texture_coord_scaled+=coord_step_scaled;
            *pix = col[tex[(texture_coord_scaled&2097151)>>14]];
            pix+=XRES;
         }
      }
      else
      {
         for(RvR_fix22 i = y_current+increment;i<=limit;i++)
         {
            texture_coord_scaled+=coord_step_scaled;
            *pix = col[tex[(texture_coord_scaled&1048575)>>14]];
            pix+=XRES;
         }
      }
   }

   return limit;
}

static int16_t draw_horizontal_column(RvR_fix22 y_current, RvR_fix22 y_to, RvR_fix22 limit0, RvR_fix22 limit1, int16_t increment, pixel_info *pixel_info)
{
   int16_t limit = RvR_clamp(y_to,limit0,limit1);

   if(increment==-1)
   {
      RvR_fix22 f_start = limit;
      RvR_fix22 f_end = y_current+increment;
      if(f_end<f_start)
         return limit;
      plane_add(pixel_info->is_horizon?-RvR_INFINITY:pixel_info->hit.fheight,pixel_info->hit.floor_tex,pixel_info->position.x,f_start,f_end);
   }
   else if(increment==1)
   {
      RvR_fix22 c_start = y_current+increment;
      RvR_fix22 c_end = limit;
      if(c_end<c_start)
         return limit;
      plane_add(pixel_info->is_horizon?RvR_INFINITY:pixel_info->hit.cheight,pixel_info->hit.ceil_tex,pixel_info->position.x,c_start,c_end);
   }

   return limit;
}

static void draw_column(RvR_hit_result *hits, uint16_t hit_count, uint16_t x, RvR_ray ray)
{
   //last written Y position, can never go backwards
   RvR_fix22 f_pos_y = YRES;
   RvR_fix22 c_pos_y = -1;

   //world coordinates (relative to camera height though)
   RvR_fix22 f_z1_world = start_floor_height;
   RvR_fix22 c_z1_world = start_ceil_height;

   pixel_info p = {0};
   p.position.x = x;
   p.tex_coords.x = 0;
   p.tex_coords.y = 0;

   int limit_c = 0;
   int limit_f = 0;

   //we'll be simulatenously drawing the floor and the ceiling now  
   for(RvR_fix22 j = 0;j<=hit_count;++j)
   {                    //^ = add extra iteration for horizon plane
      int8_t drawing_horizon = j==hit_count;

      RvR_hit_result hit;
      RvR_fix22 distance = 1;

      RvR_fix22 f_wall_height = 0, c_wall_height = 0;
      RvR_fix22 f_z2_world = 0,    c_z2_world = 0;
      RvR_fix22 f_z1_screen = 0,   c_z1_screen = 0;
      RvR_fix22 f_z2_screen = 0,   c_z2_screen = 0;

      if(!drawing_horizon)
      {
         hit = hits[j];
         distance = RvR_non_zero(hit.distance); 
         p.hit = hit;

         f_wall_height = RvR_map_floor_height_at(hit.square.x,hit.square.y);
         f_z2_world = f_wall_height-RvR_raycast_get_position().z;
         f_z1_screen = middle_row-RvR_raycast_perspective_scale_vertical((f_z1_world*YRES)/RvR_fix22_one,distance);
         f_z2_screen = middle_row-RvR_raycast_perspective_scale_vertical((f_z2_world*YRES)/RvR_fix22_one,distance);

         c_wall_height = RvR_map_ceiling_height_at(hit.square.x,hit.square.y);
         c_z2_world = c_wall_height-RvR_raycast_get_position().z;
         c_z1_screen = middle_row-RvR_raycast_perspective_scale_vertical((c_z1_world*YRES)/RvR_fix22_one,distance);
         c_z2_screen = middle_row-RvR_raycast_perspective_scale_vertical((c_z2_world*YRES)/RvR_fix22_one,distance);
      }
      else
      {
         f_z1_screen = middle_row;
         c_z1_screen = middle_row+1;

         p.hit = hits[j-1];
         p.hit.distance = RvR_fix22_one*RvR_fix22_one;
         /* ^ horizon is at infinity, but we can't use too big infinity
              because it would overflow in the following mult. */
         p.hit.position.x = (ray.direction.x*p.hit.distance)/RvR_fix22_one;
         p.hit.position.y = (ray.direction.y*p.hit.distance)/RvR_fix22_one;

         p.hit.direction = 0;
         p.hit.texture_coord = 0;
         p.hit.wall_tex = 0;
         //p.hit.floor_tex = 0;
         p.hit.ceil_tex = 0x43;
      }

      RvR_fix22 limit;

      p.is_horizon = drawing_horizon;

      //draw floor until wall
      p.depth = ((middle_row+(YRES/2))-f_pos_y)*(HORIZON_DEPTH/YRES);

      limit = draw_horizontal_column(f_pos_y,f_z1_screen,c_pos_y+1,
                                                 YRES,-1,
                                                 //^ purposfully allow outside screen bounds
                                                 &p);

      limit_f = limit;

      if(f_pos_y>limit)
         f_pos_y = limit;

      //draw ceiling until wall
      p.depth = (c_pos_y-(middle_row-(YRES/2)))*(HORIZON_DEPTH/YRES);

      limit = draw_horizontal_column(c_pos_y,c_z1_screen,
                                                -1,f_pos_y-1,1,&p);
                                                //^ purposfully allow outside screen bounds here

      limit_c = limit;

      if(c_pos_y<limit)
         c_pos_y = limit;

      if(!drawing_horizon) //don't draw walls for horizon plane
      {
         p.depth = distance;
         p.depth = RvR_max(0,RvR_min(p.depth,(MAX_STEPS-1)*DEPTH_BUFFER_PRECISION));
         p.tex_coords.x = hit.texture_coord;

         //draw floor wall
         if(f_pos_y>0)  //still pixels left?
         {
            limit = draw_wall(f_pos_y,f_z1_screen,f_z2_screen,c_pos_y+1,
                                          YRES,
                                          //^ purposfully allow outside screen bounds here
                                          f_z2_world-f_z1_world
                                          ,-1,&p);

            if(depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][2]>limit)
            {
               depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][2] = limit;
               depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][3] = p.depth;
            }

            if(depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][2]>limit_f)
            {
               depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][2] = limit_f;
               depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][3] = p.depth;
            }

            if(f_pos_y>limit)
               f_pos_y = limit;

            f_z1_world = f_z2_world; //for the next iteration
         }               //^ purposfully allow outside screen bounds here

         //draw ceiling wall
         if(c_pos_y<YRES-1) //pixels left?
         {
            limit = draw_wall(c_pos_y,c_z1_screen,c_z2_screen,
                              -1,f_pos_y-1,
                              //^ puposfully allow outside screen bounds here
                              c_z1_world-c_z2_world 
                              ,1,&p);

            p.depth = RvR_max(0,RvR_min(p.depth,(MAX_STEPS-1)*DEPTH_BUFFER_PRECISION));
            if(depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][0]<limit)
            {
               depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][0] = limit;
               depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][1] = p.depth;
            }

            if(depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][0]<limit_c)
            {
               depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][0] = limit_c;
               depth_buffer[p.depth/DEPTH_BUFFER_PRECISION][p.position.x][1] = p.depth;
            }

            if(c_pos_y<limit)
               c_pos_y = limit;

            c_z1_world = c_z2_world; //for the next iteration
         }              //^ puposfully allow outside screen bounds here 
      }
   }
}

static pixel_info map_to_screen(RvR_vec3 world_position)
{
   pixel_info result;

   RvR_vec2 to_point;

   to_point.x = world_position.x-RvR_raycast_get_position().x;
   to_point.y = world_position.y-RvR_raycast_get_position().y;

   RvR_fix22 middle_column = XRES/2;

   //rotate the point to camera space (y left/right, x forw/backw)
   //TODO: cache cosine, sinus values

   RvR_fix22 cos = RvR_fix22_cos(RvR_raycast_get_angle());
   RvR_fix22 sin = RvR_fix22_sin(RvR_raycast_get_angle());

   RvR_fix22 tmp = to_point.x;

   to_point.x = (to_point.x*cos-to_point.y*sin)/RvR_fix22_one; 
   to_point.y = (tmp*sin+to_point.y*cos)/RvR_fix22_one; 

   result.depth = to_point.x;

   result.position.x = middle_column-(RvR_raycast_perspective_scale_horizontal(to_point.y,result.depth)*middle_column)/RvR_fix22_one;

   result.position.y = (RvR_raycast_perspective_scale_vertical(world_position.z-RvR_raycast_get_position().z,result.depth)*YRES)/RvR_fix22_one;

   result.position.y = YRES/2-result.position.y+RvR_raycast_get_shear();

   return result;
}

static void plane_add(RvR_fix22 height, uint16_t tex, int x, int y0, int y1)
{
   Plane *cur = NULL;
   int i;
   x++;

   //TODO: hash + linked list?
   for(i = 0;i<planes_used;i++)
   {
      if(height!=planes[i].height)
         continue;
      if(tex!=planes[i].tex)
         continue;

      if(planes[i].top[x]!=UINT16_MAX)
      {
         //Concat planes vertically, only works if the spans are directly adjacent
         if(planes[i].top[x]-1==y1)
         {
            planes[i].top[x] = y0;
            return;
         }
         if(planes[i].bottom[x]+1==y0)
         {
            planes[i].bottom[x] = y1;
            return;
         }

         continue;
      }

      break;
   }


   //Add new plane
   if(i==planes_used)
   {
      if(planes_used==256)
         puts("Well... shit");

      cur = &planes[planes_used++];
      cur->min = XRES;
      cur->max = -1;
      cur->height = height;
      cur->tex = tex;

      //Since this is an unsigned int, we can use memset to set all values to 65535 (0xffff)
      memset(cur->top,255,sizeof(cur->top));
   }
   else
   {
      cur = &planes[i];
   }

   if(x<cur->min)
      cur->min = x;
   if(x>cur->max)
      cur->max = x;

   cur->bottom[x] = y1;
   cur->top[x] = y0;
}

static void span_horizontal_draw(int x0, int x1, int y, RvR_fix22 height, uint16_t texture)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   //Calculate the depth of the row to be rendered
   RvR_fix22 cam_height_screen_size = RvR_abs((RvR_raycast_get_position().z-height)*YRES)/1024;
   RvR_fix22 depth = RvR_raycast_perspective_scale_vertical_inverse(cam_height_screen_size,RvR_abs(y-middle_row));

   //Calculate texture mapping step size and starting coordinates
   RvR_fix22 step_x = (depth*(cam_dir1.x-cam_dir0.x))/XRES;
   RvR_fix22 step_y = (depth*(cam_dir1.y-cam_dir0.y))/XRES;
   RvR_fix22 tx = (RvR_raycast_get_position().x&1023)*1024+(depth*cam_dir0.x);
   RvR_fix22 ty = (RvR_raycast_get_position().y&1023)*1024+(depth*cam_dir0.y);
   tx+=x0*step_x;
   ty+=x0*step_y;

   //const and restrict don't influence the generated assembly
   uint8_t * restrict pix = &target->data[y*XRES+x0];
   const uint8_t * restrict col = RvR_shade_table[RvR_min(63,22+(depth>>8))];
   const uint8_t * restrict tex = RvR_texture_get(texture)->data;

   for(int x = x0;x<x1;x++)
   {
      *pix = col[tex[((tx>>14)&63)*64+((ty>>14)&63)]];
      tx+=step_x;
      ty+=step_y;
      pix++;
   }
}

static void sprite_stack_push(Sprite s)
{
   if(sprite_stack.data==NULL)
   {
      sprite_stack.data_size = 64;
      sprite_stack.data0 = RvR_malloc(sizeof(*sprite_stack.data0)*sprite_stack.data_size);
      sprite_stack.data1 = RvR_malloc(sizeof(*sprite_stack.data1)*sprite_stack.data_size);
      sprite_stack.data = sprite_stack.data0;
   }

   sprite_stack.data[sprite_stack.data_used++] = s;

   if(sprite_stack.data_used==sprite_stack.data_size)
   {
      sprite_stack.data_size+=64;
      sprite_stack.data0 = RvR_realloc(sprite_stack.data0,sizeof(*sprite_stack.data0)*sprite_stack.data_size);
      sprite_stack.data1 = RvR_realloc(sprite_stack.data1,sizeof(*sprite_stack.data1)*sprite_stack.data_size);
      sprite_stack.data = sprite_stack.data0;
   }
}

static int sort(const void *a, const void *b)
{
   return ((Sprite *)b)->s_depth-((Sprite *)a)->s_depth;
}
//-------------------------------------
