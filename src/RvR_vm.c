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
#define case_OP_INVALID default

#define case_OP_LOAD case VM_OP_LOAD
#define case_OP_LOAD_FP case VM_OP_LOAD_FP
#define case_OP_CUSTOM_0 case VM_OP_CUSTOM_0
#define case_OP_MISC_MEM case VM_OP_MISC_MEM
#define case_OP_IMM case VM_OP_IMM
#define case_OP_AUIPC case VM_OP_AUIPC
#define case_OP_AUIPC case VM_OP_AUIPC
#define case_OP_IMM_32 case VM_OP_IMM_32

#define case_OP_STORE case VM_OP_STORE
#define case_OP_STORE_FP case VM_OP_STORE_FP
#define case_OP_CUSTOM_1 case VM_OP_CUSTOM_1
#define case_OP_AMO case VM_OP_AMO
#define case_OP case VM_OP
#define case_OP_LUI case VM_OP_LUI
#define case_OP_32 case VM_OP_32

#define case_OP_MADD case VM_OP_MADD
#define case_OP_MSUB case VM_OP_MSUB
#define case_OP_NMSUB case VM_OP_NMSUB
#define case_OP_NMADD case VM_OP_NMADD
#define case_OP_FP case VM_OP_FP
//reserved
#define case_OP_CUSTOM_2 case VM_OP_CUSTOM_2

#define case_OP_BRANCH case VM_OP_BRANCH
#define case_OP_JALR case VM_OP_JALR
//reserved
#define case_OP_JAL case VM_OP_JAL
#define case_OP_SYSTEM case VM_OP_SYSTEM
//reserved
#define case_OP_CUSTOM_3 case VM_OP_CUSTOM_3
#endif
//-------------------------------------

//Typedefs
typedef enum
{
   VM_OP_LOAD = 3,
   VM_OP_MISC_MEM = 15,
   VM_OP_IMM = 19,
   VM_OP_AUIPC = 23,
   VM_OP_STORE = 35,
   VM_OP = 51,
   VM_OP_LUI = 55,
   VM_OP_BRANCH = 99,
   VM_OP_JALR = 103,
   VM_OP_JAL = 111,
   VM_OP_SYSTEM = 115,
}vm_opcode;
//-------------------------------------

//Variables
static int vm_syscall_term = 0;
//-------------------------------------

//Function prototypes
static uint32_t vm_syscall(RvR_vm *vm, uint32_t code);

static void vm_disassemble_instruction(uint32_t op);
//-------------------------------------

//Function implementations

void RvR_vm_create(RvR_vm *vm, RvR_rw *code, uint32_t stack)
{
   if(vm==NULL)
      return;
   memset(vm,0,sizeof(*vm));

   RvR_rw_seek(code,0,SEEK_END);
   size_t size = RvR_rw_tell(code);
   RvR_rw_seek(code,0,SEEK_SET);

   vm->code_size = size;
   vm->code = RvR_malloc(size);
   for(int i = 0;i<size/sizeof(uint32_t);i++)
      ((uint32_t *)vm->code)[i] = RvR_rw_read_u32(code);

   vm->mem_base = vm->code;
   vm->stack = RvR_malloc(stack);
   vm->regs[2] = (intptr_t)vm->stack-(intptr_t)vm->mem_base;
   vm->regs[2]+=stack;

   vm->pc = vm->code+4096;
}

void RvR_vm_free(RvR_vm *vm)
{
   if(vm==NULL)
      return;

   RvR_free(vm->code);
   RvR_free(vm->stack);
}

void RvR_vm_disassemble(RvR_vm *vm)
{
   if(vm==NULL)
      return;


   for(int i = 0;i<vm->code_size/4;i++)
   {
      printf("%8d|",i*4);

      int32_t op = ((uint32_t *)vm->code)[i+1024];
      vm_disassemble_instruction(op);
   }
}

void RvR_vm_run(RvR_vm *vm, uint32_t instr)
{
   if(vm==NULL)
      return;

#if RVR_VM_COMPUTED_GOTO

   const void *dispatch_table[128] =
   {
      &&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_LOAD,&&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_INVALID,
      &&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_MISC_MEM,
      &&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_INVALID,&&case_OP_IMM,
   }

#else
#define DISPATCH() vm->pc+=4; goto next
#define DISPATCH_BRANCH() goto next
#endif

   int32_t op;
   int32_t arg0;
   int32_t arg1;
   int32_t arg2;
   int32_t arg3;
   int32_t arg4;

   //R format
   //arg0 - funct7
   //arg1 - rs2
   //arg2 - rs1
   //arg3 - func3
   //arg4 - rd

   //I format
   //arg0 - imm[11:0]
   //arg1 - rs1
   //arg2 - funct3
   //arg3 - rd

   //S format
   //arg0 - imm[11:5]
   //arg1 - rs2
   //arg2 - rs1
   //arg3 - funct3
   //arg0 - imm[4:0]

   //B format
   //arg0 - imm[12|10:5]
   //arg1 - rs2
   //arg2 - rs1
   //arg3 - funct3
   //arg0 - imm[4:1|11]

   //U format
   //arg0 - imm[31,12]
   //arg1 - rd

   vm->pc = (uint8_t *)vm->code+instr;

   for(;;)
   {
#if !RVR_VM_COMPUTED_GOTO
   next:
   vm->regs[0] = 0;
   op = *((int32_t *)vm->pc);
   //vm_disassemble_instruction(op);
#endif

      switch(op&127)
      {
      case_OP_LOAD:
         //I format
         arg3 = (op>>7)&31;
         arg2 = (op>>12)&7;
         arg1 = (op>>15)&31;
         arg0 = (op>>20)&4095;
         arg0 = (arg0<<20)>>20; //sign extend

         switch(arg2)
         {
         case 0: //LB
            vm->regs[arg3] = *(((uint8_t *)vm->mem_base)+arg0+vm->regs[arg1]);
            vm->regs[arg3] = (vm->regs[arg3]<<24)>>24;
            break;
         case 1: //LH
            vm->regs[arg3] = *((uint16_t *)(((uint8_t *)vm->mem_base)+arg0+vm->regs[arg1]));
            vm->regs[arg3] = (vm->regs[arg3]<<16)>>16;
            break;
         case 2: //LW
            vm->regs[arg3] = *((uint32_t *)(((uint8_t *)vm->mem_base)+arg0+vm->regs[arg1]));
            break;
         case 4: //LBU
            vm->regs[arg3] = *(((uint8_t *)vm->mem_base)+arg0+vm->regs[arg1]);
            break;
         case 5: //LHU
            vm->regs[arg3] = *((uint16_t *)(((uint8_t *)vm->mem_base)+arg0+vm->regs[arg1]));
            break;
         }
         
         DISPATCH();

      case_OP_MISC_MEM:
         //TODO?
         DISPATCH();
      case_OP_IMM:
         //I format
         arg3 = (op>>7)&31;
         arg2 = (op>>12)&7;
         arg1 = (op>>15)&31;
         arg0 = (op>>20)&4095;
         arg0 = (arg0<<20)>>20; //sign extend

         switch(arg2)
         {
         case 0: //ADDI
            vm->regs[arg3] = vm->regs[arg1]+arg0;
            break;
         case 1: //SLLI
            vm->regs[arg3] = (uint32_t)vm->regs[arg1]<<arg0;
            break;
         case 2: //SLTI
            vm->regs[arg3] = vm->regs[arg1]<arg0;
            break;
         case 3: //SLTIU
            vm->regs[arg3] = (uint32_t)vm->regs[arg1]<(uint32_t)arg0;
            break;
         case 4: //XORI
            vm->regs[arg3] = (uint32_t)vm->regs[arg1]^arg0;
            break;
         case 5:  //SRLI/SRAI
            if(arg0&1024)
               vm->regs[arg3] = ((int32_t)vm->regs[arg1])>>arg0;
            else
               vm->regs[arg3] = ((uint32_t)vm->regs[arg1])>>arg0;
            break;
         case 6: //ORI
            vm->regs[arg3] = (uint32_t)vm->regs[arg1]|arg0;
            break;
         case 7: //ANDI
            vm->regs[arg3] = (uint32_t)vm->regs[arg1]&arg0;
            break;
         }

         DISPATCH();
      case_OP_AUIPC:
         //U format
         arg0 = op&4294963200;
         arg1 = (op>>7)&31;

         vm->regs[arg1] = (intptr_t)vm->code-(intptr_t)vm->mem_base;
         vm->regs[arg1]+=arg0;

         DISPATCH();
      case_OP_STORE:
         //S format
         arg0 = (op>>20)&4064;
         arg1 = (op>>20)&31;
         arg2 = (op>>15)&31;
         arg3 = (op>>12)&7;
         arg0|=(op>>7)&31;
         arg0 = (arg0<<20)>>20;

         switch(arg3)
         {
         case 0: //SB
            *(((uint8_t *)vm->mem_base)+arg0+vm->regs[arg2]) = (uint8_t)vm->regs[arg1];
            break;
         case 1: //SH
            *((uint16_t *)(((uint8_t *)vm->mem_base)+arg0+vm->regs[arg2])) = (uint16_t)vm->regs[arg1];
            break;
         case 2: //SW
            *((uint32_t *)(((uint8_t *)vm->mem_base)+arg0+vm->regs[arg2])) = (uint32_t)vm->regs[arg1];
            break;
         }

         DISPATCH();
      case_OP:
         //R format
         arg0 = (op>>25)&127;
         arg1 = (op>>20)&31;
         arg2 = (op>>15)&31;
         arg3 = (op>>12)&7;
         arg4 = (op>>7)&31;

         switch((arg0<<3)|arg3)
         {
         case 0: //ADD
            vm->regs[arg4] = vm->regs[arg2]+vm->regs[arg1];
            break;
         case 256: //SUB
            vm->regs[arg4] = vm->regs[arg2]-vm->regs[arg1];
            break;
         case 1: //SLL
            vm->regs[arg4] = vm->regs[arg2]<<(vm->regs[arg1]&31);
            break;
         case 2: //SLT
            vm->regs[arg4] = vm->regs[arg2]<vm->regs[arg1];
            break;
         case 3: //SLT
            vm->regs[arg4] = (uint32_t)vm->regs[arg2]<(uint32_t)vm->regs[arg1];
            break;
         case 4: //XOR
            vm->regs[arg4] = (uint32_t)vm->regs[arg2]^(uint32_t)vm->regs[arg1];
            break;
         case 5: //SRL
            vm->regs[arg4] = (uint32_t)vm->regs[arg2]>>(vm->regs[arg1]&31);
            break;
         case 261: //SRA
            vm->regs[arg4] = vm->regs[arg2]>>(vm->regs[arg1]&31);
            break;
         case 6: //OR
            vm->regs[arg4] = (uint32_t)vm->regs[arg2]|(uint32_t)vm->regs[arg1];
            break;
         case 7: //AND
            vm->regs[arg4] = (uint32_t)vm->regs[arg2]&(uint32_t)vm->regs[arg1];
            break;
         case 8: //MUL
            vm->regs[arg4] = vm->regs[arg2]*vm->regs[arg1];
            break;
         case 9: //MULH
            vm->regs[arg4] = ((int64_t)vm->regs[arg2]*(int64_t)vm->regs[arg1])>>32;
            break;
         case 10: //MULHSU
            vm->regs[arg4] = ((int64_t)vm->regs[arg2]*(uint64_t)vm->regs[arg1])>>32;
            break;
         case 11: //MULHU
            vm->regs[arg4] = ((uint64_t)vm->regs[arg2]*(uint64_t)vm->regs[arg1])>>32;
            break;
         case 12: //DIV
            vm->regs[arg4] = vm->regs[arg2]/vm->regs[arg1];
            break;
         case 13: //DIVU
            vm->regs[arg4] = (uint32_t)vm->regs[arg2]/(uint32_t)vm->regs[arg1];
            break;
         case 14: //REM
            vm->regs[arg4] = vm->regs[arg2]%vm->regs[arg1];
            break;
         case 15: //REMU
            vm->regs[arg4] = (uint32_t)vm->regs[arg2]%(uint32_t)vm->regs[arg1];
            break;
         }

         DISPATCH();
      case_OP_LUI:
         //U format
         arg0 = op&4294963200;
         arg1 = (op>>7)&31;

         vm->regs[arg1] = arg0;

         DISPATCH();
      case_OP_BRANCH:
         {
            //B format
            arg0 = (int32_t)(((int32_t)((uint32_t)(int32_t)(((((op>>19)&4096)|((op>>20)&2016))|((op>>7)&30))|((op<<4)&2048))<<19))>>19); 
            arg1 = (op>>20)&31;
            arg2 = (op>>15)&31;
            arg3 = (op>>12)&7;
            int32_t cmp = 0;

            switch(arg3)
            {
            case 0: //BEQ
               cmp = vm->regs[arg2]==vm->regs[arg1];
               break;
            case 1: //BNE
               cmp = vm->regs[arg2]!=vm->regs[arg1];
               break;
            case 4: //BLT
               cmp = vm->regs[arg2]<vm->regs[arg1];
               break;
            case 5: //BGE
               cmp = vm->regs[arg2]>=vm->regs[arg1];
               break;
            case 6: //BLTU
               cmp = (uint32_t)vm->regs[arg2]<(uint32_t)vm->regs[arg1];
               break;
            case 7: //BGEU
               cmp = (uint32_t)vm->regs[arg2]>=(uint32_t)vm->regs[arg1];
               break;
            }

            if(cmp)
            {
               vm->pc+=arg0;
               DISPATCH_BRANCH();
            }

            DISPATCH();
         }
      case_OP_JALR:
         //I format
         arg3 = (op>>7)&31;
         arg2 = (op>>12)&7;
         arg1 = (op>>15)&31;
         arg0 = (op>>20)&4095;
         arg0 = (arg0<<20)>>20; //sign extend

         uint8_t *pc = vm->pc+4;
         vm->pc = vm->code+((vm->regs[arg1]+arg0)&-2);
         vm->regs[arg3] = (intptr_t)pc-(intptr_t)vm->mem_base;

         DISPATCH_BRANCH();
      case_OP_JAL:
         //J format
         arg0 = (int32_t)(((int32_t)((uint32_t)(int32_t)(((((op>>11)&1048576)|((op>>20)&2046))|((op>>9)&2048))|((op<<0)&1044480))<<11))>>11);
         arg1 = (op>>7)&31;
         vm->regs[arg1] = ((intptr_t)vm->pc-(intptr_t)vm->mem_base)+4;
         vm->pc+=arg0;

         DISPATCH_BRANCH();
      case_OP_SYSTEM:
         //I format
         arg0 = (op>>20)&4095;
         arg0 = (arg0<<20)>>20; //sign extend
         vm_syscall_term = 0;

         switch(arg0)
         {
         case 0:
            vm->regs[10] = vm_syscall(vm,vm->regs[17]);
            if(vm_syscall_term)
               return;
            break;
         case 1:
            return;
         }

         DISPATCH();
      }
   }
}

static uint32_t vm_syscall(RvR_vm *vm, uint32_t code)
{
   switch(code)
   {
   case 0: //exit
      vm_syscall_term = 1;
      break;
   case 64: //puts
      puts((char *)((uint8_t *)vm->mem_base+vm->regs[10])); 
      break;
   }
   return 0;
}

static void vm_disassemble_instruction(uint32_t op)
{
   printf("0x%x: ",op);
   static const char *reg_names[32] = {"zero","ra","sp","gp","tp","t0","t1","t2","s0/fp","s1","a0","a1","a2","a3","a4","a5","a6","a7","s2","s3","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6"};
   int32_t arg0;
   int32_t arg1;
   int32_t arg2;
   int32_t arg3;
   int32_t arg4;

   switch(op&127)
   {
   case VM_OP_LOAD:
      //I format
      arg3 = (op>>7)&31;
      arg2 = (op>>12)&7;
      arg1 = (op>>15)&31;
      arg0 = (op>>20)&4095;
      arg0 = (arg0<<20)>>20; //sign extend

      switch(arg2)
      {
      case 0: //LB
         printf("lb %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 1: //LH
         printf("lh %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 2: //LW
         printf("lw %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 4: //LBU
         printf("lbu %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 5: //LHU
         printf("lhu %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      default:
         printf("unknown LOAD instruction %d\n",arg2);
         break;
      }
      break;
   case VM_OP_MISC_MEM:
      puts("fence");
      break;
   case VM_OP_IMM:
      //I format
      arg3 = (op>>7)&31;
      arg2 = (op>>12)&7;
      arg1 = (op>>15)&31;
      arg0 = (op>>20)&4095;
      arg0 = (arg0<<20)>>20; //sign extend

      switch(arg2)
      {
      case 0: //ADDI
         printf("addi %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 1: //SLLI
         printf("slli %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 2: //SLTI
         printf("slti %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 3: //SLTIU
         printf("sltiu %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 4: //XORI
         printf("xori %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 5:  //SRLI/SRAI
         if(arg0&1024)
            printf("srai %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         else
            printf("srli %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 6: //ORI
         printf("ori %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 7: //ANDI
         printf("andi %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      default:
         printf("unknown OP-IMM instruction %d\n",arg2);
         break;
      }
      break;
   case VM_OP_AUIPC:
      //U format
      arg0 = op&4294963200;
      arg1 = (op>>7)&31;

      printf("auipc %s,%d\n",reg_names[arg1],arg0);
      break;
   case VM_OP_STORE:
      //S format
      arg0 = (op>>20)&4064;
      arg1 = (op>>20)&31;
      arg2 = (op>>15)&31;
      arg3 = (op>>12)&7;
      arg0|=(op>>7)&31;
      arg0 = (arg0<<20)>>20;

      switch(arg3)
      {
      case 0: //SB
         printf("sb %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 1: //SH
         printf("sh %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 2: //SW
         printf("sw %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      default:
         printf("unknown STORE instruction %d\n",arg3);
         break;
      }
      break;
   case VM_OP:
      //R format
      arg0 = (op>>25)&127;
      arg1 = (op>>20)&31;
      arg2 = (op>>15)&31;
      arg3 = (op>>12)&7;
      arg4 = (op>>7)&31;

      switch((arg0<<3)|arg3)
      {
      case 0: //ADD
         printf("add %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 256: //SUB
         printf("sub %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 1: //SLL
         printf("sll %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 2: //SLT
         printf("slt %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 3: //SLTU
         printf("sltu %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 4: //XOR
         printf("xor %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 5: //SRL
         printf("srl %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 261: //SRA
         printf("sra %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 6: //OR
         printf("or %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 7: //AND
         printf("and %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 8: //MUL
         printf("mul %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 9: //MULH
         printf("mulh %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 10: //MULHSU
         printf("mulhsu %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 11: //MULHU
         printf("mulhu %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 12: //DIV
         printf("div %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 13: //DIVU
         printf("divu %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 14: //REM
         printf("rem %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 15: //REMU
         printf("remu %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      default:
         printf("unknown OP instruction %d\n",(arg0<<3)|arg3);
         break;
      }
      break;
   case VM_OP_LUI:
      //U format
      arg0 = (op>>12)&1048575;
      arg1 = (op>>7)&31;

      printf("lui %s,%d\n",reg_names[arg1],arg0);
      break;
   case VM_OP_BRANCH:
      //B format
      arg0 = (int32_t)(((int32_t)((uint32_t)(int32_t)(((((op>>19)&4096)|((op>>20)&2016))|((op>>7)&30))|((op<<4)&2048))<<19))>>19); 
      arg1 = (op>>20)&31;
      arg2 = (op>>15)&31;
      arg3 = (op>>12)&7;

      switch(arg3)
      {
      case 0: //BEQ
         printf("beq %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 1: //BNE
         printf("bne %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 4: //BLT
         printf("blt %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 5: //BGE
         printf("bge %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 6: //BLTU
         printf("bltu %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 7: //BGEU
         printf("bgeu %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      default:
         printf("unknown BRANCH instruction %d\n",arg3);
         break;
      }
      break;
   case VM_OP_JALR:
      //I format
      arg3 = (op>>7)&31;
      arg2 = (op>>12)&7;
      arg1 = (op>>15)&31;
      arg0 = (op>>20)&4095;
      arg0 = (arg0<<20)>>20; //sign extend

      printf("jalr %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
      break;
   case VM_OP_JAL:
      arg0 = (int32_t)(((int32_t)((uint32_t)(int32_t)(((((op>>11)&1048576)|((op>>20)&2046))|((op>>9)&2048))|((op<<0)&1044480))<<11))>>11);
      arg1 = (op>>7)&31;
      printf("jal %s,%d\n",reg_names[arg1],arg0);
      break;
   case VM_OP_SYSTEM:
      //I format
      arg0 = (op>>20)&4095;
      arg0 = (arg0<<20)>>20; //sign extend

      switch(arg0)
      {
      case 0:
         printf("scall\n");
         break;
      case 1:
         printf("sbreak\n");
         break;
      default:
         printf("unknown SYSTEM instruction %d\n",arg0);
         break;
      }
      break;
   default:
      printf("Unknown instruction %d\n",op&127);
      break;
   }
}
//-------------------------------------
