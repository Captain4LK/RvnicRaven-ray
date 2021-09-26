/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_COMPRESS_H_

#define _RVR_COMPRESS_H_

void  RvR_compress(FILE *in, FILE *out);
void  RvR_compress_path(const char *path_in, const char *path_out);
void  RvR_mem_compress(void *mem, int32_t length, FILE *out);
void *RvR_decompress(FILE *in, int32_t *length);
void *RvR_decompress_path(const char *path, int32_t *length);
void *RvR_mem_decompress(void *mem, int32_t length_in, int32_t *length_out);

#endif
