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
#include "state.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
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
   objects_get();

   printf("%d Burgen\n",burgen.data_used);

   //sound_init();
   
   color_init();
   draw_buffer_set(buffer_game,RVR_XRES,RVR_YRES);
   draw_buffer_clear();
   draw_buffer_set_color(1);

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

   state_update();
   state_draw();

   RvR_core_render_present();
}
//-------------------------------------
