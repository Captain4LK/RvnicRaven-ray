/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
//-------------------------------------

//#defines
#define CUTE_PATH_MAX_PATH 256
#define CUTE_PATH_MAX_EXT 32
//-------------------------------------

//Typedefs

typedef struct
{
   char name[12];
   uint32_t offset;
   uint32_t size;
}Pak_file;

typedef struct
{
    RvR_rw *in, *out;
    Pak_file *entries;
    unsigned count;
}Pak;

typedef struct
{
   char name[8];
   uint32_t path;
}Pak_lump;

typedef struct Pak_buffer
{
   char path[256];
   Pak *pak;
   struct Pak_buffer *next;
}Pak_buffer;
//-------------------------------------

//Variables
static struct
{
   uint32_t used;
   uint32_t size;
   Pak_lump *data;
}pak_lumps[256] = {0};

static struct
{
   uint32_t used;
   uint32_t size;
   char (*data)[256];
}pak_paths;

static Pak_buffer *pak_buffer = NULL;
//-------------------------------------

//Function prototypes
static void     pak_lumps_push(Pak_lump l);
static uint32_t pak_paths_push(const char *path);
static void     pak_add_csv(const char *path);
static void     pak_add_pak(const char *path);
static uint8_t  pak_xor_string(const char *str, size_t len);

//cute_path
static int pak_path_pop_ext(const char *path, char *out, char *ext);
static int pak_path_pop(const char *path, char *out, char *pop);
static int pak_path_is_slash(char c);

//stdarc.c
static unsigned pak_size(Pak *p, unsigned index);
static unsigned pak_count(Pak *p);
static char    *pak_name(Pak *p, unsigned index);
static void     pak_close(Pak *p);
static void    *pak_extract(Pak *p, unsigned index);
static int      pak_find(Pak *p, const char *filename);
static Pak     *pak_open(const char *fname, const char *mode);
static void     pak_append_file(Pak *p, const char *filename, FILE *in);
//-------------------------------------

//Function implementations

void RvR_pak_add(const char *path)
{
   RvR_error_check(path!=NULL,"RvR_pak_add","argument 'path' must be non-NULL\n");

   char ext[CUTE_PATH_MAX_EXT] = "";
   pak_path_pop_ext(path,NULL,ext);

   if(strncmp(ext,"csv",CUTE_PATH_MAX_EXT)==0)
      pak_add_csv(path);
   else if(strncmp(ext,"pak",CUTE_PATH_MAX_EXT)==0)
      pak_add_pak(path);
   else
      RvR_log_line("RvR_pak_add","unknown extension ('%s') for pak file '%s'\n",ext,path);

RvR_err:
   return;
}

void RvR_pak_create_from_csv(const char *path_csv, const char *path_pak)
{
   FILE *f = NULL;
   Pak *pak = NULL;

   RvR_error_check(path_csv!=NULL,"RvR_pak_create_from_csv","argument 'path_csv' must be non-NULL\n");
   RvR_error_check(path_pak!=NULL,"RvR_pak_create_from_csv","argument 'path_pak' must be non-NULL\n");

   f = fopen(path_csv,"r");
   RvR_error_check(f!=NULL,"RvR-pak_create_from_csv","failed to open '%s'\n",path_csv);

   pak = pak_open(path_pak,"w");
   if(pak==NULL)
      goto RvR_err;

   char base_path[CUTE_PATH_MAX_PATH] = "";
   pak_path_pop(path_csv,base_path,NULL);
   strcat(base_path,"/");

   while(!feof(f)&&!ferror(f))
   {
      //Read path
      char delim = getc(f);
      char lump_path[CUTE_PATH_MAX_PATH] = "";
      ungetc(delim,f);
      if(delim=='"')
      {
         if(fscanf(f,"\"%255[^\"]\",",lump_path)!=1)
            break;
      }
      else
      {
         if(fscanf(f,"%255[^,],",lump_path)!=1)
            break;
      }

      //Read name
      delim = getc(f);
      ungetc(delim,f);
      char lump_name[CUTE_PATH_MAX_PATH] = "";
      if(delim=='"')
      {
         if(fscanf(f,"\"%255[^\"]\"\n",lump_name)!=1)
            break;
      }
      else
      {
         if(fscanf(f,"%255[^\n]\n",lump_name)!=1)
            break;
      }

      char tmp[CUTE_PATH_MAX_PATH*2] = "";
      strncpy(tmp,base_path,CUTE_PATH_MAX_PATH);
      strncat(tmp,lump_path,CUTE_PATH_MAX_PATH);

      FILE *fp = fopen(tmp,"rb");
      RvR_error_check(fp!=NULL,"RvR_pak_create_from_csv","failed to open '%s'\n",tmp);
      pak_append_file(pak,lump_name,fp);
      fclose(fp);
   }

RvR_err:
   if(f!=NULL)
      fclose(f);
   if(pak!=NULL)
      pak_close(pak);
}

void RvR_pak_flush()
{
   Pak_buffer *b = pak_buffer;

   while(b!=NULL)
   {
      Pak_buffer *next = b->next;
      pak_close(b->pak);
      RvR_free(b);

      b = next;
   }

   pak_buffer = NULL;
}

void RvR_lump_add(const char *name, const char *path)
{
   RvR_error_check(name!=NULL,"RvR_lump_add","argument 'name' must be non-NULL\n");
   RvR_error_check(path!=NULL,"RvR_lump_add","argument 'path' must be non-NULL\n");
   RvR_error_check(strlen(name)<=8,"RvR_lump_add","lump name ('%s') too long (max 8 characters)\n",name);

   char ext[CUTE_PATH_MAX_EXT] = "";
   pak_path_pop_ext(path,NULL,ext);

   Pak_lump l;
   strncpy(l.name,name,8);
   l.path = pak_paths_push(path);

   pak_lumps_push(l);

RvR_err:
   return;
}

void *RvR_lump_get(const char *name, unsigned *size)
{
   RvR_error_check(name!=NULL,"RvR_lump_get","argument 'name' must be non-NULL\n");

   uint8_t lump_index = pak_xor_string(name,strlen(name));

   for(uint32_t i = 0;i<pak_lumps[lump_index].used;i++)
   {
      if(strncmp(name,pak_lumps[lump_index].data[i].name,8)==0)
      {
         //Check for pak file
         char ext[CUTE_PATH_MAX_EXT] = {0};
         pak_path_pop_ext(pak_paths.data[pak_lumps[lump_index].data[i].path],NULL,ext);
         if(strncmp(ext,"pak",CUTE_PATH_MAX_EXT)==0)
         {
            //Check if pak already opened
            Pak_buffer *b = pak_buffer;
            while(b)
            {
               if(strcmp(b->path,pak_paths.data[pak_lumps[lump_index].data[i].path])==0)
                  break;
               b = b->next;
            }
            if(b==NULL)
            {
               b = RvR_malloc(sizeof(*b));
               b->pak = pak_open(pak_paths.data[pak_lumps[lump_index].data[i].path],"r");
               strncpy(b->path,pak_paths.data[pak_lumps[lump_index].data[i].path],255);
               b->path[255] = '\0';
               b->next = pak_buffer;
               pak_buffer = b;
            }

            int index = pak_find(b->pak,name);
            if(size!=NULL)
               *size = pak_size(b->pak,index);
            return pak_extract(b->pak,index);
         }

         //Raw reading
         FILE *f = fopen(pak_paths.data[pak_lumps[lump_index].data[i].path],"rb");
         RvR_error_check(f!=NULL,"RvR_lump_get","failed to open '%s'\n",pak_paths.data[pak_lumps[lump_index].data[i].path]);
         fseek(f,0,SEEK_END);
         unsigned fsize = ftell(f);
         if(size!=NULL)
            *size = fsize;
         fseek(f,0,SEEK_SET);
         uint8_t *buffer = RvR_malloc(fsize+1);
         fread(buffer,fsize,1,f);
         buffer[fsize] = 0;
         fclose(f);
         return buffer;
      }
   }

   RvR_log_line("RvR_lump_get","lump '%s' not found\n",name);

RvR_err:
   if(size!=NULL)
      *size = 0;
   return NULL;
}

const char *RvR_lump_get_path(const char *name)
{
   RvR_error_check(name!=NULL,"RvR_lump_get_path","argument 'name' must be non-NULL\n");

   uint8_t lump_index = pak_xor_string(name,strlen(name));

   for(int i = pak_lumps[lump_index].used-1;i>=0;i--)
      if(strncmp(name,pak_lumps[lump_index].data[i].name,8)==0)
         return pak_paths.data[pak_lumps[lump_index].data[i].path];

   RvR_log("RvR_pak: lump %s not found\n",name);

RvR_err:
   return NULL;
}

int RvR_lump_exists(const char *name)
{
   RvR_error_check(name!=NULL,"RvR_lump_exists","argument 'name' must be non-NULL\n");

   uint8_t lump_index = pak_xor_string(name,strlen(name));

   for(int i = pak_lumps[lump_index].used-1;i>=0;i--)
      if(strncmp(name,pak_lumps[lump_index].data[i].name,8)==0)
         return 1;

RvR_err:
   return 0;
}

//Parse a csv file and add all lumps to list
static void pak_add_csv(const char *path)
{
   FILE *f = NULL;

   RvR_error_check(path!=NULL,"pak_add_csv","argument 'path' must be non-NULL\n");

   f = fopen(path,"r");
   RvR_error_check(f!=NULL,"pak_add_csv","failed to open '%s'\n",path);

   char base_path[CUTE_PATH_MAX_PATH] = "";
   pak_path_pop(path,base_path,NULL);
   strcat(base_path,"/");

   while(!feof(f)&&!ferror(f))
   {
      //Read path
      char lump_path[CUTE_PATH_MAX_PATH];
      char delim = getc(f);
      ungetc(delim,f);
      if(delim=='"')
      {
         if(fscanf(f,"\"%255[^\"]\",",lump_path)!=1)
            break;
      }
      else
      {
         if(fscanf(f,"%255[^,],",lump_path)!=1)
            break;
      }

      //Read name
      char lump_name[CUTE_PATH_MAX_PATH];
      delim = getc(f);
      ungetc(delim,f);
      if(delim=='"')
      {
         if(fscanf(f,"\"%255[^\"]\"\n",lump_name)!=1)
            break;
      }
      else
      {
         if(fscanf(f,"%255[^\n]\n",lump_name)!=1)
            break;
      }

      char tmp[CUTE_PATH_MAX_PATH*2] = "";
      strcpy(tmp,base_path);
      RvR_lump_add(lump_name,strncat(tmp,lump_path,255));
   }

RvR_err:
   if(f!=NULL)
      fclose(f);
}

//Parse a pak file and add to list
static void pak_add_pak(const char *path)
{
   RvR_error_check(path!=NULL,"pak_add_pak","argument 'path' must be non-NULL\n");

   //Add pak to list
   Pak_buffer *b = RvR_malloc(sizeof(*b));
   b->pak = pak_open(path,"r");
   strncpy(b->path,path,255);
   b->path[255] = '\0';
   b->next = pak_buffer;
   pak_buffer = b;

   //Add all files in pak to lump array
   int count = pak_count(b->pak);
   for(int i = 0;i<count;i++)
      RvR_lump_add(pak_name(b->pak,i),path);

RvR_err:
   return;
}

//Add a lump to the lump array.
//Array will dynamically expand if more 
//space is needed
static void pak_lumps_push(Pak_lump l)
{
   uint8_t lump_index = pak_xor_string(l.name,8);

   //Allocate memory for list
   if(pak_lumps[lump_index].data==NULL)
   {
      pak_lumps[lump_index].size = 4;
      pak_lumps[lump_index].used = 0;
      pak_lumps[lump_index].data = RvR_malloc(sizeof(*pak_lumps[lump_index].data)*pak_lumps[lump_index].size);
   }
  
   //Override existing entry
   for(uint32_t i = 0;i<pak_lumps[lump_index].used;i++)
   {
      if(strncmp(pak_lumps[lump_index].data[i].name,l.name,8)==0)
      {
         memcpy(&pak_lumps[lump_index].data[i],&l,sizeof(l));
         return;
      }
   }

   pak_lumps[lump_index].data[pak_lumps[lump_index].used++] = l;
   if(pak_lumps[lump_index].used==pak_lumps[lump_index].size)
   {
      pak_lumps[lump_index].size*=2;
      pak_lumps[lump_index].data = RvR_realloc(pak_lumps[lump_index].data,sizeof(*pak_lumps[lump_index].data)*pak_lumps[lump_index].size);
   }
}

static uint32_t pak_paths_push(const char *path)
{
   RvR_error_check(path!=NULL,"pak_paths_push","argument 'path' must be non-NULL\n");

   //Allocate memory for list
   if(pak_paths.data==NULL)
   {
      pak_paths.size = 16;
      pak_paths.used = 0;
      pak_paths.data = RvR_malloc(sizeof(*pak_paths.data)*pak_paths.size);
   }
  
   //Check if already added
   for(uint32_t i = 0;i<pak_paths.used;i++)
      if(strncmp(pak_paths.data[i],path,255)==0)
         return i;

   strcpy(pak_paths.data[pak_paths.used],path);
   pak_paths.used++;

   if(pak_paths.used==pak_paths.size)
   {
      pak_paths.size+=16;
      pak_paths.data = RvR_realloc(pak_paths.data,sizeof(*pak_paths.data)*pak_paths.size);
   }

RvR_err:
   return pak_paths.used-1;
}

static uint8_t pak_xor_string(const char *str, size_t len)
{
   uint8_t hash = 0;

   RvR_error_check(str!=NULL,"pak_xor_string","argument 'str' must be non-NULL\n");

   for(size_t i = 0;i<len;i++)
      hash^=str[i];

RvR_err:
   return hash;
}

//pak_path_pop, pak_path_pop_ext, pak_path_is_slash
//by RandyGaul (https://github.com/RandyGaul), cute_headers (https://github.com/RandyGaul/cute_headers/blob/master/cute_path.h)
//Original license info:
/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses - you may choose the one you like.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - zlib license
   Copyright (c) 2017 Randy Gaul http://www.randygaul.net
   This software is provided 'as-is', without any express or implied warranty.
   In no event will the authors be held liable for any damages arising from
   the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:
     1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software
        in a product, an acknowledgment in the product documentation would be
        appreciated but is not required.
     2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
     3. This notice may not be removed or altered from any source distribution.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
   software, either in source code form or as a compiled binary, for any purpose, 
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this 
   software dedicate any and all copyright interest in the software to the public 
   domain. We make this dedication for the benefit of the public at large and to 
   the detriment of our heirs and successors. We intend this dedication to be an 
   overt act of relinquishment in perpetuity of all present and future rights to 
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/

static int pak_path_pop(const char* path, char* out, char* pop)
{
   const char *original = path;
   int total_len = 0;
   int len = 0;
   int pop_len = 0; //length of substring to be popped

   while(*path)
   {
      ++total_len;
      ++path;
   }

   //ignore trailing slash from input path
   if(pak_path_is_slash(*(path-1)))
   {
      --path;
      total_len-=1;
   }

   while(!pak_path_is_slash(*--path)&&pop_len!=total_len)
      ++pop_len;
   len = total_len-pop_len; //ength to copy

   //don't ignore trailing slash if it is the first character
   if(len>1)
      len--;

   if(len>0)
   {
      if(out)
      {
         strncpy(out,original,len);
         out[len] = 0;
      }

      if(pop)
      {
         strncpy(pop, path + 1, pop_len);
         pop[pop_len] = 0;
      }

      return len;
   }
   else
   {
      if(out)
      {
         out[0] = '.';
         out[1] = 0;
      }
      if(pop)
         *pop = 0;

      return 1;
   }
}

static int pak_path_pop_ext(const char *path, char *out, char *ext)
{
   if(out!=NULL)
      *out = '\0';
   if (ext!=NULL)
      *ext = '\0';

   // skip leading dots
   const char *p = path;
   while(*p=='.')
      ++p;

   const char *last_slash = path;
   const char *last_period = NULL;
   while(*p!='\0')
   {
      if(pak_path_is_slash(*p))
         last_slash = p;
      else if(*p=='.')
         last_period = p;
      ++p;
   }

   if(last_period!=NULL&&last_period>last_slash)
   {
      if(ext!=NULL)
         strncpy(ext,last_period+1,CUTE_PATH_MAX_EXT);
   }
   else
   {
      last_period = p;
   }

   int len = (int)(last_period-path);
   if(len>CUTE_PATH_MAX_PATH-1) len = CUTE_PATH_MAX_PATH-1;

   if(out!=NULL)
   {
      strncpy(out,path,len);
      out[len] = '\0';
   }

   return len;
}

static int pak_path_is_slash(char c)
{
   return (c=='/')|(c=='\\');
}
//cute_path END

//pak_size, pak_count, pak_name, pak_close, pak_extract, pak_find, pak_open, pak_append_file
//by r-lyeh(https://github.com/r-lyeh), stdarc.c (https://github.com/r-lyeh/stdarc.c/blob/master/src/pak.c)
//Original license info (File layout information inaccurate since slightly changed in RvnicRaven):
//// pak file reading/writing/appending.
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

static unsigned pak_size(Pak *p, unsigned index)
{
   RvR_error_check(p!=NULL,"pak_name","argument 'p' must be non-NULL\n");
   RvR_error_check(p->in!=NULL,"pak_name","input file for pak is NULL, opened in write-only mode?\n");
   RvR_error_check(p->count<index,"pak_name","pak file index out of range ('%d'>='%d')\n",index,p->count);

   return p->entries[index].size;

RvR_err:
   return 0;
}

static unsigned pak_count(Pak *p)
{
   RvR_error_check(p!=NULL,"pak_count","argument 'p' must be non-NULL\n");
   RvR_error_check(p->in!=NULL,"pak_count","input file for pak is NULL, opened in write-only mode?\n");

   return p->count;

RvR_err:
   return 0;
}

static char *pak_name(Pak *p, unsigned index)
{
   RvR_error_check(p!=NULL,"pak_name","argument 'p' must be non-NULL\n");
   RvR_error_check(p->in!=NULL,"pak_name","input file for pak is NULL, opened in write-only mode?\n");
   RvR_error_check(p->count<index,"pak_name","pak file index out of range ('%d'>='%d')\n",index,p->count);

    return p->entries[index].name;

RvR_err:
    return NULL;
}

static void pak_close(Pak *p)
{
   RvR_error_check(p!=NULL,"pak_close","argument 'p' must be non-NULL\n");

   if(p->out!=NULL)
   {
      //write toc
      uint32_t seek = 12;
      uint32_t dirpos = (uint32_t)RvR_rw_tell(p->out);
      uint32_t dirlen = p->count*64;
      for(unsigned i = 0;i<p->count;i++)
      {
         Pak_file *e = &p->entries[i];

         //write name (truncated if needed), and trailing zeros
         char zero[12] = {0};
         int namelen = strlen(e->name);
         RvR_rw_write(p->out,e->name,1,namelen);
         RvR_rw_write(p->out,zero,1,12-namelen);

         //write offset + length pair
         RvR_rw_write_u32(p->out,seek);
         RvR_rw_write_u32(p->out,e->size);
         seek+=e->size;
      }

      //patch header
      RvR_rw_seek(p->out,0L,SEEK_SET);
      RvR_rw_write(p->out,"PAK",1,3);
      RvR_rw_write_u8(p->out,RVR_ENDIAN);
      RvR_rw_write_u32(p->out,dirpos);
      RvR_rw_write_u32(p->out,dirlen);
   }

RvR_err:

   if(p!=NULL)
   {
      //close streams
      if(p->in!=NULL)
      {
         RvR_rw_close(p->in);
         RvR_free(p->in);
      }
      if(p->out!=NULL)
      {
         RvR_rw_close(p->out);
         RvR_free(p->out);
      }

      //delete
      if(p->entries!=NULL)
         RvR_free(p->entries);
      RvR_free(p);
   }
}

static void *pak_extract(Pak *p, unsigned index)
{
   RvR_error_check(p!=NULL,"pak_extract","argument 'p' must be non-NULL\n");
   RvR_error_check(p->in!=NULL,"pak_extract","input file for pak is NULL, opened in write-only mode?\n");

   if(index<p->count)
   {
      Pak_file *e = &p->entries[index];
      if(RvR_rw_seek(p->in,e->offset,SEEK_SET)!=0)
         return NULL;

      void *buffer = RvR_malloc(e->size);
      if(RvR_rw_read(p->in,buffer,1,e->size)!=e->size)
      {
         RvR_free(buffer);
         return NULL;
      }

      return buffer;
   }

RvR_err:
   return NULL;
}

static int pak_find(Pak *p, const char *filename)
{
   RvR_error_check(p!=NULL,"pak_find","argument 'p' must be non-NULL\n");
   RvR_error_check(filename!=NULL,"pak_find","argument 'filename' must be non-NULL\n");
   RvR_error_check(p->in!=NULL,"pak_find","input file for pak is NULL, opened in write-only mode?\n");

   for(int i = p->count;--i>=0;)
      if(!strncmp(p->entries[i].name,filename,8))
         return i;

RvR_err:
   return -1;
}

static Pak *pak_open(const char *fname, const char *mode)
{
   Pak *p = NULL;
   RvR_rw *rw = NULL;

   RvR_error_check(fname!=NULL,"pak_open","argument 'fname' must be non-NULL\n");
   RvR_error_check(mode!=NULL,"pak_open","argument 'mode' must be non-NULL\n");

   if(mode[0]!='w'&&mode[0]!='r')
   {
      RvR_log_line("pak_open","invalid pak_open mode ('%s'), must be either \"r\" or \"w\"\n",mode);
      return NULL;
   }

   rw = RvR_malloc(sizeof(*rw));
   RvR_rw_init_path(rw,fname,mode[0]=='w'?"wb":mode[0]=='r'?"rb":"rb+");

   p = RvR_malloc(sizeof(*p));
   memset(p,0,sizeof(*p));

   if(mode[0]=='r')
   {
      //Read header
      char ident[3];
      RvR_rw_read(rw,&ident,1,3);
      uint8_t endian = RvR_rw_read_u8(rw);
      RvR_rw_endian(rw,endian);
      uint32_t offset = RvR_rw_read_u32(rw);
      uint32_t size = RvR_rw_read_u32(rw);

      RvR_error_check(strncmp(ident,"PAK",3)==0,"pak_open","pak header identifier mismath, expected \"PAK\"\n");

      unsigned num_files = size/64;

      if(RvR_rw_seek(rw,offset,SEEK_SET)!=0)
         goto RvR_err;

      p->count = num_files;
      p->entries = RvR_malloc(num_files*sizeof(Pak_file));

      for(unsigned i = 0;i<num_files;i++)
      {
         RvR_rw_read(rw,p->entries[i].name,1,sizeof(p->entries[i].name));
         p->entries[i].offset = RvR_rw_read_u32(rw);
         p->entries[i].size = RvR_rw_read_u32(rw);
      }

      p->in = rw;

      return p;
   }


   if(mode[0]=='w')
   {
      p->out = rw;

      //write temporary header
      RvR_rw_write_u8(rw,'P');
      RvR_rw_write_u8(rw,'A');
      RvR_rw_write_u8(rw,'K');
      RvR_rw_write_u8(rw,RVR_ENDIAN);
      RvR_rw_write_u32(rw,0);
      RvR_rw_write_u32(rw,0);

      return p;
   }

RvR_err:

   if(p!=NULL)
   {
      if(p->entries!=NULL)
         RvR_free(p->entries);
      RvR_free(p);
   }

   if(rw!=NULL)
   {
      RvR_rw_close(rw);
      RvR_free(rw);
   }

   return NULL;
}

static void pak_append_file(Pak *p, const char *filename, FILE *in)
{
   unsigned count_old = p->count;
   RvR_error_check(p!=NULL,"pak_append_file","argument 'p' must be non-NULL\n");
   RvR_error_check(filename!=NULL,"pak_append_file","argument 'filename' must be non-NULL\n");
   RvR_error_check(in!=NULL,"pak_append_file","argument 'in' must be non-NULL\n");
   RvR_error_check(strlen(filename)<=8,"pak_append_file","filename ('%s') too long (max 8 characters\n",filename);

   //index meta
   unsigned index = p->count++;
   p->entries = RvR_realloc(p->entries,p->count*sizeof(Pak_file));
   Pak_file *e = &p->entries[index];
   memset(e,0,sizeof(*e));
   strncpy(e->name,filename,9);
   e->offset = RvR_rw_tell(p->out);

   char buf[1<<15];
   while(!feof(in))
   {
      size_t bytes = fread(buf,1,sizeof(buf),in);
      RvR_error_check(!ferror(in),"pak_append_file","fread() failed\n");
      RvR_rw_write(p->out,buf,1,bytes);
   }

   e->size = RvR_rw_tell(p->out)-e->offset;

   return;

RvR_err:

   p->count = count_old;
   p->entries = RvR_realloc(p->entries,p->count*sizeof(Pak_file));

   return;
}

//pak.c END
//-------------------------------------

#undef CUTE_PATH_MAX_PATH
#undef CUTE_PATH_MAX_EXT
