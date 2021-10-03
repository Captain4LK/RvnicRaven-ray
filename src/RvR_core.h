/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_CORE_H_

#define _RVR_CORE_H_

typedef enum 
{
   RVR_KEY_NONE,
   RVR_KEY_A,RVR_KEY_B,RVR_KEY_C,RVR_KEY_D,RVR_KEY_E,RVR_KEY_F,RVR_KEY_G,
   RVR_KEY_H,RVR_KEY_I,RVR_KEY_J,RVR_KEY_K,RVR_KEY_L,RVR_KEY_M,RVR_KEY_N,
   RVR_KEY_O,RVR_KEY_P,RVR_KEY_Q,RVR_KEY_R,RVR_KEY_S,RVR_KEY_T,RVR_KEY_U,
   RVR_KEY_V,RVR_KEY_W,RVR_KEY_X,RVR_KEY_Y,RVR_KEY_Z,
   RVR_KEY_0,RVR_KEY_1,RVR_KEY_2,RVR_KEY_3,RVR_KEY_4,RVR_KEY_5,
   RVR_KEY_6,RVR_KEY_7,RVR_KEY_8,RVR_KEY_9,
   RVR_KEY_F1,RVR_KEY_F2,RVR_KEY_F3,RVR_KEY_F4,RVR_KEY_F5,RVR_KEY_F6,
   RVR_KEY_F7,RVR_KEY_F8,RVR_KEY_F9,RVR_KEY_F10,RVR_KEY_F11,RVR_KEY_F12,
   RVR_KEY_UP,RVR_KEY_DOWN,RVR_KEY_LEFT,RVR_KEY_RIGHT,
   RVR_KEY_SPACE,RVR_KEY_TAB,RVR_KEY_SHIFT,RVR_KEY_CTRL,RVR_KEY_INS,
   RVR_KEY_DEL,RVR_KEY_HOME,RVR_KEY_END,RVR_KEY_PGUP,RVR_KEY_PGDN,
   RVR_KEY_BACK,RVR_KEY_ESCAPE,RVR_KEY_RETURN,RVR_KEY_ENTER,RVR_KEY_PAUSE,RVR_KEY_SCROLL,
   RVR_KEY_NP0,RVR_KEY_NP1,RVR_KEY_NP2,RVR_KEY_NP3,RVR_KEY_NP4,RVR_KEY_NP5,RVR_KEY_NP6,RVR_KEY_NP7,RVR_KEY_NP8,RVR_KEY_NP9,
   RVR_KEY_NP_MUL,RVR_KEY_NP_DIV,RVR_KEY_NP_ADD,RVR_KEY_NP_SUB,RVR_KEY_NP_DECIMAL,
   RVR_KEY_MAX,
}RvR_key;

typedef enum 
{
   RVR_PAD_A, RVR_PAD_B, RVR_PAD_X, RVR_PAD_Y, RVR_PAD_BACK, RVR_PAD_GUIDE,
   RVR_PAD_START, RVR_PAD_LEFTSTICK, RVR_PAD_RIGHTSTICK, RVR_PAD_LEFTSHOULDER,
   RVR_PAD_RIGHTSHOULDER, RVR_PAD_UP, RVR_PAD_DOWN, RVR_PAD_LEFT, RVR_PAD_RIGHT,
   RVR_PAD_MAX,
}RvR_gamepad_button;

typedef enum 
{
   RVR_BUTTON_LEFT,RVR_BUTTON_RIGHT,RVR_BUTTON_MIDDLE,RVR_BUTTON_X1,RVR_BUTTON_X2,
   RVR_BUTTON_MAX,
}RvR_mouse_button;

typedef struct
{
   uint8_t r,g,b,a;
}RvR_color;

typedef struct
{
   int width;
   int height;
   uint8_t *data;
}RvR_texture;

void RvR_core_quit();
void RvR_core_init(char *title, int scale);
void RvR_core_mouse_relative(int relative);
int RvR_core_running();
void RvR_core_update();
void RvR_core_render_present();

int RvR_core_key_down(RvR_key key);
int RvR_core_key_pressed(RvR_key key);
int RvR_core_key_released(RvR_key key);
int RvR_core_mouse_down(RvR_mouse_button button);
int RvR_core_mouse_pressed(RvR_mouse_button button);
int RvR_core_mouse_released(RvR_mouse_button button);
int RvR_core_mouse_wheel_scroll();
int RvR_core_gamepad_down(int index, RvR_gamepad_button button);
int RvR_core_gamepad_pressed(int index, RvR_gamepad_button button);
int RvR_core_gamepad_released(int index, RvR_gamepad_button button);
void RvR_core_mouse_relative_pos(int *x, int *y);

uint8_t *RvR_core_framebuffer();

#include "RvR_config.h"
#include "RvR_error.h"

#endif
