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
static const char *keytostr(RvR_key key);
//-------------------------------------

//Function implementations

void config_read(const char *path)
{
   RvR_rw rw = {0};
   RvR_rw_init_path(&rw,path,"r");

   if(!RvR_rw_valid(&rw))
   {
      config_write(path);
      return;
   }

   RvR_config config = RvR_ini_parse(&rw);
   RvR_rw_close(&rw);

   RvR_ini_read(config,&config_move_forward,RVR_CONFIG_KEY,"forward"); 
   RvR_ini_read(config,&config_move_backward,RVR_CONFIG_KEY,"backward"); 
   RvR_ini_read(config,&config_strafe_left,RVR_CONFIG_KEY,"strafe_left"); 
   RvR_ini_read(config,&config_strafe_right,RVR_CONFIG_KEY,"strafe_right"); 

   RvR_ini_free(config);
}

void config_write(const char *path)
{
   RvR_rw rw = {0};
   RvR_rw_init_path(&rw,path,"w");

   if(!RvR_rw_valid(&rw))
      return;

   RvR_rw_printf(&rw,";Action\n");
   RvR_rw_printf(&rw,"forward=%s\n",keytostr(config_move_forward));
   RvR_rw_printf(&rw,"backward=%s\n",keytostr(config_move_backward));
   RvR_rw_printf(&rw,"strafe_left=%s\n",keytostr(config_strafe_left));
   RvR_rw_printf(&rw,"strafe_right=%s\n",keytostr(config_strafe_right));

   RvR_rw_close(&rw);
}

static const char *keytostr(RvR_key key)
{
   switch(key)
   {
   case RVR_KEY_A: return "a";
   case RVR_KEY_B: return "b";
   case RVR_KEY_C: return "c";
   case RVR_KEY_D: return "d";
   case RVR_KEY_E: return "e";
   case RVR_KEY_F: return "f";
   case RVR_KEY_G: return "g";
   case RVR_KEY_H: return "h";
   case RVR_KEY_I: return "i";
   case RVR_KEY_J: return "j";
   case RVR_KEY_K: return "k";
   case RVR_KEY_L: return "l";
   case RVR_KEY_M: return "m";
   case RVR_KEY_N: return "n";
   case RVR_KEY_O: return "o";
   case RVR_KEY_P: return "p";
   case RVR_KEY_Q: return "q";
   case RVR_KEY_R: return "r";
   case RVR_KEY_S: return "s";
   case RVR_KEY_T: return "t";
   case RVR_KEY_U: return "u";
   case RVR_KEY_V: return "v";
   case RVR_KEY_W: return "w";
   case RVR_KEY_X: return "x";
   case RVR_KEY_Y: return "y";
   case RVR_KEY_Z: return "z";

   case RVR_KEY_F1: return "f1";
   case RVR_KEY_F2: return "f2";
   case RVR_KEY_F3: return "f3";
   case RVR_KEY_F4: return "f4";
   case RVR_KEY_F5: return "f5";
   case RVR_KEY_F6: return "f6";
   case RVR_KEY_F7: return "f7";
   case RVR_KEY_F8: return "f8";
   case RVR_KEY_F9: return "f9";
   case RVR_KEY_F10: return "f10";
   case RVR_KEY_F11: return "f11";
   case RVR_KEY_F12: return "f12";

   case RVR_KEY_DOWN: return "down";
   case RVR_KEY_LEFT: return "left";
   case RVR_KEY_RIGHT: return "right";
   case RVR_KEY_UP: return "up";
   case RVR_KEY_RETURN: return "return";

   case RVR_KEY_BACK: return "backspace";
   case RVR_KEY_ESCAPE: return "escape";
   case RVR_KEY_TAB: return "tab";
   case RVR_KEY_DEL: return "delete";
   case RVR_KEY_HOME: return "home";
   case RVR_KEY_END: return "end";
   case RVR_KEY_PGUP: return "pgup";
   case RVR_KEY_PGDN: return "pgdn";
   case RVR_KEY_INS: return "insert";
   case RVR_KEY_LSHIFT: return "lshift";
   case RVR_KEY_RSHIFT: return "rshift";
   case RVR_KEY_LCTRL: return "lctrl";
   case RVR_KEY_RCTRL: return "rctrl";
   case RVR_KEY_LALT: return "lalt";
   case RVR_KEY_RALT: return "ralt";
   case RVR_KEY_SPACE: return "space";

   case RVR_KEY_0: return "0";
   case RVR_KEY_1: return "1";
   case RVR_KEY_2: return "2";
   case RVR_KEY_3: return "3";
   case RVR_KEY_4: return "4";
   case RVR_KEY_5: return "5";
   case RVR_KEY_6: return "6";
   case RVR_KEY_7: return "7";
   case RVR_KEY_8: return "8";
   case RVR_KEY_9: return "9";

   case RVR_KEY_COMMA: return ",";
   case RVR_KEY_PERIOD: return "."; 

   case RVR_BUTTON_LEFT: return "mouse_1";
   case RVR_BUTTON_RIGHT: return "mouse_2";
   case RVR_BUTTON_MIDDLE: return "mouse_3";
   case RVR_BUTTON_X1: return "mouse_4";
   case RVR_BUTTON_X2: return "mouse_5";

   default:
      return "UK KEY";
   }
}
//-------------------------------------
