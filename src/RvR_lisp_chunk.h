/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_LISP_CHUNK_H_

#define _RVR_LISP_CHUNK_H_

typedef enum
{
   LISP_OP_CONSTANT,
   LISP_OP_RETURN,
}Lisp_obcode;

typedef struct
{
   uint8_t *code;
   int *lines;
   int code_used;
   int code_size;

   Lisp_value_array constants;
}Lisp_chunk;

void lisp_chunk_init(Lisp_chunk *chunk);
void lisp_chunk_write(Lisp_chunk *chunk, uint8_t byte, int line);
void lisp_chunk_free(Lisp_chunk *chunk);
int lisp_chunk_add_constant(Lisp_chunk *chunk, int32_t value);

#endif
