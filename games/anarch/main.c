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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "state.h"
#include "sound.h"
#include "config.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
int config_mouse_sensitivity = 128;
int config_mouse_sensitivity_vertical = 128;
RvR_key config_move_forward = RVR_KEY_W;
RvR_key config_move_backward = RVR_KEY_S;
RvR_key config_strafe_left = RVR_KEY_A;
RvR_key config_strafe_right = RVR_KEY_D;
RvR_key config_enable_freelook = RVR_KEY_F;
RvR_key config_jump = RVR_KEY_SPACE;
int config_camera_max_shear = 192;
int config_camera_shear_step = 8;
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
   RvR_core_init("Anarch",0);
   RvR_core_mouse_relative(1);

   //RvR_pak_create_from_csv("data_anarch/main.csv","test.pak");
   //RvR_pak_add("test.pak");
   RvR_pak_add("data_anarch/main.csv");

   //User defined overwrites ('modding')
   for(int i = 1;i<argc;i++)
      RvR_pak_add(argv[i]);

   RvR_palette_load(0);
   RvR_draw_font_set(0xF000);

   sound_init();

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

   RvR_core_render_present();
}
//-------------------------------------
