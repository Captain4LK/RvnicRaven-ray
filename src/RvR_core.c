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
#include "RvnicRaven.h"
#include "RvR_backend.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int core_running = 1;
static uint32_t core_frame = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_core_quit()
{
   core_running = 0;
}

void RvR_core_init(char *title, int scale)
{
   RvR_backend_init(title,scale);
}

void RvR_core_mouse_relative(int relative)
{
   RvR_backend_mouse_relative(relative);
}

void RvR_core_mouse_show(int show)
{
   RvR_backend_mouse_show(show);
}

void RvR_core_key_repeat(int repeat)
{
   RvR_backend_key_repeat(repeat);
}

int RvR_core_running()
{
   return core_running;
}

void RvR_core_update()
{
   core_frame++;

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

void RvR_core_mouse_pos(int *x, int *y)
{
   RvR_backend_mouse_get_pos(x,y);
}

void RvR_core_mouse_set_pos(int x, int y)
{
   RvR_backend_mouse_set_pos(x, y);
}

void RvR_core_text_input_start(char *text, int max_length)
{
   RvR_backend_text_input_start(text,max_length);
}

void RvR_core_text_input_end()
{
   RvR_backend_text_input_end();
}

uint8_t *RvR_core_framebuffer()
{
   return RvR_backend_framebuffer();
}

uint32_t RvR_core_frame()
{
   return core_frame;
}

int RvR_core_frametime()
{
   return RvR_backend_frametime();
}
//-------------------------------------
