/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
#include "RvR_lisp_value.h"
#include "RvR_lisp_chunk.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void lisp_chunk_init(Lisp_chunk *chunk)
{
   chunk->code = NULL;
   chunk->lines = NULL;
   chunk->code_used = 0;
   chunk->code_size = 0;

   lisp_value_array_init(&chunk->constants);
}

void lisp_chunk_write(Lisp_chunk *chunk, uint8_t byte, int line)
{
   if(chunk->code==NULL)
   {
      chunk->code_used = 0;
      chunk->code_size = 256;
      chunk->code = RvR_malloc(sizeof(*chunk->code)*chunk->code_size);
      chunk->lines = RvR_malloc(sizeof(*chunk->lines)*chunk->code_size);
   }

   chunk->lines[chunk->code_used] = line;
   chunk->code[chunk->code_used++] = byte;

   if(chunk->code_used>=chunk->code_size)
   {
      chunk->code_size+=256;
      chunk->code = RvR_realloc(chunk->code,sizeof(*chunk->code)*chunk->code_size);
      chunk->lines = RvR_realloc(chunk->lines,sizeof(*chunk->lines)*chunk->code_size);
   }
}

void lisp_chunk_free(Lisp_chunk *chunk)
{
   if(chunk->code==NULL)
      return;

   RvR_free(chunk->code);
   RvR_free(chunk->lines);
   lisp_value_array_free(&chunk->constants);
   lisp_chunk_init(chunk);
}

int lisp_chunk_add_constant(Lisp_chunk *chunk, int32_t value)
{
   lisp_value_array_write(&chunk->constants,value);
   return chunk->constants.values_used-1;
}
//-------------------------------------
