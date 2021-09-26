/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdlib.h>
#include <SLK/SLK.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR_error.h"
#include "RvR_malloc.h"
#include "RvR_math.h"
#include "RvR_pak.h"
#include "RvR_pal.h"
#include "RvR_texture.h"
#include "state.h"
#include "sprite.h"
#include "ai.h"
#include "player.h"
#include "sound.h"
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

int main()
{
   //Init memory manager
   RvR_malloc_init(1<<25,1<<26);
   SLK_set_malloc(RvR_malloc);
   SLK_set_free(RvR_free);
   SLK_set_realloc(RvR_realloc);

   SLK_setup(XRES,YRES,1,TITLE,0,SLK_WINDOW_MAX,0);
   SLK_timer_set_fps(FPS);
   SLK_mouse_set_relative(1);

   SLK_layer_create(0,SLK_LAYER_PAL);
   SLK_layer_activate(0,1);
   SLK_layer_set_current(0);
   SLK_layer_set_dynamic(0,0);
   SLK_draw_pal_set_clear_index(0);
   SLK_draw_pal_clear();

   //Load ressources
   RvR_ini_parse("settings.ini");
   RvR_pak_add("data/main.json");
   RvR_palette_load(0);
   RvR_font_load(0xF000);
   RvR_texture_load(0x4100);

   sound_init();

   while(SLK_core_running())
   {
      SLK_update();

      mode_update();

      SLK_render_update();
   }

   return 0;
}
//-------------------------------------
