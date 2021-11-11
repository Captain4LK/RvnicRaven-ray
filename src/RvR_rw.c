/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
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

void RvR_rw_init_file(RvR_rw *rw, FILE *f)
{
   rw->type = 0;
   rw->file.fp = f;
}

void RvR_rw_init_path(RvR_rw *rw, const char *path, const char *mode)
{
   rw->type = 1;
   rw->file.fp = fopen(path,mode);
}

void RvR_rw_init_mem(RvR_rw *rw, void *mem, size_t len)
{
   rw->type = 2;
   rw->file.mem.mem = mem;
   rw->file.mem.size = len;
   rw->file.mem.pos = 0;
}

void RvR_rw_init_const_mem(RvR_rw *rw, const void *mem, size_t len)
{
   rw->type = 3;
   rw->file.cmem.mem = mem;
   rw->file.cmem.size = len;
   rw->file.cmem.pos = 0;
}

void RvR_rw_close(RvR_rw *rw)
{
   if(rw->type==1)
      fclose(rw->file.fp);
}

void RvR_rw_flush(RvR_rw *rw)
{
   if(rw->type==0||rw->type==1)
      fflush(rw->file.fp);
}

void RvR_rw_seek(RvR_rw *rw, long offset, int origin)
{
   if(rw->type==0||rw->type==1)
   {
      fseek(rw->file.fp,offset,origin);
   }
   else if(rw->type==2)
   {
      if(origin==SEEK_SET)
         rw->file.mem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->file.mem.pos+=offset;
      else if(origin==SEEK_END)
         rw->file.mem.pos = rw->file.mem.size-offset;
   }
   else if(rw->type==3)
   {
      if(origin==SEEK_SET)
         rw->file.cmem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->file.cmem.pos+=offset;
      else if(origin==SEEK_END)
         rw->file.cmem.pos = rw->file.cmem.size-offset;
   }
}

long RvR_rw_tell(RvR_rw *rw)
{
   if(rw->type==0||rw->type==1)
   {
      return ftell(rw->file.fp);
   }
   else if(rw->type==2)
   {
      return rw->file.mem.pos;
   }
   else if(rw->type==3)
   {
      return rw->file.cmem.pos;
   }

   return -1;
}

size_t RvR_rw_read(RvR_rw *rw, void *buffer, size_t size, size_t count)
{
   if(rw->type==0||rw->type==1)
   {
      return fread(buffer,size,count,rw->file.fp);
   }
   else if(rw->type==2)
   {
      uint8_t *buff_out = buffer;
      uint8_t *buff_in = rw->file.mem.mem;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->file.mem.pos+size>rw->file.mem.size)
            return i;

         memcpy(buff_out+(i*size),buff_in+rw->file.mem.pos,size);
         rw->file.mem.pos+=size;
      }

      return count;
   }
   else if(rw->type==3)
   {
      uint8_t *buff_out = buffer;
      const uint8_t *buff_in = rw->file.cmem.mem;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->file.cmem.pos+size>rw->file.cmem.size)
            return i;

         memcpy(buff_out+(i*size),buff_in+rw->file.cmem.pos,size);
         rw->file.cmem.pos+=size;
      }

      return count;
   }

   return 0;
}

int8_t RvR_rw_read_i8(RvR_rw *rw)
{
   int8_t out = 0;
   if(RvR_rw_read(rw,&out,1,1)!=1)
      RvR_log("RvR_rw_read_8: read failed, end of file reached?\n");
   return out;
}

uint8_t RvR_rw_read_u8(RvR_rw *rw)
{
   uint8_t out = 0;
   if(RvR_rw_read(rw,&out,1,1)!=1)
      RvR_log("RvR_rw_read_u8: read failed, end of file reached?\n");
   return out;
}

//TODO: endiannes byte swapping in all functions > 8
int16_t RvR_rw_read_i16(RvR_rw *rw)
{
   int16_t out = 0;
   if(RvR_rw_read(rw,&out,2,1)!=1)
      RvR_log("RvR_rw_read_16: read failed, end of file reached?\n");
   return out;
}

uint16_t RvR_rw_read_u16(RvR_rw *rw)
{
   uint16_t out = 0;
   if(RvR_rw_read(rw,&out,2,1)!=1)
      RvR_log("RvR_rw_read_u16: read failed, end of file reached?\n");
   return out;
}

int32_t RvR_rw_read_i32(RvR_rw *rw)
{
   int32_t out = 0;
   if(RvR_rw_read(rw,&out,4,1)!=1)
      RvR_log("RvR_rw_read_32: read failed, end of file reached?\n");
   return out;
}

uint32_t RvR_rw_read_u32(RvR_rw *rw)
{
   uint32_t out = 0;
   if(RvR_rw_read(rw,&out,4,1)!=1)
      RvR_log("RvR_rw_read_u32: read failed, end of file reached?\n");
   return out;
}

int64_t RvR_rw_read_i64(RvR_rw *rw)
{
   int64_t out = 0;
   if(RvR_rw_read(rw,&out,8,1)!=1)
      RvR_log("RvR_rw_read_64: read failed, end of file reached?\n");
   return out;
}

uint64_t RvR_rw_read_u64(RvR_rw *rw)
{
   uint64_t out = 0;
   if(RvR_rw_read(rw,&out,8,1)!=1)
      RvR_log("RvR_rw_read_u64: read failed, end of file reached?\n");
   return out;
}
//-------------------------------------
