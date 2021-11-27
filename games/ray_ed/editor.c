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
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "color.h"
#include "map.h"
#include "editor.h"
#include "editor2d.h"
#include "editor3d.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int editor_mode = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void editor_update()
{
   if(editor_mode==0)
      editor2d_update();
   else
      editor3d_update();

   if(RvR_core_key_pressed(RVR_KEY_ENTER))
      editor_mode = !editor_mode;
}

void editor_draw()
{
   if(editor_mode==0)
      editor2d_draw();
   else
      editor3d_draw();

   int mx,my;
   RvR_core_mouse_pos(&mx,&my);
   RvR_draw_horizontal_line(mx-4,mx-1,my,color_magenta);
   RvR_draw_horizontal_line(mx+1,mx+4,my,color_magenta);
   RvR_draw_vertical_line(mx,my-1,my-4,color_magenta);
   RvR_draw_vertical_line(mx,my+1,my+4,color_magenta);
}
//-------------------------------------
