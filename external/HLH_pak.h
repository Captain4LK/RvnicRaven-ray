/*
Corax pak format

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//Based on pak.c by rlyeh:
// pak file reading/writing/appending.
// - rlyeh, public domain.
//
// ## PAK
// - [ref] https://quakewiki.org/wiki/.pak (public domain).
// - Header: 12 bytes
//   - "PACK"           4-byte
//   - directory offset uint32
//   - directory size   uint32 (number of files by dividing this by 64, which is sizeof(pak_entry))
//
// - File Directory Entry (Num files * 64 bytes)
//   - Each Directory Entry: 64 bytes
//     - file name     56-byte null-terminated string. Includes path. Example: "maps/e1m1.bsp".
//     - file offset   uint32 from beginning of pak file.
//     - file size     uint32

#ifndef _HLH_PAK_H_
#define _HLH_PAK_H_

typedef struct HLH_pak HLH_pak;

HLH_pak *HLH_pak_open(const char *fname, const char *mode /*a,r,w*/);

//(w)rite or (a)ppend modes only
int      HLH_pak_append_file(HLH_pak*, const char *filename, uint32_t type, FILE *in);
int      HLH_pak_append_data(HLH_pak*, const char *filename, uint32_t type, const void *in, unsigned inlen);
    
//(r)ead only mode
int      HLH_pak_find(HLH_pak *p,const char *fname); // return <0 if error; index otherwise.
unsigned HLH_pak_count(HLH_pak *p);
unsigned HLH_pak_size(HLH_pak *p,unsigned index);
unsigned HLH_pak_offset(HLH_pak *p, unsigned index);
char    *HLH_pak_name(HLH_pak *p,unsigned index);
unsigned HLH_pak_type(HLH_pak *p,unsigned index);
void    *HLH_pak_extract(HLH_pak *p, unsigned index); // must free() after use

void     HLH_pak_close(HLH_pak *p);

#endif

// ---

#ifdef HLH_PAK_IMPLEMENTATION
#ifndef HLH_PAK_IMPLEMENTATION_ONCE
#define HLH_PAK_IMPLEMENTATION_ONCE

#ifndef HLH_PAK_MALLOC
#define HLH_PAK_MALLOC malloc
#endif

#ifndef HLH_PAK_FREE
#define HLH_PAK_FREE free
#endif

#ifndef HLH_PAK_REALLOC
#define HLH_PAK_REALLOC realloc
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef ERR
#define ERR(NUM, ...) (fprintf(stderr, "" __VA_ARGS__), fprintf(stderr, "(%s:%d)\n", __FILE__, __LINE__), fflush(stderr), (NUM)) // (NUM)
#endif

static inline uint32_t pak_swap32( uint32_t t ) { return (t >> 24) | (t << 24) | ((t >> 8) & 0xff00) | ((t & 0xff00) << 8); }

#if defined(_M_IX86) || defined(_M_X64) // #ifdef LITTLE
#define htob32(x) pak_swap32(x)
#define btoh32(x) pak_swap32(x)
#define htol32(x) (x)
#define ltoh32(x) (x)
#else
#define htob32(x) (x)
#define btoh32(x) (x)
#define htol32(x) pak_swap32(x)
#define ltoh32(x) pak_swap32(x)
#endif

#pragma pack(push,1)

typedef struct HLH_pak_header
{
   char id[4];
   uint32_t offset;
   uint32_t size;
}HLH_pak_header;

typedef struct pak_file
{
   char name[52];
   uint32_t type;
   uint32_t offset;
   uint32_t size;
}HLH_pak_file;

#pragma pack(pop)

typedef int static_assert_sizeof_pak_header[sizeof(HLH_pak_header)==12];
typedef int static_assert_sizeof_pak_file[sizeof(HLH_pak_file)==64];

typedef struct HLH_pak
{
    FILE *in, *out;
    int dummy;
    HLH_pak_file *entries;
    unsigned count;
}HLH_pak;

HLH_pak *HLH_pak_open(const char *fname, const char *mode)
{
   struct stat buffer;
   int exists = (stat(fname,&buffer)==0);
   if(mode[0]=='a'&&!exists) 
      mode = "wb";

   if(mode[0]!='w'&&mode[0]!='r'&& mode[0]!='a')
      return NULL;

   FILE *fp = fopen(fname,mode[0] =='w'?"wb":mode[0]=='r'?"rb":"r+b");
   if(!fp)
      return ERR(NULL,"cant open file '%s' in '%s' mode",fname,mode);

   HLH_pak *p = HLH_PAK_MALLOC(sizeof(*p)), zero = {0};
   if(!p)
      return fclose(fp), ERR(NULL,"out of mem");
   *p = zero;

   if(mode[0]=='r'||mode[0]=='a')
   {
      HLH_pak_header header = {0};

      if(fread(&header,1,sizeof(header),fp)!=sizeof(header))
      {
         return fclose(fp), ERR(NULL,"read error");
      }

      if(memcmp(header.id,"PACK",4))
      {
         return fclose(fp), ERR(NULL,"not a .pak file");
      }

      header.offset = ltoh32(header.offset);
      header.size = ltoh32(header.size);

      unsigned num_files = header.size/sizeof(HLH_pak_file);

      if(fseek(fp,header.offset,SEEK_SET)!=0)
      {
         return fclose(fp), ERR(NULL,"read error");
      }

      p->count = num_files;
      p->entries = HLH_PAK_MALLOC(num_files*sizeof(HLH_pak_file));

      if(fread(p->entries,num_files,sizeof(HLH_pak_file),fp)!=sizeof(HLH_pak_file))
      {
         goto fail;
      }

      for(unsigned i = 0;i<num_files;++i)
      {
         HLH_pak_file *e = &p->entries[i];
         e->offset = ltoh32(e->offset);
         e->size = ltoh32(e->size);
         e->type = ltoh32(e->type);
      }

      if(mode[0]=='a')
      {
         // resize (by truncation)
         size_t resize = header.offset;
         int fd = fileno(fp);
         if( fd != -1 )
         {
#ifdef _WIN32
            _chsize_s(fd,resize);
#else
            ftruncate(fd,(off_t)resize);
#endif
            fflush(fp);
            fseek(fp,0L,SEEK_END);
         }

         p->out = fp;
         p->in = NULL;
      }
      else
      {
         p->in = fp;
      }

      return p;
   }


   if(mode[0]=='w')
   {
      p->out = fp;

      // write temporary header
      char header[12] = {0};
      if( fwrite(header,1,12,p->out)!=12)
         goto fail;

      return p;
   }

   fail:;
   if(fp)
      fclose(fp);
   if(p->entries)
      HLH_PAK_FREE(p->entries);
   if(p)
      HLH_PAK_FREE(p);

   return NULL;
}

int HLH_pak_append_data(HLH_pak *p, const char *filename, uint32_t type, const void *in, unsigned inlen)
{
   if(!p->out)
      return ERR(0,"read-only pak file");

   // index meta
   unsigned index = p->count++;
   p->entries = HLH_PAK_REALLOC(p->entries,p->count*sizeof(HLH_pak_file));
   HLH_pak_file *e = &p->entries[index], zero = {0};
   *e = zero;
   snprintf(e->name,51,"%s",filename); // @todo: verify 56 chars limit
   e->size = inlen;
   e->offset = ftell(p->out);
   e->type = type;

   // write blob
   fwrite(in,1,inlen,p->out);

   return !ferror(p->out);
}

int HLH_pak_append_file(HLH_pak *p, const char *filename, uint32_t type, FILE *in)
{
   //index meta
   unsigned index = p->count++;
   p->entries = HLH_PAK_REALLOC(p->entries,p->count*sizeof(HLH_pak_file));
   HLH_pak_file *e = &p->entries[index], zero = {0};
   *e = zero;
   snprintf(e->name,51,"%s",filename); //@todo: verify 56 chars limit
   e->offset = ftell(p->out);
   e->type = type;

   char buf[1<<15];
   while(!feof(in)&&!ferror(in))
   {
      size_t bytes = fread(buf,1,sizeof(buf),in);
      fwrite(buf,1,bytes,p->out);
   }

   e->size = ftell(p->out)-e->offset;

   return !ferror(p->out);
}

void HLH_pak_close(HLH_pak *p)
{
   if(p->out)
   {
      // write toc
      uint32_t seek = 0+12, dirpos = (uint32_t)ftell(p->out), dirlen = p->count*64;
      for(unsigned i = 0;i<p->count;++i)
      {
         HLH_pak_file *e = &p->entries[i];
         // write name (truncated if needed), and trailing zeros
         char zero[52] = {0};
         int namelen = strlen(e->name);
         fwrite(e->name,1,namelen>=52?51:namelen,p->out);
         fwrite(zero,1,namelen>=52?1:52-namelen,p->out);
         // write offset + length pair
         uint32_t ptype = htol32(e->type); fwrite(&ptype,1,4,p->out);
         uint32_t pseek = htol32(seek);    fwrite(&pseek,1,4,p->out);
         uint32_t psize = htol32(e->size); fwrite(&psize,1,4,p->out);
         seek+=e->size;
      }

      // patch header
      fseek(p->out,0L,SEEK_SET);
      fwrite("PACK",1,4,p->out);
      dirpos = htol32(dirpos); fwrite(&dirpos,1,4,p->out);
      dirlen = htol32(dirlen); fwrite(&dirlen,1,4,p->out);
   }

   // close streams
   if(p->in)
      fclose(p->in);
   if(p->out)
      fclose(p->out);

   // clean up
   //What? Why?
   //for(unsigned i = 0;i<p->count;++i)
   //{
      //HLH_pak_file *e = &p->entries[i];
   //}

   HLH_PAK_FREE(p->entries);

   // delete
   HLH_pak zero = {0};
   *p = zero;
   HLH_PAK_FREE(p);
}

int HLH_pak_find(HLH_pak *p, const char *filename)
{
   if(p->in)
   {
      for(int i = p->count;--i>=0;)
      {
         if(!strcmp(p->entries[i].name,filename))
            return i;
      }
   }

   return -1;
}

unsigned HLH_pak_count(HLH_pak *p)
{
   return p->in?p->count:0;
}

unsigned HLH_pak_offset(HLH_pak *p, unsigned index)
{
    return p->in&&index<p->count?p->entries[index].offset:0;
}

unsigned HLH_pak_size(HLH_pak *p, unsigned index)
{
    return p->in&&index<p->count?p->entries[index].size:0;
}

char *HLH_pak_name(HLH_pak *p, unsigned index)
{
    return p->in&&index<p->count?p->entries[index].name:NULL;
}

unsigned HLH_pak_type(HLH_pak *p,unsigned index)
{
    return p->in&&index<p->count?p->entries[index].type:0;
}

void *HLH_pak_extract(HLH_pak *p, unsigned index)
{
   if(p->in&&index<p->count )
   {
      HLH_pak_file *e = &p->entries[index];
      if(fseek(p->in,e->offset,SEEK_SET)!=0)
      {
         return ERR(NULL,"cant seek");
      }
      void *buffer = HLH_PAK_MALLOC(e->size);
      if(!buffer)
      {
         return ERR(NULL,"out of mem");
      }
      if(fread(buffer,1,e->size,p->in)!=e->size)
      {
         HLH_PAK_FREE(buffer);
         return ERR(NULL,"cant read");
      }
      return buffer;
   }
   return NULL;
}

#endif
#endif
