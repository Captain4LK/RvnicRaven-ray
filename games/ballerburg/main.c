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
   0,0,0,70,30,70,10,65,10,50,30,50,10,30,10,10,30,10,50,50,70,50,80,45,120,45,
120,70,150,70,130,60,130,45,150,45,150,0,-9, -2,2,11, 0,70,15,100,30,70,-9,
120,70,135,100,150,70,-9, -1
};
//-------------------------------------

//Function prototypes
static void loop();
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   //Init memory manager
   RvR_malloc_init(1<<25,1<<26);

   //Init RvnicRaven core
   RvR_core_init("RvnicRaven - ballerburg",0);
   RvR_core_mouse_relative(1);

   //RvR_pak_create_from_csv("data_anarch/main.csv","test.pak");
   //RvR_pak_add("test.pak");
   RvR_pak_add("data_baller/main.csv");

   //User defined overwrites (used for modding)
   for(int i = 1;i<argc;i++)
      RvR_pak_add(argv[i]);

   //config_read("settings.ini");

   RvR_palette_load(0);
   RvR_draw_font_set(0xF000);

   //sound_init();

   color_init();
   draw_buffer_set_fg(color_black);
   draw_buffer_set_bg(color_white);
   draw_buffer_set_fill(color_black);
   draw_buffer_clear();

   draw_buffer_shape(64,64,trohn);
   draw_buffer_shape(128,64,kanon);
   draw_buffer_shape(192,64,sack);
   draw_buffer_shape(256,64,fass);
   draw_buffer_shape(320,64,kuge);
   draw_buffer_shape(384,64,turm);
   draw_buffer_shape(384,256,burg);

   #ifdef __EMSCRIPTEN__
   emscripten_set_main_loop(loop,RVR_FPS,1);
#else
   while(RvR_core_running())
   {
      loop();
   }
#endif

   return 0;
}

static void loop()
{
   RvR_core_update();

   draw_buffer();

   //state_update();

   RvR_core_render_present();
}
//-------------------------------------
