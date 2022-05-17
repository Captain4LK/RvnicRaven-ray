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
#include <string.h>
#include <inttypes.h>
#include "../src/RvnicRaven.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define READ_ARG(I) \
   ((++(I))<argc?argv[(I)]:NULL)

#define MIN(a,b) ((a)<(b)?(a):(b))
//-------------------------------------

//Typedefs
typedef struct
{
   uint32_t type;
   uint32_t p_offset;
   uint32_t p_vaddr;
   uint32_t p_filesz;
   uint32_t p_memsz;
   uint32_t flags;
}Entry;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void print_help(char **argv);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   const char *path_in = NULL;
   const char *path_out = NULL;

   for(int i = 1;i<argc;i++)
   {
      if(strcmp(argv[i],"--help")==0||
         strcmp(argv[i],"-help")==0||
         strcmp(argv[i],"-h")==0||
         strcmp(argv[i],"?")==0)
      { print_help(argv); return 0; }
      else if(strcmp(argv[i],"-fin")==0)
         path_in = READ_ARG(i);
      else if(strcmp(argv[i],"-fout")==0)
         path_out = READ_ARG(i);
   }
   
   if(path_in==NULL)
   {
      RvR_log("input texture not specified, try %s -h for more info\n",argv[0]);
      return 0;
   }
   if(path_out==NULL)
   {
      RvR_log("output texture not specified, try %s -h for more info\n",argv[0]);
      return 0;
   }

   RvR_rw rw;
   RvR_rw_init_path(&rw,path_in,"rb");
   RvR_rw rw_out;
   RvR_rw_init_path(&rw_out,path_out,"wb");

   //header
   //Magic num
   RvR_error_check(RvR_rw_read_u8(&rw)==0x7f,"ELF header","magic num incorrect");
   RvR_error_check(RvR_rw_read_u8(&rw)=='E',"ELF header","magic num incorrect");
   RvR_error_check(RvR_rw_read_u8(&rw)=='L',"ELF header","magic num incorrect");
   RvR_error_check(RvR_rw_read_u8(&rw)=='F',"ELF header","magic num incorrect");

   //32/64bit
   RvR_error_check(RvR_rw_read_u8(&rw)==1,"ELF header","incompatible architecture, expected 32bit");

   //Endianes
   RvR_error_check(RvR_rw_read_u8(&rw)==1,"ELF header","incompatible endianess, expected little endian");

   //Skip version, OS ABI and 8 padding bytes
   for(int i = 0;i<10;i++)
      RvR_rw_read_u8(&rw);
   
   //Skip relocatetable/executable/shared/core field
   RvR_rw_read_u16(&rw);

   //Instruction set
   RvR_error_check(RvR_rw_read_u16(&rw)==0xf3,"ELF header","incompatible instruction set, expected risc-v");

   //Skip elf version
   RvR_rw_read_u32(&rw);

   //Read actually needed info
   uint32_t entry_pos = RvR_rw_read_u32(&rw); //Program entry position
   uint32_t program_table_pos = RvR_rw_read_u32(&rw);
   uint32_t section_table_pos = RvR_rw_read_u32(&rw);
   uint32_t program_flags = RvR_rw_read_u32(&rw);
   uint16_t header_size = RvR_rw_read_u16(&rw);
   uint16_t program_entry_size = RvR_rw_read_u16(&rw);
   uint16_t program_entry_count = RvR_rw_read_u16(&rw);
   uint16_t section_entry_size = RvR_rw_read_u16(&rw);
   uint16_t section_entry_count = RvR_rw_read_u16(&rw);
   uint16_t section_table_names = RvR_rw_read_u16(&rw);
   //-------------------------------------

   //program header
   Entry *entries = RvR_malloc(sizeof(*entries)*program_entry_count);
   uint32_t mem_size = 0;
   uint32_t entry_count = 0;
   RvR_rw_seek(&rw,program_table_pos,SEEK_SET);
   for(int i = 0;i<program_entry_count;i++)
   {
      entries[i].type = RvR_rw_read_u32(&rw);
      entries[i].p_offset = RvR_rw_read_u32(&rw);
      entries[i].p_vaddr = RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      entries[i].p_filesz = RvR_rw_read_u32(&rw);
      entries[i].p_memsz = RvR_rw_read_u32(&rw);
      entries[i].flags = RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);

      RvR_log("|type    |%16d|\n",entries[i].type);
      RvR_log("|p_offset|%16d|\n",entries[i].p_offset);
      RvR_log("|p_vaddr |%16d|\n",entries[i].p_vaddr);
      RvR_log("|p_filesz|%16d|\n",entries[i].p_filesz);
      RvR_log("|p_memsz |%16d|\n",entries[i].p_memsz);
      RvR_log("|-------------------------|\n",entries[i].p_memsz);

      if(entries[i].type!=1)
         continue;
      
      entry_count++;
      if(entries[i].p_vaddr+entries[i].p_memsz>mem_size)
         mem_size = entries[i].p_vaddr+entries[i].p_memsz;
   }
   //-------------------------------------

   //Write new header
   RvR_rw_write_u32(&rw_out,entry_pos);
   RvR_rw_write_u32(&rw_out,mem_size);
   RvR_rw_write_u32(&rw_out,entry_count);
   uint32_t offset = 12+entry_count*16;
   for(int i = 0;i<program_entry_count;i++)
   {
      if(entries[i].type!=1)
         continue;

      RvR_rw_write_u32(&rw_out,offset);
      RvR_rw_write_u32(&rw_out,entries[i].p_vaddr);
      RvR_rw_write_u32(&rw_out,entries[i].p_filesz);
      RvR_rw_write_u32(&rw_out,entries[i].p_memsz);

      offset+=entries[i].p_filesz;
   }
   //-------------------------------------

   //Write data sections
   for(int i = 0;i<program_entry_count;i++)
   {
      if(entries[i].type!=1)
         continue;

      void *mem = RvR_malloc(entries[i].p_filesz);
      RvR_rw_seek(&rw,entries[i].p_offset,SEEK_SET);
      RvR_rw_read(&rw,mem,1,entries[i].p_filesz);
      RvR_rw_write(&rw_out,mem,1,entries[i].p_filesz);
      RvR_free(mem);
   }
   //-------------------------------------

RvR_err:
   RvR_rw_close(&rw);
   RvR_rw_close(&rw_out);

   return 0;
}

static void print_help(char **argv)
{
   RvR_log("%s usage:\n"
          "%s -fin filename -fout filename ]\n"
          "   -fin        input texture path\n"
          "   -fout       output texture path\n",
         argv[0],argv[0]);
}
//-------------------------------------
