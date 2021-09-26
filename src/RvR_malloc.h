/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_MALLOC_H_

#define _RVR_MALLOC_H_

void RvR_malloc_init(int min, int max);
void *RvR_malloc(size_t size);
void RvR_free(void *ptr);
void *RvR_realloc(void *ptr, size_t size);
void RvR_malloc_report();

#endif
