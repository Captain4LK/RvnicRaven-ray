/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/*
Implements rv32im
No float/double extensions since RvnicRaven only uses fixed point anyway
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
//-------------------------------------

//#defines
#if !RVR_VM_COMPUTED_GOTO

#define case_OP_INVALID  default
#define case_OP_LOAD     case VM_OP_LOAD
#define case_OP_MISC_MEM case VM_OP_MISC_MEM
#define case_OP_IMM      case VM_OP_IMM
#define case_OP_AUIPC    case VM_OP_AUIPC
#define case_OP_STORE    case VM_OP_STORE
#define case_OP          case VM_OP
#define case_OP_LUI      case VM_OP_LUI
#define case_OP_BRANCH   case VM_OP_BRANCH
#define case_OP_JALR     case VM_OP_JALR
#define case_OP_JAL      case VM_OP_JAL
#define case_OP_SYSTEM   case VM_OP_SYSTEM

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

static void *vm_stack = NULL;
//-------------------------------------

//Function prototypes
static uint32_t vm_syscall(RvR_vm *vm, uint32_t code);

static void vm_disassemble_instruction(uint32_t op);
//-------------------------------------

//Function implementations

void RvR_vm_create(RvR_vm *vm, RvR_rw *code)
{
   if(vm==NULL)
      return;

   if(vm_stack==NULL)
   {
      vm_stack = RvR_malloc(RVR_VM_STACK_SIZE);
      memset(vm_stack,0,RVR_VM_STACK_SIZE);
   }

   memset(vm,0,sizeof(*vm));

   vm->pc_entry = RvR_rw_read_u32(code);
   size_t size = RvR_rw_read_u32(code);
   uint32_t entry_count = RvR_rw_read_u32(code);

   vm->code_size = size;
   vm->code = RvR_malloc(size);

   for(int i = 0;i<entry_count;i++)
   {
      uint32_t offset = RvR_rw_read_u32(code);
      uint32_t addr = RvR_rw_read_u32(code);
      uint32_t filesz = RvR_rw_read_u32(code);
      uint32_t memsz = RvR_rw_read_u32(code);

      size_t pos = RvR_rw_tell(code);
      memset(vm->code+addr,0,memsz);
      RvR_rw_seek(code,offset,SEEK_SET);
      RvR_rw_read(code,vm->code+addr,1,filesz);
      RvR_rw_seek(code,pos,SEEK_SET);
   }

   vm->mem_base = vm->code;
}

void RvR_vm_free(RvR_vm *vm)
{
   if(vm==NULL)
      return;

   RvR_free(vm->code);
   RvR_free(vm->stack);
}

void RvR_vm_stack_free()
{
   if(vm_stack!=NULL)
      RvR_free(vm_stack);
}

void RvR_vm_disassemble(RvR_vm *vm)
{
   if(vm==NULL)
      return;

   for(int i = 0;i<vm->code_size/4;i++)
   {
      RvR_log("%8d|",i*4);

      int32_t op = ((uint32_t *)vm->code)[i+1024];
      vm_disassemble_instruction(op);
   }
}

void RvR_vm_run(RvR_vm *vm, uint32_t instr)
{
   if(vm==NULL)
      return;

   vm->pc = (uint8_t *)vm->code+instr;
   vm->regs[2] = ((intptr_t)vm_stack-(intptr_t)vm->mem_base)+RVR_VM_STACK_SIZE;
   int32_t op;
   int32_t arg0;
   int32_t arg1;
   int32_t arg2;
   int32_t arg3;
   int32_t arg4;

#if RVR_VM_COMPUTED_GOTO

   void *dispatch_table[128];
   for(int i = 0;i<128;i++)
      dispatch_table[i] = &&case_OP_INVALID;
   dispatch_table[3] = &&case_OP_LOAD;
   dispatch_table[15] = &&case_OP_MISC_MEM;
   dispatch_table[19] = &&case_OP_IMM;
   dispatch_table[23] = &&case_OP_AUIPC;
   dispatch_table[35] = &&case_OP_STORE;
   dispatch_table[51] = &&case_OP;
   dispatch_table[55] = &&case_OP_LUI;
   dispatch_table[99] = &&case_OP_BRANCH;
   dispatch_table[103] = &&case_OP_JALR;
   dispatch_table[111] = &&case_OP_JAL;
   dispatch_table[115] = &&case_OP_SYSTEM;

#define DISPATCH() vm->pc+=4; vm->regs[0] = 0; op = *((int32_t *)vm->pc); goto *dispatch_table[op&127]
#define DISPATCH_BRANCH() vm->regs[0] = 0; op = *((int32_t *)vm->pc); goto *dispatch_table[op&127]

DISPATCH_BRANCH();

#else
#define DISPATCH() vm->pc+=4; goto next
#define DISPATCH_BRANCH() goto next
#endif

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

   //J format
   //arg0 - imm[20]
   //arg0 - imm[10:1]
   //arg0 - imm[11]
   //arg0 - imm[19:12]
   //arg1 - rd

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
            vm->regs[arg3] = *(((int8_t *)vm->mem_base)+arg0+vm->regs[arg1]);
            break;
         case 1: //LH
            vm->regs[arg3] = *((int16_t *)(((uint8_t *)vm->mem_base)+arg0+vm->regs[arg1]));
            break;
         case 2: //LW
            vm->regs[arg3] = *((int32_t *)(((uint8_t *)vm->mem_base)+arg0+vm->regs[arg1]));
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
            vm->regs[arg3] = vm->regs[arg1]^arg0;
            break;
         case 5:  //SRLI/SRAI
            if(arg0&1024)
               vm->regs[arg3] = ((int32_t)vm->regs[arg1])>>arg0;
            else
               vm->regs[arg3] = ((uint32_t)vm->regs[arg1])>>arg0;
            break;
         case 6: //ORI
            vm->regs[arg3] = vm->regs[arg1]|arg0;
            break;
         case 7: //ANDI
            vm->regs[arg3] = vm->regs[arg1]&arg0;
            break;
         }

         DISPATCH();
      case_OP_AUIPC:
         //U format
         arg0 = op&4294963200;
         arg1 = (op>>7)&31;

         vm->regs[arg1] = (intptr_t)vm->pc-(intptr_t)vm->mem_base;
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
         case 3: //SLTU
            vm->regs[arg4] = (uint32_t)vm->regs[arg2]<(uint32_t)vm->regs[arg1];
            break;
         case 4: //XOR
            vm->regs[arg4] = vm->regs[arg2]^vm->regs[arg1];
            break;
         case 5: //SRL
            vm->regs[arg4] = (uint32_t)vm->regs[arg2]>>(vm->regs[arg1]&31);
            break;
         case 261: //SRA
            vm->regs[arg4] = vm->regs[arg2]>>(vm->regs[arg1]&31);
            break;
         case 6: //OR
            vm->regs[arg4] = vm->regs[arg2]|vm->regs[arg1];
            break;
         case 7: //AND
            vm->regs[arg4] = vm->regs[arg2]&vm->regs[arg1];
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
      case_OP_INVALID:
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
   case 1: //memchr
      {
         void *res = memchr((uint8_t *)vm->mem_base+vm->regs[10],vm->regs[11],vm->regs[12]);
         return res==NULL?0:(intptr_t)res-(intptr_t)vm->mem_base;
      }
   case 2: //memcmp
      return memcmp((uint8_t *)vm->mem_base+vm->regs[10],(uint8_t *)vm->mem_base+vm->regs[11],vm->regs[12]);
   case 3: //memcpy
      return (intptr_t)memcpy((uint8_t *)vm->mem_base+vm->regs[10],(uint8_t *)vm->mem_base+vm->regs[11],vm->regs[12])-(intptr_t)vm->mem_base;
   case 4: //memmove
      return (intptr_t)memmove((uint8_t *)vm->mem_base+vm->regs[10],(uint8_t *)vm->mem_base+vm->regs[11],vm->regs[12])-(intptr_t)vm->mem_base;
   case 5: //memset
      return (intptr_t)memset((uint8_t *)vm->mem_base+vm->regs[10],vm->regs[11],vm->regs[12])-(intptr_t)vm->mem_base;
   case 6: //strcat
      return (intptr_t)strcat((char *)((uint8_t *)vm->mem_base+vm->regs[10]),(char *)((uint8_t *)vm->mem_base+vm->regs[11]))-(intptr_t)vm->mem_base;
   case 7: //strchr
      {
         void *res = strchr((char *)((uint8_t *)vm->mem_base+vm->regs[10]),vm->regs[11]);
         return res==NULL?0:(intptr_t)res-(intptr_t)vm->mem_base;
      }
   case 8: //strcmp
      return strcmp((char *)((uint8_t *)vm->mem_base+vm->regs[10]),(char *)((uint8_t *)vm->mem_base+vm->regs[11]));
   case 9: //strcpy
      return (intptr_t)strcpy((char *)((uint8_t *)vm->mem_base+vm->regs[10]),(char *)((uint8_t *)vm->mem_base+vm->regs[11]))-(intptr_t)vm->mem_base;
   case 10: //strcspn
      return strcspn((char *)((uint8_t *)vm->mem_base+vm->regs[10]),(char *)((uint8_t *)vm->mem_base+vm->regs[11]));
   case 11: //strlen
      return strlen((char *)((uint8_t *)vm->mem_base+vm->regs[10]));
   case 12: //strncat
      return (intptr_t)strncat((char *)((uint8_t *)vm->mem_base+vm->regs[10]),(char *)((uint8_t *)vm->mem_base+vm->regs[11]),vm->regs[12])-(intptr_t)vm->mem_base;
   case 13: //strncmp
      return strncmp((char *)((uint8_t *)vm->mem_base+vm->regs[10]),(char *)((uint8_t *)vm->mem_base+vm->regs[11]),vm->regs[12]);
   case 14: //strncpy
      return (intptr_t)strncmp((char *)((uint8_t *)vm->mem_base+vm->regs[10]),(char *)((uint8_t *)vm->mem_base+vm->regs[11]),vm->regs[12])-(intptr_t)vm->mem_base;
   case 15: //strpbrk
      {
         void *res = strpbrk((char *)((uint8_t *)vm->mem_base+vm->regs[10]),(char *)((uint8_t *)vm->mem_base+vm->regs[11]));
         return res==NULL?0:(intptr_t)res-(intptr_t)vm->mem_base;
      }
   case 16: //strrchr
      {
         void *res = strrchr((char *)((uint8_t *)vm->mem_base+vm->regs[10]),vm->regs[11]);
         return res==NULL?0:(intptr_t)res-(intptr_t)vm->mem_base;
      }
   case 17: //strspn
      return strspn((char *)((uint8_t *)vm->mem_base+vm->regs[10]),(char *)((uint8_t *)vm->mem_base+vm->regs[11]));
   case 18: //abort
      vm_syscall_term = 1;
      break;
   case 19: //atoi
      return atoi((char *)((uint8_t *)vm->mem_base+vm->regs[10]));
   case 20: //free
      RvR_free((uint8_t *)vm->mem_base+vm->regs[10]);
      break;
   case 21: //malloc
      {
         void *res = RvR_malloc(vm->regs[10]);
         return res==NULL?0:(intptr_t)res-(intptr_t)vm->mem_base;
      }
   case 22: //rand
      return rand();
   case 23: //realloc
      {
         void *res = RvR_realloc((uint8_t *)vm->mem_base+vm->regs[10],vm->regs[11]);
         return res==NULL?0:(intptr_t)res-(intptr_t)vm->mem_base;
      }
   }
   return 0;
}

static void vm_disassemble_instruction(uint32_t op)
{
   RvR_log("%8x|",op);
   static const char *reg_names[32] = {"zero","ra","sp","gp","tp","t0","t1","t2","s0","s1","a0","a1","a2","a3","a4","a5","a6","a7","s2","s3","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6"};
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
         RvR_log("lb %s,%d(%s)\n",reg_names[arg3],arg0,reg_names[arg1]);
         break;
      case 1: //LH
         RvR_log("lh %s,%d(%s)\n",reg_names[arg3],arg0,reg_names[arg1]);
         break;
      case 2: //LW
         RvR_log("lw %s,%d(%s)\n",reg_names[arg3],arg0,reg_names[arg1]);
         break;
      case 4: //LBU
         RvR_log("lbu %s,%d(%s)\n",reg_names[arg3],arg0,reg_names[arg1]);
         break;
      case 5: //LHU
         RvR_log("lhu %s,%d(%s)\n",reg_names[arg3],arg0,reg_names[arg1]);
         break;
      default:
         RvR_log("unknown LOAD instruction %d\n",arg2);
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
         RvR_log("addi %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 1: //SLLI
         RvR_log("slli %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 2: //SLTI
         RvR_log("slti %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 3: //SLTIU
         RvR_log("sltiu %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 4: //XORI
         RvR_log("xori %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 5:  //SRLI/SRAI
         if(arg0&1024)
            RvR_log("srai %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         else
            RvR_log("srli %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 6: //ORI
         RvR_log("ori %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      case 7: //ANDI
         RvR_log("andi %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
         break;
      default:
         RvR_log("unknown OP-IMM instruction %d\n",arg2);
         break;
      }
      break;
   case VM_OP_AUIPC:
      //U format
      arg0 = (op>>12)&1048575;
      arg1 = (op>>7)&31;

      RvR_log("auipc %s,%d\n",reg_names[arg1],arg0);
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
         RvR_log("sb %s,%d(%s)\n",reg_names[arg1],arg0,reg_names[arg1]);
         break;
      case 1: //SH
         RvR_log("sh %s,%d(%s)\n",reg_names[arg1],arg0,reg_names[arg1]);
         break;
      case 2: //SW
         RvR_log("sw %s,%d(%s)\n",reg_names[arg1],arg0,reg_names[arg1]);
         break;
      default:
         RvR_log("unknown STORE instruction %d\n",arg3);
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
         RvR_log("add %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 256: //SUB
         RvR_log("sub %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 1: //SLL
         RvR_log("sll %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 2: //SLT
         RvR_log("slt %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 3: //SLTU
         RvR_log("sltu %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 4: //XOR
         RvR_log("xor %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 5: //SRL
         RvR_log("srl %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 261: //SRA
         RvR_log("sra %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 6: //OR
         RvR_log("or %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 7: //AND
         RvR_log("and %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 8: //MUL
         RvR_log("mul %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 9: //MULH
         RvR_log("mulh %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 10: //MULHSU
         RvR_log("mulhsu %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 11: //MULHU
         RvR_log("mulhu %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 12: //DIV
         RvR_log("div %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 13: //DIVU
         RvR_log("divu %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 14: //REM
         RvR_log("rem %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      case 15: //REMU
         RvR_log("remu %s,%s,%s\n",reg_names[arg4],reg_names[arg2],reg_names[arg1]);
         break;
      default:
         RvR_log("unknown OP instruction %d\n",(arg0<<3)|arg3);
         break;
      }
      break;
   case VM_OP_LUI:
      //U format
      arg0 = (op>>12)&1048575;
      arg1 = (op>>7)&31;

      RvR_log("lui %s,%d\n",reg_names[arg1],arg0);
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
         RvR_log("beq %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 1: //BNE
         RvR_log("bne %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 4: //BLT
         RvR_log("blt %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 5: //BGE
         RvR_log("bge %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 6: //BLTU
         RvR_log("bltu %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      case 7: //BGEU
         RvR_log("bgeu %s,%s,%d\n",reg_names[arg2],reg_names[arg1],arg0);
         break;
      default:
         RvR_log("unknown BRANCH instruction %d\n",arg3);
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

      RvR_log("jalr %s,%s,%d\n",reg_names[arg3],reg_names[arg1],arg0);
      break;
   case VM_OP_JAL:
      arg0 = (int32_t)(((int32_t)((uint32_t)(int32_t)(((((op>>11)&1048576)|((op>>20)&2046))|((op>>9)&2048))|((op<<0)&1044480))<<11))>>11);
      arg1 = (op>>7)&31;
      RvR_log("jal %s,%d\n",reg_names[arg1],arg0);
      break;
   case VM_OP_SYSTEM:
      //I format
      arg0 = (op>>20)&4095;
      arg0 = (arg0<<20)>>20; //sign extend

      switch(arg0)
      {
      case 0:
         RvR_log("ecall\n");
         break;
      case 1:
         RvR_log("ebreak\n");
         break;
      default:
         RvR_log("unknown SYSTEM instruction %d\n",arg0);
         break;
      }
      break;
   default:
      RvR_log("Unknown instruction %d\n",op&127);
      break;
   }
}
//-------------------------------------
