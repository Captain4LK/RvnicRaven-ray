/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
typedef struct ray_plane
{
   int32_t min;
   int32_t max;
   RvR_fix22 height;
   uint16_t tex;
   uint16_t start[RVR_XRES+2];
   uint16_t end[RVR_XRES+2];

   struct ray_plane *next;
}ray_plane;

typedef struct
{
   RvR_vec3 p;
   RvR_vec2 p0;
   RvR_vec2 p1;
   RvR_vec3 sp;
   RvR_vec3 sp0;
   RvR_fix22 st0;
   RvR_vec3 sp1;
   RvR_fix22 st1;
   uint16_t texture;
   uint32_t flags;
   RvR_fix22 angle;
}ray_sprite;
//-------------------------------------

//Variables
static RvR_ray_depth_buffer ray_depth_buffer = {0};
static ray_plane *ray_planes[128] = {0};

//Linked list pools
static ray_plane *ray_plane_pool = NULL;
static RvR_ray_depth_buffer_entry *ray_depth_buffer_entry_pool = NULL;

static RvR_fix22 ray_start_floor_height = 0;
static RvR_fix22 ray_start_ceil_height = 0;
static int32_t ray_middle_row = 0;

struct
{
   ray_sprite * restrict data;
   uint_fast16_t * restrict data_proxy;
   uint32_t data_used;
   uint32_t data_size;
}ray_sprite_stack = {0};

static RvR_fix22 ray_fov_factor_x;
static RvR_fix22 ray_fov_factor_y;
static RvR_fix22 ray_sin;
static RvR_fix22 ray_cos;
static RvR_fix22 ray_sin_fov;
static RvR_fix22 ray_cos_fov;
//-------------------------------------

//Function prototypes
static void ray_plane_add(RvR_fix22 height, uint16_t tex, int x, int y0, int y1);

static void ray_span_draw_tex(int x0, int x1, int y, RvR_fix22 height, const RvR_texture *texture);
static void ray_span_draw_flat(int x0, int x1, int y, uint8_t color);

static int16_t ray_draw_wall(RvR_fix22 y_current, RvR_fix22 y_from, RvR_fix22 y_to, RvR_fix22 limit0, RvR_fix22 limit1, RvR_fix22 height, int16_t increment, RvR_ray_pixel_info *pixel_info, RvR_ray_hit_result *hit);
static void ray_draw_column(RvR_ray_hit_result *hits, int hits_len, uint16_t x, RvR_ray ray);

static void ray_sprite_stack_push(ray_sprite s);

static int ray_sprite_order(int a, int b);
static void ray_sprite_draw_wall(ray_sprite *sp);
static void ray_sprite_draw_floor(ray_sprite *sp);
static void ray_sprite_draw_billboard(ray_sprite *sp);

static RvR_ray_depth_buffer_entry *ray_depth_buffer_entry_new();
static void ray_depth_buffer_entry_free(RvR_ray_depth_buffer_entry *ent);

static ray_plane *ray_plane_new();
static void ray_plane_free(ray_plane *pl);
//-------------------------------------

//Function implementations

void RvR_ray_draw_begin()
{
   //Clear depth buffer
   for(int i = 0;i<RVR_XRES;i++)
   {
      ray_depth_buffer_entry_free(ray_depth_buffer.floor[i]);
      ray_depth_buffer_entry_free(ray_depth_buffer.ceiling[i]);

      ray_depth_buffer.floor[i] = NULL;
      ray_depth_buffer.ceiling[i] = NULL;
   }

   //Clear planes
   for(int i = 0;i<128;i++)
   {
      ray_plane_free(ray_planes[i]);
      ray_planes[i] = NULL;
   }

   //Initialize needed vars
   ray_fov_factor_x = RvR_fix22_tan(RvR_ray_get_fov()/2);
   ray_fov_factor_y = (RVR_YRES*ray_fov_factor_x*2)/RVR_XRES;

   ray_middle_row = (RVR_YRES/2)+RvR_ray_get_shear();

   ray_start_floor_height = RvR_ray_map_floor_height_at(RvR_ray_get_position().x/1024,RvR_ray_get_position().y/1024)-RvR_ray_get_position().z;
   ray_start_ceil_height = RvR_ray_map_ceiling_height_at(RvR_ray_get_position().x/1024,RvR_ray_get_position().y/1024)-RvR_ray_get_position().z;

   ray_cos = RvR_fix22_cos(RvR_ray_get_angle());
   ray_sin = RvR_fix22_sin(RvR_ray_get_angle());
   ray_cos_fov = (ray_cos*ray_fov_factor_x)/1024;
   ray_sin_fov = (ray_sin*ray_fov_factor_x)/1024;
}

void RvR_ray_draw_end()
{
   while(ray_sprite_stack.data_used>0)
   {
      uint8_t certain[ray_sprite_stack.data_used];
      memset(certain,0,sizeof(*certain)*ray_sprite_stack.data_used);
      int16_t far = 0;
      certain[far] = 1;
      int done = 0;

      do
      {
         done = 1;
         for(int i = 0;i<ray_sprite_stack.data_used;i++)
         {
            if(certain[i])
               continue;

            int order = ray_sprite_order(i,far);
            if(order<0)
               continue;

            certain[i] = 1;
            if(!order)
            {
               done = 0;
               far = i;
               certain[far] = 1;
            }
         }
      }while(!done);

      if(ray_sprite_stack.data[ray_sprite_stack.data_proxy[far]].flags&8)
         ray_sprite_draw_wall(&ray_sprite_stack.data[ray_sprite_stack.data_proxy[far]]);
      else if(ray_sprite_stack.data[ray_sprite_stack.data_proxy[far]].flags&16)
         ray_sprite_draw_floor(&ray_sprite_stack.data[ray_sprite_stack.data_proxy[far]]);
      else
         ray_sprite_draw_billboard(&ray_sprite_stack.data[ray_sprite_stack.data_proxy[far]]);

      ray_sprite_stack.data_used--;
      ray_sprite_stack.data_proxy[far] = ray_sprite_stack.data_proxy[ray_sprite_stack.data_used];
   }

   ray_sprite_stack.data_used = 0;
}

RvR_ray_depth_buffer *RvR_ray_draw_depth_buffer()
{
   return &ray_depth_buffer;
}

//raycastlib by drummyfish: https://gitlab.com/drummyfish/raycastlib
//Heavily modified: more efficient wall rendering, added floor/ceiling rendering
//Original header:
//raycastlib (RCL) - Small C header-only raycasting library for embedded and
//low performance computers, such as Arduino. Only uses integer math and stdint
//standard library.

void RvR_ray_draw_sprite(RvR_vec3 pos, RvR_fix22 angle, uint16_t tex, uint32_t flags)
{
   //Sprite tagged as invisible --> don't draw
   if(flags&1)
      return;

   ray_sprite sprite_new = {0};
   sprite_new.texture = tex;
   sprite_new.flags = flags;

   //Wall alligned sprite
   if(flags&8)
   {
      //Translate sprite to world space
      RvR_vec2 dir = RvR_vec2_rot(angle);
      RvR_fix22 half_width = RvR_texture_get(tex)->width*8;
      sprite_new.p0.x = (-dir.y*half_width)/1024+pos.x;
      sprite_new.p0.y = (dir.x*half_width)/1024+pos.y;
      sprite_new.p1.x = (dir.y*half_width)/1024+pos.x;
      sprite_new.p1.y = (-dir.x*half_width)/1024+pos.y;
      sprite_new.p = pos;
      sprite_new.st0 = 0;
      sprite_new.st1 = 1023;
      sprite_new.angle = angle;

      //Translate to camera space
      RvR_fix22 x0 = sprite_new.p0.x-RvR_ray_get_position().x;
      RvR_fix22 y0 = sprite_new.p0.y-RvR_ray_get_position().y;
      RvR_fix22 x1 = sprite_new.p1.x-RvR_ray_get_position().x;
      RvR_fix22 y1 = sprite_new.p1.y-RvR_ray_get_position().y;
      RvR_vec2 to_point0;
      RvR_vec2 to_point1;
      to_point0.x = (-x0*ray_sin+y0*ray_cos)/1024; 
      to_point0.y = (x0*ray_cos_fov+y0*ray_sin_fov)/1024; 
      to_point1.x = (-x1*ray_sin+y1*ray_cos)/1024; 
      to_point1.y = (x1*ray_cos_fov+y1*ray_sin_fov)/1024; 

      //Behind camera
      if(to_point0.y<-128&&to_point1.y<-128)
         return;

      //Sprite not facing camera
      //--> swap p0 and p1 and toggle y-axis mirror flag
      if((to_point0.x*to_point1.y-to_point1.x*to_point0.y)/65536>0)
      {
         RvR_vec2 tmp = to_point0;
         to_point0 = to_point1;
         to_point1 = tmp;
         sprite_new.flags^=2;
      }

      //Here we can treat everything as if we have a 90 degree
      //fov, since the rotation to camera space transforms it to
      //that
      //Check if in fov
      //Left point in fov
      if(to_point0.x>=-to_point0.y)
      {
         //Sprite completely out of sight
         if(to_point0.x>to_point0.y)
            return;

         sprite_new.sp0.x = RvR_min(RVR_XRES/2+(to_point0.x*(RVR_XRES/2))/to_point0.y,RVR_XRES-1);
         sprite_new.sp0.z = to_point0.y;
      }
      //Left point to the left of fov
      else
      {
         //Sprite completely out of sight
         if(to_point1.x<-to_point1.y)
            return;

         sprite_new.sp0.x = 0;

         //Basically just this equation: (0,0)+n(-1,1) = (to_point0.x,to_point0.y)+m(to_point1.x-to_point0.x,to_point1.y-to_point0.y), reordered to n = ...
         //This is here to circumvent a multiplication overflow while also minimizing the resulting error
         //TODO: the first version might be universally better, making the if statement useless
         RvR_fix22 dx0 = to_point1.x-to_point0.x;
         RvR_fix22 dx1 = to_point0.y+to_point0.x;
         if(RvR_abs(dx0)>RvR_abs(dx1))
            sprite_new.sp0.z = (dx1*((dx0*1024)/RvR_non_zero(to_point1.y-to_point0.y+to_point1.x-to_point0.x)))/1024-to_point0.x;
         else
            sprite_new.sp0.z = (dx0*((dx1*1024)/RvR_non_zero(to_point1.y-to_point0.y+to_point1.x-to_point0.x)))/1024-to_point0.x;

         sprite_new.st0 = ((to_point0.x+to_point0.y)*1024)/RvR_non_zero(-to_point1.y+to_point0.y-to_point1.x+to_point0.x);
      }

      //Right point in fov
      if(to_point1.x<=to_point1.y)
      {
         //sprite completely out of sight
         if(to_point1.x<-to_point1.y)
            return;

         sprite_new.sp1.x = RvR_min(RVR_XRES/2+(to_point1.x*(RVR_XRES/2))/to_point1.y-1,RVR_XRES-1);
         sprite_new.sp1.z = to_point1.y;
      }
      //Right point to the right of fov
      else
      {
         //sprite completely out of sight
         if(to_point0.x>to_point0.y)
            return;

         sprite_new.sp1.x = RVR_XRES-1;

         //Basically just this equation: (0,0)+n(1,1) = (to_point0.x,to_point0.y)+m(to_point1.x-to_point0.x,to_point1.y-to_point0.y), reordered to n = ...
         //This is here to circumvent a multiplication overflow while also minimizing the resulting error
         //TODO: the first version might be universally better, making the if statement useless
         RvR_fix22 dx0 = to_point1.x-to_point0.x;
         RvR_fix22 dx1 = to_point0.y-to_point0.x;
         if(RvR_abs(dx0)>RvR_abs(dx1))
            sprite_new.sp1.z = to_point0.x-(dx1*((dx0*1024)/RvR_non_zero(to_point1.y-to_point0.y-to_point1.x+to_point0.x)))/1024;
         else
            sprite_new.sp1.z = to_point0.x-(dx0*((dx1*1024)/RvR_non_zero(to_point1.y-to_point0.y-to_point1.x+to_point0.x)))/1024;

         sprite_new.st1 = ((to_point0.y-to_point0.x)*1024)/RvR_non_zero(-to_point1.y+to_point0.y+to_point1.x-to_point0.x);
      }

      //Near clip sprite 
      if(sprite_new.sp0.z<16||sprite_new.sp1.z<16)
         return;

      //Far clip sprite
      if(sprite_new.sp0.z>RVR_RAY_MAX_STEPS*1024&&sprite_new.sp1.z>RVR_RAY_MAX_STEPS*1024)
         return;

      if(sprite_new.sp0.x>sprite_new.sp1.x)
         return;

      sprite_new.sp0.y = ((sprite_new.p.z-RvR_ray_get_position().z)*1024)/RvR_non_zero((ray_fov_factor_y*sprite_new.sp0.z)/1024);
      sprite_new.sp0.y = RVR_YRES*512-RVR_YRES*sprite_new.sp0.y+RvR_ray_get_shear()*1024;
      sprite_new.sp1.y = ((sprite_new.p.z-RvR_ray_get_position().z)*1024)/RvR_non_zero((ray_fov_factor_y*sprite_new.sp1.z)/1024);
      sprite_new.sp1.y = RVR_YRES*512-RVR_YRES*sprite_new.sp1.y+RvR_ray_get_shear()*1024;

      if(sprite_new.flags&2)
      {
         sprite_new.st0 = 1023-sprite_new.st0;
         sprite_new.st1 = 1023-sprite_new.st1;
      }

      ray_sprite_stack_push(sprite_new);

      return;
   }

   //Floor alligned sprite
   if(flags&16)
   {
      return;
   }

   //Billboarded sprite

   //Translate sprite to world space coordinates
   RvR_fix22 half_width = (RvR_texture_get(tex)->width*1024/64)/2;
   sprite_new.p0.x = (-ray_sin*half_width)/1024+pos.x;
   sprite_new.p0.y = (ray_cos*half_width)/1024+pos.y;
   sprite_new.p1.x = (ray_sin*half_width)/1024+pos.x;
   sprite_new.p1.y = (-ray_cos*half_width)/1024+pos.y;
   sprite_new.p = pos;

   //Translate to camera space
   RvR_fix22 x0 = sprite_new.p0.x-RvR_ray_get_position().x;
   RvR_fix22 y0 = sprite_new.p0.y-RvR_ray_get_position().y;
   RvR_fix22 x1 = sprite_new.p1.x-RvR_ray_get_position().x;
   RvR_fix22 y1 = sprite_new.p1.y-RvR_ray_get_position().y;
   RvR_vec2 to_point0;
   RvR_vec2 to_point1;
   to_point0.x = (-x0*ray_sin+y0*ray_cos)/1024; 
   to_point0.y = (x0*ray_cos_fov+y0*ray_sin_fov)/1024; 
   to_point1.x = (-x1*ray_sin+y1*ray_cos)/1024; 
   to_point1.y = (x1*ray_cos_fov+y1*ray_sin_fov)/1024; 

   //Behind camera
   if(to_point0.y<-128&&to_point1.y<-128)
      return;

   //Sprite not facing camera
   //--> swap p0 and p1
   if((to_point0.x*to_point1.y-to_point1.x*to_point0.y)/65536>0)
   {
      RvR_vec2 tmp = to_point0;
      to_point0 = to_point1;
      to_point1 = tmp;
   }

   //Here we can treat everything as if we have a 90 degree
   //fov, since the rotation to camera space transforms it to
   //that
   //Check if in fov
   //Left point in fov
   if(to_point0.x>=-to_point0.y)
   {
      if(to_point0.x>to_point0.y)
         return;

      sprite_new.sp0.x = RvR_min(RVR_XRES/2+(to_point0.x*(RVR_XRES/2))/to_point0.y,RVR_XRES-1);
   }
   //Left point to the left of fov
   else
   {
      if(to_point1.x<-to_point1.y)
         return;

      sprite_new.sp0.x = 0;
   }

   //Right point in fov
   if(to_point1.x<=to_point1.y)
   {
      if(to_point1.x<-to_point1.y)
         return;

      sprite_new.sp1.x = RvR_min(RVR_XRES/2+(to_point1.x*(RVR_XRES/2))/to_point1.y-1,RVR_XRES-1);
   }
   else
   {
      if(to_point0.x>to_point0.y)
         return;
      sprite_new.sp1.x = RVR_XRES-1;
   }

   //Project to screen
   RvR_ray_pixel_info p = RvR_ray_map_to_screen(sprite_new.p);
   sprite_new.sp.x = p.position.x;
   sprite_new.sp.y = p.position.y;
   sprite_new.sp.z = p.depth;

   //Clipping
   //Behind camera
   if(sprite_new.sp.z<=0)
      return;
   //Too far away
   if(sprite_new.sp.z>RVR_RAY_MAX_STEPS*1024)
      return;

   ray_sprite_stack_push(sprite_new);
}

void RvR_ray_draw_map()
{
   RvR_fix22 ray_span_start[RVR_YRES];

   //Render walls and fill plane data
   RvR_rays_cast_multi_hit_draw(ray_draw_column);
   //-------------------------------------
   
   //Render floor planes
   for(int i = 0;i<128;i++)
   {
      ray_plane *pl = ray_planes[i];
      while(pl!=NULL)
      {
         if(pl->min>pl->max)
            goto next;

         //Sky texture is rendered differently (vertical collumns instead of horizontal ones)
         if(pl->tex==RvR_ray_map_sky_tex())
         {
            RvR_texture *texture = RvR_texture_get(RvR_ray_map_sky_tex());
            int skyw = 1<<RvR_log2(texture->width);
            int skyh = 1<<RvR_log2(texture->height);
            int mask = skyh-1;

            RvR_fix22 angle_step = (skyw*1024)/RVR_XRES;
            RvR_fix22 tex_step = (1024*skyh-1)/RVR_YRES;

            RvR_fix22 angle = (RvR_ray_get_angle())*256;
            angle+=(pl->min-1)*angle_step;

            for(int x = pl->min;x<pl->max+1;x++)
            {
               //Sky is rendered fullbright, no lut needed
               uint8_t * restrict pix = &RvR_core_framebuffer()[(pl->start[x])*RVR_XRES+x-1];
               const uint8_t * restrict tex = &texture->data[((angle>>10)&(skyw-1))*skyh];
               const uint8_t * restrict col = RvR_shade_table(32);

               //Split in two parts: above and below horizon
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
                  *pix = col[tex[(texture_coord>>10)&mask]];
                  texture_coord-=tex_step;
                  pix+=RVR_XRES;
               }

               angle+=angle_step;
            }

            goto next;
         }

         //Convert plane to horizontal spans
         RvR_texture *texture = RvR_texture_get(pl->tex);
         for(int x = pl->min;x<pl->max+2;x++)
         {
            RvR_fix22 s0 = pl->start[x-1];
            RvR_fix22 s1 = pl->start[x];
            RvR_fix22 e0 = pl->end[x-1];
            RvR_fix22 e1 = pl->end[x];

            //End spans top
            for(;s0<s1&&s0<=e0;s0++)
            {
#if RVR_RAY_DRAW_PLANES==1
               ray_span_draw_flat(ray_span_start[s0],x-1,s0,(i+1)&255);
#elif RVR_RAY_DRAW_PLANES==2
               ray_span_draw_tex(ray_span_start[s0],x-1,s0,pl->height,texture);
#endif
            }

            //End spans bottom
            for(;e0>e1&&e0>=s0;e0--)
            {
#if RVR_RAY_DRAW_PLANES==1
               ray_span_draw_flat(ray_span_start[e0],x-1,e0,(i+1)&255);
#elif RVR_RAY_DRAW_PLANES==2
               ray_span_draw_tex(ray_span_start[e0],x-1,e0,pl->height,texture);
#endif
            }

            //Start spans top
            for(;s1<s0&&s1<=e1;s1++)
               ray_span_start[s1] = x-1;

            //Start spans bottom
            for(;e1>e0&&e1>=s1;e1--)
               ray_span_start[e1] = x-1;
         }

next:
         pl = pl->next;
      }
   }
   //-------------------------------------
}

void RvR_ray_draw_debug(uint8_t index)
{
   char tmp[128];
   snprintf(tmp,128,"%03d.%01d ms\n",RvR_core_frametime_average()/10,RvR_core_frametime_average()%10);
   RvR_draw_string(2,2,1,tmp,index);

   for(int i = 0;i<128;i++)
   {
      int num = 0;
      ray_plane *pl = ray_planes[i];
      while(pl!=NULL)
      {
         pl = pl->next;
         num++;
      }
      RvR_draw_vertical_line(RVR_XRES-128+i,RVR_YRES-num,RVR_YRES,index);
   }
}

static int16_t ray_draw_wall(RvR_fix22 y_current, RvR_fix22 y_from, RvR_fix22 y_to, RvR_fix22 limit0, RvR_fix22 limit1, RvR_fix22 height, int16_t increment, RvR_ray_pixel_info *pixel_info, RvR_ray_hit_result *hit)
{
   int16_t limit = RvR_clamp(y_to,limit0,limit1);
   int start = 0;
   int end = 0;

   if(increment==-1)
   {
      start = limit;
      end = y_current+increment;
   }
   else if(increment==1)
   {
      start = y_current+increment;
      end = limit;
   }

   if(end<start)
      return limit;

   //Sky texture is handled differently and instead added as a plane
   if(increment==-1&&hit->wall_ftex==RvR_ray_map_sky_tex())
   {
      ray_plane_add(hit->fheight,hit->wall_ftex,pixel_info->position.x,start,end);
      return limit;
   }
   else if(increment==1&&hit->wall_ctex==RvR_ray_map_sky_tex())
   {
      ray_plane_add(hit->cheight,hit->wall_ctex,pixel_info->position.x,start,end);
      return limit;
   }

   RvR_texture *texture = NULL;
   RvR_fix22 coord_step_scaled = (4*ray_fov_factor_y*pixel_info->depth)/RVR_YRES;
   RvR_fix22 texture_coord_scaled = height*4096+(start-ray_middle_row+1)*coord_step_scaled;

   if(increment==-1)
      texture = RvR_texture_get(hit->wall_ftex);
   else if(increment==1)
      texture = RvR_texture_get(hit->wall_ctex);

   uint8_t * restrict pix = &RvR_core_framebuffer()[start*RVR_XRES+pixel_info->position.x];
   const uint8_t * restrict col = RvR_shade_table(RvR_min(63,(hit->direction&1)*10+(pixel_info->depth>>9)));
   const uint8_t * restrict tex = &texture->data[(hit->texture_coord>>4)*texture->height];
   RvR_fix22 y_and = (1<<RvR_log2(texture->height))-1;

#if RVR_UNROLL

   int count = end-start+1;
   int n = (count+7)/8;
   switch(count%8)
   {
   case 0: do {
           *pix = col[tex[(texture_coord_scaled>>16)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RVR_XRES; //fallthrough
   case 7: *pix = col[tex[(texture_coord_scaled>>16)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RVR_XRES; //fallthrough
   case 6: *pix = col[tex[(texture_coord_scaled>>16)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RVR_XRES; //fallthrough
   case 5: *pix = col[tex[(texture_coord_scaled>>16)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RVR_XRES; //fallthrough
   case 4: *pix = col[tex[(texture_coord_scaled>>16)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RVR_XRES; //fallthrough
   case 3: *pix = col[tex[(texture_coord_scaled>>16)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RVR_XRES; //fallthrough
   case 2: *pix = col[tex[(texture_coord_scaled>>16)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RVR_XRES; //fallthrough
   case 1: *pix = col[tex[(texture_coord_scaled>>16)&y_and]]; texture_coord_scaled+=coord_step_scaled; pix+=RVR_XRES; //fallthrough
           }while(--n>0);
   }

#else

   for(int i = start;i<=end;i++)
   {
      *pix = col[tex[(texture_coord_scaled>>16)&y_and]];
      texture_coord_scaled+=coord_step_scaled;
      pix+=RVR_XRES;
   }

#endif

   return limit;
}

static void ray_draw_column(RvR_ray_hit_result *hits, int hits_len, uint16_t x, RvR_ray ray)
{
   //last written Y position, can never go backwards
   RvR_fix22 f_pos_y = RVR_YRES;
   RvR_fix22 c_pos_y = -1;

   //world coordinates (relative to camera height though)
   RvR_fix22 f_z1_world = ray_start_floor_height;
   RvR_fix22 c_z1_world = ray_start_ceil_height;

   RvR_ray_pixel_info p = {0};
   RvR_ray_hit_result h = {0};
   p.position.x = x;

   //we'll be simulatenously drawing the floor and the ceiling now  
   for(RvR_fix22 j = 0;j<=hits_len;++j)
   {                    //^ = add extra iteration for horizon plane
      int8_t drawing_horizon = j==(hits_len);
      int limit_c = 0;
      int limit_f = 0;

      RvR_fix22 distance = 0;

      RvR_fix22 f_z2_world = 0,    c_z2_world = 0;
      RvR_fix22 f_z1_screen = 0,   c_z1_screen = 0;
      RvR_fix22 f_z2_screen = 0,   c_z2_screen = 0;

      if(!drawing_horizon)
      {
         RvR_ray_hit_result hit = hits[j];
         distance = RvR_non_zero(hit.distance); 
         h = hit;

         RvR_fix22 wall_height = RvR_ray_map_floor_height_at(hit.square.x,hit.square.y);
         f_z2_world = wall_height-RvR_ray_get_position().z;
         f_z1_screen = ray_middle_row-((f_z1_world*RVR_YRES)/RvR_non_zero((ray_fov_factor_y*distance)/1024));
         f_z2_screen = ray_middle_row-((f_z2_world*RVR_YRES)/RvR_non_zero((ray_fov_factor_y*distance)/1024));

         wall_height = RvR_ray_map_ceiling_height_at(hit.square.x,hit.square.y);
         c_z2_world = wall_height-RvR_ray_get_position().z;
         c_z1_screen = ray_middle_row-((c_z1_world*RVR_YRES)/RvR_non_zero((ray_fov_factor_y*distance)/1024));
         c_z2_screen = ray_middle_row-((c_z2_world*RVR_YRES)/RvR_non_zero((ray_fov_factor_y*distance)/1024));
      }
      else
      {
         f_z1_screen = ray_middle_row;
         c_z1_screen = ray_middle_row+1;

         h = hits[j-1];
         h.distance = 1024*1024;
         /* ^ horizon is at infinity, but we can't use too big infinity
              because it would overflow in the following mult. */
         h.position.x = (ray.direction.x*h.distance)/1024;
         h.position.y = (ray.direction.y*h.distance)/1024;

         h.direction = 0;
         h.texture_coord = 0;
         h.wall_ftex = RvR_ray_map_sky_tex();
         h.wall_ctex = RvR_ray_map_sky_tex();
         h.floor_tex = RvR_ray_map_sky_tex();
         h.ceil_tex = RvR_ray_map_sky_tex();
         h.fheight = RvR_fix22_infinity;
         h.cheight = RvR_fix22_infinity;
      }

      RvR_fix22 limit;

      //draw floor until wall
      limit_f = limit = RvR_clamp(f_z1_screen,c_pos_y+1,RVR_YRES);
      if(f_pos_y>limit)
      {
         ray_plane_add(h.fheight,h.floor_tex,p.position.x,limit,f_pos_y-1);
         f_pos_y = limit;
      }

      //draw ceiling until wall
      limit_c = limit = RvR_clamp(c_z1_screen,-1,f_pos_y-1);
      if(limit>c_pos_y)
      {
         ray_plane_add(h.cheight,h.ceil_tex,p.position.x,c_pos_y+1,limit);
         c_pos_y = limit;
      }

      if(!drawing_horizon) //don't draw walls for horizon plane
      {
         p.depth = distance;
         p.depth = RvR_max(0,RvR_min(p.depth,(RVR_RAY_MAX_STEPS)*1024));

         //draw floor wall
         if(f_z1_world!=f_z2_world)
         {
            if(f_pos_y>0)  //still pixels left?
            {
               limit = ray_draw_wall(f_pos_y,f_z1_screen,f_z2_screen,c_pos_y+1,
                                             RVR_YRES,
                                             //^ purposfully allow outside screen bounds here
                                             f_z2_world
                                             ,-1,&p,&h);
               if(f_pos_y>limit)
                  f_pos_y = limit;

               f_z1_world = f_z2_world; //for the next iteration
                           //^ purposfully allow outside screen bounds here
            }

            int limit_clip = RvR_min(limit,limit_f);
            RvR_ray_depth_buffer_entry *entry = ray_depth_buffer_entry_new();
            entry->depth = p.depth;
            entry->limit = limit_clip;
            entry->next = ray_depth_buffer.floor[p.position.x];
            ray_depth_buffer.floor[p.position.x] = entry;
         }

         //draw ceiling wall
         if(c_z1_world!=c_z2_world)
         {
            if(c_pos_y<RVR_YRES-1) //pixels left?
            {
               limit = ray_draw_wall(c_pos_y,c_z1_screen,c_z2_screen,
                                 -1,f_pos_y-1,
                                 //^ puposfully allow outside screen bounds here
                                 c_z2_world 
                                 ,1,&p,&h);

               if(c_pos_y<limit)
                  c_pos_y = limit;

               c_z1_world = c_z2_world; //for the next iteration
                          //^ puposfully allow outside screen bounds here 
            }

            int limit_clip = RvR_max(limit,limit_c);
            RvR_ray_depth_buffer_entry *entry = ray_depth_buffer_entry_new();
            entry->depth = p.depth;
            entry->limit = limit_clip+1;
            entry->next = ray_depth_buffer.ceiling[p.position.x];
            ray_depth_buffer.ceiling[p.position.x] = entry;
         }
      }
   }
}

RvR_ray_pixel_info RvR_ray_map_to_screen(RvR_vec3 world_position)
{
   RvR_ray_pixel_info result;

   RvR_vec2 to_point;

   to_point.x = world_position.x-RvR_ray_get_position().x;
   to_point.y = world_position.y-RvR_ray_get_position().y;

   RvR_fix22 middle_column = RVR_XRES/2;

   //rotate the point to camera space (y left/right, x forw/backw)
   RvR_fix22 tmp = to_point.x;
   to_point.x = (to_point.x*ray_cos+to_point.y*ray_sin)/1024; 
   to_point.y = (tmp*ray_sin-to_point.y*ray_cos)/1024; 

   result.depth = to_point.x;

   result.position.x = middle_column-(((to_point.y*1024)/RvR_non_zero((ray_fov_factor_x*RvR_abs(result.depth))/1024))*middle_column)/1024;
   result.position.y = ((((world_position.z-RvR_ray_get_position().z)*1024)/RvR_non_zero((ray_fov_factor_y*result.depth)/1024))*RVR_YRES)/1024;
   result.position.y = RVR_YRES/2-result.position.y+RvR_ray_get_shear();

   return result;
}

static void ray_plane_add(RvR_fix22 height, uint16_t tex, int x, int y0, int y1)
{
   x+=1;
   //Div height by 128, since it's usually in these increments
   int hash = ((height>>7)*7+tex*3)&127;

   ray_plane *pl = ray_planes[hash];
   while(pl!=NULL)
   {
      //ray_planes need to have the same height...
      if(height!=pl->height)
         goto next;
      //... and the same texture to be valid for concatination
      if(tex!=pl->tex)
         goto next;

      //Additionally the spans collumn needs to be either empty...
      if(pl->start[x]!=UINT16_MAX)
      {
         //... or directly adjacent to each other vertically, in that case
         //Concat planes vertically
         if(pl->start[x]-1==y1)
         {
            pl->start[x] = y0;
            return;
         }
         if(pl->end[x]+1==y0)
         {
            pl->end[x] = y1;
            return;
         }

         goto next;
      }

      break;
next:
      pl = pl->next;
   }

   if(pl==NULL)
   {
      pl = ray_plane_new();
      pl->next= ray_planes[hash];
      ray_planes[hash] = pl;

      pl->min = RVR_XRES;
      pl->max = -1;
      pl->height = height;
      pl->tex = tex;

      //Since this is an unsigned int, we can use memset to set all values to 65535 (0xffff)
      memset(pl->start,255,sizeof(pl->start));
   }

   if(x<pl->min)
      pl->min = x;
   if(x>pl->max)
      pl->max = x;

   pl->end[x] = y1;
   pl->start[x] = y0;
}

static void ray_span_draw_tex(int x0, int x1, int y, RvR_fix22 height, const RvR_texture *texture)
{
   //Shouldn't happen
   if(x0>=x1)
      return;

   //Calculate the depth of the row to be rendered
   RvR_fix22 dy = ray_middle_row-y;
   RvR_fix22 depth = (RvR_abs(RvR_ray_get_position().z-height)*1024)/RvR_non_zero(ray_fov_factor_y);
   depth = (depth*RVR_YRES)/RvR_non_zero(RvR_abs(dy));

   //Calculate texture mapping step size and starting coordinates
   RvR_fix22 step_x = ((ray_sin*(RvR_ray_get_position().z-height)))/RvR_non_zero((dy));
   RvR_fix22 step_y = ((ray_cos*(RvR_ray_get_position().z-height)))/RvR_non_zero((dy));
   //RvR_fix22 step_x = ((ray_sin*(RvR_ray_get_position().z-height))+(dy)/2)/RvR_non_zero((dy));
   //RvR_fix22 step_y = ((ray_cos*(RvR_ray_get_position().z-height))+(dy)/2)/RvR_non_zero((dy));
   RvR_fix22 tx = (RvR_ray_get_position().x&1023)*1024+ray_cos*depth+((x0-RVR_XRES/2)*step_x);
   RvR_fix22 ty = -(RvR_ray_get_position().y&1023)*1024-ray_sin*depth+((x0-RVR_XRES/2)*step_y);

   uint8_t * restrict pix = &RvR_core_framebuffer()[y*RVR_XRES+x0];
   const uint8_t * restrict col = RvR_shade_table(RvR_min(63,(depth>>9)));
   const uint8_t * restrict tex = texture->data;
   RvR_fix22 x_and = (1<<RvR_log2(texture->width))-1;
   RvR_fix22 y_and = (1<<RvR_log2(texture->height))-1;
   RvR_fix22 y_log2 = RvR_log2(texture->height);

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

static void ray_span_draw_flat(int x0, int x1, int y, uint8_t color)
{
   uint8_t * restrict pix = &RvR_core_framebuffer()[y*RVR_XRES+x0];

#if RVR_UNROLL

   int count = x1-x0;
   int n = (count+7)/8;
   switch(count%8)
   {
   case 0: do {
           *pix = color; pix++; //fallthrough
   case 7: *pix = color; pix++; //fallthrough
   case 6: *pix = color; pix++; //fallthrough
   case 5: *pix = color; pix++; //fallthrough
   case 4: *pix = color; pix++; //fallthrough
   case 3: *pix = color; pix++; //fallthrough
   case 2: *pix = color; pix++; //fallthrough
   case 1: *pix = color; pix++; //fallthrough
           }while(--n>0);
   }

#else

   for(int x = x0;x<x1;x++)
   {
      *pix = color;
      pix++;
   }

#endif
}

static void ray_sprite_stack_push(ray_sprite s)
{
   if(ray_sprite_stack.data==NULL)
   {
      ray_sprite_stack.data_size = 64;
      ray_sprite_stack.data = RvR_malloc(sizeof(*ray_sprite_stack.data)*ray_sprite_stack.data_size);
      ray_sprite_stack.data_proxy = RvR_malloc(sizeof(*ray_sprite_stack.data_proxy)*ray_sprite_stack.data_size);
   }

   ray_sprite_stack.data_proxy[ray_sprite_stack.data_used] = ray_sprite_stack.data_used;
   ray_sprite_stack.data[ray_sprite_stack.data_used++] = s;

   if(ray_sprite_stack.data_used==ray_sprite_stack.data_size)
   {
      ray_sprite_stack.data_size+=64;
      ray_sprite_stack.data = RvR_realloc(ray_sprite_stack.data,sizeof(*ray_sprite_stack.data)*ray_sprite_stack.data_size);
      ray_sprite_stack.data_proxy = RvR_realloc(ray_sprite_stack.data_proxy,sizeof(*ray_sprite_stack.data_proxy)*ray_sprite_stack.data_size);
   }
}

//Returns:
//-1 sprites don't obstruct each other
//0 sprite a is obstructed by sprite b --> sprite a first
//1 sprite b is obstructed by sprite a --> sprite b first
static int ray_sprite_order(int a, int b)
{
   ray_sprite *sa = &ray_sprite_stack.data[ray_sprite_stack.data_proxy[a]];
   ray_sprite *sb = &ray_sprite_stack.data[ray_sprite_stack.data_proxy[b]];

   if(sa->sp0.x>=sb->sp1.x||sb->sp0.x>=sa->sp1.x)
      return -1;

   //Shortcut if both are billboarded sprites
   if(!(sa->flags&8)&&!(sb->flags&8))
      return sa->sp.z<sb->sp.z;

   RvR_fix22 x00 = sa->p0.x;
   RvR_fix22 y00 = sa->p0.y;
   RvR_fix22 x01 = sa->p1.x;
   RvR_fix22 y01 = sa->p1.y;
   RvR_fix22 x10 = sb->p0.x;
   RvR_fix22 y10 = sb->p0.y;
   RvR_fix22 x11 = sb->p1.x;
   RvR_fix22 y11 = sb->p1.y;

   //(x00/y00) is origin, all calculations centered arround it
   //t0 is relation of (b,p0) to sprite a
   //t1 is relation of (b,p1) to sprite a
   RvR_fix22 x = x01-x00;
   RvR_fix22 y = y01-y00;
   RvR_fix22 t0 = ((x10-x00)*y-(y10-y00)*x)/1024;
   RvR_fix22 t1 = ((x11-x00)*y-(y11-y00)*x)/1024;

   //sprite on the same line (identicall or adjacent)
   if(t0==0&&t1==0)
      return -1;

   //(b,p0) on extension of sprite a (shared corner, etc)
   //Set t0 = t1 to trigger RvR_sign_equal check (and for the return !RvR_sign_equal to be correct)
   if(t0==0)
      t0 = t1;

   //(b,p1) on extension of sprite a
   //Set t0 = t1 to trigger RvR_sign_equal check
   if(t1==0)
      t1 = t0;

   //Sprite either completely to the left or to the right of other wall
   if(RvR_sign_equal(t0,t1))
   {
      //Compare with player position relative to sprite a
      //if sprite b and the player share the same relation, sprite a needs to be drawn first
      t1 = ((RvR_ray_get_position().x-x00)*y-(RvR_ray_get_position().y-y00)*x)/1024;
      return !RvR_sign_equal(t0,t1);
   }

   //Extension of sprite a intersects with sprite b
   //--> check sprite b instead
   //(x10/y10) is origin, all calculations centered arround it
   x = x11-x10;
   y = y11-y10;
   t0 = ((x00-x10)*y-(y00-y10)*x)/1024;
   t1 = ((x01-x10)*y-(y01-y10)*x)/1024;

   //sprite on the same line (identicall or adjacent)
   if(t0==0&&t1==0)
      return -1;

   //(a,p0) on extension of sprite b
   if(t0==0)
      t0 = t1;

   //(a,p1) on extension of sprite b
   if(t1==0)
      t1 = t0;

   //sprite either completely to the left or to the right of other sprite
   if(RvR_sign_equal(t0,t1))
   {
      //Compare with player position relative to sprite b
      //if sprite a and the player share the same relation, sprite b needs to be drawn first
      t1 = ((RvR_ray_get_position().x-x10)*y-(RvR_ray_get_position().y-y10)*x)/1024;
      return RvR_sign_equal(t0,t1);
   }

   //Invalid case: sprites are intersecting
   return -1;
}

static void ray_sprite_draw_wall(ray_sprite *sp)
{
   int x0 = sp->sp0.x;
   int x1 = sp->sp1.x;

   //Shouldn't happen
   if(x1<0||x0>x1||x0>RVR_XRES)
      return;

   RvR_texture *texture = RvR_texture_get(sp->texture);
   int mask = (1<<RvR_log2(texture->height))-1;
   RvR_fix22 scale_vertical = texture->height*16;
   int size0 = RVR_YRES*((scale_vertical*1024)/RvR_non_zero((ray_fov_factor_y*sp->sp0.z)/1024));
   int size1 = RVR_YRES*((scale_vertical*1024)/RvR_non_zero((ray_fov_factor_y*sp->sp1.z)/1024));
   int y0 = sp->sp0.y;
   int y1 = sp->sp1.y;

   //Floor and ceiling clip
   int clip_bottom = RVR_YRES;
   int clip_top = 0;
   /*RvR_vec3 floor_wpos;
   floor_wpos.x = sp->p.x;
   floor_wpos.y = sp->p.y;
   floor_wpos.z = RvR_ray_map_floor_height_at(sp->p.x/1024,sp->p.y/1024);
   int clip_bottom = RvR_ray_map_to_screen(floor_wpos).position.y;
   floor_wpos.z = RvR_ray_map_ceiling_height_at(sp->p.x/1024,sp->p.y/1024);
   int clip_top = RvR_ray_map_to_screen(floor_wpos).position.y;
   clip_bottom = clip_bottom>RVR_YRES?RVR_YRES:clip_bottom;
   clip_top = clip_top<0?0:clip_top;*/

   RvR_fix22 depth0 = INT32_MAX/RvR_non_zero(sp->sp0.z);
   RvR_fix22 depth1 = INT32_MAX/RvR_non_zero(sp->sp1.z);
   RvR_fix22 step_depth = (depth1-depth0)/RvR_non_zero(x1-x0);
   RvR_fix22 depth_i = depth0;

   RvR_fix22 st0 = (sp->st0*1024*1024)/RvR_non_zero(sp->sp0.z);
   RvR_fix22 st1 = (sp->st1*1024*1024)/RvR_non_zero(sp->sp1.z);
   RvR_fix22 step_u = (st1-st0)/RvR_non_zero(x1-x0);
   RvR_fix22 u_i = st0;

   RvR_fix22 t0 = y0-size0+512;
   RvR_fix22 t1 = y1-size1+512;
   RvR_fix22 step_t = (t1-t0)/RvR_non_zero(x1-x0);
   RvR_fix22 top = t0;

   RvR_fix22 b0 = y0-1024;
   RvR_fix22 b1 = y1-1024;
   RvR_fix22 step_b = (b1-b0)/RvR_non_zero(x1-x0);
   RvR_fix22 bot = b0;

   const uint8_t * restrict col = NULL;
   uint8_t * restrict dst = NULL;
   const uint8_t * restrict tex = NULL;

   RvR_fix22 u_clamp = sp->st0*1024;
   int clamp_dir = sp->st0<sp->st1;
   for(int i = x0;i<=x1;i++)
   {
      RvR_fix22 depth = INT32_MAX/RvR_non_zero(depth_i);
      RvR_fix22 u = (u_i/1024)*depth;

      //Most ugly fix ever
      //Stops the texture coordinate from skipping back and forth due to
      //fixed point inaccuracies by clamping it to the last value
      if(clamp_dir)
         u = RvR_max(u_clamp,u);
      else
         u = RvR_min(u_clamp,u);
      u_clamp = u;
      u*=texture->width;

      RvR_fix22 step_v = (4*ray_fov_factor_y*depth)/RVR_YRES;
      RvR_fix22 y = top/1024;

      int sy = 0;
      int ey = bot/1024-top/1024;
      if(y<clip_top)
         sy = clip_top-y;
      if(y+ey>clip_bottom)
         ey = (clip_bottom-y);
      y = y<clip_top?clip_top:y;

      int ys = y;
      int ye = ey;

      //Clip floor
      RvR_ray_depth_buffer_entry *clip = ray_depth_buffer.floor[i];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&y+(ye-sy)>clip->limit)
            ye = clip->limit-y+sy;
         clip = clip->next;
      }

      //Clip ceiling
      clip = ray_depth_buffer.ceiling[i];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ys<clip->limit)
         {
            int diff = ys-clip->limit;
            ys = clip->limit;
            ye+=diff;
         }
         clip = clip->next;
      }

      RvR_fix22 v = (sp->p.z-RvR_ray_get_position().z)*4096+(ys-ray_middle_row+1)*step_v;

      tex = &texture->data[texture->height*(u>>20)];
      dst = &RvR_core_framebuffer()[ys*RVR_XRES+i];
      col = RvR_shade_table(RvR_min(63,depth>>9));

      if(sp->flags&32)
      {
         for(int yi = sy;yi<ye;yi++,dst+=RVR_XRES)
         {
            uint8_t index = tex[(v>>16)&mask];
            *dst = RvR_blend(col[index],*dst);
            v+=step_v;
         }
      }
      else if(sp->flags&64)
      {
         for(int yi = sy;yi<ye;yi++,dst+=RVR_XRES)
         {
            uint8_t index = tex[(v>>16)&mask];
            *dst = RvR_blend(*dst,col[index]);
            v+=step_v;
         }
      }
      else
      {
         for(int yi = sy;yi<ye;yi++,dst+=RVR_XRES)
         {
            uint8_t index = tex[(v>>16)&mask];
            *dst = index?col[index]:*dst;
            v+=step_v;
         }
      }

      depth_i+=step_depth;
      u_i+=step_u;

      top+=step_t;
      bot+=step_b;
   }
}

static void ray_sprite_draw_floor(ray_sprite *sp)
{
}

//TODO: reimplement flipped sprites
static void ray_sprite_draw_billboard(ray_sprite *sp)
{
   RvR_texture *texture = RvR_texture_get(sp->texture);
   int mask = (1<<RvR_log2(texture->height))-1;

   RvR_fix22 tpx = sp->p.x-RvR_ray_get_position().x;
   RvR_fix22 tpy = sp->p.y-RvR_ray_get_position().y;
   RvR_fix22 depth = (tpx*ray_cos+tpy*ray_sin)/1024;
   tpx = (tpx*ray_sin-tpy*ray_cos)/1024;

   //Dimensions
   RvR_fix22 top = ((sp->p.z-RvR_ray_get_position().z+texture->height*16)*1024)/RvR_non_zero((depth*ray_fov_factor_y)/1024);
   top = ray_middle_row*1024-top*RVR_YRES;
   int y0 = (top+1023)/1024;

   RvR_fix22 bot = ((sp->p.z-RvR_ray_get_position().z)*1024)/RvR_non_zero((depth*ray_fov_factor_y)/1024);
   bot = ray_middle_row*1024-bot*RVR_YRES;
   int y1 = (bot-1)/1024;

   RvR_fix22 left = ((tpx+texture->width*8)*1024)/RvR_non_zero((depth*ray_fov_factor_x)/1024);
   left = RVR_XRES*512-left*(RVR_XRES/2);
   int x0 = (left+1023)/1024;

   RvR_fix22 right = ((tpx-texture->width*8)*1024)/RvR_non_zero((depth*ray_fov_factor_x)/1024);
   right = RVR_XRES*512-right*(RVR_XRES/2);
   int x1 = (right-1)/1024;

   //Floor and ceiling clip
   RvR_fix22 cy = ((RvR_ray_map_floor_height_at(sp->p.x/1024,sp->p.y/1024)-RvR_ray_get_position().z)*1024)/RvR_non_zero((depth*ray_fov_factor_y)/1024);
   cy = ray_middle_row*1024-cy*RVR_YRES;
   int clip_bottom = RvR_min(cy/1024,RVR_YRES);

   cy = ((RvR_ray_map_ceiling_height_at(sp->p.x/1024,sp->p.y/1024)-RvR_ray_get_position().z)*1024)/RvR_non_zero((depth*ray_fov_factor_y)/1024);
   cy = ray_middle_row*1024-cy*RVR_YRES;
   int clip_top = RvR_max(cy/1024,0);

   y0 = RvR_max(y0,clip_top);
   y1 = RvR_min(y1,clip_bottom);
   x1 = RvR_min(x1,RVR_XRES);
   RvR_fix22 step_v = (4*ray_fov_factor_y*depth)/RVR_YRES;
   RvR_fix22 step_u = (8*ray_fov_factor_x*depth)/RVR_XRES;
   RvR_fix22 u = (step_u*(x0*1024-left))/1024;

   if(x0<0)
   {
      u+=(-x0)*step_u;
      x0 = 0;
   }

   //Draw
   const uint8_t * restrict col = RvR_shade_table(RvR_min(63,depth>>9));
   uint8_t * restrict dst = NULL;
   const uint8_t * restrict tex = NULL;
   for(int x = x0;x<x1;x++)
   {
      //Clip against walls
      int ys = y0;
      int ye = y1;

      //Clip floor
      RvR_ray_depth_buffer_entry *clip = ray_depth_buffer.floor[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ye>clip->limit)
            ye = clip->limit;
         clip = clip->next;
      }

      //Clip ceiling
      clip = ray_depth_buffer.ceiling[x];
      while(clip!=NULL)
      {
         if(depth>clip->depth&&ys<clip->limit)
            ys = clip->limit;
         clip = clip->next;
      }

      tex = &texture->data[texture->height*(u>>16)];
      dst = &RvR_core_framebuffer()[ys*RVR_XRES+x];
      RvR_fix22 v = (sp->p.z-RvR_ray_get_position().z)*4096+(ys-ray_middle_row+1)*step_v;

      if(sp->flags&32)
      {
         for(int y = ys;y<ye;y++,dst+=RVR_XRES)
         {
            uint8_t index = tex[(v>>16)&mask];
            *dst = RvR_blend(col[index],*dst);
            v+=step_v;
         }
      }
      else if(sp->flags&64)
      {
         for(int y = ys;y<ye;y++,dst+=RVR_XRES)
         {
            uint8_t index = tex[(v>>16)&mask];
            *dst = RvR_blend(*dst,col[index]);
            v+=step_v;
         }
      }
      else
      {
         for(int y = ys;y<ye;y++,dst+=RVR_XRES)
         {
            uint8_t index = tex[(v>>16)&mask];
            *dst = index?col[index]:*dst;
            v+=step_v;
         }
      }

      u+=step_u;
   }
}

static ray_plane *ray_plane_new()
{
   if(ray_plane_pool==NULL)
   {
      ray_plane *p = RvR_malloc(sizeof(*p)*8);
      memset(p,0,sizeof(*p)*8);

      for(int i = 0;i<7;i++)
         p[i].next = &p[i+1];
      ray_plane_pool = p;
   }

   ray_plane *p = ray_plane_pool;
   ray_plane_pool = p->next;
   p->next = NULL;

   return p;
}

static void ray_plane_free(ray_plane *pl)
{
   if(pl==NULL)
      return;

   //Find last
   ray_plane *last = pl;
   while(last->next!=NULL)
      last = last->next;

   last->next = ray_plane_pool;
   ray_plane_pool = pl;
}

static RvR_ray_depth_buffer_entry *ray_depth_buffer_entry_new()
{
   if(ray_depth_buffer_entry_pool==NULL)
   {
      RvR_ray_depth_buffer_entry *e = RvR_malloc(sizeof(*e)*256);
      memset(e,0,sizeof(*e)*256);

      for(int i = 0;i<255;i++)
         e[i].next = &e[i+1];
      ray_depth_buffer_entry_pool = e;
   }

   RvR_ray_depth_buffer_entry *e = ray_depth_buffer_entry_pool;
   ray_depth_buffer_entry_pool = e->next;
   e->next = NULL;

   return e;
}

static void ray_depth_buffer_entry_free(RvR_ray_depth_buffer_entry *ent)
{
   if(ent==NULL)
      return;

   //Find last
   RvR_ray_depth_buffer_entry *last = ent;
   while(last->next!=NULL)
      last = last->next;

   last->next = ray_depth_buffer_entry_pool;
   ray_depth_buffer_entry_pool = ent;
}
//-------------------------------------
