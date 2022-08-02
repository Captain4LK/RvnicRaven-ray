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
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int interior;
static int style;
static uint8_t fg_color;
static uint8_t bg_color;
static uint8_t fill_color;

static uint8_t buffer[RVR_XRES*RVR_YRES];
//-------------------------------------

//Function prototypes
static void draw(int x, int y, uint8_t color, int mode);
static void draw_line(int x0, int y0, int x1, int y1, uint8_t index, int mode);
static void draw_lines(int count, int *pos, uint8_t color);
static void fill_lines(int count, int *pos, uint8_t color);
//-------------------------------------

//Function implementations

void draw_buffer_set_fg(uint8_t c)
{
   fg_color = c;
}

void draw_buffer_set_bg(uint8_t c)
{
   bg_color = c;
}

void draw_buffer_set_fill(uint8_t c)
{
   fill_color = c;
}

void draw_buffer_shape(int x, int y, const int *shape)
{
   int pos[256];
   int pos_count = 0;
   int fill = 1;

   interior = 2;
   style = 9;

   while(*shape!=-1)
   {
      if(*shape==-2)
      {
         interior = *++shape;
         style = *++shape;
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
            pos[pos_count++] = x+*shape++;
            pos[pos_count++] = y-*shape++;
         }
         pos[pos_count++] = pos[0];
         pos[pos_count++] = pos[1];

         if(fill)
            fill_lines(pos_count/2,pos,1);
            //draw_lines(line_count/2-1,lines,1);
         else
            draw_lines(pos_count/2-1,pos,1);
      }

      shape++;
   }
}

void draw_buffer_clear(void)
{
   memset(buffer,bg_color,sizeof(buffer));
}

void draw_buffer(void)
{
   memcpy(RvR_core_framebuffer(),buffer,sizeof(buffer));
}

void draw_buffer_line(int x0, int y0, int x1, int y1, uint16_t pattern)
{
}

static void draw(int x, int y, uint8_t color, int mode)
{
   if(x<0||x>=RVR_XRES)
      return;
   if(y<0||y>=RVR_YRES)
      return;

   if(mode==0)
      color = fg_color;
   else if(mode==1)
      color = fill_color;

   if(mode==1)
   {
      switch(interior)
      {
      case 0: //blank
         return;
      case 1: //solid
         break;
      case 2: //pattern
         switch(style)
         {
         case 1:
            if(y&1)
               return;
            if(x&3)
               return;
            break;
         case 2:
            if(y&1)
               return;
            if(x&1)
               return;
            break;
         case 8:
            break;
         default:
            printf("mode %d %d unimplemented\n",interior,style);
            break;
         }
         break;
      default:
         printf("mode %d %d unimplemented\n",interior,style);
         break;
      }
   }

   buffer[y*RVR_XRES+x] = color;
}

void draw_line(int x0, int y0, int x1, int y1, uint8_t index, int mode)
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
   draw(x0,y0,index,mode);

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

         draw(x0,y0,index,mode);
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

         draw(x0,y0,index,mode);
      }
   }
}

static void draw_lines(int count, int *pos, uint8_t color)
{
   for(int i = 0;i<count-1;i++)
   {
      draw_line(pos[2*i],pos[2*i+1],pos[2*i+2],pos[2*i+3],color,0);
   }
}

static void fill_lines(int count, int *pos, uint8_t color)
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
   for(int i = 0;i<count-1;i++)
   {
      draw_line(pos[2*i],pos[2*i+1],pos[2*i+2],pos[2*i+3],color,0);
   }

   for(int y = ymin;y<ymax;y++)
   {
      for(int x = xmin;x<xmax;x++)
      {
         int inside = 0;
         int crossed = 0;

         for(int i = 0;i<count-1;i++)
         {
            int x0 = pos[2*i]-x;
            int y0 = pos[2*i+1]-y;
            int x1 = pos[2*i+2]-x;
            int y1 = pos[2*i+3]-y;

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
            draw(x,y,color,1);
      }
   }
}
//-------------------------------------
