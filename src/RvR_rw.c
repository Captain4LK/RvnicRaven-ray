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
   rw->endian = RVR_LITTLE_ENDIAN;
   rw->as.fp = f;

   return;

RvR_err:
   rw->type = RVR_RW_INVALID;
   return;
}

void RvR_rw_init_path(RvR_rw *rw, const char *path, const char *mode)
{
   RvR_error_check(rw!=NULL,"RvR_rw_init_path","argument 'rw' must be non-NULL\n");
   RvR_error_check(path!=NULL,"RvR_rw_init_path","argument 'path' must be non-NULL\n");
   RvR_error_check(mode!=NULL,"RvR_rw_init_path","argument 'mode' must be non-NULL\n");

   rw->type = RVR_RW_STD_FILE_PATH;
   rw->endian = RVR_LITTLE_ENDIAN;
   rw->as.fp = fopen(path,mode);
   RvR_error_check(rw->as.fp!=NULL,"RvR_rw_init_path","failed to open '%s'\n",path);

   return;

RvR_err:
   rw->type = RVR_RW_INVALID;
   return;
}

void RvR_rw_init_mem(RvR_rw *rw, void *mem, size_t len, size_t clen)
{
   RvR_error_check(rw!=NULL,"RvR_rw_init_mem","argument 'rw' must be non-NULL\n");
   RvR_error_check(mem!=NULL,"RvR_rw_init_mem","argument 'mem' must be non-NULL\n");

   rw->type = RVR_RW_MEM;
   rw->endian = RVR_LITTLE_ENDIAN;
   rw->as.mem.mem = mem;
   rw->as.mem.size = len;
   rw->as.mem.pos = 0;
   rw->as.mem.csize = clen;

   return;

RvR_err:
   rw->type = RVR_RW_INVALID;
   return;
}

void RvR_rw_init_const_mem(RvR_rw *rw, const void *mem, size_t len)
{
   RvR_error_check(rw!=NULL,"RvR_rw_init_const_mem","argument 'rw' must be non-NULL\n");
   RvR_error_check(mem!=NULL,"RvR_rw_init_const_mem","argument 'mem' must be non-NULL\n");

   rw->type = RVR_RW_CONST_MEM;
   rw->endian = RVR_LITTLE_ENDIAN;
   rw->as.cmem.mem = mem;
   rw->as.cmem.size = len;
   rw->as.cmem.pos = 0;

   return;

RvR_err:
   rw->type = RVR_RW_INVALID;
   return;
}

void RvR_rw_init_dyn_mem(RvR_rw *rw, size_t base_len, size_t min_grow)
{
   RvR_error_check(rw!=NULL,"RvR_rw_init_dyn_mem","argument 'rw' must be non-NULL\n");

   rw->type = RVR_RW_DYN_MEM;
   rw->endian = RVR_LITTLE_ENDIAN;
   rw->as.dmem.mem = RvR_malloc(base_len);
   rw->as.dmem.size = base_len;
   rw->as.dmem.csize = 0;
   rw->as.dmem.pos = 0;
   rw->as.dmem.min_grow = min_grow;

   return;

RvR_err:
   rw->type = RVR_RW_INVALID;
   return;
}

int RvR_rw_valid(RvR_rw *rw)
{
   return rw->type!=RVR_RW_INVALID;
}

void RvR_rw_endian(RvR_rw *rw, uint8_t endian)
{
   RvR_error_check(rw!=NULL,"RvR_rw_endian","argument 'rw' must be non-NULL\n");
   RvR_error_check(rw->type!=RVR_RW_INVALID,"RvR_rw_endian","rw instance is invalid\n");

   rw->endian = endian;

RvR_err:
   return;
}

void RvR_rw_close(RvR_rw *rw)
{
   RvR_error_check(rw!=NULL,"RvR_rw_close","argument 'rw' must be non-NULL\n");
   RvR_error_check(rw->type!=RVR_RW_INVALID,"RvR_rw_close","rw instance is invalid\n");

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
   RvR_error_check(rw->type!=RVR_RW_INVALID,"RvR_rw_flush","rw instance is invalid\n");

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
      RvR_error_check(fflush(rw->as.fp)==0,"RvR_rw_flush","fflush() failed\n");;

RvR_err:
   return;
}

int RvR_rw_seek(RvR_rw *rw, long offset, int origin)
{
   RvR_error_check(rw!=NULL,"RvR_rw_seek","argument 'rw' must be non-NULL\n");
   RvR_error_check(rw->type!=RVR_RW_INVALID,"RvR_rw_seek","rw instance is invalid\n");

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
         rw->as.mem.pos = rw->as.mem.csize+offset;

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
         rw->as.cmem.pos = rw->as.cmem.size+offset;

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
         rw->as.dmem.pos = rw->as.dmem.csize+offset;

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
   RvR_error_check(rw->type!=RVR_RW_INVALID,"RvR_rw_tell","rw instance is invalid\n");

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
   RvR_error_check(rw->type!=RVR_RW_INVALID,"RvR_rw_eof","rw instance is invalid\n");

   if(rw->type==RVR_RW_STD_FILE||rw->type==RVR_RW_STD_FILE_PATH)
      return feof(rw->as.fp);
   else if(rw->type==RVR_RW_MEM)
      return rw->as.mem.pos>=rw->as.mem.csize;
   else if(rw->type==RVR_RW_CONST_MEM)
      return rw->as.cmem.pos>=rw->as.cmem.size;
   else if(rw->type==RVR_RW_DYN_MEM)
      return rw->as.dmem.pos>=rw->as.dmem.csize;

   RvR_log_line("RvR_rw_eof", "invalid RvR_rw type, handle might be corrupt\n");

RvR_err:
   return 1;
}

size_t RvR_rw_read(RvR_rw *rw, void *buffer, size_t size, size_t count)
{
   RvR_error_check(rw!=NULL,"RvR_rw_read","argument 'rw' must be non-NULL\n");
   RvR_error_check(buffer!=NULL,"RvR_rw_read","argument 'buffer' must be non-NULL\n");
   RvR_error_check(rw->type!=RVR_RW_INVALID,"RvR_rw_read","rw instance is invalid\n");

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
         if(rw->as.mem.pos+(long)size>rw->as.mem.csize)
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
         if(rw->as.dmem.pos+(long)size>rw->as.dmem.csize)
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
   RvR_error_check(rw->type!=RVR_RW_INVALID,"RvR_rw_write","rw instance is invalid\n");

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

         rw->as.mem.csize = RvR_max(rw->as.mem.csize,rw->as.mem.pos);
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

         rw->as.dmem.csize = RvR_max(rw->as.dmem.csize,rw->as.dmem.pos);
         memcpy(buff_out+rw->as.dmem.pos,buff_in+(i*size),size);
         rw->as.dmem.pos+=size;
      }

      return count;
   }

   RvR_log_line("RvR_rw_write","invalid RvR_rw type, handle might be corrupt\n");

RvR_err:
   return 0;
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

uint16_t RvR_rw_read_u16(RvR_rw *rw)
{
   uint16_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_u16","argument 'rw' must be non-NULL\n");

   uint16_t b0 = RvR_rw_read_u8(rw);;
   uint16_t b1 = RvR_rw_read_u8(rw);;

   if(rw->endian==RVR_LITTLE_ENDIAN)
      return (b1<<8)|b0;
   if(rw->endian==RVR_BIG_ENDIAN)
      return (b0<<8)|b1;

RvR_err:
   return 0;
}

uint32_t RvR_rw_read_u32(RvR_rw *rw)
{
   uint32_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_u32","argument 'rw' must be non-NULL\n");

   uint32_t b0 = RvR_rw_read_u8(rw);;
   uint32_t b1 = RvR_rw_read_u8(rw);;
   uint32_t b2 = RvR_rw_read_u8(rw);;
   uint32_t b3 = RvR_rw_read_u8(rw);;

   if(rw->endian==RVR_LITTLE_ENDIAN)
      return (b3<<24)|(b2<<16)|(b1<<8)|b0;
   if(rw->endian==RVR_BIG_ENDIAN)
      return (b0<<24)|(b1<<16)|(b2<<8)|b3;

RvR_err:
   return 0;
}

uint64_t RvR_rw_read_u64(RvR_rw *rw)
{
   uint64_t out = 0;

   RvR_error_check(rw!=NULL,"RvR_rw_read_u64","argument 'rw' must be non-NULL\n");

   uint64_t b0 = RvR_rw_read_u8(rw);;
   uint64_t b1 = RvR_rw_read_u8(rw);;
   uint64_t b2 = RvR_rw_read_u8(rw);;
   uint64_t b3 = RvR_rw_read_u8(rw);;
   uint64_t b4 = RvR_rw_read_u8(rw);;
   uint64_t b5 = RvR_rw_read_u8(rw);;
   uint64_t b6 = RvR_rw_read_u8(rw);;
   uint64_t b7 = RvR_rw_read_u8(rw);;

   if(rw->endian==RVR_LITTLE_ENDIAN)
      return (b7<<56)|(b6<<48)|(b5<<40)|(b4<<32)|(b3<<24)|(b2<<16)|(b1<<8)|b0;
   if(rw->endian==RVR_BIG_ENDIAN)
      return (b0<<56)|(b1<<48)|(b2<<40)|(b3<<32)|(b4<<24)|(b5<<16)|(b6<<8)|(b7);

RvR_err:
   return 0;
}

void RvR_rw_write_u8 (RvR_rw *rw, uint8_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_u8","argument 'rw' must be non-NULL\n");

   if(RvR_rw_write(rw,&val,1,1)!=1)
      RvR_log("RvR_rw_write_u8: write failed, end of buffer reached/no more disk space?\n");

RvR_err:
   return;
}

void RvR_rw_write_u16(RvR_rw *rw, uint16_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_u16","argument 'rw' must be non-NULL\n");

   if(rw->endian==RVR_LITTLE_ENDIAN)
   {
      RvR_rw_write_u8(rw,val&255);
      RvR_rw_write_u8(rw,(val>>8)&255);
   }
   else if(rw->endian==RVR_BIG_ENDIAN)
   {
      RvR_rw_write_u8(rw,(val>>8)&255);
      RvR_rw_write_u8(rw,val&255);
   }

RvR_err:
   return;
}

void RvR_rw_write_u32(RvR_rw *rw, uint32_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_u32","argument 'rw' must be non-NULL\n");

   if(rw->endian==RVR_LITTLE_ENDIAN)
   {
      RvR_rw_write_u8(rw,val&255);
      RvR_rw_write_u8(rw,(val>>8)&255);
      RvR_rw_write_u8(rw,(val>>16)&255);
      RvR_rw_write_u8(rw,(val>>24)&255);
   }
   else if(rw->endian==RVR_BIG_ENDIAN)
   {
      RvR_rw_write_u8(rw,(val>>24)&255);
      RvR_rw_write_u8(rw,(val>>16)&255);
      RvR_rw_write_u8(rw,(val>>8)&255);
      RvR_rw_write_u8(rw,val&255);
   }

RvR_err:
   return;
}

void RvR_rw_write_u64(RvR_rw *rw, uint64_t val)
{
   RvR_error_check(rw!=NULL,"RvR_rw_write_u64","argument 'rw' must be non-NULL\n");

   if(rw->endian==RVR_LITTLE_ENDIAN)
   {
      RvR_rw_write_u8(rw,val&255);
      RvR_rw_write_u8(rw,(val>>8)&255);
      RvR_rw_write_u8(rw,(val>>16)&255);
      RvR_rw_write_u8(rw,(val>>24)&255);
      RvR_rw_write_u8(rw,(val>>32)&255);
      RvR_rw_write_u8(rw,(val>>40)&255);
      RvR_rw_write_u8(rw,(val>>48)&255);
      RvR_rw_write_u8(rw,(val>>56)&255);
   }
   else if(rw->endian==RVR_BIG_ENDIAN)
   {
      RvR_rw_write_u8(rw,(val>>56)&255);
      RvR_rw_write_u8(rw,(val>>48)&255);
      RvR_rw_write_u8(rw,(val>>40)&255);
      RvR_rw_write_u8(rw,(val>>32)&255);
      RvR_rw_write_u8(rw,(val>>24)&255);
      RvR_rw_write_u8(rw,(val>>16)&255);
      RvR_rw_write_u8(rw,(val>>8)&255);
      RvR_rw_write_u8(rw,val&255);
   }

RvR_err:
   return;
}
//-------------------------------------
