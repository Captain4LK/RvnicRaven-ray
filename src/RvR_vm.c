/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/*
Limitations: 
   * no floating point support: RvR doesn't use floating point in general, why should the vm?
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef enum
{
   VM_OP_RETURN = 1,

   //Branches
   VM_OP_JUMP = 2, //Jump
   VM_OP_BEQ = 3, //Branch if equal
   VM_OP_BNE = 4, //Brain if not equal
   VM_OP_BLT = 5, //Branch if less than
   VM_OP_BLE = 6, //Branch if less than or equal
   VM_OP_BGT = 7, //Branch if greater than
   VM_OP_BGE = 8, //Branch if greater than or equal
   VM_OP_BLTU = 9, //Branch if less than, unsigned
   VM_OP_BLEU = 10, //Branch if less than or equal, unsigned
   VM_OP_BGTU = 11, //Branch if greater than, unsigned
   VM_OP_BGEU = 12, //Branch if greater than or equal, unsigned
   
   //Arithmetic
   VM_OP_ADD = 13,
   VM_OP_SUB = 14,
   VM_OP_NEGI = 15,
   VM_OP_DIVI = 16,
   VM_OP_DIVU = 17,
   VM_OP_MULI = 18,
   VM_OP_MULU = 19,
   VM_OP_MODI = 20,
   VM_OP_MODU = 21,

   //Bitwise manipulation
   VM_OP_AND = 22,
   VM_OP_OR = 23,
   VM_OP_XOR = 24,
   VM_OP_NOT = 25,
   VM_OP_LSH = 26,
   VM_OP_RSHI = 26,
   VM_OP_RSHU = 26,
   VM_OP_SEX8 = 27,
   VM_OP_SEX16 = 28,
}VM_opcode;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_vm_create(RvR_vm *vm, RvR_rw *code, RvR_vm_func_call callback)
{
   if(vm==NULL)
      return;

   vm->callback = callback;

   //Read header
   uint8_t endian = RvR_rw_read_u8(code);
   RvR_rw_endian(code,endian);
   int32_t code_len = RvR_rw_read_i32(code);
   int32_t code_offset = RvR_rw_read_i32(code);
   int32_t code_count = RvR_rw_read_i32(code);
   //int32_t data_len = RvR_rw_read_i32(code);
   int32_t data_offset = RvR_rw_read_i32(code);

   //Populate vm struct with info read from header
   vm->instruction_count = code_count;
   vm->instruction_pointers = RvR_malloc(sizeof(*vm->instruction_pointers)*code_count);

   //Read instructions and make them int32 aligned (TODO: does this improve or worsen performance?)
   vm->code = RvR_malloc(code_len*sizeof(*vm->code)); 
   RvR_rw_seek(code,code_offset,SEEK_SET);
   int code_current = 0;
   int instr_current = 0;
   while(code_current<code_count)
   {
      uint8_t opcode = RvR_rw_read_u8(code);

      vm->instruction_pointers[instr_current++] = code_current;
      vm->code[code_current++] = opcode;


      switch(opcode)
      {
      //Instructions with 32bit argument
      case VM_OP_BEQ:
      case VM_OP_BNE:
      case VM_OP_BLT:
      case VM_OP_BLE:
      case VM_OP_BGT:
      case VM_OP_BGE:
      case VM_OP_BLTU:
      case VM_OP_BLEU:
      case VM_OP_BGTU:
      case VM_OP_BGEU:
         vm->code[code_current++] = RvR_rw_read_u32(code);
         break;
      }
   }
}

void RvR_vm_free(RvR_vm *vm)
{
   if(vm==NULL)
      return;
}

void RvR_vm_call(RvR_vm *vm, uint8_t opcode, ...)
{
   if(vm==NULL)
      return;
}
//-------------------------------------
