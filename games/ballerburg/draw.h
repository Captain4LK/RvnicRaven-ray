/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _DRAW_H_

#define _DRAW_H_

void draw_buffer_set(uint8_t *buff, int width, int height);
void draw_buffer_set_color(uint8_t c); //Black - 0, White - 1
void draw_buffer_set_text_color(uint8_t c);
void draw_buffer_set_text_write(int m);
void draw_buffer_set_write(int m);
void draw_buffer_set_flip(int f);
void draw_buffer_set_pattern(int pattern);
void draw_buffer(int x, int y);

void draw_buffer_clear(void);
void draw_buffer_shape(int x, int y, const int *shape);
void draw_buffer_line(int x0, int y0, int x1, int y1, uint16_t pattern);
void draw_buffer_hline(int x0, int y0, int x1, uint16_t pattern);
void draw_buffer_vline(int x0, int y0, int y1, uint16_t pattern);
void draw_buffer_rectangle(int x, int y, int width, int height);
void draw_buffer_rectangle_line(int x, int y, int width, int height, int border_width);
void draw_buffer_text(int x, int y, const char *str);

#endif
