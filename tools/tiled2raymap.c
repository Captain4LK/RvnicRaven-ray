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

#define CRUSH_C
#include "../external/crush.c"

#define HLH_JSON_IMPLEMENTATION
#include "../external/HLH_json.h"

#define CUTE_PATH_IMPLEMENTATION
#include "../external/cute_path.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define READ_ARG(I) \
   ((++(I))<argc?argv[(I)]:NULL)
//-------------------------------------

//Typedefs

typedef uint32_t fix22;
typedef struct
{
   fix22 x,y,z; 
}vec3;

typedef struct
{
   uint16_t type;
   vec3 pos;
   int32_t extra0;
   int32_t extra1;
   int32_t extra2;
   int32_t extra3;
}Sprite;
//-------------------------------------

//Variables
static uint16_t *level_wall_ftex = NULL;
static uint16_t *level_wall_ctex = NULL;
static uint16_t *level_floor_tex = NULL;
static uint16_t *level_ceil_tex = NULL;
static int8_t *level_floor = NULL;
static int8_t *level_ceiling = NULL;
static uint16_t level_width = 0;
static uint16_t level_height = 0;
static uint8_t level_floor_color = 23;
static uint16_t level_sky_tex = 0;
static uint32_t level_sprite_count = 0;
static Sprite *level_sprites = NULL;

static int offset_height = 0;
static int offset_texture = 0;
//-------------------------------------

//Function prototypes
static void map_write(const char *path);
static void util_mem_compress(void *mem, int32_t length, FILE *out);
static void print_help(char **argv);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   const char *path_project = NULL;
   const char *path_out = NULL;

   for(int i = 1;i<argc;i++)
   {
      if(strcmp(argv[i],"--help")==0||
         strcmp(argv[i],"-help")==0||
         strcmp(argv[i],"-h")==0||
         strcmp(argv[i],"?")==0)
      { print_help(argv); return 0; }
      else if(strcmp(argv[i],"-fin")==0)
         path_project = READ_ARG(i);
      else if(strcmp(argv[i],"-fout")==0)
         path_out = READ_ARG(i);
   }

   if(path_project==NULL)
   {
      printf("project file not specified, try %s --help for more info\n",argv[0]);
      return 0;
   }

   HLH_json5_root *json = HLH_json_parse_file(path_project);

   //Read tileset offsets
   HLH_json5 *tilesets = HLH_json_get_object(&json->root,"tilesets");
   for(int i = 0;i<HLH_json_get_array_size(tilesets);i++)
   {
      HLH_json5 *tileset = HLH_json_get_array_item(tilesets,i);
      char path_tmp[512];

      //Read path
      path_pop(path_project,path_tmp,NULL);
      path_concat(path_tmp,HLH_json_get_object_string(tileset,"source","(NULL)"),path_tmp,512);

      //Open tileset json file
      HLH_json5_root *json_tileset = HLH_json_parse_file(path_tmp);

      //Read name and assign offset accordingly
      const char *name = HLH_json_get_object_string(&json_tileset->root,"name","(NULL)");
      if(strcmp(name,"heightmap")==0)
         offset_height = HLH_json_get_object_integer(tileset,"firstgid",0);
      if(strcmp(name,"textures")==0)
         offset_texture = HLH_json_get_object_integer(tileset,"firstgid",0);

      HLH_json_free(json_tileset);
   }

   //Read map dimensions
   level_width = HLH_json_get_object_integer(&json->root,"width",1);
   level_height = HLH_json_get_object_integer(&json->root,"height",1);

   //Read properties
   HLH_json5 *properties = HLH_json_get_object(&json->root,"properties");
   for(int i = 0;i<HLH_json_get_array_size(properties);i++)
   {
      HLH_json5 *property = HLH_json_get_array_item(properties,i);

      const char *name = HLH_json_get_object_string(property,"name","(NULL)");
      if(strcmp(name,"sky_tex")==0)
         level_sky_tex = HLH_json_get_object_integer(property,"value",0);
      if(strcmp(name,"floor_color")==0)
         level_floor_color = HLH_json_get_object_integer(property,"value",0);
   }

   //Allocate memory for map data
   level_wall_ftex = calloc(level_width*level_height,sizeof(*level_wall_ftex));
   level_wall_ctex = calloc(level_width*level_height,sizeof(*level_wall_ctex));
   level_floor_tex = calloc(level_width*level_height,sizeof(*level_floor_tex));
   level_ceil_tex = calloc(level_width*level_height,sizeof(*level_ceil_tex));
   level_floor = calloc(level_width*level_height,sizeof(*level_floor));
   level_ceiling = calloc(level_width*level_height,sizeof(*level_ceiling));

   //Read layers
   HLH_json5 *layers = HLH_json_get_object(&json->root,"layers");
   HLH_json5 *layer_entity = NULL;
   for(int i = 0;i<HLH_json_get_array_size(layers);i++)
   {
      HLH_json5 *layer = HLH_json_get_array_item(layers,i);
      HLH_json5 *array = HLH_json_get_object(layer,"data");

      const char *name = HLH_json_get_object_string(layer,"name","(NULL)");

      if(strcmp(name,"wall_ctex")==0)
         for(int j = 0;j<level_width*level_height;j++)
            level_wall_ctex[j] = HLH_json_get_array_integer(array,j,0)-offset_texture;

      if(strcmp(name,"wall_ftex")==0)
         for(int j = 0;j<level_width*level_height;j++)
            level_wall_ftex[j] = HLH_json_get_array_integer(array,j,0)-offset_texture;

      if(strcmp(name,"floor_tex")==0)
         for(int j = 0;j<level_width*level_height;j++)
            level_floor_tex[j] = HLH_json_get_array_integer(array,j,0)-offset_texture;
      
      if(strcmp(name,"ceil_tex")==0)
         for(int j = 0;j<level_width*level_height;j++)
            level_ceil_tex[j] = HLH_json_get_array_integer(array,j,0)-offset_texture;

      if(strcmp(name,"ceiling_height")==0)
         for(int j = 0;j<level_width*level_height;j++)
            level_ceiling[j] = HLH_json_get_array_integer(array,j,0)-offset_height-128;

      if(strcmp(name,"floor_height")==0)
         for(int j = 0;j<level_width*level_height;j++)
            level_floor[j] = HLH_json_get_array_integer(array,j,0)-offset_height-128;

      if(strcmp(name,"entities")==0)
         layer_entity = HLH_json_get_object(layer,"objects");
   }

   //Read sprites
   level_sprite_count = HLH_json_get_array_size(layer_entity);
   level_sprites = calloc(sizeof(*level_sprites),level_sprite_count);
   for(int i = 0;i<HLH_json_get_array_size(layer_entity);i++)
   {
      HLH_json5 *entity = HLH_json_get_array_item(layer_entity,i);
      HLH_json5 ob = {0};
      HLH_json5 *ent_properties = HLH_json_get_object_array(entity,"properties",&ob);
      level_sprites[i].pos.x = (HLH_json_get_object_integer(entity,"x",0)/64)*1024+512;
      level_sprites[i].pos.y = (HLH_json_get_object_integer(entity,"y",0)/64)*1024+512;

      for(int p = 0;p<HLH_json_get_array_size(ent_properties);p++)
      {
         HLH_json5 *ent_prop = HLH_json_get_array_item(ent_properties,p);
         const char *name = HLH_json_get_object_string(ent_prop,"name","(NULL)");

         if(strcmp(name,"z")==0)
            level_sprites[i].pos.z = HLH_json_get_object_integer(ent_prop,"value",0);
         if(strcmp(name,"type")==0)
            level_sprites[i].type = HLH_json_get_object_integer(ent_prop,"value",0);
         if(strcmp(name,"extra0")==0)
            level_sprites[i].extra0 = HLH_json_get_object_integer(ent_prop,"value",0);
         if(strcmp(name,"extra1")==0)
            level_sprites[i].extra1 = HLH_json_get_object_integer(ent_prop,"value",0);
         if(strcmp(name,"extra2")==0)
            level_sprites[i].extra2 = HLH_json_get_object_integer(ent_prop,"value",0);
         if(strcmp(name,"extra3")==0)
            level_sprites[i].extra3 = HLH_json_get_object_integer(ent_prop,"value",0);
      }
   }

   //Write map file
   if(path_out==NULL)
      map_write("map_out.rvr");
   else
      map_write(path_out);

   free(level_wall_ftex);
   free(level_wall_ctex);
   free(level_floor_tex);
   free(level_ceil_tex);
   free(level_floor);
   free(level_ceiling);
   free(level_sprites);

   HLH_json_free(json);

   return 0;
}

static void map_write(const char *path)
{
   //Calculate needed memory
   int size = 0;
   size+=2; //level_width
   size+=2; //level_height
   size+=4; //level_height
   size+=4; //level_sprite_count
   size+=1; //level_floor_color
   size+=2; //level_sky_tex
   size+=level_width*level_height*2; //level_fwall_tex
   size+=level_width*level_height*2; //level_cwall_tex
   size+=level_width*level_height*2; //level_floor_tex
   size+=level_width*level_height*2; //level_ceil_tex
   size+=level_width*level_height; //level_floor
   size+=level_width*level_height; //level_ceiling
   size+=level_sprite_count*sizeof(Sprite); //level_sprites

   uint8_t *mem = malloc(size);
   int pos = 0;

   //Write width and height
   *(uint16_t *)(mem+pos) = level_width; pos+=2;
   *(uint16_t *)(mem+pos) = level_height; pos+=2;

   //Write sprite count
   *(uint32_t *)(mem+pos) = level_sprite_count; pos+=4;

   //Write floor color
   *(uint8_t *)(mem+pos) = level_floor_color; pos+=1;

   //Write sky texture
   *(uint16_t *)(mem+pos) = level_sky_tex; pos+=2;

   //Write texture, floor and ceiling
   memcpy(mem+pos,level_wall_ftex,level_width*level_height*2); pos+=level_width*level_height*2;
   memcpy(mem+pos,level_wall_ctex,level_width*level_height*2); pos+=level_width*level_height*2;
   memcpy(mem+pos,level_floor_tex,level_width*level_height*2); pos+=level_width*level_height*2;
   memcpy(mem+pos,level_ceil_tex,level_width*level_height*2); pos+=level_width*level_height*2;
   memcpy(mem+pos,level_floor,level_width*level_height); pos+=level_width*level_height;
   memcpy(mem+pos,level_ceiling,level_width*level_height); pos+=level_width*level_height;

   //Write sprites
   for(int i = 0;i<level_sprite_count;i++)
   {
      *(uint16_t *)(mem+pos) = level_sprites[i].type; pos+=2;
      *(int32_t *)(mem+pos) = level_sprites[i].pos.x; pos+=4;
      *(int32_t *)(mem+pos) = level_sprites[i].pos.y; pos+=4;
      *(int32_t *)(mem+pos) = level_sprites[i].pos.z; pos+=4;
      *(int32_t *)(mem+pos) = level_sprites[i].extra0; pos+=4;
      *(int32_t *)(mem+pos) = level_sprites[i].extra1; pos+=4;
      *(int32_t *)(mem+pos) = level_sprites[i].extra2; pos+=4;
      *(int32_t *)(mem+pos) = level_sprites[i].extra3; pos+=4;
   }

   //Compress and write to disk
   FILE *f = fopen(path,"wb");
   util_mem_compress(mem,size,f);
   fclose(f);

   //Free temp buffer
   free(mem);
}

static void util_mem_compress(void *mem, int32_t length, FILE *out)
{
   char *buffer_out = malloc(length+1);

   fwrite(&length,4,1,out);
   int32_t size = crush_encode(mem,length,buffer_out,length,9);
   fwrite(buffer_out,size,1,out);

   free(buffer_out);
}

static void print_help(char **argv)
{
   printf("%s usage:\n"
          "%s -fin filename -fout filename ]\n"
          "   -fin\t\tpath to project file\n"
          "   -fout\tpath to save map to\n",
         argv[0],argv[0]);
}
//-------------------------------------
