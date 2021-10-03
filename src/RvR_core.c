/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvR_core.h"
#include "RvR_backend.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int running = 1;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_core_quit()
{
   running = 0;
}

void RvR_core_init(char *title, int scale)
{
   RvR_backend_init(title,scale);
}

void RvR_core_mouse_relative(int relative)
{
   RvR_backend_mouse_relative(relative);
}

int RvR_core_running()
{
   return running;
}

void RvR_core_update()
{
   RvR_backend_update();
}

void RvR_core_render_present()
{
   RvR_backend_render_present();
}

int RvR_core_key_down(RvR_key key)
{
   return RvR_backend_key_down(key);
}

int RvR_core_key_pressed(RvR_key key)
{
   return RvR_backend_key_pressed(key);
}

int RvR_core_key_released(RvR_key key)
{
   return RvR_backend_key_released(key);
}

int RvR_core_mouse_down(RvR_mouse_button button)
{
   return RvR_backend_mouse_down(button);
}

int RvR_core_mouse_pressed(RvR_mouse_button button)
{
   return RvR_backend_mouse_pressed(button);
}

int RvR_core_mouse_released(RvR_mouse_button button)
{
   return RvR_backend_mouse_released(button);
}

int RvR_core_mouse_wheel_scroll()
{
   return RvR_backend_mouse_wheel_get_scroll();
}

int RvR_core_gamepad_down(int index, RvR_gamepad_button button)
{
   return RvR_backend_gamepad_down(index,button);
}

int RvR_core_gamepad_pressed(int index, RvR_gamepad_button button)
{
   return RvR_backend_gamepad_pressed(index,button);
}

int RvR_core_gamepad_released(int index, RvR_gamepad_button button)
{
   return RvR_backend_gamepad_released(index,button);
}

void RvR_core_mouse_relative_pos(int *x, int *y)
{
   RvR_backend_mouse_get_relative_pos(x,y);
}

uint8_t *RvR_core_framebuffer()
{
   return RvR_backend_framebuffer();
}
//-------------------------------------
