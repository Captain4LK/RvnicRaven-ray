/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
   RvR_error_check(rw!=NULL,"RvR_rw_init_file","argument 'rw' must be non-NULL\n");
   RvR_error_check(f!=NULL,"RvR_rw_init_file","argument 'f' must be non-NULL\n");

   rw->type = RVR_RW_STD_FILE;
   rw->endian = RVR_ENDIAN;
   rw->as.fp = f;

RvR_err:
   return;
}

void RvR_rw_init_path(RvR_rw *rw, const char *path, const char *mode)
{
   RvR_error_check(rw!=NULL,"RvR_rw_init_path","argument 'rw' must be non-NULL\n");
   RvR_error_check(path!=NULL,"RvR_rw_init_path","argument 'path' must be non-NULL\n");
   RvR_error_check(mode!=NULL,"RvR_rw_init_path","argument 'mode' must be non-NULL\n");

   rw->type = RVR_RW_STD_FILE_PATH;
   rw->endian = RVR_ENDIAN;
   rw->as.fp = fopen(path,mode);
   RvR_error_check(rw->as.fp!=NULL,"RvR_rw_init_path","failed to open '%s'\n",path);

RvR_err:
   return;
}

void RvR_rw_init_mem(RvR_rw *rw, void *mem, size_t len)
{
   RvR_error_check(rw!=NULL,"RvR_rw_init_mem","argument 'rw' must be non-NULL\n");
   RvR_error_check(mem!=NULL,"RvR_rw_init_mem","argument 'mem' must be non-NULL\n");

   rw->type = RVR_RW_MEM;
   rw->endian = RVR_ENDIAN;
   rw->as.mem.mem = mem;
   rw->as.mem.size = len;
   rw->as.mem.pos = 0;

RvR_err:
   return;
}

void RvR_rw_init_const_mem(RvR_rw *rw, const void *mem, size_t len)
{
   RvR_error_check(rw!=NULL,"RvR_rw_init_const_mem","argument 'rw' must be non-NULL\n");
   RvR_error_check(mem!=NULL,"RvR_rw_init_const_mem","argument 'mem' must be non-NULL\n");

   rw->type = RVR_RW_CONST_MEM;
   rw->endian = RVR_ENDIAN;
   rw->as.cmem.mem = mem;
   rw->as.cmem.size = len;
   rw->as.cmem.pos = 0;

RvR_err:
   return;
}

void RvR_rw_init_dyn_mem(RvR_rw *rw, size_t base_len, size_t min_grow)
{
   RvR_error_check(rw!=NULL,"RvR_rw_init_dyn_mem","argument 'rw' must be non-NULL\n");

   rw->type = RVR_RW_DYN_MEM;
   rw->endian = RVR_ENDIAN;
   rw->as.dmem.mem = RvR_malloc(base_len);
   rw->as.dmem.size = base_len;
   rw->as.dmem.pos = 0;
   rw->as.dmem.min_grow = min_grow;

RvR_err:
   return;
}

void RvR_rw_endian(RvR_rw *rw, uint8_t endian)
{
   RvR_error_check(rw!=NULL,"RvR_rw_endian","argument 'rw' must be non-NULL\n");

   rw->endian = endian;

RvR_err:
   return;
}

void RvR_rw_close(RvR_rw *rw)
{
   RvR_error_check(rw!=NULL,"RvR_rw_close","argument 'rw' must be non-NULL\n");

   if(rw->type==RVR_RW_STD_FILE_PATH)
      RvR_error_check(fclose(rw->as.fp)!=EOF,"RvR_rw_close","fclose() failed\n");
   else if(rw->type==RVR_RW_DYN_MEM)
      RvR_free(rw->as.dmem.mem);

RvR_err:
   return;
}

void RvR_rw_flush(RvR_rw *rw)
{
   RvR_error_check(rw!=NULL,"RvR_rw_flush","argument 'rw' must be non-NULL\n");

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
      RvR_error_check(fflush(rw->as.fp)==0,"RvR_rw_flush","fflush() failed\n");;

RvR_err:
   return;
}

int RvR_rw_seek(RvR_rw *rw, long offset, int origin)
{
   RvR_error_check(rw!=NULL,"RvR_rw_seek","argument 'rw' must be non-NULL\n");

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
   {
      return fseek(rw->as.fp,offset,origin);
   }
   else if(rw->type==RVR_RW_MEM)
   {
      if(origin==SEEK_SET)
         rw->as.mem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->as.mem.pos+=offset;
      else if(origin==SEEK_END)
         rw->as.mem.pos = rw->as.mem.size-offset;

      if(rw->as.mem.pos<0)
      {
         rw->as.mem.pos = 0;
         return 1;
      }
      return 0;
   }
   else if(rw->type==RVR_RW_CONST_MEM)
   {
      if(origin==SEEK_SET)
         rw->as.cmem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->as.cmem.pos+=offset;
      else if(origin==SEEK_END)
         rw->as.cmem.pos = rw->as.cmem.size-offset;

      if(rw->as.cmem.pos<0)
      {
         rw->as.cmem.pos = 0;
         return 1;
      }

      return 0;
   }
   else if(rw->type==RVR_RW_DYN_MEM)
   {
      if(origin==SEEK_SET)
         rw->as.dmem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->as.dmem.pos+=offset;
      else if(origin==SEEK_END)
         rw->as.dmem.pos = rw->as.dmem.size-offset;

      if(rw->as.dmem.pos<0)
      {
         rw->as.dmem.pos = 0;
         return 1;
      }

      return 0;
   }

   RvR_log_line("RvR_rw_seek ", "invalid RvR_rw type, handle might be corrupt\n");

RvR_err:
   return 1;
}

long RvR_rw_tell(RvR_rw *rw)
{
   RvR_error_check(rw!=NULL,"RvR_rw_tell","argument 'rw' must be non-NULL\n");

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
   {
      long size = ftell(rw->as.fp);
      RvR_error_check(size!=EOF,"RvR_rw_tell","ftell() failed\n");
      return size;
   }
   else if(rw->type==RVR_RW_MEM)
   {
      return rw->as.mem.pos;
   }
   else if(rw->type==RVR_RW_CONST_MEM)
   {
      return rw->as.cmem.pos;
   }
   else if(rw->type==RVR_RW_DYN_MEM)
   {
      return rw->as.dmem.pos;
   }

   RvR_log_line("RvR_rw_tell", "invalid RvR_rw type, handle might be corrupt\n");

RvR_err:
   return EOF;
}

int RvR_rw_eof(RvR_rw *rw)
{
   RvR_error_check(rw!=NULL,"RvR_rw_eof","argument 'rw' must be non-NULL\n");

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
      return feof(rw->as.fp);
   else if(rw->type==RVR_RW_MEM)
      return rw->as.mem.pos>=rw->as.mem.size;
   else if(rw->type==RVR_RW_CONST_MEM)
      return rw->as.cmem.pos>=rw->as.cmem.size;
   else if(rw->type==RVR_RW_DYN_MEM)
      return rw->as.dmem.pos>=rw->as.dmem.size;

   RvR_log_line("RvR_rw_eof", "invalid RvR_rw type, handle might be corrupt\n");

RvR_err:
   return 1;
}

size_t RvR_rw_read(RvR_rw *rw, void *buffer, size_t size, size_t count)
{
   RvR_error_check(rw!=NULL,"RvR_rw_read","argument 'rw' must be non-NULL\n");
   RvR_error_check(buffer!=NULL,"RvR_rw_read","argument 'buffer' must be non-NULL\n");

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
   {
      return fread(buffer,size,count,rw->as.fp);
   }
   else if(rw->type==RVR_RW_MEM)
   {
      uint8_t *buff_out = buffer;
      uint8_t *buff_in = rw->as.mem.mem;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->as.mem.pos+(long)size>rw->as.mem.size)
            return i;

         memcpy(buff_out+(i*size),buff_in+rw->as.mem.pos,size);
         rw->as.mem.pos+=size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_CONST_MEM)
   {
      uint8_t *buff_out = buffer;
      const uint8_t *buff_in = rw->as.cmem.mem;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->as.cmem.pos+(long)size>rw->as.cmem.size)
            return i;

         memcpy(buff_out+(i*size),buff_in+rw->as.cmem.pos,size);
         rw->as.cmem.pos+=size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_DYN_MEM)
   {
      uint8_t *buff_out = buffer;
      const uint8_t *buff_in = rw->as.dmem.mem;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->as.dmem.pos+(long)size>rw->as.dmem.size)
            return i;

         memcpy(buff_out+(i*size),buff_in+rw->as.dmem.pos,size);
         rw->as.dmem.pos+=size;
      }

      return count;
   }

   RvR_log_line("RvR_rw_read", "invalid RvR_rw type, handle might be corrupt\n");

RvR_err:
   return 0;
}

size_t RvR_rw_write(RvR_rw *rw, const void *buffer, size_t size, size_t count)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write","argument 'rw' must be non-NULL\n");
   RvR_error_check(buffer!=NULL,"RvR_rw_write","argument 'buffer' must be non-NULL\n");

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
   {
      return fwrite(buffer,size,count,rw->as.fp);
   }
   else if(rw->type==RVR_RW_MEM)
   {
      uint8_t *buff_out = rw->as.mem.mem;
      const uint8_t *buff_in = buffer;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->as.mem.pos+(long)size>rw->as.mem.size)
            return i;

         memcpy(buff_out+rw->as.mem.pos,buff_in+(i*size),size);
         rw->as.mem.pos+=size;
      }

      return count;
   }
   else if(rw->type==RVR_RW_CONST_MEM)
   {
      RvR_log_line("RvR_rw_write","writing to const RvR_rw stream is not supported\n");

      return 0;
   }
   else if(rw->type==RVR_RW_DYN_MEM)
   {
      uint8_t *buff_out = rw->as.dmem.mem;
      const uint8_t *buff_in = buffer;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->as.dmem.pos+(long)size>rw->as.dmem.size)
         {
            rw->as.dmem.size+=RvR_max(rw->as.dmem.min_grow,rw->as.dmem.pos+(long)size-rw->as.dmem.size);
            rw->as.dmem.mem = RvR_realloc(rw->as.dmem.mem,rw->as.dmem.size);
         }

         memcpy(buff_out+rw->as.dmem.pos,buff_in+(i*size),size);
         rw->as.dmem.pos+=size;
      }

      return count;
   }

   RvR_log_line("RvR_rw_write","invalid RvR_rw type, handle might be corrupt\n");

RvR_err:
   return 0;
}

int8_t RvR_rw_read_i8(RvR_rw *rw)
{
   int8_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_i8","argument 'rw' must be non-NULL\n");

   if(RvR_rw_read(rw,&out,1,1)!=1)
      RvR_log("RvR_rw_read_i8: read failed, end of file reached?\n");

RvR_err:
   return out;
}

uint8_t RvR_rw_read_u8(RvR_rw *rw)
{
   uint8_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_u8","argument 'rw' must be non-NULL\n");

   if(RvR_rw_read(rw,&out,1,1)!=1)
      RvR_log("RvR_rw_read_u8: read failed, end of file reached?\n");

RvR_err:
   return out;
}

int16_t RvR_rw_read_i16(RvR_rw *rw)
{
   int16_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_i16","argument 'rw' must be non-NULL\n");

   if(RvR_rw_read(rw,&out,2,1)!=1)
      RvR_log("RvR_rw_read_i16: read failed, end of file reached?\n");

RvR_err:
   return RvR_endian_swap16(out,rw->endian);
}

uint16_t RvR_rw_read_u16(RvR_rw *rw)
{
   uint16_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_u16","argument 'rw' must be non-NULL\n");

   if(RvR_rw_read(rw,&out,2,1)!=1)
      RvR_log("RvR_rw_read_u16: read failed, end of file reached?\n");

RvR_err:
   return RvR_endian_swap16(out,rw->endian);
}

int32_t RvR_rw_read_i32(RvR_rw *rw)
{
   int32_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_i32","argument 'rw' must be non-NULL\n");

   if(RvR_rw_read(rw,&out,4,1)!=1)
      RvR_log("RvR_rw_read_i32: read failed, end of file reached?\n");

RvR_err:
   return RvR_endian_swap32(out,rw->endian);
}

uint32_t RvR_rw_read_u32(RvR_rw *rw)
{
   uint32_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_u32","argument 'rw' must be non-NULL\n");

   if(RvR_rw_read(rw,&out,4,1)!=1)
      RvR_log("RvR_rw_read_u32: read failed, end of file reached?\n");

RvR_err:
   return RvR_endian_swap32(out,rw->endian);
}

int64_t RvR_rw_read_i64(RvR_rw *rw)
{
   int64_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_i64","argument 'rw' must be non-NULL\n");

   if(RvR_rw_read(rw,&out,8,1)!=1)
      RvR_log("RvR_rw_read_i64: read failed, end of file reached?\n");

RvR_err:
   return RvR_endian_swap64(out,rw->endian);
}

uint64_t RvR_rw_read_u64(RvR_rw *rw)
{
   uint64_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_u64","argument 'rw' must be non-NULL\n");

   if(RvR_rw_read(rw,&out,8,1)!=1)
      RvR_log("RvR_rw_read_u64: read failed, end of file reached?\n");

RvR_err:
   return RvR_endian_swap64(out,rw->endian);
}

void RvR_rw_write_i8 (RvR_rw *rw, int8_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_i8","argument 'rw' must be non-NULL\n");

   if(RvR_rw_write(rw,&val,1,1)!=1)
      RvR_log("RvR_rw_write_i8: write failed, end of buffer reached/no more disk space?\n");

RvR_err:
   return;
}

void RvR_rw_write_u8 (RvR_rw *rw, uint8_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_u8","argument 'rw' must be non-NULL\n");

   if(RvR_rw_write(rw,&val,1,1)!=1)
      RvR_log("RvR_rw_write_u8: write failed, end of buffer reached/no more disk space?\n");

RvR_err:
   return;
}

void RvR_rw_write_i16(RvR_rw *rw, int16_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_i16","argument 'rw' must be non-NULL\n");

   int16_t v = RvR_endian_swap16(val,rw->endian);
   if(RvR_rw_write(rw,&v,2,1)!=1)
      RvR_log("RvR_rw_write_i16: write failed, end of buffer reached/no more disk space?\n");

RvR_err:
   return;
}

void RvR_rw_write_u16(RvR_rw *rw, uint16_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_u16","argument 'rw' must be non-NULL\n");

   uint16_t v = RvR_endian_swap16(val,rw->endian);
   if(RvR_rw_write(rw,&v,2,1)!=1)
      RvR_log("RvR_rw_write_u16: write failed, end of buffer reached/no more disk space?\n");

RvR_err:
   return;
}

void RvR_rw_write_i32(RvR_rw *rw, int32_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_i32","argument 'rw' must be non-NULL\n");

   int32_t v = RvR_endian_swap32(val,rw->endian);
   if(RvR_rw_write(rw,&v,4,1)!=1)
      RvR_log("RvR_rw_write_i32: write failed, end of buffer reached/no more disk space?\n");

RvR_err:
   return;
}

void RvR_rw_write_u32(RvR_rw *rw, uint32_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_u32","argument 'rw' must be non-NULL\n");

   uint32_t v = RvR_endian_swap32(val,rw->endian);
   if(RvR_rw_write(rw,&v,4,1)!=1)
      RvR_log("RvR_rw_write_u32: write failed, end of buffer reached/no more disk space?\n");

RvR_err:
   return;
}

void RvR_rw_write_i64(RvR_rw *rw, int64_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_i64","argument 'rw' must be non-NULL\n");

   int64_t v = RvR_endian_swap64(val,rw->endian);
   if(RvR_rw_write(rw,&v,8,1)!=1)
      RvR_log("RvR_rw_write_i64: write failed, end of buffer reached/no more disk space?\n");

RvR_err:
   return;
}

void RvR_rw_write_u64(RvR_rw *rw, uint64_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_u64","argument 'rw' must be non-NULL\n");

   uint64_t v = RvR_endian_swap64(val,rw->endian);
   if(RvR_rw_write(rw,&v,8,1)!=1)
      RvR_log("RvR_rw_write_u64: write failed, end of buffer reached/no more disk space?\n");

RvR_err:
   return;
}
//-------------------------------------
