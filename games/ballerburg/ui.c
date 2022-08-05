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
//-------------------------------------

//Function implementations

void menu_bar_update(Object *tree)
{
}

void menu_bar_draw(Object *tree)
{
   puts("--------------------");
   object_draw(tree,0,20,0);
}

void object_draw(Object *tree, int x, int y, int r)
{
   for(int i = 0;i<r;i++) printf("\t");
   printf("%d %d %d %d %d\n",tree->type,tree->x,tree->y,tree->width,tree->height);
   switch(tree->type)
   {
   case 20: //BOX
      draw_buffer_set_color(tree->as.box.color_inside);
      draw_buffer_set_write(tree->as.box.writing_mode);
      draw_buffer_set_pattern(tree->as.box.pattern);
      draw_buffer_rectangle(tree->x+x,tree->y+y,tree->width,tree->height);
      break;
   case 25: //IBOX
      break;
   case 28: //STRING
      draw_buffer_set_write(2);
      draw_buffer_text(tree->x+x,tree->y+y,tree->as.str);
      draw_buffer_set_write(1);
      break;
   case 32: //TITLE
      draw_buffer_set_write(2);
      draw_buffer_text(tree->x+x,tree->y+y,tree->as.str);
      draw_buffer_set_write(1);
      break;
   default:
      printf("Type %d not implemented\n",tree->type);
   }

   if(tree->head<0)
      return;
   Object *cur = &objects[tree->head];
   while(cur!=tree)
   {
      if(cur->next<0)
         break;
      object_draw(cur,tree->x+x,tree->y+y,r+1);
      cur = &objects[cur->next];
   }
}
//-------------------------------------
