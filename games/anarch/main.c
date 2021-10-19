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
//-------------------------------------

//Function implementations

int main()
{
   //Init memory manager
   RvR_malloc_init(1<<25,1<<26);

   //Init RvnicRaven core
   RvR_core_init("Anarch",0);
   RvR_core_mouse_relative(1);

   //Load ressources
   RvR_config config = RvR_ini_parse("settings.ini");
   RvR_ini_read(config,&config_mouse_sensitivity,RVR_CONFIG_INT,"mouse_sensitivity");
   RvR_ini_read(config,&config_mouse_sensitivity_vertical,RVR_CONFIG_INT,"mouse_sensitivity_vertical");
   RvR_ini_read(config,&config_move_forward,RVR_CONFIG_KEY,"move_forward");
   RvR_ini_read(config,&config_move_backward,RVR_CONFIG_KEY,"move_backward");
   RvR_ini_read(config,&config_strafe_left,RVR_CONFIG_KEY,"strafe_left");
   RvR_ini_read(config,&config_strafe_right,RVR_CONFIG_KEY,"strafe_right");
   RvR_ini_read(config,&config_enable_freelook,RVR_CONFIG_KEY,"enable_freelook");
   RvR_ini_read(config,&config_jump,RVR_CONFIG_KEY,"jump");
   RvR_ini_read(config,&config_camera_max_shear,RVR_CONFIG_INT,"camera_max_shear");
   RvR_ini_read(config,&config_camera_shear_step,RVR_CONFIG_INT,"camera_shear_step");
   RvR_ini_free(config);

   RvR_pak_add("data_anarch/main.json");
   RvR_palette_load(0);
   RvR_font_load(0xF000);

   //Title screen textures
   RvR_texture_load(0x4100);
   RvR_texture_load(0x4101);

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
