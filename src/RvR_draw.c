/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
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
static RvR_texture *draw_font = NULL;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_draw_clear(uint8_t index)
{
   memset(RvR_core_framebuffer(),index,RVR_XRES*RVR_YRES);
}

void RvR_draw_texture(RvR_texture *t, int x, int y)
{
   if(t==NULL)
      return;

   //Clip source texture
   int draw_start_y = 0;
   int draw_start_x = 0;
   int draw_end_x = t->width;
   int draw_end_y = t->height;
   if(x<0)
      draw_start_x = -x;
   if(y<0)
      draw_start_y = -y;
   if(x+draw_end_x>RVR_XRES)
      draw_end_x = t->width+(RVR_XRES-x-draw_end_x);
   if(y+draw_end_y>RVR_YRES)
      draw_end_y = t->height+(RVR_YRES-y-draw_end_y);

   //Clip dst sprite
   x = x<0?0:x;
   y = y<0?0:y;

   const uint8_t *src = &t->data[draw_start_x+draw_start_y*t->width];
   uint8_t *dst = &RvR_core_framebuffer()[x+y*RVR_XRES];
   int src_step = -(draw_end_x-draw_start_x)+t->width;
   int dst_step = RVR_XRES-(draw_end_x-draw_start_x);
    
   for(int y1 = draw_start_y;y1<draw_end_y;y1++,dst+=dst_step,src+=src_step)
      for(int x1 = draw_start_x;x1<draw_end_x;x1++,src++,dst++)
         *dst = *src?*src:*dst;
}

void RvR_draw_texture2(RvR_texture *t, int x, int y)
{
   //This function is awfull...
   if(t==NULL)
      return;

   //Clip source texture
   int draw_start_y = 0;
   int draw_start_x = 0;
   int draw_end_x = t->width*2;
   int draw_end_y = t->height*2;
   if(x<0)
      draw_start_x = -x;
   if(y<0)
      draw_start_y = -y;
   if(x+draw_end_x>RVR_XRES)
      draw_end_x = t->width*2+(RVR_XRES-x-draw_end_x);
   if(y+draw_end_y>RVR_YRES)
      draw_end_y = t->height*2+(RVR_YRES-y-draw_end_y);

   //Clip dst sprite
   x = x<0?0:x;
   y = y<0?0:y;

   const uint8_t *src = &t->data[draw_start_x/2+(draw_start_y/2)*t->width];
   uint8_t *dst = &RvR_core_framebuffer()[x+y*RVR_XRES];
   int src_step = -((draw_end_x-draw_start_x)/2)+t->width;
   int dst_step = RVR_XRES-(draw_end_x-draw_start_x);
   int next = !(draw_start_x&1);
    
   for(int y1 = draw_start_y;y1<draw_end_y;y1++,dst+=dst_step)
   {
      const uint8_t *src_old = src;
      uint8_t *dst_old = dst;

      for(int x1 = draw_start_x;x1<draw_end_x;x1+=2,src++,dst+=2)
         *dst = *src?*src:*dst;
      dst = dst_old+1;
      src = src_old;
      for(int x1 = draw_start_x+1;x1<draw_end_x;x1+=2,src++,dst+=2)
         *dst = *src?*src:*dst;
      //What?
      //src+=t->width;
      dst--;

      if(next)
         src = src_old;
      else
         src+=src_step;
      next = !next;
   }
}

void RvR_draw_rectangle(int x, int y, int width, int height, uint8_t index)
{
   RvR_draw_horizontal_line(x,x+width-1,y,index);
   RvR_draw_horizontal_line(x,x+width-1,y+height-1,index);
   RvR_draw_vertical_line(x,y,y+height-1,index);
   RvR_draw_vertical_line(x+width-1,y,y+height-1,index);
}

void RvR_draw_rectangle_fill(int x, int y, int width, int height, uint8_t index)
{
   //Clip src rect
   int draw_start_y = 0;
   int draw_start_x = 0;
   int draw_end_x = width;
   int draw_end_y = height;

   if(x<0)
      draw_start_x = -x;
   if(y<0)
      draw_start_y = -y;
   if(x+draw_end_x>RVR_XRES)
      draw_end_x = width+(RVR_XRES-x-draw_end_x);
   if(y+draw_end_y>RVR_YRES)
      draw_end_y = height+(RVR_YRES-y-draw_end_y);
    
   //Clip dst rect
   x = x<0?0:x;
   y = y<0?0:y;

   uint8_t *dst = &RvR_core_framebuffer()[x+y*RVR_XRES];
   int dst_step = RVR_XRES-(draw_end_x-draw_start_x);
    
   for(int y1 = draw_start_y;y1<draw_end_y;y1++,dst+=dst_step)
      for(int x1 = draw_start_x;x1<draw_end_x;x1++,dst++)
         *dst = index;
}

void RvR_draw_circle(int x, int y, int radius, uint8_t index)
{
   int x_ = 0;
   int y_ = radius;
   int d = 1-radius;

   RvR_draw(x,y+radius,index);
   RvR_draw(x,y-radius,index);
   RvR_draw(x+radius,y,index);
   RvR_draw(x-radius,y,index);

   while(x_<y_)
   {
      if(d<0)
      {
         d = d+2*x_+3;
         x_+=1;
      }
      else
      {
         d = d+2*(x_-y_)+5;
         x_+=1;
         y_-=1;
      }

      RvR_draw(x+x_,y+y_,index);
      RvR_draw(x+x_,y-y_,index);
      RvR_draw(x-x_,y+y_,index);
      RvR_draw(x-x_,y-y_,index);

      RvR_draw(x+y_,y+x_,index);
      RvR_draw(x+y_,y-x_,index);
      RvR_draw(x-y_,y+x_,index);
      RvR_draw(x-y_,y-x_,index);
   }
}

void RvR_draw_set_font(RvR_texture *t)
{
   draw_font = t;
}

void RvR_draw_string(int x, int y, int scale, const char *text, uint8_t index)
{
   int x_dim = draw_font->width/16;
   int y_dim = draw_font->height/6;
   int sx = 0;
   int sy = 0;

   for(int i = 0;text[i]&&i<1024;i++)
   {
      if(text[i]=='\n')
      {
         sx = 0; 
         sy+=y_dim*scale;
         continue;
      }

      int ox = (text[i]-32)&15;
      int oy = (text[i]-32)/16;
      for(int y_ = 0;y_<y_dim;y_++)
      {
         for(int x_ = 0;x_<x_dim;x_++)
         {
            if(draw_font->data[(y_+oy*y_dim)*draw_font->width+x_+ox*x_dim])
               continue;
            for(int m = 0;m<scale;m++)
               for(int o = 0;o<scale;o++)
                  RvR_draw(x+sx+(x_*scale)+o,y+sy+(y_*scale)+m,index);
         }
      }
      sx+=x_dim*scale;
   }
}

void RvR_draw(int x, int y, uint8_t index)
{
   if(x<0||x>=RVR_XRES)
      return;
   if(y<0||y>=RVR_YRES)
      return;

   RvR_core_framebuffer()[y*RVR_XRES+x] = index;
}

void RvR_draw_line(int x0, int y0, int x1, int y1, uint8_t index)
{
   if(x0>x1||y0>y1)
   {
      int t = x0;
      x0 = x1;
      x1 = t;
      t = y0;
      y0 = y1;
      y1 = t;
   }
   int dx = x1-x0;
   int ix = (dx>0)-(dx<0);
   dx = abs(dx)<<1;
   int dy = y1-y0;
   int iy = (dy>0)-(dy<0);
   dy = abs(dy)<<1;
   RvR_draw(x0,y0,index);

   if(dx>=dy)
   {
      int error = dy-(dx>>1);
      while(x0!=x1)
      {
         if(error>0||(!error&&ix>0))
         {
            error-=dx;
            y0+=iy;
         }

         error+=dy;
         x0+=ix;

         RvR_draw(x0,y0,index);
      }
   }
   else
   {
      int error = dx-(dy>>1);

      while(y0!=y1)
      {
         if(error>0||(!error&&iy>0))
         {
            error-=dy;
            x0+=ix;
         }

         error+=dx;
         y0+=iy;

         RvR_draw(x0,y0,index);
      }
   }
}

void RvR_draw_vertical_line(int x, int y0, int y1, uint8_t index)
{
   if(y0>y1)
   {
      int t = y0;
      y0 = y1;
      y1 = t;
   }

   if(x<0||x>=RVR_XRES||y0>=RVR_YRES||y1<0)
      return;

   if(y0<0)
      y0 = 0;
   if(y1>=RVR_YRES)
      y1 = RVR_YRES-1;

   uint8_t *buff = RvR_core_framebuffer();
   for(int y = y0;y<=y1;y++)
      buff[y*RVR_XRES+x] = index;
}

void RvR_draw_horizontal_line(int x0, int x1, int y, uint8_t index)
{
   if(x0>x1)
   {
      int t = x0;
      x0 = x1;
      x1 = t;
   }
	   
   if(y<0||y>=RVR_YRES||x0>=RVR_XRES||x1<0)
      return;

   if(x0<0)
      x0 = 0;
   if(x1>=RVR_XRES)
      x1 = RVR_XRES-1;

   uint8_t *dst = &RvR_core_framebuffer()[y*RVR_XRES+x0];
   for(int x = x0;x<=x1;x++,dst++)
      *dst = index;
}
//-------------------------------------
