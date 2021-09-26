/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_TEXTURE_H_

#define _RVR_TEXTURE_H_

SLK_Pal_sprite *RvR_texture_get(uint16_t id);
void RvR_texture_load_begin();
void RvR_texture_load_end();
void RvR_texture_load(uint16_t id);
void RvR_font_load(uint16_t id);
void RvR_font_unload(uint16_t id);

#endif
