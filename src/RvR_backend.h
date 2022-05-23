/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_BACKEND_H_

#define _RVR_BACKEND_H_

void RvR_backend_init(const char *title, int scale);
void RvR_backend_mouse_relative(int relative);
void RvR_backend_mouse_show(int show);
void RvR_backend_key_repeat(int repeat);

void RvR_backend_update();
void RvR_backend_render_present();
int RvR_backend_frametime();

int RvR_backend_key_down(int key);
int RvR_backend_key_pressed(int key);
int RvR_backend_key_released(int key);
int RvR_backend_mouse_wheel_get_scroll();
int RvR_backend_gamepad_down(int index, int key);
int RvR_backend_gamepad_pressed(int index, int key);
int RvR_backend_gamepad_released(int index, int key);
void RvR_backend_mouse_get_relative_pos(int *x, int *y);
void RvR_backend_mouse_get_pos(int *x, int *y);
void RvR_backend_mouse_set_pos(int x, int y);
uint8_t *RvR_backend_framebuffer();
void RvR_backend_text_input_start(char *text, int max_length);
void RvR_backend_text_input_end();

#endif
