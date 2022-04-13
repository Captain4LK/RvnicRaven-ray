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

enum
{
   COMP_W_BITS = 21, //Window size [17,23]
   COMP_W_SIZE = 1<<COMP_W_BITS,
   COMP_W_MASK = COMP_W_SIZE-1,
   COMP_SLOT_BITS = 4,
   COMP_NUM_SLOTS = 1<<COMP_SLOT_BITS,

   COMP_A_BITS = 2, //1 xx
   COMP_B_BITS = 2, //01 xx
   COMP_C_BITS = 2, //001 xx
   COMP_D_BITS = 3, //0001 xxx
   COMP_E_BITS = 5, //00001 xxxxx
   COMP_F_BITS = 9, //00000 xxxxxxxxx
   COMP_A = 1<<COMP_A_BITS,
   COMP_B = (1<<COMP_B_BITS)+COMP_A,
   COMP_C = (1<<COMP_C_BITS)+COMP_B,
   COMP_D = (1<<COMP_D_BITS)+COMP_C,
   COMP_E = (1<<COMP_E_BITS)+COMP_D,
   COMP_F = (1<<COMP_F_BITS)+COMP_E,
   COMP_MIN_MATCH = 3,
   COMP_MAX_MATCH = (COMP_F-1)+COMP_MIN_MATCH,

   COMP_TOO_FAR = 1<<16,

   COMP_HASH1_LEN = COMP_MIN_MATCH,
   COMP_HASH2_LEN = COMP_MIN_MATCH+1,
   COMP_HASH1_BITS = 21,
   COMP_HASH2_BITS = 24,
   COMP_HASH1_SIZE = 1<<COMP_HASH1_BITS,
   COMP_HASH2_SIZE = 1<<COMP_HASH2_BITS,
   COMP_HASH1_MASK = COMP_HASH1_SIZE-1,
   COMP_HASH2_MASK = COMP_HASH2_SIZE-1,
   COMP_HASH1_SHIFT = (COMP_HASH1_BITS+COMP_HASH1_LEN-1)/COMP_HASH1_LEN,
   COMP_HASH2_SHIFT = (COMP_HASH2_BITS+COMP_HASH2_LEN-1)/COMP_HASH2_LEN,
};

typedef struct
{
   RvR_rw *inbuf;
   RvR_rw *outbuf;
   unsigned bit_buf;
   unsigned bit_count;
}comp_bits;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void     comp_crush_compress(const uint8_t *buf, size_t size, RvR_rw *outbuf, size_t level);
static void     comp_crush_decompress(RvR_rw *inbuf, uint8_t *outbuf, uint32_t outlen);
static int      comp_update_hash1(int h, int c);
static int      comp_update_hash2(int h, int c);
static int      comp_get_penalty(int a, int b);
static void     comp_bits_init(comp_bits *b, RvR_rw *inbuf, RvR_rw *outbuf);
static void     comp_bits_put(comp_bits *b, unsigned n, unsigned x);
static void     comp_bits_flush(comp_bits *b);
static unsigned comp_bits_get(comp_bits *b, unsigned n);
//-------------------------------------

//Function implementations

void RvR_compress(RvR_rw *in, RvR_rw *out, unsigned level)
{
   uint8_t *buffer_in = NULL;
   int32_t size = 0;
   uint8_t endian = RVR_ENDIAN;

   RvR_rw_seek(in,0,SEEK_END);
   size = RvR_rw_tell(in);
   RvR_rw_seek(in,0,SEEK_SET);

   buffer_in = RvR_malloc(size+1);

   RvR_rw_read(in,buffer_in,size,1);
   buffer_in[size] = 0;

   RvR_rw_seek(out,0,SEEK_END);
   RvR_rw_write_i32(out,size);
   RvR_rw_write_u8(out,endian);
   comp_crush_compress(buffer_in,size,out,level);

   RvR_free(buffer_in);
}

void *RvR_decompress(RvR_rw *in, int32_t *length, uint8_t *endian)
{
   RvR_rw_seek(in,0,SEEK_SET);
   *length = RvR_rw_read_i32(in);
   *endian = RvR_rw_read_u8(in);
   *length = RvR_endian_swap32(*length,*endian);

   uint8_t *buffer_out = RvR_malloc((*length)+1);
   comp_crush_decompress(in,buffer_out,*length);
   buffer_out[*length] = 0;

   return buffer_out;
}

//crush_compress, crush_decompress, update_hash1, update_hash2, bits_init, get_penalty, bits_put, bits_get
//by r-lyeh(https://github.com/r-lyeh), stdpack.c (https://github.com/r-lyeh/stdpack.c/blob/master/src/crush.c)
//Original license info:
// crush.cpp
// Written and placed in the public domain by Ilya Muravyov
// Additional code by @r-lyeh (public domain). @todo: honor unused args inlen/outlen

static void comp_crush_compress(const uint8_t *buf, size_t size, RvR_rw *outbuf, size_t level)
{
   static int head[COMP_HASH1_SIZE+COMP_HASH2_SIZE];
   static int prev[COMP_W_SIZE];

   const int max_chain[11] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 1<<12}; //[0fastest..10uber]
   const size_t max_level = sizeof(max_chain)/sizeof(max_chain[0]);
   level = level > max_level ? max_level : level;

   comp_bits bits;

   for(int i = 0;i<COMP_HASH1_SIZE+COMP_HASH2_SIZE;++i)
      head[i]=-1;

   int h1 = 0;
   int h2 = 0;
   for(int i = 0;i<COMP_HASH1_LEN;++i)
      h1 = comp_update_hash1(h1, buf[i]);
   for(int i = 0;i<COMP_HASH2_LEN;++i)
      h2 = comp_update_hash2(h2,buf[i]);

   comp_bits_init(&bits, NULL, outbuf);

   size_t p=0;
   while (p<size)
   {
      int len=COMP_MIN_MATCH-1;
      int offset=COMP_W_SIZE;

      const int max_match=RvR_min((int)COMP_MAX_MATCH, (int)size-p);
      const int limit=RvR_max((int)p-COMP_W_SIZE, 0);

      if(head[h1]>=limit)
      {
         int s = head[h1];
         if(buf[s]==buf[p])
         {
            int l=0;
            while(++l<max_match)
               if(buf[s+l]!=buf[p+l])
                  break;
            if(l>len)
            {
               len = l;
               offset = p-s;
            }
         }
      }

      if(len<COMP_MAX_MATCH)
      {
         int chain_len = max_chain[level];
         int s = head[h2+COMP_HASH1_SIZE];

         while((chain_len--!=0)&&(s>=limit))
         {
            if((buf[s+len]==buf[p+len])&&(buf[s]==buf[p]))
            {
               int l=0;
               while(++l<max_match)
                  if(buf[s+l]!=buf[p+l])
                     break;
               if(l>len+comp_get_penalty((p-s)>>4,offset))
               {
                  len=l;
                  offset=p-s;
               }
               if(l==max_match)
                  break;
            }
            s = prev[s&COMP_W_MASK];
         }
      }

      if((len==COMP_MIN_MATCH)&&(offset>COMP_TOO_FAR))
         len = 0;

      if((level>=2)&&(len>=COMP_MIN_MATCH)&&(len<max_match))
      {
         const int next_p = p+1;
         const int max_lazy = RvR_min((int)len+4, (int)max_match);

         int chain_len = max_chain[level];
         int s = head[comp_update_hash2(h2, buf[next_p+(COMP_HASH2_LEN-1)])+COMP_HASH1_SIZE];

         while((chain_len--!=0)&&(s>=limit))
         {
            if((buf[s+len]==buf[next_p+len])&&(buf[s]==buf[next_p]))
            {
               int l=0;
               while(++l<max_lazy)
                  if(buf[s+l]!=buf[next_p+l])
                     break;
               if(l>len+comp_get_penalty(next_p-s, offset))
               {
                  len=0;
                  break;
               }
               if(l==max_lazy)
                  break;
            }
            s = prev[s&COMP_W_MASK];
         }
      }

      if(len>=COMP_MIN_MATCH) // Match
      {
         comp_bits_put(&bits, 1, 1);

         const int l=len-COMP_MIN_MATCH;
         if(l<COMP_A)
         {
            comp_bits_put(&bits, 1, 1); // 1
            comp_bits_put(&bits, COMP_A_BITS, l);
         }
         else if(l<COMP_B)
         {
            comp_bits_put(&bits, 2, 1<<1); // 01
            comp_bits_put(&bits, COMP_B_BITS, l-COMP_A);
         }
         else if(l<COMP_C)
         {
            comp_bits_put(&bits, 3, 1<<2); // 001
            comp_bits_put(&bits, COMP_C_BITS, l-COMP_B);
         }
         else if(l<COMP_D)
         {
            comp_bits_put(&bits, 4, 1<<3); // 0001
            comp_bits_put(&bits, COMP_D_BITS, l-COMP_C);
         }
         else if(l<COMP_E)
         {
            comp_bits_put(&bits, 5, 1<<4); // 00001
            comp_bits_put(&bits, COMP_E_BITS, l-COMP_D);
         }
         else
         {
            comp_bits_put(&bits, 5, 0); // 00000
            comp_bits_put(&bits, COMP_F_BITS, l-COMP_E);
         }

         --offset;
         int log=COMP_W_BITS-COMP_NUM_SLOTS;
         while(offset>=(2<<log))
            ++log;
         comp_bits_put(&bits, COMP_SLOT_BITS, log-(COMP_W_BITS-COMP_NUM_SLOTS));
         if(log>(COMP_W_BITS-COMP_NUM_SLOTS))
            comp_bits_put(&bits, log, offset-(1<<log));
         else
            comp_bits_put(&bits, COMP_W_BITS-(COMP_NUM_SLOTS-1),offset);
      }
      else //Literal
      {
         len = 1;
         comp_bits_put(&bits,9,buf[p]<<1); //0 xxxxxxxx
      }

      while(len--!=0) //Insert new strings
      {
         head[h1] = p;
         prev[p&COMP_W_MASK] = head[h2+COMP_HASH1_SIZE];
         head[h2+COMP_HASH1_SIZE] = p;
         p++;
         h1 = comp_update_hash1(h1,buf[p+(COMP_HASH1_LEN-1)]);
         h2 = comp_update_hash2(h2,buf[p+(COMP_HASH2_LEN-1)]);
      }
   }

   comp_bits_flush(&bits);
}

static void comp_crush_decompress(RvR_rw *inbuf, uint8_t *outbuf, uint32_t outlen)
{
   unsigned p = 0;
   int s = 0;
   comp_bits bits;
   comp_bits_init(&bits,inbuf,NULL);

   while(p<outlen)
   {
      if(comp_bits_get(&bits,1))
      {
         unsigned len;
         if(comp_bits_get(&bits,1))      len = comp_bits_get(&bits,COMP_A_BITS);
         else if(comp_bits_get(&bits,1)) len = comp_bits_get(&bits,COMP_B_BITS)+COMP_A;
         else if(comp_bits_get(&bits,1)) len = comp_bits_get(&bits,COMP_C_BITS)+COMP_B;
         else if(comp_bits_get(&bits,1)) len = comp_bits_get(&bits,COMP_D_BITS)+COMP_C;
         else if(comp_bits_get(&bits,1)) len = comp_bits_get(&bits,COMP_E_BITS)+COMP_D;
         else                            len = comp_bits_get(&bits,COMP_F_BITS)+COMP_E;

         unsigned log = comp_bits_get(&bits,COMP_SLOT_BITS)+(COMP_W_BITS-COMP_NUM_SLOTS);
         if(log>COMP_W_BITS-COMP_NUM_SLOTS)
            s = comp_bits_get(&bits,log)+(1<<log);
         else
            s = comp_bits_get(&bits,COMP_W_BITS-COMP_NUM_SLOTS+1);
         s = ~s+p;

         RvR_error_check(s>=0&&s+len+3<=outlen,"RvR_decompress","corrupted stream (s=%d p=%d): s out of bounds\n",s,p);
         RvR_error_check(p+len+3<=outlen,"RvR_decompress","corrupted stream (s=%d p=%d): longer than specified by length\n",s,p);

         outbuf[p++] = outbuf[s++];
         outbuf[p++] = outbuf[s++];
         outbuf[p++] = outbuf[s++];
         while(len--!=0)
            outbuf[p++] = outbuf[s++];
      }
      else
      {
         outbuf[p++] = comp_bits_get(&bits,8);
         RvR_error_check(p<=outlen,"RvR_decompress","corrupted stream (s=%d p=%d): longer than specified by length\n",s,p);
      }
   }

RvR_err:
   return;
}

static int comp_update_hash1(int h, int c)
{
   return ((h<<COMP_HASH1_SHIFT)+c)&COMP_HASH1_MASK;
}

static int comp_update_hash2(int h, int c)
{
   return ((h<<COMP_HASH2_SHIFT)+c)&COMP_HASH2_MASK;
}

static int comp_get_penalty(int a, int b)
{
   int p = 0;
   while(a>b)
   {
      a>>=3;
      p++;
   }

   return p;
}

static void comp_bits_init(comp_bits *b, RvR_rw *inbuf, RvR_rw *outbuf)
{
   b->bit_count = b->bit_buf = 0;
   b->inbuf = inbuf;
   b->outbuf = outbuf;
}

//Write n bits of value x to stream
static void comp_bits_put(comp_bits *b, unsigned n, unsigned x)
{
   b->bit_buf|=x<<b->bit_count;
   b->bit_count+=n;

   //Write filled bytes to output stream
   while(b->bit_count>=8)
   {
      RvR_rw_write_u8(b->outbuf,b->bit_buf);
      b->bit_buf>>=8;
      b->bit_count-=8;
   }
}

//Forces bit buffer flush
static void comp_bits_flush(comp_bits *b)
{
   comp_bits_put(b,7,0);
   b->bit_count = b->bit_buf = 0;
}

//Read n bits from input stream
static unsigned comp_bits_get(comp_bits *b, unsigned n)
{
   //Fill bit buffer from input stream
   while(b->bit_count<n)
   {
      b->bit_buf|=RvR_rw_read_u8(b->inbuf)<<b->bit_count;
      b->bit_count+=8;
   }

   unsigned x = b->bit_buf&((1<<n)-1);
   b->bit_buf>>=n;
   b->bit_count-=n;

   return x;
}
//-------------------------------------
