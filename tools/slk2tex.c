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
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define READ_ARG(I) \
   ((++(I))<argc?argv[(I)]:NULL)
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
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   const char *path_in = NULL;
   const char *path_out = NULL;

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

   SLK_Pal_sprite *sp = SLK_pal_sprite_load(path_in);

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
          "   -wall\tflag sprite as wall texture\n"
          "   -sprite\tflag sprite as sprite texture\n",
         argv[0],argv[0]);
}

static void util_mem_compress(void *mem, int32_t length, FILE *out)
{
   char *buffer_out = malloc(length+1);

   fwrite(&length,4,1,out);
   int32_t size = crush_encode(mem,length,buffer_out,length,9);
   fwrite(buffer_out,size,1,out);

   free(buffer_out);
}
//-------------------------------------
