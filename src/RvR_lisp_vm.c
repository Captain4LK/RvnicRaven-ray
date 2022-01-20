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
#include "RvR_lisp_vm.h"
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

void lisp_vm_init(Lisp_vm *vm)
{
   lisp_vm_stack_reset(vm);
}

void lisp_vm_free(Lisp_vm *vm)
{
}

void lisp_vm_stack_reset(Lisp_vm *vm)
{
   vm->stack_top = vm->stack;
}

void lisp_vm_stack_push(Lisp_vm *vm, int32_t value)
{
   *vm->stack_top = value;
   vm->stack_top++;
}

int32_t lisp_vm_stack_pop(Lisp_vm *vm)
{
   vm->stack_top--;
   return *vm->stack_top;
}

Lisp_interpret_result lisp_interpret(Lisp_vm *vm, Lisp_chunk *chunk)
{
   vm->chunk = chunk;
   vm->ip = vm->chunk->code;

   return lisp_run(vm);
}

Lisp_interpret_result lisp_run(Lisp_vm *vm)
{
   for(;;)
   {
#if RVR_LISP_TRACE_EXECUTION
      lisp_disassemble_instruction(vm->chunk,(int)(vm->ip-vm->chunk->code));
#endif

      uint8_t instruction;
      switch((instruction = (*vm->ip++)))
      {
      case LISP_OP_CONSTANT:
      {
         int32_t constant = vm->chunk->constants.values[(*vm->ip++)];
         break;
      }
      case LISP_OP_RETURN:
         return LISP_INTERPRET_OK;
      }
   }
}
//-------------------------------------
