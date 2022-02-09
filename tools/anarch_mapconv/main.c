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
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define READ_ARG(I) \
   ((++(I))<argc?argv[(I)]:NULL)

#define MIN(a,b) ((a)<(b)?(a):(b))

#define SFG_MAP_SIZE 64
#define SFG_TILE_DICTIONARY_SIZE 64

/**
  Defines a single game map tile. The format is following:

    MSB aaabbbbb cccddddd LSB

   aaa:   ceiling texture index (from texture available on the map), 111
          means completely transparent texture
   bbbbb: ceiling height (1111 meaning no ceiling) ABOVE the floor
   ccc:   floor texture index, 111 means completely transparent texture
   ddddd: floor height
*/
typedef uint16_t SFG_TileDefinition;

#define SFG_TILE_CEILING_MAX_HEIGHT 31
#define SFG_TILE_TEXTURE_TRANSPARENT 7

typedef SFG_TileDefinition SFG_TileDictionary[SFG_TILE_DICTIONARY_SIZE];

/// helper macros for SFG_TileDefinition
#define SFG_TD(floorH, ceilH, floorT, ceilT)\
  ((floorH & 0x001f) |\
   ((floorT & 0x0007) << 5) |\
   ((ceilH & 0x001f) << 8) |\
   ((ceilT & 0x0007) << 13))

#define SFG_TILE_FLOOR_HEIGHT(tile) (tile & 0x1f)
#define SFG_TILE_FLOOR_TEXTURE(tile) ((tile & 0xe0) >> 5)
#define SFG_TILE_CEILING_HEIGHT(tile) (((tile) & 0x1f00) >> 8)
#define SFG_TILE_CEILING_TEXTURE(tile) ((tile & 0xe000) >> 13)

#define SFG_OUTSIDE_TILE SFG_TD(63,0,7,7)

/**
  Game map represented as a 2D array. Array item has this format:

    MSB aabbbbbb LSB

    aa:     type of square, possible values:
      00:   normal
      01:   moving floor (elevator), moves from floor height to ceililing height
            (there is no ceiling above)
      10:   moving ceiling, moves from ceiling height to floor height
      11:   door
    bbbbbb: index into tile dictionary
*/
typedef uint8_t SFG_MapArray[SFG_MAP_SIZE * SFG_MAP_SIZE];

#define SFG_TILE_PROPERTY_MASK 0xc0
#define SFG_TILE_PROPERTY_NORMAL 0x00
#define SFG_TILE_PROPERTY_ELEVATOR 0x40
#define SFG_TILE_PROPERTY_SQUEEZER 0x80
#define SFG_TILE_PROPERTY_DOOR 0xc0

/**
  Serves to place elements (items, enemies etc.) into the game level.
*/
typedef struct
{
  uint8_t type;
  uint8_t coords[2];
} SFG_LevelElement;

#define SFG_MAX_LEVEL_ELEMENTS 128

/*
  Definitions of level element type. These values must leave the highest bit
  unused because that will be used by the game engine, so the values must be
  lower than 128.
*/
#define SFG_LEVEL_ELEMENT_NONE 0
#define SFG_LEVEL_ELEMENT_BARREL 0x01
#define SFG_LEVEL_ELEMENT_HEALTH 0x02
#define SFG_LEVEL_ELEMENT_BULLETS 0x03
#define SFG_LEVEL_ELEMENT_ROCKETS 0x04
#define SFG_LEVEL_ELEMENT_PLASMA 0x05
#define SFG_LEVEL_ELEMENT_TREE 0x06
#define SFG_LEVEL_ELEMENT_FINISH 0x07
#define SFG_LEVEL_ELEMENT_TELEPORTER 0x08
#define SFG_LEVEL_ELEMENT_TERMINAL 0x09
#define SFG_LEVEL_ELEMENT_COLUMN 0x0a
#define SFG_LEVEL_ELEMENT_RUIN 0x0b
#define SFG_LEVEL_ELEMENT_LAMP 0x0c
#define SFG_LEVEL_ELEMENT_CARD0 0x0d ///< Access card, unlocks doors with lock.
#define SFG_LEVEL_ELEMENT_CARD1 0x0e
#define SFG_LEVEL_ELEMENT_CARD2 0x0f
#define SFG_LEVEL_ELEMENT_LOCK0 0x10 /**< Special level element that must be
                                     placed on a tile with door. This door is
                                     then locked until taking the corresponding
                                     access card. */
#define SFG_LEVEL_ELEMENT_LOCK1 0x11
#define SFG_LEVEL_ELEMENT_LOCK2 0x12
#define SFG_LEVEL_ELEMENT_BLOCKER 0x13 ///< invisible wall

#define SFG_LEVEL_ELEMENT_MONSTER_SPIDER 0x20
#define SFG_LEVEL_ELEMENT_MONSTER_DESTROYER 0x21
#define SFG_LEVEL_ELEMENT_MONSTER_WARRIOR 0x22
#define SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT 0x23
#define SFG_LEVEL_ELEMENT_MONSTER_ENDER 0x24
#define SFG_LEVEL_ELEMENT_MONSTER_TURRET 0x25
#define SFG_LEVEL_ELEMENT_MONSTER_EXPLODER 0x26

#define SFG_MONSTERS_TOTAL 7

#define SFG_MONSTER_TYPE_TO_INDEX(monsterType) \
  ((monsterType) - SFG_LEVEL_ELEMENT_MONSTER_SPIDER)

#define SFG_MONSTER_INDEX_TO_TYPE(monsterIndex) \
  ((monsterIndex) + SFG_LEVEL_ELEMENT_MONSTER_SPIDER)

#define SFG_LEVEL_ELEMENT_TYPE_IS_MOSTER(t) \
  ((t) >= SFG_LEVEL_ELEMENT_MONSTER_SPIDER)

typedef struct
{
  SFG_MapArray mapArray;
  SFG_TileDictionary tileDictionary;
  uint8_t textureIndices[7]; /**< Says which textures are used on the map. There
                             can be at most 7 because of 3bit indexing (one
                             value is reserved for special transparent
                             texture). */
  uint8_t doorTextureIndex; /**< Index (global, NOT from textureIndices) of a
                             texture used for door. */
  uint8_t floorColor;
  uint8_t ceilingColor;
  uint8_t playerStart[3];   /**< Player starting location: square X, square Y,
                                 direction (fourths of RCL_Unit). */
  uint8_t backgroundImage;  ///< Index of level background image.
  SFG_LevelElement elements[SFG_MAX_LEVEL_ELEMENTS];
} SFG_Level;

static inline SFG_TileDefinition SFG_getMapTile
(
  const SFG_Level *level,
  int16_t x,
  int16_t y,
  uint8_t *properties
)
{
  if (x >= 0 && x < SFG_MAP_SIZE && y >= 0 && y < SFG_MAP_SIZE)
  {
    uint8_t tile = level->mapArray[y * SFG_MAP_SIZE + x];

    *properties = tile & 0xc0;
    return level->tileDictionary[tile & 0x3f];
  }

  *properties = SFG_TILE_PROPERTY_NORMAL;
  return SFG_OUTSIDE_TILE;
}

#define SFG_NUMBER_OF_LEVELS 10
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
SFG_Level level = 
#include "level.h"
;
//-------------------------------------

//Function prototypes
static void print_help(char **argv);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   const char *path_out = NULL;

   for(int i = 1;i<argc;i++)
   {
      if(strcmp(argv[i],"--help")==0||
         strcmp(argv[i],"-help")==0||
         strcmp(argv[i],"-h")==0||
         strcmp(argv[i],"?")==0)
      { print_help(argv); return 0; }
      else if(strcmp(argv[i],"-fout")==0)
         path_out = READ_ARG(i);
   }
   
   if(path_out==NULL)
   {
      RvR_log("output map path not specified, try %s -h for more info\n",argv[0]);
      return 0;
   }

   RvR_ray_map_create(64,64);
   RvR_ray_map_cache *map = RvR_ray_map_cache_get();
   map->sky_tex = level.backgroundImage;

   //Read tiles
   //TODO: elevators/doors/squeezer
   for(int y = 0;y<64;y++)
   {
      for(int x = 0;x<64;x++)
      {
         //Read tile data
         uint8_t map_entry = level.mapArray[y*64+x];
         uint8_t type = map_entry&SFG_TILE_PROPERTY_MASK;
         map_entry = map_entry&(~SFG_TILE_PROPERTY_MASK);
         uint16_t tile_dict = level.tileDictionary[map_entry];
         int8_t floor_height = SFG_TILE_FLOOR_HEIGHT(tile_dict);
         int8_t ceiling_height = SFG_TILE_CEILING_HEIGHT(tile_dict);
         uint16_t floor_tex = SFG_TILE_FLOOR_TEXTURE(tile_dict);
         uint16_t ceiling_tex = SFG_TILE_CEILING_TEXTURE(tile_dict);

         //Geometry and Textures
         map->floor[y*64+x] = 2*floor_height;
         if(ceiling_height==31)
            map->ceiling[y*64+x] = INT8_MAX;
         else
            map->ceiling[y*64+x] = 2*floor_height+2*ceiling_height;
         if(floor_tex==7)
         {
            map->wall_ftex[y*64+x] = level.backgroundImage;
            map->floor_tex[y*64+x] = level.backgroundImage;
         }
         else
         {
            map->wall_ftex[y*64+x] = level.textureIndices[floor_tex]+3;
            map->floor_tex[y*64+x] = level.textureIndices[floor_tex]+3;
         }
         if(ceiling_height==31)
         {
            map->wall_ctex[y*64+x] = level.backgroundImage;
            map->ceil_tex[y*64+x] = level.backgroundImage;
         }
         else if(ceiling_tex==7)
         {
            map->wall_ctex[y*64+x] = level.backgroundImage;
            map->ceil_tex[y*64+x] = level.textureIndices[ceiling_tex]+3;
         }
         else
         {
            map->wall_ctex[y*64+x] = level.textureIndices[ceiling_tex]+3;
            map->ceil_tex[y*64+x] = level.textureIndices[ceiling_tex]+3;
         }

         //Type
         if(type==SFG_TILE_PROPERTY_ELEVATOR)
         {
            int index = map->sprite_count++;
            map->sprites = RvR_realloc(map->sprites,sizeof(*map->sprites)*map->sprite_count);
            map->sprites[index].type = 2050;
            map->sprites[index].pos.x = x*1024+512;
            map->sprites[index].pos.y = y*1024+512;
            map->sprites[index].extra0 = map->floor[y*64+x]*128;
            map->sprites[index].extra1 = map->ceiling[y*64+x]*128;

            map->wall_ctex[y*64+x] = level.backgroundImage;
            map->ceil_tex[y*64+x] = level.backgroundImage;
            map->ceiling[y*64+x] = INT8_MAX;
         }
         else if(type==SFG_TILE_PROPERTY_SQUEEZER)
         {
            int index = map->sprite_count++;
            map->sprites = RvR_realloc(map->sprites,sizeof(*map->sprites)*map->sprite_count);
            map->sprites[index].type = 2051;
            map->sprites[index].pos.x = x*1024+512;
            map->sprites[index].pos.y = y*1024+512;
            map->sprites[index].extra0 = map->ceiling[y*64+x]*128;
            map->sprites[index].extra1 = map->floor[y*64+x]*128;
         }
         else if(type==SFG_TILE_PROPERTY_DOOR)
         {
            int index = map->sprite_count++;
            map->sprites = RvR_realloc(map->sprites,sizeof(*map->sprites)*map->sprite_count);
            map->sprites[index].type = 2049;
            map->sprites[index].pos.x = x*1024+512;
            map->sprites[index].pos.y = y*1024+512;
            map->sprites[index].extra0 = map->floor[y*64+x]*128-1024;
            map->sprites[index].extra1 = map->ceiling[y*64+x]*128;
            map->wall_ftex[y*64+x] = level.doorTextureIndex+3;
            map->floor_tex[y*64+x] = level.doorTextureIndex+3;
         }
      }
   }

   //Entities
   for(int i = 0;i<SFG_MAX_LEVEL_ELEMENTS;i++)
   {
      if(level.elements[i].type!=SFG_LEVEL_ELEMENT_NONE)
      {
         int index = map->sprite_count++;
         map->sprites = RvR_realloc(map->sprites,sizeof(*map->sprites)*map->sprite_count);

         int pos_x = level.elements[i].coords[0];
         int pos_y = level.elements[i].coords[1];
         map->sprites[index].pos.x = pos_x*1024+512;
         map->sprites[index].pos.y = pos_y*1024+512;
         map->sprites[index].pos.z = map->floor[pos_y*64+pos_x]*128;

         switch(level.elements[i].type)
         {
         case SFG_LEVEL_ELEMENT_BARREL: map->sprites[index].type = 1028; break;
         case SFG_LEVEL_ELEMENT_HEALTH: map->sprites[index].type = 1033; break;
         case SFG_LEVEL_ELEMENT_BULLETS: map->sprites[index].type = 1029; break;
         case SFG_LEVEL_ELEMENT_ROCKETS: map->sprites[index].type = 1036; break;
         case SFG_LEVEL_ELEMENT_PLASMA: map->sprites[index].type = 1035; break;
         case SFG_LEVEL_ELEMENT_TREE: map->sprites[index].type = 1040; break;
         case SFG_LEVEL_ELEMENT_FINISH: map->sprites[index].type = 1032; break;
         case SFG_LEVEL_ELEMENT_TELEPORTER: map->sprites[index].type = 1038; break;
         case SFG_LEVEL_ELEMENT_TERMINAL: map->sprites[index].type = 1039; break;
         case SFG_LEVEL_ELEMENT_COLUMN: map->sprites[index].type = 1031; break;
         case SFG_LEVEL_ELEMENT_RUIN: map->sprites[index].type = 1037; break;
         case SFG_LEVEL_ELEMENT_LAMP: map->sprites[index].type = 1034; break;
         case SFG_LEVEL_ELEMENT_CARD0: map->sprites[index].type = 1030; map->sprites[index].extra0 = 1; break;
         case SFG_LEVEL_ELEMENT_CARD1: map->sprites[index].type = 1030; map->sprites[index].extra0 = 2; break;
         case SFG_LEVEL_ELEMENT_CARD2: map->sprites[index].type = 1030; map->sprites[index].extra0 = 4; break;
         case SFG_LEVEL_ELEMENT_LOCK0: map->sprites[index].type = 2049; map->sprites[index].extra2 = 1; break;
         case SFG_LEVEL_ELEMENT_LOCK1: map->sprites[index].type = 2049; map->sprites[index].extra2 = 2; break;
         case SFG_LEVEL_ELEMENT_LOCK2: map->sprites[index].type = 2049; map->sprites[index].extra2 = 4; break;
         case SFG_LEVEL_ELEMENT_BLOCKER: map->sprites[index].type = 2052; break;
         case SFG_LEVEL_ELEMENT_MONSTER_SPIDER: map->sprites[index].type = 1053; break;
         case SFG_LEVEL_ELEMENT_MONSTER_DESTROYER: map->sprites[index].type = 1043; break;
         case SFG_LEVEL_ELEMENT_MONSTER_WARRIOR: map->sprites[index].type = 1059; break;
         case SFG_LEVEL_ELEMENT_MONSTER_PLASMABOT: map->sprites[index].type = 1051; break;
         case SFG_LEVEL_ELEMENT_MONSTER_ENDER: map->sprites[index].type = 1047; break;
         case SFG_LEVEL_ELEMENT_MONSTER_TURRET: map->sprites[index].type = 1056; break;
         case SFG_LEVEL_ELEMENT_MONSTER_EXPLODER: map->sprites[index].type = 1050; break;
         default: map->sprites[index].type = 0; break;
         }
      }
   }

   //Add player
   int index = map->sprite_count++;
   map->sprites = RvR_realloc(map->sprites,sizeof(*map->sprites)*map->sprite_count);
   map->sprites[index].type = 2048;
   map->sprites[index].pos.x = level.playerStart[0]*1024+512;
   map->sprites[index].pos.y = level.playerStart[1]*1024+512;
   map->sprites[index].pos.z = map->floor[level.playerStart[1]*64+level.playerStart[0]]*128;
   map->sprites[index].direction = (1024-level.playerStart[2]*4);

   RvR_ray_map_save(path_out);

   return 0;
}

static void print_help(char **argv)
{
   RvR_log("%s usage:\n"
          "%s -fout filename ]\n"
          "   -fout       output map path\n",
         argv[0],argv[0]);
}
//-------------------------------------
