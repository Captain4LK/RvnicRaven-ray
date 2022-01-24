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
   RvR_error_check(rw!=NULL,0x101);
   RvR_error_check(f!=NULL,0x101);

   rw->type = 0;
   rw->endian = RVR_ENDIAN;
   rw->file.fp = f;

   return;

RvR_err:

   RvR_log("RvR error %s\n",RvR_error_get_string());
}

void RvR_rw_init_path(RvR_rw *rw, const char *path, const char *mode)
{
   RvR_error_check(rw!=NULL,0x101);
   RvR_error_check(path!=NULL,0x101);
   RvR_error_check(mode!=NULL,0x101);

   rw->type = 1;
   rw->endian = RVR_ENDIAN;
   rw->file.fp = fopen(path,mode);
   RvR_error_check(rw->file.fp!=NULL,0x006);

   return;

RvR_err:

   RvR_log("RvR error %s\n",RvR_error_get_string());
}

void RvR_rw_init_mem(RvR_rw *rw, void *mem, size_t len)
{
   RvR_error_check(rw!=NULL,0x101);
   RvR_error_check(mem!=NULL,0x101);

   rw->type = 2;
   rw->endian = RVR_ENDIAN;
   rw->file.mem.mem = mem;
   rw->file.mem.size = len;
   rw->file.mem.pos = 0;

   return;

RvR_err:

   RvR_log("RvR error %s\n",RvR_error_get_string());
}

void RvR_rw_init_const_mem(RvR_rw *rw, const void *mem, size_t len)
{
   RvR_error_check(rw!=NULL,0x101);
   RvR_error_check(mem!=NULL,0x101);

   rw->type = 3;
   rw->endian = RVR_ENDIAN;
   rw->file.cmem.mem = mem;
   rw->file.cmem.size = len;
   rw->file.cmem.pos = 0;

   return;

RvR_err:

   RvR_log("RvR error %s\n",RvR_error_get_string());
}

void RvR_rw_endian(RvR_rw *rw, uint8_t endian)
{
   rw->endian = endian;
}

void RvR_rw_close(RvR_rw *rw)
{
   if(rw->type==1)
      RvR_error_check(fclose(rw->file.fp)!=EOF,0x007);

   return;

RvR_err:

   RvR_log("RvR error %s\n",RvR_error_get_string());
}

void RvR_rw_flush(RvR_rw *rw)
{
   if(rw->type==0||rw->type==1)
      fflush(rw->file.fp);
}

int RvR_rw_seek(RvR_rw *rw, long offset, int origin)
{
   if(rw->type==0||rw->type==1)
   {
      return fseek(rw->file.fp,offset,origin);
   }
   else if(rw->type==2)
   {
      if(origin==SEEK_SET)
         rw->file.mem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->file.mem.pos+=offset;
      else if(origin==SEEK_END)
         rw->file.mem.pos = rw->file.mem.size-offset;

      if(rw->file.mem.pos<0)
      {
         rw->file.mem.pos = 0;
         return 1;
      }
   }
   else if(rw->type==3)
   {
      if(origin==SEEK_SET)
         rw->file.cmem.pos = offset;
      else if(origin==SEEK_CUR)
         rw->file.cmem.pos+=offset;
      else if(origin==SEEK_END)
         rw->file.cmem.pos = rw->file.cmem.size-offset;

      if(rw->file.mem.pos<0)
      {
         rw->file.mem.pos = 0;
         return 1;
      }
   }

   RvR_log_line("RvR_rw_seek ", "invalid RvR_rw type, handle might be corrupt\n");
   return 1;
}

long RvR_rw_tell(RvR_rw *rw)
{
   if(rw->type==0||rw->type==1)
   {
      long size = ftell(rw->file.fp);
      RvR_error_check(size!=EOF,0x005);
      return size;
   }
   else if(rw->type==2)
   {
      return rw->file.mem.pos;
   }
   else if(rw->type==3)
   {
      return rw->file.cmem.pos;
   }

   RvR_log_line("RvR_rw_tell", "invalid RvR_rw type, handle might be corrupt\n");

   return EOF;

RvR_err:

   RvR_log("RvR error %s\n",RvR_error_get_string());

   return EOF;
}

int RvR_rw_eof(RvR_rw *rw)
{
   if(rw->type==0||rw->type==1)
      return feof(rw->file.fp);
   else if(rw->type==2)
      return rw->file.mem.pos>=rw->file.mem.size;
   else if(rw->type==3)
      return rw->file.cmem.pos>=rw->file.cmem.size;

   return 1;
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

   RvR_log_line("RvR_rw_read", "invalid RvR_rw type, handle might be corrupt\n");

   return 0;
}

size_t RvR_rw_write(RvR_rw *rw, const void *buffer, size_t size, size_t count)
{
   if(rw->type==0||rw->type==1)
   {
      return fwrite(buffer,size,count,rw->file.fp);
   }
   else if(rw->type==2)
   {
      uint8_t *buff_out = rw->file.mem.mem;
      const uint8_t *buff_in = buffer;

      for(size_t i = 0;i<count;i++)
      {
         if(rw->file.mem.pos+size>rw->file.mem.size)
            return i;

         memcpy(buff_out+rw->file.mem.pos,buff_in+(i*size),size);
         rw->file.mem.pos+=size;
      }

      return count;
   }
   else if(rw->type==3)
   {
      RvR_log_line("RvR_rw_write","writing to const RvR_rw stream is not supported\n");

      return 0;
   }

   RvR_log_line("RvR_rw_write","invalid RvR_rw type, handle might be corrupt\n");

   return 0;
}

int8_t RvR_rw_read_i8(RvR_rw *rw)
{
   int8_t out = 0;
   if(RvR_rw_read(rw,&out,1,1)!=1)
      RvR_log("RvR_rw_read_i8: read failed, end of file reached?\n");
   return out;
}

uint8_t RvR_rw_read_u8(RvR_rw *rw)
{
   uint8_t out = 0;
   if(RvR_rw_read(rw,&out,1,1)!=1)
      RvR_log("RvR_rw_read_u8: read failed, end of file reached?\n");
   return out;
}

int16_t RvR_rw_read_i16(RvR_rw *rw)
{
   int16_t out = 0;
   if(RvR_rw_read(rw,&out,2,1)!=1)
      RvR_log("RvR_rw_read_i16: read failed, end of file reached?\n");
   return RvR_endian_swap16(out,rw->endian);
}

uint16_t RvR_rw_read_u16(RvR_rw *rw)
{
   uint16_t out = 0;
   if(RvR_rw_read(rw,&out,2,1)!=1)
      RvR_log("RvR_rw_read_u16: read failed, end of file reached?\n");
   return RvR_endian_swap16(out,rw->endian);
}

int32_t RvR_rw_read_i32(RvR_rw *rw)
{
   int32_t out = 0;
   if(RvR_rw_read(rw,&out,4,1)!=1)
      RvR_log("RvR_rw_read_i32: read failed, end of file reached?\n");
   return RvR_endian_swap32(out,rw->endian);
}

uint32_t RvR_rw_read_u32(RvR_rw *rw)
{
   uint32_t out = 0;
   if(RvR_rw_read(rw,&out,4,1)!=1)
      RvR_log("RvR_rw_read_u32: read failed, end of file reached?\n");
   return RvR_endian_swap32(out,rw->endian);
}

int64_t RvR_rw_read_i64(RvR_rw *rw)
{
   int64_t out = 0;
   if(RvR_rw_read(rw,&out,8,1)!=1)
      RvR_log("RvR_rw_read_i64: read failed, end of file reached?\n");
   return RvR_endian_swap64(out,rw->endian);
}

uint64_t RvR_rw_read_u64(RvR_rw *rw)
{
   uint64_t out = 0;
   if(RvR_rw_read(rw,&out,8,1)!=1)
      RvR_log("RvR_rw_read_u64: read failed, end of file reached?\n");
   return RvR_endian_swap64(out,rw->endian);
}

void RvR_rw_write_i8 (RvR_rw *rw, int8_t val)
{
   if(RvR_rw_write(rw,&val,1,1)!=1)
      RvR_log("RvR_rw_write_i8: write failed, end of buffer reached/no more disk space?\n");
}

void RvR_rw_write_u8 (RvR_rw *rw, uint8_t val)
{
   if(RvR_rw_write(rw,&val,1,1)!=1)
      RvR_log("RvR_rw_write_u8: write failed, end of buffer reached/no more disk space?\n");
}

void RvR_rw_write_i16(RvR_rw *rw, int16_t val)
{
   int16_t v = RvR_endian_swap16(val,rw->endian);
   if(RvR_rw_write(rw,&v,2,1)!=1)
      RvR_log("RvR_rw_write_i16: write failed, end of buffer reached/no more disk space?\n");
}

void RvR_rw_write_u16(RvR_rw *rw, uint16_t val)
{
   uint16_t v = RvR_endian_swap16(val,rw->endian);
   if(RvR_rw_write(rw,&v,2,1)!=1)
      RvR_log("RvR_rw_write_u16: write failed, end of buffer reached/no more disk space?\n");
}

void RvR_rw_write_i32(RvR_rw *rw, int32_t val)
{
   int32_t v = RvR_endian_swap32(val,rw->endian);
   if(RvR_rw_write(rw,&v,4,1)!=1)
      RvR_log("RvR_rw_write_i32: write failed, end of buffer reached/no more disk space?\n");
}

void RvR_rw_write_u32(RvR_rw *rw, uint32_t val)
{
   uint32_t v = RvR_endian_swap32(val,rw->endian);
   if(RvR_rw_write(rw,&v,4,1)!=1)
      RvR_log("RvR_rw_write_u32: write failed, end of buffer reached/no more disk space?\n");
}

void RvR_rw_write_i64(RvR_rw *rw, int64_t val)
{
   int64_t v = RvR_endian_swap64(val,rw->endian);
   if(RvR_rw_write(rw,&v,8,1)!=1)
      RvR_log("RvR_rw_write_i64: write failed, end of buffer reached/no more disk space?\n");
}

void RvR_rw_write_u64(RvR_rw *rw, uint64_t val)
{
   uint64_t v = RvR_endian_swap64(val,rw->endian);
   if(RvR_rw_write(rw,&v,8,1)!=1)
      RvR_log("RvR_rw_write_u64: write failed, end of buffer reached/no more disk space?\n");
}
//-------------------------------------
