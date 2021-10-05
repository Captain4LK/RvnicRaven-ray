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
#include "state.h"
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

   //Init RvnicRaven core
   RvR_core_init("To be determined",0);
   RvR_core_mouse_relative(1);

   //Load ressources
   RvR_ini_parse("settings.ini");
   RvR_pak_add("data/main.json");
   RvR_palette_load(0);
   RvR_font_load(0xF000);
   RvR_texture_load(0x4100);

   sound_init();

   while(RvR_core_running())
   {
      RvR_core_update();

      mode_update();

      RvR_core_render_present();
   }

   return 0;
}
//-------------------------------------
