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
#include "../../src/RvnicRaven.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"

#define CUTE_PATH_IMPLEMENTATION
#include "../../external/cute_path.h"

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "../../external/optparse.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define MIN(a,b) ((a)<(b)?(a):(b))
//-------------------------------------

//Typedefs
typedef struct
{
   uint8_t r,g,b,a;
}Color;

typedef struct
{
   int colors_used;
   Color colors[256];
}Palette;

typedef struct
{
   int width;
   int height;
   Color *data;
}Sprite_rgb;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void print_help(char **argv);
static Palette *palette_pal(FILE *f);
static Palette *palette_png(FILE *f);
static Palette *palette_gpl(FILE *f);
static Palette *palette_hex(FILE *f);
static Sprite_rgb *image_load(FILE *f);
static int chartoi(char in);

static Sprite_rgb *sprite_rgb_create(int width, int height);
static void sprite_rgb_destroy(Sprite_rgb *s);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   struct optparse_long longopts[] = 
   {
      {"in",'i',OPTPARSE_REQUIRED},
      {"out",'o',OPTPARSE_REQUIRED},
      {"help",'h',OPTPARSE_NONE},
      {0},
   };

   const char *path_in = NULL;
   const char *path_out = NULL;

   int option;
   struct optparse options;
   optparse_init(&options,argv);
   while((option = optparse_long(&options,longopts,NULL))!=-1)
   {
      switch(option)
      {
      case 'h':
         print_help(argv);
         exit(EXIT_SUCCESS);
      case 'i':
         path_in = options.optarg;
         break;
      case 'o':
         path_out = options.optarg;
         break;
      case '?':
         fprintf(stderr,"%s: %s\n",argv[0],options.errmsg);
         exit(EXIT_FAILURE);
         break;
      }
   }
   
   if(path_in==NULL)
   {
      RvR_log("input palette not specified, try %s -h for more info\n",argv[0]);
      return 0;
   }
   if(path_out==NULL)
   {
      RvR_log("output palette not specified, try %s -h for more info\n",argv[0]);
      return 0;
   }

   char ext[33] = {0};
   path_pop_ext(path_in,NULL,ext);
   Palette *pal = NULL;
   FILE *fpal = fopen(path_in,"r");
   if(fpal!=NULL)
   {
      if(strncmp(ext,"pal",32)==0)
         pal = palette_pal(fpal);
      else if(strncmp(ext,"png",32)==0)
         pal = palette_png(fpal);
      else if(strncmp(ext,"gpl",32)==0)
         pal = palette_gpl(fpal);
      else if(strncmp(ext,"hex",32)==0)
         pal = palette_hex(fpal);
      fclose(fpal);
   }

   RvR_rw rw;
   RvR_rw_init_path(&rw,path_out,"wb");
   for(int i = 0;i<256;i++)
   {
      RvR_rw_write_u8(&rw,pal->colors[i].r);
      RvR_rw_write_u8(&rw,pal->colors[i].g);
      RvR_rw_write_u8(&rw,pal->colors[i].b);
   }
   RvR_rw_close(&rw);

   RvR_free(pal);

   return 0;
}

static void print_help(char **argv)
{
   RvR_log("%s usage:\n"
          "%s --in filename --out filename ]\n"
          "   -i, --in        input palette path\n"
          "   -o, --out       output palette path\n",
         argv[0],argv[0]);
}

static Palette *palette_pal(FILE *f)
{
   Palette *p = RvR_malloc(sizeof(*p));

   fscanf(f,"JASC-PAL\n0100\n%d\n",&p->colors_used);
   for(int i = 0;i<p->colors_used;i++)
      fscanf(f,"%"SCNu8 "%"SCNu8 "%"SCNu8 "\n",&p->colors[i].r,&p->colors[i].g,&p->colors[i].b);
   return p;
}

static Palette *palette_png(FILE *f)
{
   Sprite_rgb *s = image_load(f);   
   Palette *p = RvR_malloc(sizeof(*p));
   memset(p,0,sizeof(*p));
   p->colors_used = MIN(256,s->width*s->height);
   for(int i = 0;i<p->colors_used;i++)
      p->colors[i] = s->data[i];
   sprite_rgb_destroy(s);

   return p;
}

static Palette *palette_gpl(FILE *f)
{
   Palette *p = RvR_malloc(sizeof(*p));
   if(!p)
      return NULL;
   memset(p,0,sizeof(*p));
   char buffer[512];
   int c = 0;
   int r,g,b;

   while(fgets(buffer,512,f))
   {
      if(buffer[0]=='#')
         continue;
      if(sscanf(buffer,"%d %d %d",&r,&g,&b)==3)
      {
         p->colors[c].r = r;
         p->colors[c].g = g;
         p->colors[c].b = b;
         p->colors[c].a = 255;
         c++;
      }
   }
   p->colors_used = c;

   return p;
}

static Palette *palette_hex(FILE *f)
{

   Palette *p = RvR_malloc(sizeof(*p));
   if(!p)
      return NULL;
   memset(p,0,sizeof(*p));
   char buffer[512];
   int c = 0;

   while(fgets(buffer,512,f))
   {
      p->colors[c].r = chartoi(buffer[0])*16+chartoi(buffer[1]);
      p->colors[c].g = chartoi(buffer[2])*16+chartoi(buffer[3]);
      p->colors[c].b = chartoi(buffer[4])*16+chartoi(buffer[5]);
      p->colors[c].a = 255;
      c++;
   }
   p->colors_used= c;

   return p;
}

static Sprite_rgb *image_load(FILE *f)
{
   unsigned char *data = NULL;
   int width = 1;
   int height = 1;
   Sprite_rgb *out;

   data = stbi_load_from_file(f,&width,&height,NULL,4);
   if(data==NULL)
   {
      RvR_log("Failed to load imgage\n");
      return sprite_rgb_create(1,1);
   }

   out = sprite_rgb_create(width,height);
   memcpy(out->data,data,width*height*sizeof(*out->data));

   stbi_image_free(data);

   return out;
}

//Helper function for palette_hex
static int chartoi(char in)
{
   if(in>='0'&&in<='9')
      return in-'0';
   if(in>='a'&&in<='f')
      return in-'a'+10;
   if(in>='A'&&in<='F')
      return in-'A'+10;
   return 0;
}

static Sprite_rgb *sprite_rgb_create(int width, int height)
{
   Sprite_rgb *s = RvR_malloc(sizeof(*s));
   s->width = width;
   s->height = height;
   s->data = RvR_malloc(sizeof(*s->data)*s->width*s->height);
   return s;
}

static void sprite_rgb_destroy(Sprite_rgb *s)
{
   if(s==NULL)
      return;
   if(s->data!=NULL)
      RvR_free(s->data);
   RvR_free(s);
}
//-------------------------------------
