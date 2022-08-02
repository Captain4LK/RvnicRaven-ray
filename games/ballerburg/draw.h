/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _DRAW_H_

#define _DRAW_H_

void draw_buffer_set_fg(uint8_t c);
void draw_buffer_set_bg(uint8_t c);
void draw_buffer_set_fill(uint8_t c);
void draw_buffer(void);

void draw_buffer_clear(void);
void draw_buffer_shape(int x, int y, const int *shape);
void draw_buffer_line(int x0, int y0, int x1, int y1, uint16_t pattern);

#endif
