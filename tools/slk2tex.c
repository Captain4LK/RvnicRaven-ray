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
#include <string.h>
#include <SLK/SLK.h>

#define CRUSH_C
#include "../external/crush.c"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

#define CUTE_PATH_IMPLEMENTATION
#include "../external/cute_path.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define READ_ARG(I) \
   ((++(I))<argc?argv[(I)]:NULL)

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
//-------------------------------------

//Typedefs
typedef enum
{
   SPRITE_NONE,
   SPRITE_WALL = 1,
   SPRITE_SPRITE = 2,
}Sprite_flag;
//-------------------------------------

//Variables
static uint64_t flag = SPRITE_NONE;
//-------------------------------------

//Function prototypes
static void print_help(char **argv);
static void util_mem_compress(void *mem, int32_t length, FILE *out);
static SLK_Pal_sprite *texture_load(const char *path, const char *path_pal);
static SLK_Palette *palette_png(FILE *f);
static SLK_Palette *palette_gpl(FILE *f);
static SLK_Palette *palette_hex(FILE *f);
SLK_RGB_sprite *image_load(const char *path);
static int chartoi(char in);
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
   
   if(path_in==NULL)
   {
      printf("input texture not specified, try %s --help for more info\n",argv[0]);
      return 0;
   }
   if(path_out==NULL)
   {
      printf("output texture not specified, try %s --help for more info\n",argv[0]);
      return 0;
   }

   SLK_Pal_sprite *sp = texture_load(path_in,path_pal);

   if(flag&SPRITE_WALL)
   {
      SLK_Pal_sprite *csp = SLK_pal_sprite_create(sp->width,sp->height);
      SLK_pal_sprite_copy(csp,sp);

      for(int x = 0;x<sp->width;x++)
         for(int y = 0;y<sp->height;y++)
            sp->data[x*sp->height+y] = csp->data[y*csp->width+x];

      SLK_pal_sprite_destroy(csp);
   }
   if(flag&SPRITE_SPRITE)
   {
      SLK_Pal_sprite *csp = SLK_pal_sprite_create(sp->width,sp->height);
      SLK_pal_sprite_copy(csp,sp);

      for(int x = 0;x<sp->width;x++)
         for(int y = 0;y<sp->height;y++)
            sp->data[x*sp->height+y] = csp->data[y*csp->width+x];

      SLK_pal_sprite_destroy(csp);
   }

   int len = sizeof(uint8_t)*sp->width*sp->height+2*sizeof(int32_t);
   uint8_t *mem = malloc(len);

   *(((int32_t *)mem)) = sp->width;
   *(((int32_t *)mem)+1) = sp->height;
   for(int i = 0;i<sp->width*sp->height;i++)
      mem[2*sizeof(int32_t)+i] = sp->data[i];

   FILE *f = fopen(path_out,"wb");
   util_mem_compress(mem,len,f);
   fclose(f);

   SLK_pal_sprite_destroy(sp);
   free(mem);

   return 0;
}

static void print_help(char **argv)
{
   printf("%s usage:\n"
          "%s -fin filename -fout filename ]\n"
          "   -fin\t\tinput texture path\n"
          "   -fout\toutput texture path\n"
          "   -pal\t\tpalette to use for assigning indices, only for non .slk files\n"
          "   -wall\tflag sprite as wall texture\n"
          "   -sprite\tflag sprite as sprite texture\n",
         argv[0],argv[0]);
}

static void util_mem_compress(void *mem, int32_t length, FILE *out)
{
   char *buffer_out = malloc(length+1);
   uint8_t endian = 0;

   fwrite(&length,4,1,out);
   fwrite(&endian,1,1,out);
   int32_t size = crush_encode(mem,length,buffer_out,length,9);
   fwrite(buffer_out,size,1,out);

   free(buffer_out);
}

static SLK_Pal_sprite *texture_load(const char *path, const char *path_pal)
{
   char ext[33] = {0};
   path_pop_ext(path,NULL,ext);
   
   if(strncmp(ext,"slk",32)==0)
      return SLK_pal_sprite_load(path);

   if(path_pal==NULL)
   {
      puts("No palette specified but needed for non .slk files");
      exit(-1);
   }

   path_pop_ext(path_pal,NULL,ext);
   SLK_Palette *pal = NULL;
   if(strncmp(ext,"pal",32)==0)
   {
      pal = SLK_palette_load(path_pal);
   }
   else if(strncmp(ext,"png",32)==0)
   {
      FILE *f = fopen(path_pal,"r");
      if(f!=NULL)
      {
         pal = palette_png(f);
         fclose(f);
      }
   }
   else if(strncmp(ext,"gpl",32)==0)
   {
      FILE *f = fopen(path_pal,"r");
      if(f!=NULL)
      {
         pal = palette_gpl(f);
         fclose(f);
      }
   }
   else if(strncmp(ext,"hex",32)==0)
   {
      FILE *f = fopen(path_pal,"r");
      if(f!=NULL)
      {
         pal = palette_hex(f);
         fclose(f);
      }
   }

   if(pal==NULL)
   {
      puts("Failed to load palette");
      exit(-1);
   }

   SLK_RGB_sprite *tex_in = image_load(path);
   SLK_Pal_sprite *tex_out = SLK_pal_sprite_create(tex_in->width,tex_in->height);

   for(int i = 0;i<tex_in->width*tex_in->height;i++)
   {
      int min_dist = INT_MAX;
      int min_index = 0;
      SLK_Color color = tex_in->data[i];
      if(color.a==0)
      {
         tex_out->data[i] = 0;
         continue;
      }

      for(int j = 0;j<pal->used;j++)
      {
         SLK_Color pal_color = pal->colors[j];
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

   free(pal);
   SLK_rgb_sprite_destroy(tex_in);

   return tex_out;
}

static SLK_Palette *palette_png(FILE *f)
{
   SLK_RGB_sprite *s = SLK_rgb_sprite_load_file(f);   
   SLK_Palette *p = malloc(sizeof(*p));
   if(!p)
      return NULL;
   memset(p,0,sizeof(*p));
   p->used = MIN(256,s->width*s->height);
   for(int i = 0;i<p->used;i++)
      p->colors[i] = s->data[i];
   SLK_rgb_sprite_destroy(s);

   return p;
}

static SLK_Palette *palette_gpl(FILE *f)
{
   SLK_Palette *p = malloc(sizeof(*p));
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
   p->used = c;

   return p;
}

static SLK_Palette *palette_hex(FILE *f)
{

   SLK_Palette *p = malloc(sizeof(*p));
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
   p->used= c;

   return p;
}

SLK_RGB_sprite *image_load(const char *path)
{
   unsigned char *data = NULL;
   int width = 1;
   int height = 1;
   SLK_RGB_sprite *out;

   data = stbi_load(path,&width,&height,NULL,4);
   if(data==NULL)
   {
      printf("Failed to load %s\n",path);
      return SLK_rgb_sprite_create(1,1);
   }

   out = SLK_rgb_sprite_create(width,height);
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
//-------------------------------------
