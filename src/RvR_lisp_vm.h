/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_LISP_VM_H_

#define _RVR_LISP_VM_H_

typedef struct
{
   Lisp_chunk *chunk;
   uint8_t *ip;

   int32_t stack[RVR_LISP_STACK_MAX];
   int32_t *stack_top;
}Lisp_vm;

typedef enum
{
   LISP_INTERPRET_OK,
   LISP_COMPILE_ERROR,
   LISP_RUNTIME_ERROR,
}Lisp_interpret_result;

void lisp_vm_init(Lisp_vm *vm);
void lisp_vm_free(Lisp_vm *vm);
void lisp_vm_stack_reset(Lisp_vm *vm);
void lisp_vm_stack_push(Lisp_vm *vm, int32_t value);
int32_t lisp_vm_stack_pop(Lisp_vm *vm);
Lisp_interpret_result lisp_interpret(Lisp_vm *vm, Lisp_chunk *chunk);
Lisp_interpret_result lisp_run(Lisp_vm *vm);

#endif
