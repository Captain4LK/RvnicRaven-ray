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
#include <limits.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "object.h"
#include "ressource.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Object *objects;
uint32_t *trees;
//-------------------------------------

//Function prototypes
static char *read_string(RvR_rw *rw, int pos);
//-------------------------------------

//Function implementations

int ressource_load(const char *path)
{
   RvR_rw rw;
   RvR_rw_init_path(&rw,path,"rb");
   RvR_rw_endian(&rw,RVR_BIG_ENDIAN);

   RvR_rw_read_u16(&rw);
   uint16_t object_offset = RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   uint16_t object_tree_offset = RvR_rw_read_u16(&rw);
   uint16_t num_objects = RvR_rw_read_u16(&rw);
   uint16_t num_trees = RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   RvR_rw_read_u16(&rw);
   uint16_t file_size = RvR_rw_read_u16(&rw);

   //Read objects
   RvR_rw_seek(&rw,object_offset,SEEK_SET);
   objects = RvR_malloc(sizeof(*objects)*num_objects);
   for(int i = 0;i<num_objects;i++)
   {
      Object o;
      o.next = RvR_rw_read_u16(&rw);
      o.head = RvR_rw_read_u16(&rw);
      o.tail = RvR_rw_read_u16(&rw);
      o.type = RvR_rw_read_u16(&rw);
      o.flags = RvR_rw_read_u16(&rw);
      o.state = RvR_rw_read_u16(&rw);
      o.spec = RvR_rw_read_u32(&rw);
      o.x = RvR_rw_read_u16(&rw);
      o.y = RvR_rw_read_u16(&rw);
      o.width = RvR_rw_read_u16(&rw);
      o.height = RvR_rw_read_u16(&rw);
      objects[i] = o;

      /*printf("Object %d:\n",i);
      printf("Next %d\n",o.next);
      printf("First child %d\n",o.head);
      printf("Last child %d\n",o.tail);
      printf("Type %d\n",o.type);
      printf("Flags %d\n",o.flags);
      printf("state %d\n",o.state);
      printf("spec %d\n",o.spec);
      printf("x %d\n",o.x);
      printf("y %d\n",o.y);
      printf("width %d\n",o.width);
      printf("height %d\n",o.height);
      puts("--------------------------");*/
   }

   //Read trees
   RvR_rw_seek(&rw,object_tree_offset,SEEK_SET);
   trees = RvR_malloc(sizeof(*trees)*num_trees);
   for(int i = 0;i<num_trees;i++)
   {
      trees[i] = RvR_rw_read_u32(&rw);
      trees[i] = (trees[i]-object_offset)/24;

      //printf("Tree %d: %d\n",i,trees[i]);
   }

   //Post process objects --> read spec fields and fix x,y,width,height
   for(int i = 0;i<num_objects;i++)
   {
      Object *o = &objects[i];

      //Fix pos
      o->x = (o->x&255)*8+(int8_t)(o->x>>8);
      o->y = (o->y&255)*16+(int8_t)(o->y>>8);
      o->width = o->width==80?RVR_XRES:(o->width&255)*8+(int8_t)(o->width>>8);
      o->height = (o->height&255)*16+(int8_t)(o->height>>8);

      RvR_rw_seek(&rw,o->spec,SEEK_SET);
      switch(o->type)
      {
      case 20: //BOX
      case 25: //IBOX
      case 27: //BOXCHAR
         {
            uint16_t spec = RvR_rw_read_u16(&rw);
            o->as.box.color_inside = spec&15;
            o->as.box.pattern = (spec>>4)&7;
            o->as.box.writing_mode = ((spec>>7)&1)?1:2;
            o->as.box.color_text = (spec>>8)&15;
            o->as.box.color_border = (spec>>12)&15;
         }
         break;
      case 26: //BUTTON
      case 28: //STRING
      case 32: //TITLE
         o->as.str = read_string(&rw,o->spec);
         //puts(o->as.str);
         break;
      case 21: //TEXT
      case 22: //BOXTEXT
      case 30: //FBOXTEXT
         {
            uint32_t str_pos = RvR_rw_read_u32(&rw);
            uint32_t template_pos = RvR_rw_read_u32(&rw);
            uint32_t validate_pos = RvR_rw_read_u32(&rw);

            o->as.text.font = RvR_rw_read_u16(&rw);
            o->as.text.font_id = RvR_rw_read_u16(&rw);
            o->as.text.justify = RvR_rw_read_u16(&rw);
            uint16_t spec = RvR_rw_read_u16(&rw);
            o->as.text.color_inside = spec&15;
            o->as.text.pattern = (spec>>4)&7;
            o->as.text.writing_mode = ((spec>>7)&1)?1:2;
            o->as.text.color_text = (spec>>8)&15;
            o->as.text.color_border = (spec>>12)&15;
            o->as.text.font_size = RvR_rw_read_u16(&rw);
            o->as.text.thickness = RvR_rw_read_u16(&rw);
            o->as.text.str_len = RvR_rw_read_u16(&rw);
            o->as.text.template_len = RvR_rw_read_u16(&rw);
            o->as.text.str = read_string(&rw,str_pos);
            o->as.text.template = read_string(&rw,template_pos);
            o->as.text.validate = read_string(&rw,validate_pos);

            //printf("%d %d %d %d %d %d %d %d\n",o->as.text.font,o->as.text.font_id,o->as.text.justify,o->as.text.color,o->as.text.font_size,o->as.text.thickness,o->as.text.str_len,o->as.text.template_len);
         }
         break;
      default:
         printf("Type %d not implemented\n",o->type);
         break;
      }
   }
   RvR_rw_close(&rw);

   return 1;
}

void *ressource_get(int type, int index)
{
   if(type==0)
   {
      return &objects[trees[index]];
   }

   return NULL;
}

static char *read_string(RvR_rw *rw, int pos)
{
   RvR_rw_seek(rw,pos,SEEK_SET);

   //Read length
   int len = 1;
   uint8_t c = RvR_rw_read_u8(rw);
   while(c!='\0')
   {
      len++;
      c = RvR_rw_read_u8(rw);
   }

   //Read string
   char *str = RvR_malloc(len);
   RvR_rw_seek(rw,pos,SEEK_SET);
   len = 0;
   str[len] = c = RvR_rw_read_u8(rw);
   while(c!='\0')
   {
      len++;
      str[len] = c = RvR_rw_read_u8(rw);
   }

   return str;
}
//-------------------------------------
