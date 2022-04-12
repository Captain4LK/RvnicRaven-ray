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
#include <inttypes.h>
#include "../../src/RvnicRaven.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"

#define CUTE_PATH_IMPLEMENTATION
#include "../../external/cute_path.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define READ_ARG(I) \
   ((++(I))<argc?argv[(I)]:NULL)

#define MIN(a,b) ((a)<(b)?(a):(b))
//-------------------------------------

//Typedefs
typedef enum
{
   SPRITE_NONE,
   SPRITE_WALL = 1,
   SPRITE_SPRITE = 2,
}Sprite_flag;

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

typedef struct
{
   int width;
   int height;
   uint8_t *data;
}Sprite_pal;
//-------------------------------------

//Variables
static uint64_t flag = SPRITE_NONE;
//-------------------------------------

//Function prototypes
static void print_help(char **argv);
static Sprite_pal *texture_load(const char *path, const char *path_pal);
static Palette *palette_pal(FILE *f);
static Palette *palette_png(FILE *f);
static Palette *palette_gpl(FILE *f);
static Palette *palette_hex(FILE *f);
static Sprite_rgb *image_load(FILE *f);
static int chartoi(char in);

static Sprite_rgb *sprite_rgb_create(int width, int height);
static Sprite_pal *sprite_pal_create(int width, int height);
static void sprite_rgb_destroy(Sprite_rgb *s);
static void sprite_pal_destroy(Sprite_pal *s);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   const char *path_in = NULL;
   const char *path_out = NULL;
   const char *path_pal = NULL;

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
      else if(strcmp(argv[i],"-pal")==0)
         path_pal = READ_ARG(i);
      else if(strcmp(argv[i],"-wall")==0)
         flag|=SPRITE_WALL;
      else if(strcmp(argv[i],"-sprite")==0)
         flag|=SPRITE_SPRITE;
   }

   FILE *fp = fopen("out.rvr","wb");
   RvR_compress(fopen("../../../lovecraft.txt","rb"),fp);
   fclose(fp);
   int32_t clen;
   uint8_t cendian;
   char *cmem = (RvR_decompress(fopen("out.rvr","rb"),&clen,&cendian));
   for(int i = 0;i<clen;i++)
      fputc(cmem[i],stdout);
   printf("%d\n",clen);
   puts("");
   puts("------------------");
   
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

   Sprite_pal *sp = texture_load(path_in,path_pal);

   if(flag&SPRITE_WALL)
   {
      Sprite_pal *csp = sprite_pal_create(sp->width,sp->height);
      memcpy(csp->data,sp->data,sizeof(*sp->data)*sp->width*sp->height);

      for(int x = 0;x<sp->width;x++)
         for(int y = 0;y<sp->height;y++)
            sp->data[x*sp->height+y] = csp->data[y*csp->width+x];

      sprite_pal_destroy(csp);
   }
   if(flag&SPRITE_SPRITE)
   {
      Sprite_pal *csp = sprite_pal_create(sp->width,sp->height);
      memcpy(csp->data,sp->data,sizeof(*sp->data)*sp->width*sp->height);

      for(int x = 0;x<sp->width;x++)
         for(int y = 0;y<sp->height;y++)
            sp->data[x*sp->height+y] = csp->data[y*csp->width+x];

      sprite_pal_destroy(csp);
   }

   int len = sizeof(uint8_t)*sp->width*sp->height+2*sizeof(int32_t);
   uint8_t *mem = RvR_malloc(len);
   RvR_rw rw;
   RvR_rw_init_mem(&rw,mem,len,len);
   RvR_rw_write_i32(&rw,sp->width);
   RvR_rw_write_i32(&rw,sp->height);
   for(int i = 0;i<sp->width*sp->height;i++)
      RvR_rw_write_u8(&rw,sp->data[i]);
   RvR_rw_close(&rw);

   FILE *f = fopen(path_out,"wb");
   RvR_mem_compress(mem,len,f);
   fclose(f);

   sprite_pal_destroy(sp);
   RvR_free(mem);

   return 0;
}

static void print_help(char **argv)
{
   RvR_log("%s usage:\n"
          "%s -fin filename -fout filename ]\n"
          "   -fin        input texture path\n"
          "   -fout       output texture path\n"
          "   -pal        palette to use for assigning indices (.pal,.png,.hex,.gpl\n"
          "   -wall       flag sprite as wall texture\n"
          "   -sprite     flag sprite as sprite texture\n",
         argv[0],argv[0]);
}

static Sprite_pal *texture_load(const char *path, const char *path_pal)
{
   char ext[33] = {0};
   path_pop_ext(path,NULL,ext);
   
   if(path_pal==NULL)
   {
      RvR_log("No palette specified but needed for converting textures\n");
      exit(-1);
   }

   path_pop_ext(path_pal,NULL,ext);
   Palette *pal = NULL;
   FILE *fpal = fopen(path_pal,"r");
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

   if(pal==NULL)
   {
      RvR_log("Failed to load palette\n");
      exit(-1);
   }

   FILE *ftex_in = fopen(path,"rb");
   Sprite_rgb *tex_in = image_load(ftex_in);
   if(ftex_in!=NULL)
      fclose(ftex_in);
   Sprite_pal *tex_out = sprite_pal_create(tex_in->width,tex_in->height);

   for(int i = 0;i<tex_in->width*tex_in->height;i++)
   {
      int min_dist = INT_MAX;
      int min_index = 0;
      Color color = tex_in->data[i];
      if(color.a==0)
      {
         tex_out->data[i] = 0;
         continue;
      }

      for(int j = 0;j<pal->colors_used;j++)
      {
         Color pal_color = pal->colors[j];
         int dist_r = pal_color.r-color.r;
         int dist_g = pal_color.g-color.g;
         int dist_b = pal_color.b-color.b;
         int dist = dist_r*dist_r+dist_g*dist_g+dist_b*dist_b;
         if(dist<min_dist)
         {
            min_dist = dist;
            min_index = j;
         }
      }
      tex_out->data[i] = min_index;
   }

   RvR_free(pal);
   sprite_rgb_destroy(tex_in);

   return tex_out;
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

static Sprite_pal *sprite_pal_create(int width, int height)
{
   Sprite_pal *s = RvR_malloc(sizeof(*s));
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

static void sprite_pal_destroy(Sprite_pal *s)
{
   if(s==NULL)
      return;
   if(s->data!=NULL)
      RvR_free(s->data);
   RvR_free(s);
}
//-------------------------------------
