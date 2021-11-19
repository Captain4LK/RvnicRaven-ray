/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
//-------------------------------------

//#defines
#define CUTE_PATH_MAX_PATH 1024
#define CUTE_PATH_MAX_EXT 32
#define JSON5_ASSERT do { RvR_log("RvR_pak: error L%d while parsing '%c' in '%.16s'\n", __LINE__, p[0], p); assert(0); } while(0)
//-------------------------------------

//Typedefs

typedef struct Pak_header
{
   char id[3];
   uint8_t endian;
   uint32_t offset;
   uint32_t size;
}Pak_header;

typedef struct Pak_file
{
   char name[56];
   uint32_t offset;
   uint32_t size;
}Pak_file;

typedef struct Pak
{
    RvR_rw *in, *out;
    Pak_file *entries;
    unsigned count;
}Pak;

typedef struct
{
   char name[8];
   uint32_t path;
}Lump;

typedef struct Pak_buffer
{
   char path[256];
   Pak *pak;
   struct Pak_buffer *next;
}Pak_buffer;

typedef enum Json5_type 
{
   Json5_undefined,
   Json5_null,
   Json5_bool,
   Json5_object,
   Json5_string,
   Json5_array,
   Json5_integer,
   Json5_real,
}Json5_type;

typedef struct 
{
   uint32_t used;
   uint32_t size;
   void *data;
}Json5_dyn_array;

typedef struct Json5
{
   char *name;
   unsigned type:3;
   unsigned count:29;
   union 
   {
      Json5_dyn_array array;
      Json5_dyn_array nodes;
      int64_t integer;
      double real;
      char *string;
      int boolean;
   }content;
}Json5;

typedef struct
{
   char *data;
   size_t data_size;
   Json5 root;
}Json5_root;
//-------------------------------------

//Variables
static struct
{
   uint32_t used;
   uint32_t size;
   Lump *data;
}lumps = {0};

static struct
{
   uint32_t used;
   uint32_t size;
   char (*data)[256];
}pak_paths;

static Pak_buffer *pak_buffer = NULL;
//-------------------------------------

//Function prototypes
static void lumps_push(Lump l);
static uint32_t pak_paths_push(const char *path);
static void add_json_path(const char *path);
static void add_pak_path(const char *path);

//cute_path
static int path_pop_ext(const char *path, char *out, char *ext);
static int path_pop(const char *path, char *out, char *pop);
static int path_is_slash(char c);

//tinyjson5
static void json_free(Json5_root *r);
static void json5_free(Json5 *root);
static void json5_array_free(Json5_dyn_array *array);
static void json5_push(Json5_dyn_array *array, Json5 ob);
static Json5_root *json_parse_file(const char *path);
Json5_root *json_parse_char_buffer(const char *buffer, size_t size);
static char *json5_parse(Json5 *root, char *p);
static char *json5__trim(char *p);
static char *json5__parse_value(Json5 *obj, char *p, char **err_code);
static char *json5__parse_string(Json5 *obj, char *p);
static char *json5__parse_object(Json5 *obj, char *p, char **err_code);
static Json5 *json_get_object(Json5 *json, const char *name);
static int json_get_array_size(const Json5 *json);
static Json5 *json_get_array_item(Json5 *json, int index);
static char *json_get_object_string(Json5 *json, const char *name, char *fallback);

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
   char ext[33] = {0};
   path_pop_ext(path,NULL,ext);

   if(strncmp(ext,"json",32)==0)
      add_json_path(path);
   else if(strncmp(ext,"pak",32)==0)
      add_pak_path(path);
}

void RvR_pak_create_from_json(const char *path_json, const char *path_pak)
{
   Pak *pak = pak_open(path_pak,"w");

   char base_path[256] = {0};
   char tmp[256] = {0};
   path_pop(path_json,base_path,NULL);
   strcat(base_path,"/");

   Json5_root *root = json_parse_file(path_json);

   Json5 *array = json_get_object(&root->root,"lumps");
   if(array==NULL)
      RvR_log("RvR_pak: no 'lumps' array in json\n");

   int count = json_get_array_size(array);
   for(int i = 0;i<count;i++)
   {
      Json5 *el = json_get_array_item(array,i);

      strcpy(tmp,base_path);
      strcat(tmp,json_get_object_string(el,"path","NULL"));

      FILE *f = fopen(tmp,"rb");
      pak_append_file(pak,json_get_object_string(el,"name","NULL"),f);
      fclose(f);
   }

   pak_close(pak);

   json_free(root);
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
   if(strlen(name)>8)
      RvR_log("RvR_pak: lump name too long (max 8 characters)\n");

   char ext[33] = "";
   path_pop_ext(path,NULL,ext);

   Lump l;
   strncpy(l.name,name,8);
   l.path = pak_paths_push(path);

   lumps_push(l);
}

void *RvR_lump_get(const char *name, unsigned *size)
{
   for(int i = lumps.used-1;i>=0;i--)
   {
      if(strncmp(name,lumps.data[i].name,8)==0)
      {
         //Check for pak file
         char ext[33] = {0};
         path_pop_ext(pak_paths.data[lumps.data[i].path],NULL,ext);
         if(strncmp(ext,"pak",32)==0)
         {
            //Check if pak already opened
            Pak_buffer *b = pak_buffer;
            while(b)
            {
               if(strcmp(b->path,pak_paths.data[lumps.data[i].path])==0)
                  break;
               b = b->next;
            }
            if(b==NULL)
            {
               b = RvR_malloc(sizeof(*b));
               b->pak = pak_open(pak_paths.data[lumps.data[i].path],"r");
               strncpy(b->path,pak_paths.data[lumps.data[i].path],255);
               b->path[255] = '\0';
               b->next = pak_buffer;
               pak_buffer = b;
            }

            int index = pak_find(b->pak,name);
            *size = pak_size(b->pak,index);
            return pak_extract(b->pak,index);
         }

         //Raw reading
         FILE *f = fopen(pak_paths.data[lumps.data[i].path],"rb");
         if(!f)
         {
            RvR_log("RvR_pak: failed to open %s\n",lumps.data[i].path);
            *size = 0;
            return NULL;
         }
         fseek(f,0,SEEK_END);
         *size = ftell(f);
         fseek(f,0,SEEK_SET);
         uint8_t *buffer = RvR_malloc(*size+1);
         fread(buffer,*size,1,f);
         buffer[*size] = 0;
         fclose(f);
         return buffer;
      }
   }

   RvR_log("RvR_pak: lump %s not found\n",name);
   *size = 0;
   return NULL;
}

const char *RvR_lump_get_path(const char *name)
{
   for(int i = lumps.used-1;i>=0;i--)
      if(strncmp(name,lumps.data[i].name,8)==0)
         return pak_paths.data[lumps.data[i].path];

   RvR_log("RvR_pak: lump %s not found\n",name);
   return NULL;
}

//Parse a json file and add all lumps to list
static void add_json_path(const char *path)
{
   char base_path[256] = {0};
   char tmp[256] = {0};
   path_pop(path,base_path,NULL);
   strcat(base_path,"/");

   Json5_root *root = json_parse_file(path);

   Json5 *array = json_get_object(&root->root,"lumps");
   if(array==NULL)
      RvR_log("RvR_pak: no 'lumps' array in json\n");

   int count = json_get_array_size(array);
   for(int i = 0;i<count;i++)
   {
      Json5 *el = json_get_array_item(array,i);

      strcpy(tmp,base_path);
      RvR_lump_add(json_get_object_string(el,"name","NULL"),strcat(tmp,json_get_object_string(el,"path","NULL")));
   }

   json_free(root);
}

//Parse a pak file and add to list
static void add_pak_path(const char *path)
{
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
}

//Add a lump to the lump array.
//Array will dynamically expand if more 
//space is needed
static void lumps_push(Lump l)
{
   //Allocate memory for list
   if(lumps.data==NULL)
   {
      lumps.size = 16;
      lumps.used = 0;
      lumps.data = RvR_malloc(sizeof(*lumps.data)*lumps.size);
   }
  
   //Override existing entry
   for(uint32_t i = 0;i<lumps.used;i++)
   {
      if(strncmp(pak_paths.data[lumps.data[i].path],l.name,8)==0)
      {
         memcpy(&lumps.data[i],&l,sizeof(l));
         return;
      }
   }

   lumps.data[lumps.used++] = l;
   if(lumps.used==lumps.size)
   {
      lumps.size+=16;
      lumps.data = RvR_realloc(lumps.data,sizeof(*lumps.data)*lumps.size);
   }
}

static uint32_t pak_paths_push(const char *path)
{
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

   return pak_paths.used-1;
}

//path_pop, path_pop_ext, path_is_slash
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

static int path_pop(const char* path, char* out, char* pop)
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
   if(path_is_slash(*(path-1)))
   {
      --path;
      total_len-=1;
   }

   while(!path_is_slash(*--path)&&pop_len!=total_len)
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

static int path_pop_ext(const char *path, char *out, char *ext)
{
   int initial_skipped_periods = 0;
   const char *period = NULL;
   char c = 0;
   int has_period = 0;
   int len = 0;

   while(*path == '.')
   {
      ++path;
      ++initial_skipped_periods;
   }

   period = path;
   while((c = *period++))
      if(c=='.')
         break;

   has_period = c=='.';
   len = (int)(period-path)-1+initial_skipped_periods;
   if(len>CUTE_PATH_MAX_PATH-1)
      len = CUTE_PATH_MAX_PATH-1;

   if(out)
   {
      strncpy(out,path-initial_skipped_periods,len);
      out[len] = 0;
   }

   if(ext)
   {
      if(has_period)
         strncpy(ext,path-initial_skipped_periods+len+1,CUTE_PATH_MAX_EXT);
      else
         ext[0] = 0;
   }
   return len;
}

static int path_is_slash(char c)
{
   return (c=='/')|(c=='\\');
}
//cute_path END

static void json_free(Json5_root *r)
{
   RvR_free(r->data);
   json5_free(&r->root);
   RvR_free(r);
}

static void json5_array_free(Json5_dyn_array *array)
{
   RvR_free(array->data);
   array->data = NULL;
   array->size = 0;
   array->used = 0;
}

static void json5_push(Json5_dyn_array *array, Json5 ob)
{
   if(array->data==NULL)
   {
      array->size = 16;
      array->used = 0;
      array->data = RvR_malloc(sizeof(ob)*array->size);
   }
   
   ((Json5 *)(array->data))[array->used++] = ob;
   if(array->used==array->size)
   {
      array->size+=16;
      array->data = RvR_realloc(array->data,sizeof(ob)*array->size);
   }
}

static Json5_root *json_parse_file(const char *path)
{
   //Load data from file
   Json5_root *r = RvR_malloc(sizeof(*r));
   memset(r,0,sizeof(*r));
   FILE *f = fopen(path,"rb");
   fseek(f,0,SEEK_END);
   r->data_size = ftell(f);
   fseek(f,0,SEEK_SET);
   r->data = RvR_malloc(r->data_size+1);
   fread(r->data,r->data_size,1,f);
   r->data[r->data_size] = '\0';
   fclose(f);

   //Parsing
   json5_parse(&r->root,r->data);

   return r;
}

Json5_root *json_parse_char_buffer(const char *buffer, size_t size)
{
   Json5_root *r = RvR_malloc(sizeof(*r));
   memset(r,0,sizeof(*r));
   r->data = RvR_malloc(size+1);
   memcpy(r->data,buffer,size);
   r->data[size] = '\0';
   r->data_size = size;

   //Parsing
   json5_parse(&r->root,r->data);

   return r;
}

static Json5 *json_get_object(Json5 *json, const char *name)
{
   //Not an object
   if(json->type!=Json5_object)
      return NULL;

   for(int i = 0;i<json->count;i++)
   {
      if(strcmp(name,((Json5 *)(json->content.nodes.data))[i].name)==0)
         return &((Json5 *)(json->content.nodes.data))[i];
   }

   //Not found
   return NULL;
}

static int json_get_array_size(const Json5 *json)
{
   if(json->type!=Json5_array)
      return -1;

   return json->count;
}

static Json5 *json_get_array_item(Json5 *json, int index)
{
   //Not an array
   if(json->type!=Json5_array)
      return NULL;

   //Out of bounds
   if(index<0||index>=json->count)
      return NULL;

   return &((Json5 *)(json->content.array.data))[index];
}

static char *json_get_object_string(Json5 *json, const char *name, char *fallback)
{
   if(!json||json->type!=Json5_object)
      return fallback;

   Json5 *o = json_get_object(json,name);
   if(!o)
      return fallback;

   if(o->type==Json5_string)
      return o->content.string;

   return fallback;
}

//json5_free, json5_parse, json5__trim, json5__parse_value, json5__parse_object, json5__parse_string
//by r-lyeh(https://github.com/r-lyeh), tinybits (https://github.com/r-lyeh/tinybits/blob/master/tinyjson5.c)
//Original license info:
//
// JSON5 + SJSON parser module
//
// License:
// This software is dual-licensed to the public domain and under the following
// license: you are granted a perpetual, irrevocable license to copy, modify,
// publish, and distribute this file as you see fit.
// No warranty is implied, use at your own risk.
//
// Credits:
// Dominik Madarasz (original code) (GitHub: zaklaus)
// r-lyeh (fork)

static void json5_free(Json5 *root)
{
   if(root->type==Json5_array&&root->content.array.data!=NULL) 
   {
      for(int i = 0, cnt = root->content.array.used;i<cnt;i++) 
      {
         json5_free(&((Json5 *)(root->content.array.data))[i]);
      }
      json5_array_free(&root->content.array);
   } 

   if(root->type==Json5_object&&root->content.nodes.data!=NULL) 
   {
      for(int i = 0,cnt = root->content.nodes.used;i<cnt;i++) 
      {
         json5_free(&((Json5 *)(root->content.nodes.data))[i]);
      }
      json5_array_free(&root->content.nodes);
   }

   *root = (Json5) {0}; // needed?
}

static char *json5_parse(Json5 *root, char *p)
{
   assert(root&&p);

   char *err_code = "";
   *root = (Json5) {0};

   p = json5__trim(p);
   if(*p=='[') 
   { /* <-- for SJSON */
      json5__parse_value(root, p, &err_code);
   } 
   else 
   {
      json5__parse_object(root,p,&err_code); /* <-- for SJSON */
   }

   return err_code[0] ? err_code : 0;
}

static char *json5__trim(char *p)
{
   while (*p) 
   {
      if(isspace(*p)) 
      {
         ++p;
      }
      else if(p[0]=='/'&&p[1]=='*') 
      { 
         //skip C comment
         for(p+=2;*p&&!(p[0]=='*'&&p[1]=='/');++p);
         if(*p) 
            p+=2;
      }
      else if(p[0]=='/'&&p[1]=='/') 
      { 
         //skip C++ comment
         for(p+=2;*p&&p[0]!='\n';++p);
         if( *p ) 
            ++p;
      }
      else 
      {
         break;
      }
   }

   return p;
}

static char *json5__parse_value(Json5 *obj, char *p, char **err_code) 
{
   assert(obj&&p);

   p = json5__trim(p);

   char *is_string = json5__parse_string(obj,p);

   if(is_string) 
   {
      p = is_string;
      if(*err_code[0])
      {
         return NULL;
      }
   }
   else if(*p=='{') 
   {
      p = json5__parse_object(obj,p,err_code);
      if(*err_code[0]) 
      {
         return NULL;
      }
   }
   else if(*p=='[')
   {
      obj->type = Json5_array;
      obj->content.array.data = NULL;

      while (*p) 
      {
         Json5 elem = {0};
         elem.content.array.data = NULL;

         do
         { 
            p = json5__trim(p+1); 
         } 
         while(*p==',');

         if(*p ==']') 
         { 
            ++p; 
            break; 
         }

         p = json5__parse_value(&elem,p,err_code);

         if(*err_code[0]) 
         {
            return NULL;
         }

         if(elem.type!=Json5_undefined) 
         {
            json5_push(&obj->content.array, elem);
            ++obj->count;
         }
         if(*p==']') 
         { 
            ++p; 
            break; 
         }
      }
   }
   else if(isalpha(*p)||(*p=='-'&&!isdigit(p[1]))) 
   {
      const char *labels[] = { "null", "on","true", "off","false", "nan","NaN", "-nan","-NaN", "inf","Infinity", "-inf","-Infinity" };
      const int lenghts[] = { 4, 2,4, 3,5, 3,3, 4,4, 3,8, 4,9 };
      for(int i = 0;labels[i];++i)
      {
         if(!strncmp(p,labels[i],lenghts[i])) 
         {
            p+=lenghts[i];
#ifdef _MSC_VER // somehow, NaN is apparently signed in MSC
            if(i>=5) 
            {
               obj->type = Json5_real;
               obj->real = i>=11?-INFINITY:i>=9?INFINITY:i>=7?NAN:-NAN;
            }
#else
            if(i>=5)
            {
               obj->type = Json5_real;
               obj->content.real = i>=11?-INFINITY:i>=9?INFINITY:i>=7?-NAN:NAN;
            }
#endif
            else if(i>=1) 
            {
               obj->type = Json5_bool;
               obj->content.boolean = i <= 2;
            }
            else
            {
               obj->type = Json5_null;
            }
            break;
         }
      }
      if(obj->type==Json5_undefined ) 
      {
         JSON5_ASSERT; *err_code = "json5_error_invalid_value";
         return NULL;
      }
   }
   else if(isdigit(*p)||*p=='+'||*p=='-'||*p=='.')
   {
      char buffer[16] = {0};
      char *buf = buffer;
      char is_hex = 0;
      char is_dbl = 0;
      while(*p&&strchr("+-.xX0123456789aAbBcCdDeEfF",*p)) 
      {
         is_hex |= (*p | 32) == 'x';
         is_dbl |= *p == '.';
         *buf++ = *p++;
      }
      obj->type = is_dbl?Json5_real:Json5_integer;
      if(is_dbl) 
      {
         sscanf(buffer,"%lf",&obj->content.real);
      }
      else if(is_hex) 
      {
         uint64_t hex;
         sscanf(buffer,"%"SCNx64,&hex); // SCNx64 -> inttypes.h
         obj->content.integer = (int64_t)hex;
      }
      else
      {
         sscanf(buffer,"%" SCNd64,&obj->content.integer); // SCNd64 -> inttypes.h
      }
   }
   else 
   {
      return NULL;
   }

   return p;
}

static char *json5__parse_string(Json5 *obj, char *p)
{
   assert(obj&&p);

   if(*p=='"'||*p=='\''||*p=='`') 
   {
      obj->type = Json5_string;
      obj->content.string = p+1;

      char eos_char = *p;
      char *b = obj->content.string;
      char *e = b;
      while (*e) 
      {
         if(*e=='\\'&&(e[1]==eos_char)) 
            ++e;
         else if(*e=='\\'&&(e[1]=='\r'||e[1]=='\n')) 
            *e = ' ';
         else if(*e==eos_char) 
            break;
         ++e;
      }

      *e = '\0';
      return (p = e+1);
   }

   //JSON5_ASSERT; *err_code = "json5_error_invalid_value";
   return NULL;
}

static char *json5__parse_object(Json5 *obj, char *p, char **err_code)
{
   assert(obj&&p);

   if(1) /* <-- for SJSON */
   { 
      int skip = *p=='{'; /* <-- for SJSON */
      obj->type = Json5_object;
      obj->content.nodes.data = NULL;
      obj->content.nodes.size = 0;
      obj->content.nodes.used = 0;

      while(*p)
      {
         Json5 node = {0};
         do 
         { 
            p = json5__trim(p+skip);
            skip = 1; 
         }
         while(*p ==',');

         if(*p =='}') 
         {
            ++p;
            break;
         }
         // @todo: is_unicode() (s[0] == '\\' && isxdigit(s[1]) && isxdigit(s[2]) && isxdigit(s[3]) && isxdigit(s[4]))) {
         else if(isalpha(*p)||*p=='_'||*p=='$') 
         { 
             // also || is_unicode(p)
            node.name = p;

            do 
            {
               ++p;
            } 
            while(*p&&(*p=='_'||isalpha(*p)||isdigit(*p))); // also || is_unicode(p)

            char *e = p;
            p = json5__trim(p);
            *e = '\0';
         }
         else 
         { 
            //if( *p == '"' || *p == '\'' || *p == '`' ) {
            char *ps = json5__parse_string(&node,p);
            if(!ps) 
            {
               return NULL;
            }
            p = ps;
            node.name = node.content.string;
            p = json5__trim(p);
         }

         // @todo: https://www.ecma-international.org/ecma-262/5.1/#sec-7.6
         if(!(node.name && node.name[0])) 
         { 
            // !json5__validate_name(node.name) ) {
            JSON5_ASSERT; *err_code = "json5_error_invalid_name";
            return NULL;
         }

         if(!p||(*p&&(*p!=':'&&*p!='='/*<-- for SJSON */))) 
         {
            JSON5_ASSERT; *err_code = "json5_error_invalid_name";
            return NULL;
         }
         p = json5__trim(p + 1);
         p = json5__parse_value(&node, p, err_code);

         if(*err_code[0]) 
         {
            return NULL;
         }

         if(node.type!=Json5_undefined) 
         {
            json5_push(&obj->content.nodes, node);
            ++obj->count;
         }

         if(*p =='}') 
         { 
            ++p; 
            break; 
         }
      }
      return p;
   }

   JSON5_ASSERT; *err_code = "json5_error_invalid_value";
   return NULL;
}
//tinyjson5 END

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
   return (p->in!=NULL)&&index<p->count?p->entries[index].size:0;
}

static unsigned pak_count(Pak *p)
{
   return (p->in!=NULL)?p->count:0;
}

static char *pak_name(Pak *p, unsigned index)
{
    return (p->in!=NULL)&&index<p->count?p->entries[index].name:NULL;
}

static void pak_close(Pak *p)
{
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
         char zero[56] = {0};
         int namelen = strlen(e->name);
         RvR_rw_write(p->out,e->name,1,namelen>=56?55:namelen);
         RvR_rw_write(p->out,zero,1,namelen>=56?1:56-namelen);

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
   RvR_free(p->entries);
   RvR_free(p);
}

static void *pak_extract(Pak *p, unsigned index)
{
   if((p->in!=NULL)&&index<p->count )
   {
      Pak_file *e = &p->entries[index];
      if(RvR_rw_seek(p->in,e->offset,SEEK_SET)!=0)
         return NULL;

      void *buffer = RvR_malloc(e->size);
      if(buffer==NULL)
         return NULL;

      if(RvR_rw_read(p->in,buffer,1,e->size)!=e->size)
      {
         RvR_free(buffer);
         return NULL;
      }

      return buffer;
   }
   return NULL;
}

static int pak_find(Pak *p, const char *filename)
{
   if(p->in!=NULL)
      for(int i = p->count;--i>=0;)
         if(!strcmp(p->entries[i].name,filename))
            return i;

   return -1;
}

static Pak *pak_open(const char *fname, const char *mode)
{
   Pak *p = NULL;
   RvR_rw *rw = NULL;

   if(mode[0]!='w'&&mode[0]!='r')
   {
      RvR_log("RvR_pak: invalid pak_open mode, must be either \"r\" or \"w\"\n");
      return NULL;
   }

   rw = RvR_malloc(sizeof(*rw));
   RvR_error_check(rw!=NULL,0x001);
   RvR_rw_init_path(rw,fname,mode[0]=='w'?"wb":mode[0]=='r'?"rb":"rb+");

   p = RvR_malloc(sizeof(*p));
   RvR_error_check(p!=NULL,0x001);
   memset(p,0,sizeof(*p));

   if(mode[0]=='r')
   {
      Pak_header header = {0};

      RvR_rw_read(rw,header.id,1,3);
      header.endian = RvR_rw_read_u8(rw);
      RvR_rw_endian(rw,header.endian);
      header.offset = RvR_rw_read_u32(rw);
      header.size = RvR_rw_read_u32(rw);

      if(memcmp(header.id,"PAK",3))
      {
         RvR_log("RvR_pak: pak header identifier mismatch, expected \"PAK\"\n");
         goto err;
      }

      unsigned num_files = header.size/64;

      if(RvR_rw_seek(rw,header.offset,SEEK_SET)!=0)
         goto err;

      p->count = num_files;
      p->entries = RvR_malloc(num_files*sizeof(Pak_file));

      for(unsigned i = 0;i<num_files;i++)
      {
         RvR_rw_read(rw,p->entries[i].name,1,56);
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
      uint8_t header[12] = {0};
      RvR_rw_write(rw,header,sizeof(header),1);

      return p;
   }

RvR_err:

   RvR_log("RvR error %s\n",RvR_error_get_string());

err:

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
   //index meta
   unsigned index = p->count++;
   p->entries = RvR_realloc(p->entries, p->count*sizeof(Pak_file));
   Pak_file *e = &p->entries[index];
   memset(e,0,sizeof(*e));
   snprintf(e->name,55,"%s",filename); //TODO: verify 56 chars limit
   e->offset = RvR_rw_tell(p->out);

   char buf[1<<15];
   while(!feof(in)&&!ferror(in))
   {
      size_t bytes = fread(buf,1,sizeof(buf),in);
      RvR_rw_write(p->out,buf,1,bytes);
   }

   e->size = RvR_rw_tell(p->out) - e->offset;
}

//pak.c END
//-------------------------------------

#undef CUTE_PATH_MAX_PATH
#undef CUTE_PATH_MAX_EXT
#undef JSON5_ASSERT
