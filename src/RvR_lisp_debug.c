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
#include "RvR_lisp_debug.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static int instruction_simple(const char *name, int offset);
static int instruction_constant(const char *name, int offset, Lisp_chunk *chunk);
//-------------------------------------

//Function implementations

void lisp_disassemble_chunk(Lisp_chunk *chunk, const char *name)
{
   RvR_log("== %s ==\n",name);

   for(int i = 0;i<chunk->code_used;)
      i = lisp_disassemble_instruction(chunk,i);
}

int lisp_disassemble_instruction(Lisp_chunk *chunk, int offset)
{
   RvR_log("%4d ",offset);
   if(offset>0&&chunk->lines[offset]==chunk->lines[offset-1])
      RvR_log("   | ");
   else
      RvR_log("%4d ",chunk->lines[offset]);

   uint8_t instruction = chunk->code[offset];
   switch(instruction)
   {
   case LISP_OP_CONSTANT:
      return instruction_constant("OP_CONSTANT",offset,chunk);
   case LISP_OP_RETURN:
      return instruction_simple("OP_RETURN",offset);
   default:
      RvR_log_line("RvR_lisp","unknown opcode %d\n",instruction);
      return offset+1;
   }
}

static int instruction_simple(const char *name, int offset)
{
   RvR_log("%s\n",name);

   return offset+1;
}

static int instruction_constant(const char *name, int offset, Lisp_chunk *chunk)
{
   uint8_t constant = chunk->code[offset+1];
   RvR_log("%-16s %4d '",name,constant);
   RvR_log("%d\n",chunk->constants.values[constant]);

   return offset+2;
}
//-------------------------------------
