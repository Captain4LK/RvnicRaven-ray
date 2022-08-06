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
#include <time.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "draw.h"
#include "color.h"
#include "burg.h"
#include "game.h"
#include "object.h"
#include "ressource.h"
#include "ui.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables

static uint8_t buffer_game[RVR_XRES*RVR_YRES];
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

   //RvR_pak_create_from_csv("data_anarch/main.csv","test.pak");
   //RvR_pak_add("test.pak");
   RvR_pak_add("data_baller/main.csv");

   //User defined overwrites (used for modding)
   for(int i = 1;i<argc;i++)
      RvR_pak_add(argv[i]);
   
   srand(time(NULL));

   //config_read("settings.ini");

   RvR_palette_load(0);
   RvR_draw_font_set(0xF000);

   RvR_rw rw;
   RvR_rw_init_path(&rw,"BALLER.DAT","rb");
   burgen_add(&rw);
   RvR_rw_close(&rw);

   ressource_load("BALLER.RSC");
   //ressource_load("DOODLE.RSC");
   objects_get();

   printf("%d Burgen\n",burgen.data_used);

   //sound_init();
   
   color_init();
   draw_buffer_set(buffer_game,RVR_XRES,RVR_YRES);
   draw_buffer_clear();
   draw_buffer_set_color(1);

   //draw_buffer_set_write(4);
   //draw_buffer_shape(0,320,burgen.data[0].shape);
   //draw_buffer_set_write(1);

   /*draw_buffer_shape(64,64,trohn);
   draw_buffer_shape(128,64,kanon);
   draw_buffer_shape(192,64,sack);
   draw_buffer_shape(256,64,fass);
   draw_buffer_shape(320,64,kuge);
   draw_buffer_shape(384,64,turm);*/

   //draw_buffer_set_write(4);
   //draw_buffer_shape(0,320,burg);
   //draw_buffer_set_write(1);
   
   game_new();

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

   /*int points[64] = {-91,60,-91,30,-91,30,-86,5,-86,5,-61,5,-61,5,-81,15,-81,15,-81,30,-81,30,-61,30,-61,30,-16,-5,-16,-5,-11,0,-11,0,-21,10,-21,10,-21,25,-21,25,-1,25,-1,25,9,15,9,15,9,0,9,0,39,0,39,0,39,60,39,60,-91,60};
   RvR_draw_clear(color_white);
   RvR_draw_line(0,RVR_YRES/2,RVR_XRES,RVR_YRES/2,0);
   RvR_draw_line(RVR_XRES/2,0,RVR_XRES/2,RVR_YRES,0);
   for(int i = 0;i<16;i++)
   {
      RvR_draw_line(points[i*4]+RVR_XRES/2,points[i*4+1]+RVR_YRES/2,points[i*4+2]+RVR_XRES/2,points[i*4+3]+RVR_YRES/2,color_black);
   }*/

   draw_buffer(0,0);
   //object_draw(objects,0,20,0);
   object_draw(a_sie,0,20,0);
   //menu_bar_draw(a_dra);

   /*static int burg = 0;
   if(RvR_core_key_pressed(RVR_KEY_RIGHT))
   {
      burg = (burg+1)%burgen.data_used;

      draw_buffer_clear();
      draw_buffer_set_write(4);
      draw_buffer_shape(0,320,burgen.data[burg].shape);
      draw_buffer_set_write(1);
   }*/

   //state_update();

   RvR_core_render_present();
}
//-------------------------------------
