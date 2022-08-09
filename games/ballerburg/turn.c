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
#include <math.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "baller.h"
#include "draw.h"
#include "turn.h"
#include "object.h"
#include "ui.h"
#include "game.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int ftx,fty,ftw,fth;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void turn_init(void)
{
   int n = zug&1;
   a_dra->x = 5+(RVR_XRES-1-a_dra->width)*n;
   a_dra->y = 25;
   char *txt = (a_dra+DWIN)->as.str;
   //TODO: convert numbers to ascii
   txt[0] = txt[5] = 4+28*(!wind)-(wind>0);
   int i = RvR_abs(wind);
   txt[2] = 48+i/10;
   txt[3] = 48+i%10;
   if(players[n].fahne[0]<0)
   {
      txt[0] = txt[5] = 32;
      txt[2] = txt[3] = '?';
   }

   object_offset(a_dra,a_dra+DOK,&ftx,&fty);
   ftw = (a_dra+DOK)->width;
   fth = (a_dra+DOK)->height;

   //Draw playing field fahne
   draw_buffer_set(buffer_game,RVR_XRES,RVR_YRES);
   int c = wind>0?1:-1;
   double wk = (double)(c*wind)/15.0;
   int s1 = c*20*sin(wk);
   int c1 = 20*cos(wk);
   int ws = s1/2.0;
   ws+=!ws;
   int wc = c1/2.0;
   int m = -1;
   while(++m<2)
      if(players[m].fahne[0]>-1)
      {
         draw_buffer_set_color(0);
         draw_buffer_set_pattern(8);
         draw_buffer_rectangle(players[m].fahne[0]-10,players[m].fahne[1]-15,20,15);
         draw_buffer_set_color(1);
         draw_buffer_line(players[m].fahne[0],players[m].fahne[1],players[m].fahne[0],players[m].fahne[1]-15,0xffff);
         if(m==n)
         {
            draw_buffer_line(players[m].fahne[0],players[m].fahne[1]-15,players[m].fahne[0]+ws,players[m].fahne[1]-13+wc,0xffff);
            draw_buffer_line(players[m].fahne[0],players[m].fahne[1]-11,players[m].fahne[0]+ws,players[m].fahne[1]-13+wc,0xffff);
         }
      }

   //Draw turn number
   char tmp[64];
   snprintf(tmp,64,"%3d",zug/2+1);
   draw_buffer_text(332,475,tmp);
}

void turn_update(void)
{
}

void turn_draw(void)
{
   draw_buffer(0,0);
   draw_buffer_set(RvR_core_framebuffer(),RVR_XRES,RVR_YRES);
   menu_bar_draw(a_men);
   object_draw(a_dra,0,0,0);

   //Draw fahne
   int n = zug&1;
   int x,y;
   object_offset(a_dra,a_dra+DWBX,&x,&y);
   int w = (a_dra+DWBX)->width;
   int h = (a_dra+DWBX)->height;
   int c = wind>0?1:-1;
   double wk = (double)(c*wind)/15.0;
   double wl = wk*0.82;
   if(wk>1.57)
      wk = 1.57;
   if(wl>1.57)
      wl = 1.57;
   int s1 = c*20*sin(wk);
   int c1 = 20*cos(wk);
   int s2 = c*20*sin(wl);
   int c2 = 20*cos(wl);
   if(players[n].fahne[0]>-1)
   {
      x+=w/2;
      draw_buffer_set_color(1);
      draw_buffer_line(x,y+h,x,y+5,0xffff);
      draw_buffer_line(x+1,y+h,x+1,y+5,0xffff);

      int xy[12];
      xy[0] = xy[2] = x+1;
      xy[1] = y+5;
      xy[3] = y+11;
      if(wk<0.2)
      {
         xy[0] = x-1;
         xy[1] = xy[3] = y+5;
         xy[2] = x+2;
      }
      xy[4] = xy[2]+s1;
      xy[5] = xy[3]+c1;
      xy[8] = xy[0]+s1;
      xy[9] = xy[1]+c1;
      xy[10] = xy[0];
      xy[11] = xy[1];
      xy[6] = (xy[4]+xy[8])/2+s2;
      xy[7] = (xy[5]+xy[9])/2+c2;
      draw_buffer_lines(6,xy);
   }
}
//-------------------------------------
