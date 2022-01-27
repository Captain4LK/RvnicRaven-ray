/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _TEXTURE_H_

#define _TEXTURE_H_

typedef struct
{
   uint16_t *data;
   unsigned data_used;
   unsigned data_size;
}Texture_list;

typedef struct
{
   uint16_t data[TEXTURE_MRU_SIZE];
   unsigned data_last;
}Texture_list_used;

extern Texture_list texture_list;
extern Texture_list_used texture_list_used;
extern uint16_t texture_sky;

void texture_list_create();
void texture_list_used_create();
void texture_list_used_add(uint16_t tex);

int texture_valid(uint16_t tex);

#define texture_list_used_wrap(a) ((a)&(TEXTURE_MRU_SIZE-1))

#endif
