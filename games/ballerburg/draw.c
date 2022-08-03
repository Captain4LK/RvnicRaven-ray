/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "../../src/RvnicRaven.h"
#include "draw.h"
#include "color.h"
//-------------------------------------

//#defines
#define PATTERN_MAX 16
//-------------------------------------

//Typedefs
typedef struct
{
   uint16_t planes;
   uint16_t pattern[PATTERN_MAX];
}Pattern;
//-------------------------------------

//Variables
static int draw_interior;
static int draw_style;
static int draw_write = 1;
static int draw_flip = 1;
static uint8_t draw_color = 1;

static uint16_t pattern_line;

static uint8_t *buffer;
static int buffer_width;
static int buffer_height;

static const Pattern pattern_pattern[25] = 
{
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 4, .pattern = {0x1111,0x0000,0x4444,0x0000}}, 
   {.planes = 2, .pattern = {0x5555,0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 8, .pattern = {0xffff,0x1010,0x1010,0x1010,0xffff,0x0101,0x0101,0x0101}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 8, .pattern = {0x0a0a,0x0000,0x0000,0x4040,0xa0a0,0x0000,0x0000,0x0404}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
};
//-------------------------------------

//Function prototypes
static void draw(int x, int y, int mode);
static void draw_line(int x0, int y0, int x1, int y1, int mode);
static void draw_lines(int count, int *pos);
static void fill_lines(int count, int *pos);

static int point_on_line(int x0, int y0, int x1, int y1, int x, int y);
//-------------------------------------

//Function implementations

void draw_buffer_set(uint8_t *buff, int width, int height)
{
   buffer = buff;
   buffer_width = width;
   buffer_height = height;
}

void draw_buffer_set_color(uint8_t c)
{
   draw_color = c;
}

void draw_buffer_set_write(int m)
{
   draw_write = RvR_min(4,RvR_max(1,m));
}

void draw_buffer_set_flip(int f)
{
   draw_flip = f;
}

void draw_buffer_shape(int x, int y, const int *shape)
{
   int pos[256];
   int pos_count;
   int fill = 1;

   draw_interior = 2;
   draw_style = 9;

   while(*shape!=-1)
   {
      if(*shape==-2)
      {
         draw_interior = *++shape;
         draw_style = *++shape;
      }
      else if(*shape==-4)
      {
         fill = !fill;
      }
      else
      {
         pos_count = 0;
         while(*shape>-1)
         {
            pos[pos_count++] = x+(*shape++)*draw_flip;
            pos[pos_count++] = y-*shape++;
         }
         pos[pos_count++] = pos[0];
         pos[pos_count++] = pos[1];

         if(fill)
            fill_lines(pos_count/2,pos);
         else
            draw_lines(pos_count/2-1,pos);
      }

      shape++;
   }
}

void draw_buffer_clear(void)
{
   memset(buffer,color_white,sizeof(*buffer)*buffer_width*buffer_height);
}

void draw_buffer(int x, int y)
{
   int draw_start_y = 0;
   int draw_start_x = 0;
   int draw_end_x = buffer_width;
   int draw_end_y = buffer_height;
   if(x<0)
      draw_start_x = -x;
   if(y<0)
      draw_start_y = -y;
   if(x+draw_end_x>RVR_XRES)
      draw_end_x = buffer_width+(RVR_XRES-x-draw_end_x);
   if(y+draw_end_y>RVR_YRES)
      draw_end_y = buffer_height+(RVR_YRES-y-draw_end_y);

   //Clip dst sprite
   x = x<0?0:x;
   y = y<0?0:y;

   const uint8_t *src = &buffer[draw_start_x+draw_start_y*buffer_width];
   uint8_t *dst = &RvR_core_framebuffer()[x+y*RVR_XRES];
   int src_step = -(draw_end_x-draw_start_x)+buffer_width;
   int dst_step = RVR_XRES-(draw_end_x-draw_start_x);
    
   for(int y1 = draw_start_y;y1<draw_end_y;y1++,dst+=dst_step,src+=src_step)
      for(int x1 = draw_start_x;x1<draw_end_x;x1++,src++,dst++)
         *dst = *src?*src:*dst;
}

void draw_buffer_line(int x0, int y0, int x1, int y1, uint16_t pattern)
{
   pattern_line = pattern;
   draw_line(x0,y0,x1,y1,2);
}

void draw_buffer_rectangle(int x, int y, int width, int height)
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
   if(x+draw_end_x>buffer_width)
      draw_end_x = width+(buffer_width-x-draw_end_x);
   if(y+draw_end_y>buffer_height)
      draw_end_y = height+(buffer_height-y-draw_end_y);
    
   //Clip dst rect
   x = x<0?0:x;
   y = y<0?0:y;

   uint8_t *dst = &buffer[x+y*buffer_width];
   int dst_step = buffer_width-(draw_end_x-draw_start_x);
    
   for(int y1 = draw_start_y;y1<draw_end_y;y1++,dst+=dst_step)
      for(int x1 = draw_start_x;x1<draw_end_x;x1++,dst++)
         *dst = draw_color?color_black:color_white;
}

static void draw(int x, int y, int mode)
{
   if(x<0||x>=RVR_XRES)
      return;
   if(y<0||y>=RVR_YRES)
      return;

   int col = draw_color;
   if(mode==1)
   {
      switch(draw_interior)
      {
      case 0: //blank
         return;
      case 1: //solid
         break;
      case 2: //pattern
         if(!(pattern_pattern[draw_style].pattern[y%pattern_pattern[draw_style].planes]&(1<<(x&15))))
            col = !col;
         break;
      default:
         printf("mode %d %d unimplemented\n",draw_interior,draw_style);
         break;
      }
   }
   else if(mode==2)
   {
      if(!(pattern_line&(1<<(x&15))))
         col = !col;
   }

   unsigned mask = col;
   unsigned new = 0;
   unsigned fg = 1;
   unsigned bg = 0;
   unsigned c = buffer[y*RVR_XRES+x]==color_black;

   switch(draw_write)
   {
   case 1:
      new = (fg&mask)|(bg&~mask);
      break;
   case 2:
      new = (fg&mask)|(c&~mask);
      break;
   case 3:
      new = c^mask;
      break;
   case 4:
      new = (c&mask)|(fg&~mask);
      break;
   }

   buffer[y*RVR_XRES+x] = new?color_black:color_white;
}

void draw_line(int x0, int y0, int x1, int y1, int mode)
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
   draw(x0,y0,mode);

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

         draw(x0,y0,mode);
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

         draw(x0,y0,mode);
      }
   }
}

static void draw_lines(int count, int *pos)
{
   for(int i = 0;i<count-1;i++)
   {
      draw_line(pos[2*i],pos[2*i+1],pos[2*i+2],pos[2*i+3],0);
   }
}

static void fill_lines(int count, int *pos)
{
   //It's stupid but it works
   int xmin = pos[0];
   int xmax = pos[0];
   int ymin = pos[1];
   int ymax = pos[1];
   for(int i = 0;i<count;i++)
   {
      xmin = RvR_min(pos[2*i],xmin);
      xmax = RvR_max(pos[2*i],xmax);
      ymin = RvR_min(pos[2*i+1],ymin);
      ymax = RvR_max(pos[2*i+1],ymax);
   }

   for(int y = ymin;y<=ymax;y++)
   {
      for(int x = xmin;x<=xmax;x++)
      {
         int inside = 0;
         int crossed = 0;

         for(int i = 0;i<count-1;i++)
         {
            int x0 = pos[2*i]-x;
            int y0 = pos[2*i+1]-y;
            int x1 = pos[2*i+2]-x;
            int y1 = pos[2*i+3]-y;

            //Check if point on line --> abort and set inside
            if(point_on_line(pos[2*i],pos[2*i+1],pos[2*i+2],pos[2*i+3],x,y))
            {
               crossed = 1;
               break;
            }

            if(!RvR_sign_equal(y0,y1))
            {
               if(RvR_sign_equal(x0,x1))
                  crossed+=RvR_negative(x1);
               else
                  crossed+=!RvR_sign_equal(x0*y1-x1*y0,y1);
            }
         }

         inside = RvR_odd(crossed);

         if(inside)
            draw(x,y,1);
      }
   }

   //Outline
   for(int i = 0;i<count-1;i++)
   {
      draw_line(pos[2*i],pos[2*i+1],pos[2*i+2],pos[2*i+3],0);
   }
}

static int point_on_line(int x0, int y0, int x1, int y1, int x, int y)
{
   int dx0 = x-x0;
   int dy0 = y-y0;
   int dx1 = x1-x0;
   int dy1 = y1-y0;

   int cross = dx0*dy1-dy0*dx1;
   if(cross!=0)
      return 0;

   if(RvR_abs(dx1)>=RvR_abs(dy1))
      return dx1>0?x0<=x&&x<=x1:x1<=x&&x<=x0;
   else
      return dy1>0?y0<=y&&y<=y1:y1<=y&&y<=y0;
}
//-------------------------------------
