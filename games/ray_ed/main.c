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

#define CUTE_PATH_IMPLEMENTATION
#include "../../external/cute_path.h"

#define CUTE_FILES_IMPLEMENTATION
#include "../../external/cute_files.h"
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "config.h"
#include "color.h"
#include "texture.h"
#include "map.h"
#include "editor.h"
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

int main(int argc, char **argv)
{
   if(argc<2)
   {
      puts("No pak path specified!");
      return -1;
   }

   //Init memory manager
   RvR_malloc_init(1<<25,1<<26);

   //Init RvnicRaven core
   RvR_core_init("Rayed",0);
   RvR_core_mouse_relative(0);
   RvR_core_mouse_show(0);
   RvR_core_key_repeat(1);

   RvR_pak_add(argv[1]);
   RvR_palette_load(0);
   RvR_font_load(0xF000);

   map_new(64,64);
   colors_find();
   texture_list_create();
   texture_list_used_create();

   char path[512];
   path_pop(argv[0],path,NULL);
   strcat(path,"/");
   map_path_add(path);
   path_pop(argv[1],path,NULL);
   strcat(path,"/");
   map_path_add(path);

   editor_init();

   while(RvR_core_running())
   {
      RvR_core_update();

      editor_update();
      editor_draw();

      if(RvR_core_key_pressed(RVR_KEY_M))
         RvR_malloc_report();

      RvR_core_render_present();
   }

   map_set_path("autosave.map");
   map_save();

   return 0;
}
//-------------------------------------
