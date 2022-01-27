/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "draw.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void draw_fit64(int x, int y, uint16_t tex)
{
   RvR_texture *texture = RvR_texture_get(tex);
   if(texture==NULL)
      return;

   int fwidth;
   int fheight;
   if(texture->width>texture->height)
   {
      fwidth = 64;
      fheight = (((texture->height*1024)/texture->width)*64)/1024;
   }
   else
   {
      fheight = 64;
      fwidth = (((texture->width*1024)/texture->height)*64)/1024;
   }

   RvR_fix22 stepx = (texture->width*1024)/fwidth;
   RvR_fix22 stepy = (texture->height*1024)/fheight;
   RvR_fix22 texsx = 0;
   RvR_fix22 texsy = 0;
   RvR_fix22 texx = 0;
   RvR_fix22 texy = 0;

   int draw_start_x = 0;
   int draw_start_y = 0;
   int draw_end_x = fwidth;
   int draw_end_y = fheight;

   if(x<0)
   {
      draw_start_x = -x;
      texsx = RvR_abs(x)*stepx;
   }
   if(y<0)
   {
      draw_start_y = -y;
      texsy = RvR_abs(y)*stepy;
   }
   if(x+draw_end_x>RVR_XRES)
      draw_end_x = fwidth+(RVR_XRES-x-draw_end_x);
   if(y+draw_end_y>RVR_YRES)
      draw_end_y = fheight+(RVR_YRES-y-draw_end_y);

   x = x<0?0:x;
   y = y<0?0:y;

   uint8_t * restrict dst = &RvR_core_framebuffer()[x+y*RVR_XRES];
   int dst_step = -((draw_end_y-draw_start_y))*RVR_XRES+1;
   texx = texsx;

   for(int x1 = draw_start_x;x1<draw_end_x;x1++)
   {
      const uint8_t * restrict src = &texture->data[(texx/1024)*texture->height];
      texy = texsy;
      for(int y1 = draw_start_y;y1<draw_end_y;y1++,dst+=RVR_XRES)
      {
         *dst = src[texy/1024];
         texy+=stepy;
      }

      texx+=stepx;
      dst+=dst_step;
   }
}
//-------------------------------------
