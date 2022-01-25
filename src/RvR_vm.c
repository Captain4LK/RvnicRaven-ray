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
#include <stdarg.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
//-------------------------------------

//#defines
#if !RVR_VM_COMPUTED_GOTO
#define case_OP_RETURN case VM_OP_RETURN
#define case_OP_LOAD case VM_OP_LOAD
#define case_OP_LB case VM_OP_LB
#define case_OP_LH case VM_OP_LH
#define case_OP_LW case VM_OP_LW
#define case_OP_SB case VM_OP_SB
#define case_OP_SH case VM_OP_SH
#define case_OP_SW case VM_OP_SW
#define case_OP_JUMP case VM_OP_JUMP
#define case_OP_BEQ case VM_OP_BEQ
#define case_OP_BNE case VM_OP_BNE
#define case_OP_BLT case VM_OP_BLT
#define case_OP_BLE case VM_OP_BLE
#define case_OP_BGT case VM_OP_BGT
#define case_OP_BGE case VM_OP_BGE
#define case_OP_BLTU case VM_OP_BLTU
#define case_OP_BLEU case VM_OP_BLEU
#define case_OP_BGTU case VM_OP_BGTU
#define case_OP_BGEU case VM_OP_BGEU
#define case_OP_ADD case VM_OP_ADD
#define case_OP_SUB case VM_OP_SUB
#define case_OP_NEGI case VM_OP_NEGI
#define case_OP_DIVI case VM_OP_DIVI
#define case_OP_DIVU case VM_OP_DIVU
#define case_OP_MULI case VM_OP_MULI
#define case_OP_MULU case VM_OP_MULU
#define case_OP_MODI case VM_OP_MODI
#define case_OP_MODU case VM_OP_MODU
#define case_OP_AND case VM_OP_AND
#define case_OP_OR case VM_OP_OR
#define case_OP_XOR case VM_OP_XOR
#define case_OP_NOT case VM_OP_NOT
#define case_OP_LSH case VM_OP_LSH
#define case_OP_RSHI case VM_OP_RSHI
#define case_OP_RSHU case VM_OP_RSHU
#define case_OP_SEX8 case VM_OP_SEX8
#define case_OP_SEX16 case VM_OP_SEX16
#endif
//-------------------------------------

//Typedefs
typedef enum
{
   VM_OP_RETURN = 1,

   //
   VM_OP_LOAD = 2, //Load constant
   VM_OP_LB = 3, //Load Byte (8 bit)
   VM_OP_LH = 4, //Load Halfword (16 bit)
   VM_OP_LW = 5, //Load Word (32 bit)
   VM_OP_SB = 6, //Store Byte (8 bit)
   VM_OP_SH = 7, //Store Halfword (16 bit)
   VM_OP_SW = 8, //Store Word (32 bit)

   //Branches
   VM_OP_JUMP = 9, //Jump
   VM_OP_BEQ = 10, //Branch if equal
   VM_OP_BNE = 11, //Brain if not equal
   VM_OP_BLT = 12, //Branch if less than
   VM_OP_BLE = 13, //Branch if less than or equal
   VM_OP_BGT = 14, //Branch if greater than
   VM_OP_BGE = 15, //Branch if greater than or equal
   VM_OP_BLTU = 16, //Branch if less than, unsigned
   VM_OP_BLEU = 17, //Branch if less than or equal, unsigned
   VM_OP_BGTU = 18, //Branch if greater than, unsigned
   VM_OP_BGEU = 19, //Branch if greater than or equal, unsigned
   
   //Arithmetic
   VM_OP_ADD = 20,
   VM_OP_SUB = 21,
   VM_OP_NEGI = 22,
   VM_OP_DIVI = 23,
   VM_OP_DIVU = 24,
   VM_OP_MULI = 25,
   VM_OP_MULU = 26,
   VM_OP_MODI = 27,
   VM_OP_MODU = 28,

   //Bitwise manipulation
   VM_OP_AND = 29,
   VM_OP_OR = 30,
   VM_OP_XOR = 31,
   VM_OP_NOT = 32,
   VM_OP_LSH = 33,
   VM_OP_RSHI = 34,
   VM_OP_RSHU = 35,
   VM_OP_SEX8 = 36,
   VM_OP_SEX16 = 37,
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
   int32_t data_offset = RvR_rw_read_i32(code);
   int32_t data_len = RvR_rw_read_i32(code);
   int32_t str_len = RvR_rw_read_i32(code);
   int32_t bss_len = RvR_rw_read_i32(code);

   //Populate vm struct with info read from header
   vm->instruction_count = code_count;
   vm->instruction_pointers = RvR_malloc(sizeof(*vm->instruction_pointers)*code_count);

   //Allocate memory for data, round up length to next power of two
   //Using: https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
   int len = (data_len+str_len+bss_len-1)*2; //Subtract 1 and multiply by two --> next power of two is largest bit set
   int r = 0;
   while(len>>=1)
      r++;
   len = 1<<r;
   vm->data_len = len+4;
   vm->data_mask = len-1;
   vm->data = RvR_malloc(vm->data_len);
   memset(vm->data,0,vm->data_len);
   
   //Read data
   //.data section may need to be byteswapped (int32)
   RvR_rw_seek(code,data_offset,SEEK_SET);
   for(int i = 0;i<data_len/4;i++)
      vm->data[i] = RvR_rw_read_u32(code);
   for(int i = 0;i<str_len;i++)
      *(((uint8_t *)vm->data)+i) = RvR_rw_read_u8(code);

   //Read instructions and make them int32 aligned (TODO: does this improve or worsen performance?)
   vm->code = RvR_malloc(code_len*sizeof(*vm->code)); 
   RvR_rw_seek(code,code_offset,SEEK_SET);
   int code_current = 0;
   int instr_current = 0;
   while(instr_current<vm->instruction_count)
   {
      uint8_t opcode = RvR_rw_read_u8(code);

      vm->instruction_pointers[instr_current++] = code_current;
      vm->code[code_current++] = opcode;


      switch(opcode)
      {
      //Instructions with 32bit argument
      case VM_OP_LOAD:
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

   //Remap jump indexes for branch instructions
   code_current = 0;
   instr_current = 0;
   while(instr_current<vm->instruction_count)
   {
      uint8_t opcode = vm->code[code_current++];
      instr_current++;

      switch(opcode)
      {
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
         vm->code[code_current] = vm->instruction_pointers[vm->code[code_current]];
         code_current++;
         break;
      case VM_OP_LOAD:
         code_current++;
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

   int args[16] = {0};
   args[0] = opcode;

   va_list va;
   va_start(va,opcode);
   for(int i = 1;i<16;i++)
      args[i] = va_arg(va,int);
   va_end(va);

   int32_t stack_array[RVR_VM_STACK_SIZE] = {0};
   int32_t *stack = stack_array;

   int32_t *ip = vm->code;
   int32_t r0;
   int32_t r1;

#if RVR_VM_XOMPUTED_GOTO
   static const void *dispatch_tabel[] = 
   {
      &&case_OP_RETURN,
   };
   #define DISPATCH() goto *dispatch_table[*ip++]
   #define DISPATCH_R() r0 = *stack; r1 = *(stack-1); DISPATCH()
#else
   #define DISPATCH_R() goto next_r
   #define DISPATCH() goto next
#endif

   for(;;)
   {
   next_r:
      r0 = *stack;
      r1 = *(stack-1);
   next:

      switch(*ip++)
      {
      case_OP_RETURN: break;

      case_OP_LOAD: stack++; r1 = r0; r0 = *ip; *stack = *ip; ip++; DISPATCH();

      case_OP_JUMP: ip = vm->code+vm->instruction_pointers[r0]; stack--; DISPATCH_R();
      case_OP_BEQ: stack-=2; if(r1==r0) { ip = vm->code+(*ip); DISPATCH_R(); } else { ip++; DISPATCH_R(); }
      case_OP_BNE: stack-=2; if(r1!=r0) { ip = vm->code+(*ip); DISPATCH_R(); } else { ip++; DISPATCH_R(); }
      case_OP_BLT: stack-=2; if(r1<r0) { ip = vm->code+(*ip); DISPATCH_R(); } else { ip++; DISPATCH_R(); }
      case_OP_BLE: stack-=2; if(r1<=r0) { ip = vm->code+(*ip); DISPATCH_R(); } else { ip++; DISPATCH_R(); }
      case_OP_BGT: stack-=2; if(r1>r0) { ip = vm->code+(*ip); DISPATCH_R(); } else { ip++; DISPATCH_R(); }
      case_OP_BGE: stack-=2; if(r1>=r0) { ip = vm->code+(*ip); DISPATCH_R(); } else { ip++; DISPATCH_R(); }
      case_OP_BLTU: stack-=2; if((uint32_t)r1<(uint32_t)r0) { ip = vm->code+(*ip); DISPATCH_R(); } else { ip++; DISPATCH_R(); }
      case_OP_BLEU: stack-=2; if((uint32_t)r1<=(uint32_t)r0) { ip = vm->code+(*ip); DISPATCH_R(); } else { ip++; DISPATCH_R(); }
      case_OP_BGTU: stack-=2; if((uint32_t)r1>(uint32_t)r0) { ip = vm->code+(*ip); DISPATCH_R(); } else { ip++; DISPATCH_R(); }
      case_OP_BGEU: stack-=2; if((uint32_t)r1>=(uint32_t)r0) { ip = vm->code+(*ip); DISPATCH_R(); } else { ip++; DISPATCH_R(); }

      case_OP_ADD: stack--; *stack = r1+r0; DISPATCH_R();
      case_OP_SUB: stack--; *stack = r1-r0; DISPATCH_R();
      case_OP_NEGI: *stack = -r0; DISPATCH_R();
      case_OP_DIVI: stack--; *stack = r1/r0; DISPATCH_R();
      case_OP_DIVU: stack--; *stack = (uint32_t)r1/(uint32_t)r0; DISPATCH_R();
      case_OP_MULI: stack--; *stack = r1*r0; DISPATCH_R();
      case_OP_MULU: stack--; *stack = (uint32_t)r1*(uint32_t)r0; DISPATCH_R();
      case_OP_MODI: stack--; *stack = r1%r0; DISPATCH_R();
      case_OP_MODU: stack--; *stack = (uint32_t)r1%(uint32_t)r0; DISPATCH_R();

      case_OP_AND: stack--; *stack = (uint32_t)r1&(uint32_t)r0; DISPATCH_R();
      case_OP_OR: stack--; *stack = (uint32_t)r1|(uint32_t)r0; DISPATCH_R();
      case_OP_XOR: stack--; *stack = (uint32_t)r1^(uint32_t)r0; DISPATCH_R();
      case_OP_NOT: *stack = ~(uint32_t)r0; DISPATCH_R();
      case_OP_LSH: stack--; *stack = r1<<r0; DISPATCH_R();
      case_OP_RSHI: stack--; *stack = r1>>r0; DISPATCH_R();
      case_OP_RSHU: stack--; *stack = (uint32_t)r1>>r0; DISPATCH_R();
      case_OP_SEX8: *stack = (uint8_t)*stack; DISPATCH_R();
      case_OP_SEX16: *stack = (uint16_t)*stack; DISPATCH_R();
      }
   }
}
//-------------------------------------
