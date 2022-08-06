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
#include "ui.h"
#include "draw.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void object_draw_help(Object *tree, Object *cur, int x, int y, int r);
//-------------------------------------

//Function implementations

void menu_bar_update(Object *tree)
{
}

void menu_bar_draw(Object *tree)
{
   object_draw(tree,0,0,0);
}

void object_draw(Object *tree, int x, int y, int r)
{
   object_draw_help(tree,tree,x,y,r);
}

static void object_draw_help(Object *tree, Object *cur, int x, int y, int r)
{
   //for(int i = 0;i<r;i++) printf("\t");
   ////printf("%d %d %d %d %d %d\n",cur->type,cur->flags,cur->x,cur->y,cur->width,cur->height);
   switch(cur->type)
   {
   case 20: //BOX
      printf("%d %d\n",cur->as.box.pattern,cur->as.box.color_inside);
      draw_buffer_set_color(RvR_min(1,cur->as.box.color_border));
      draw_buffer_rectangle(cur->x+x-2,cur->y+y-2,cur->width+4,cur->height+4);
      draw_buffer_set_color(RvR_min(1,cur->as.box.color_inside));
      draw_buffer_set_text_write(cur->as.box.writing_mode);
      draw_buffer_set_pattern(cur->as.box.pattern);
      draw_buffer_rectangle(cur->x+x,cur->y+y,cur->width,cur->height);
      draw_buffer_set_text_color(RvR_min(1,cur->as.box.color_text));
      draw_buffer_set_pattern(0);
      break;
   case 21: //TEXT
      draw_buffer_set_text_write(2);
      draw_buffer_set_text_color(1);
      draw_buffer_text(cur->x+x+(cur->width-strlen(cur->as.str)*8)/2,cur->y+y+14,cur->as.str);
      draw_buffer_set_text_write(1);
      break;
   case 22: //BOXTEXAT
      printf("%d %d\n",cur->as.text.pattern,cur->as.text.color_inside);
      draw_buffer_set_color(RvR_min(1,cur->as.text.color_border));
      draw_buffer_rectangle(cur->x+x-2,cur->y+y-2,cur->width+4,cur->height+4);
      draw_buffer_set_color(RvR_min(1,cur->as.text.color_inside));
      draw_buffer_set_text_write(cur->as.text.writing_mode);
      draw_buffer_set_pattern(cur->as.text.pattern);
      draw_buffer_rectangle(cur->x+x,cur->y+y,cur->width,cur->height);
      draw_buffer_set_text_color(RvR_min(1,cur->as.text.color_text));
      draw_buffer_set_pattern(0);
      break;
   case 25: //IBOX
      break;
   case 26: //BUTTON
      //draw_buffer_set_color(!RvR_min(1,cur->as.box.color_border));
      //draw_buffer_rectangle(cur->x+x,cur->y+y,cur->width,cur->height);
      //draw_buffer_set_color(!RvR_min(1,cur->as.box.color_inside));
      //draw_buffer_set_write(cur->as.box.writing_mode);
      //draw_buffer_set_pattern(cur->as.box.pattern);
      draw_buffer_set_color(1);
      draw_buffer_rectangle(cur->x+x-2,cur->y+y-2,cur->width+4,cur->height+4);
      draw_buffer_set_color(0);
      draw_buffer_rectangle(cur->x+x,cur->y+y,cur->width,cur->height);

      draw_buffer_set_text_write(2);
      draw_buffer_set_text_color(1);
      draw_buffer_text(cur->x+x+(cur->width-strlen(cur->as.str)*8)/2,cur->y+y+14,cur->as.str);
      draw_buffer_set_text_write(1);
      break;
   case 27: //BOXCHAR
      draw_buffer_set_color(RvR_min(1,cur->as.box.color_border));
      draw_buffer_rectangle(cur->x+x-2,cur->y+y-2,cur->width+4,cur->height+4);
      draw_buffer_set_color(RvR_min(1,cur->as.box.color_inside));
      draw_buffer_set_text_write(cur->as.box.writing_mode);
      draw_buffer_set_pattern(cur->as.box.pattern-2);
      draw_buffer_rectangle(cur->x+x,cur->y+y,cur->width,cur->height);
      draw_buffer_set_text_color(RvR_min(1,cur->as.box.color_text));
      draw_buffer_set_pattern(0);
      break;
   case 28: //STRING
      draw_buffer_set_text_write(2);
      draw_buffer_set_text_color(1);
      draw_buffer_text(cur->x+x+(cur->width-strlen(cur->as.str)*8)/2,cur->y+y+14,cur->as.str);
      draw_buffer_set_text_write(1);
      break;
   case 32: //TITLE
      draw_buffer_set_text_write(2);
      draw_buffer_set_text_color(1);
      draw_buffer_text(cur->x+x+(cur->width-strlen(cur->as.str)*8)/2,cur->y+y+14,cur->as.str);
      draw_buffer_set_text_write(1);
      break;
   default:
      printf("Type %d not implemented\n",cur->type);
   }

   if(cur->head<0)
      return;
   Object *next = tree+cur->head;
   while(next!=cur)
   {
      if(next->next<0)
         break;
      if(!(next->flags&128))
         object_draw_help(tree,next,cur->x+x,cur->y+y,r+1);
      next = tree+next->next;
   }
}
//-------------------------------------
