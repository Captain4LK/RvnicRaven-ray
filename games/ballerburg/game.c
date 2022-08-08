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
#include <limits.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "game.h"
#include "burg.h"
#include "draw.h"
#include "state.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   int burg;
   int burg_y;

   int geld;
   int pulver;
   int kugeln;
   int volk;
   int konig;
   int steuern;

   struct
   {
      int x;
      int y;
      int winkel;
      int pulver;
   }kanonen[10];

   int fahne[2];
   struct
   {
      int x;
      int y;
   }forderturme[5];

   char name[64];
}Player;

typedef enum
{
   GAME_TURN,
   GAME_SIMULATE,
}Game_state;
//-------------------------------------

//Variables
static int prices[6];
static int wind;
static int zug = 0;
static int ende = 0;
static Game_state game_state;

static Player players[2];

int trohn[]={ -2,2,2, 8,3,5,12,8,12,12,16,12,17,14,15,
   15,15,17,17,17,16,21,11,24,11,21,3,-9, -4, 3,0,5,2,24,2,26,0,-9,
   19,4,18,9,-9, 10,4,11,9,-9, 14,4,15,4,-9, 8,10,11,11,-9, 21,10,18,11,-9,
   14,17,15,17,-9, 13,19,13,19,-9, 16,19,16,19,-9, 12,22,12,21,15,21,14,22,
   17,21,17,22,-9, 6,13,6,17,5,16,5,17,7,17,7,16,-9, -1 };

int kanon[]={ -2,1,1, 1,0,3,3,5,0,3,3,3,11,2,11,2,9,4,9,4,11,3,11,3,8,0,5,
   3,8,6,5,3,8,3,3,-9, 11,0,13,2,15,0,16,0,17,1,17,2,15,4,19,8,16,11,8,3,-9,
   -1 };
      int sack[]={ -4, 1,1,1,4,2,5,2,0,3,0,3,8,2,8,4,8,3,8,3,6,4,6,4,0,5,0,
   5,6,6,5,6,1,-9, -1 };
      int fass[]={ -4, 2,0,6,0,8,3,8,5,6,8,2,8,0,5,0,3,2,0,3,0,3,3,2,4,3,5,3,8,
   5,8,5,5,6,4,5,3,5,1,-9, -1 };
      int kuge[]={ -2,1,1, 1,0,0,1,0,3,1,4,3,4,4,3,4,1,3,0,-9,-1 };
      int turm[]={ -4, 23,30,27,0,3,10,24,20,6,30,23,30,5,20,26,10,2,0,6,30,-9,
   8,0,8,31,9,34,11,36,14,37,15,37,18,36,20,34,21,31,21,29,20,26,18,24,15,23,
   14,23,11,24,9,26,8,29,-9, 14,23,15,37,-9, 10,25,19,35,-9, 10,35,19,25,-9,
   21,30,21,0,-9, -1 };
int burg[] = 
{
   0,0,0,30,5,55,30,55,10,45,10,30,30,30,75,65,80,60,70,50,70,35,90,35,100,45,
100,60,130,60,130,0,-9, -2,2,11, 5,55,18,70,30,55,-9, 100,60,115,70,130,60,-9,
-1

   //0,0,0,30,5,55,30,55,10,45,10,30,30,30,75,65,80,60,70,50,70,35,90,35,100,45,100,60,130,60,130,0,-9,-2,2,11,5,55,18,70,30,55,-9,100,60,115,70,130,60,-9,-1
};
//-------------------------------------

//Function prototypes
static void burg_init(int n);
static void kanone_init(int n, int k, int xr);

static void draw_geld(int n);
static void draw_pulver(int n);
static void draw_kugeln(int n);
//-------------------------------------

//Function implementations

void game_new(void)
{
   wind = rand()%60-30;
   players[0].steuern = 20;
   players[1].steuern = 20;
   players[0].konig = 0;
   players[1].konig = 0;

   static const int base_prices[6] = {200,500,400,150,50,50};
   for(int i = 0;i<6;i++)
      prices[i] = (base_prices[i]*(95+rand()%11))/100;

   //Generate playing field
   draw_buffer_clear();

   //TODO: bild(), incomplete
   players[0].burg_y = rand()%80+340&~3;
   players[1].burg_y = rand()%80+340&~3;

   int y = RVR_YRES;
   int x0 = 0;
   int x1 = 2556;
   int v0 = 2;
   int v1 = 2;
   draw_buffer_set_color(1);
   while(x0<x1&&--y>20)
   {
      draw_buffer_line(x0/4,y,x1/4,y,~((257<<(rand()&7))*(y&1)));

      if(y==players[0].burg_y)
         x0 = (burgen.data[players[0].burg].width)*4;
      if(y==players[1].burg_y)
         x1 = (RVR_XRES-1-burgen.data[players[1].burg].width)*4;
      if(x0)
      {
         v0+=rand()%5-2;
         v0 = RvR_max(0,RvR_min(7,v0));
         x0+=v0;
      }
      if(x1<2556)
      {
         v1+=rand()%5-2;
         v1 = RvR_max(0,RvR_min(7,v1));
         x1-=v1;
      }
   }

   for(int i = 0;i<10;i++)
   {
      players[0].kanonen[i].x = -1;
      players[0].kanonen[i].y = -1;
      players[0].kanonen[i].pulver = -1;
      players[0].kanonen[i].winkel = -1;
      players[1].kanonen[i].x = -1;
      players[1].kanonen[i].y = -1;
      players[1].kanonen[i].pulver = -1;
      players[1].kanonen[i].winkel = -1;
   }
   burg_init(0);
   burg_init(1);

   draw_buffer_text(276,475," Runde     ");
   //-------------------------------------

   for(int n = 0;n<2;n++)
   {
      int bg = players[n].burg;
      players[n].fahne[0] = n?RVR_XRES-1-burgen.data[bg].fahne[0]:burgen.data[bg].fahne[0];
      players[n].fahne[1] = players[n].burg_y-burgen.data[bg].fahne[1];
      for(int f = 0;f<5;f++)
         players[n].forderturme[f].x = -1;
   }

   zug = ende = 0;
   state_set(STATE_TURN);

   /*static short pr[6]={ 200,500,400,150,50,50 };
   short j;

   wnd=Random()%60-30;
   st[0]=st[1]=20; kn[0]=kn[1]=0;
   for( j=0;j<6;j++ )
      p[j]=pr[j]*(95+Random()%11)/100;
   bild();
   for( n=0;n<2;n++ )
   {
      bg=burgen[bur[n]]; wx[n]=n? 639-bg[23]:bg[23]; wy[n]=by[n]-bg[24];
      for( f=0;f<5; ) ft[n][f++].x=-1;
   }
   zug=n=end=0; f=1;*/
}

void game_update(void)
{
   switch(game_state)
   {
   case GAME_TURN:
      break;
   case GAME_SIMULATE:
      break;
   }
}

void game_draw(void)
{
   draw_buffer(0,0);
}

int ein_zug(void)
{
   int n = zug&1;
   int f = n&1?-1:1;
   wind = (wind*9)/10+rand()%12-6;
   players[n].konig&=~16;

   draw_buffer_set_flip(f);

   return 1;
}

static void burg_init(int n)
{
   int f = n&1?-1:1;
   draw_buffer_set_flip(f);
   int xr = (RVR_XRES-1)*n;
   int bg = players[n].burg;

   //Initialize starting values
   players[n].geld = burgen.data[bg].geld_start;
   players[n].pulver = burgen.data[bg].pulver_start;
   players[n].kugeln = burgen.data[bg].kugeln_start;
   players[n].volk = burgen.data[bg].volk_start;

   //Draw burg
   draw_buffer_set_color(1);
   draw_buffer_set_write(4);
   draw_buffer_shape(xr,players[n].burg_y,burgen.data[bg].shape);
   draw_buffer_set_write(1);

   //Draw thron
   draw_buffer_set_color(0);
   int dim[4] = 
   {
      burgen.data[bg].trohn[0],
      burgen.data[bg].trohn[1],
      burgen.data[bg].trohn[2],
      burgen.data[bg].trohn[3],
   };
   draw_buffer_set_pattern(0);
   draw_buffer_rectangle(xr+f*dim[0]-n*dim[2],players[n].burg_y-dim[1]-dim[3],dim[2],dim[3]);
   draw_buffer_set_color(1);
   draw_buffer_shape(xr+f*dim[0],players[n].burg_y-dim[1],trohn);

   for(int i = 0;i<10&&burgen.data[bg].kanonen[2*i+1]>-1;i++)
      kanone_init(n,i,xr);
   draw_geld(n);
   draw_pulver(n);
   draw_kugeln(n);

   draw_buffer_set_flip(1);
}

static void kanone_init(int n, int k, int xr)
{
   int f = n&1?-1:1;
   int bg = players[n].burg;
   int x = xr+burgen.data[bg].kanonen[k*2]*f;
   int y = players[n].burg_y-burgen.data[bg].kanonen[1+k*2];
   draw_buffer_shape(x,y,kanon);

   players[n].kanonen[k].x = x;
   players[n].kanonen[k].y = y;
   players[n].kanonen[k].winkel = 45;
   players[n].kanonen[k].pulver = 12;
}

static void draw_geld(int n)
{
   int f = n&1?-1:1;
   int bg = players[n].burg;
   int xr = (RVR_XRES-1)*n+f*burgen.data[bg].geld[0];
   int yr = players[n].burg_y-burgen.data[bg].geld[1];
   int i = (players[n].geld+149)/150;

   draw_buffer_set_color(0);
   draw_buffer_set_pattern(0);
   draw_buffer_rectangle(xr-burgen.data[bg].geld[2]*n-!n,yr-burgen.data[bg].geld[3],burgen.data[bg].geld[2]+1,burgen.data[bg].geld[3]+1);
   draw_buffer_set_color(1);

   int z,y;
   for(y = z = 0;i>0&&y<burgen.data[bg].geld[3];y+=10)
      for(int x = 0;i>0&&x<burgen.data[bg].geld[2];x+=7,z++,i--)
         draw_buffer_shape(xr+f*x,yr-y,sack);

   if(i>0)
      players[n].geld = z*150;
}

static void draw_pulver(int n)
{
   int f = n&1?-1:1;
   int bg = players[n].burg;
   int xr = (RVR_XRES-1)*n+f*burgen.data[bg].pulver[0];
   int yr = players[n].burg_y-burgen.data[bg].pulver[1];
   int i = (players[n].pulver+29)/30;

   draw_buffer_set_color(0);
   draw_buffer_set_pattern(0);
   draw_buffer_rectangle(xr-burgen.data[bg].pulver[2]*n-!n,yr-burgen.data[bg].pulver[3],burgen.data[bg].pulver[2]+1,burgen.data[bg].pulver[3]+1);
   draw_buffer_set_color(1);

   int z,y;
   for(y = z = 0;i>0&&y<burgen.data[bg].pulver[3];y+=9)
      for(int x = 0;i>0&&x<burgen.data[bg].pulver[2];x+=9,z++,i--)
         draw_buffer_shape(xr+f*x,yr-y,fass);

   if(i>0)
      players[n].pulver = z*30;
}

static void draw_kugeln(int n)
{
   int f = n&1?-1:1;
   int bg = players[n].burg;
   int xr = (RVR_XRES-1)*n+f*burgen.data[bg].kugeln[0];
   int yr = players[n].burg_y-burgen.data[bg].kugeln[1];
   int i = players[n].kugeln;

   draw_buffer_set_color(0);
   draw_buffer_set_pattern(0);
   draw_buffer_rectangle(xr-burgen.data[bg].kugeln[2]*n-!n,yr-burgen.data[bg].kugeln[3],burgen.data[bg].kugeln[2]+1,burgen.data[bg].kugeln[3]+1);
   draw_buffer_set_color(1);

   int z,y;
   for(y = z = 0;i>0&&y<burgen.data[bg].kugeln[3];y+=6)
      for(int x = 0;i>0&&x<burgen.data[bg].kugeln[2];x+=6,z++,i--)
         draw_buffer_shape(xr+f*x,yr-y,kuge);

   if(i>0)
      players[n].kugeln = z;
}
//-------------------------------------
