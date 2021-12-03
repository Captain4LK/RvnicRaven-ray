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
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static int scroll_x = 0;
static int scroll_y = 0;
static int grid_size = 24;
static int mouse_scroll = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void editor2d_update()
{
   if(RvR_core_mouse_pressed(RVR_BUTTON_RIGHT))
   {
      mouse_scroll = 1;

      int mx,my;
      RvR_core_mouse_pos(&mx,&my);
      RvR_core_mouse_relative(1);
      
      camera.pos.x = ((scroll_x+mx)*1024)/grid_size;
      camera.pos.y = ((scroll_y+my)*1024)/grid_size;
   }

   if(RvR_core_mouse_released(RVR_BUTTON_RIGHT))
   {
      mouse_scroll = 0;
      RvR_core_mouse_relative(0);
      RvR_core_mouse_set_pos(RVR_XRES/2,RVR_YRES/2);
   }

   if(mouse_scroll)
   {
      int mx,my;
      RvR_core_mouse_relative_pos(&mx,&my);

      camera.pos.x+=(mx*1024)/grid_size;
      camera.pos.y+=(my*1024)/grid_size;
   }
   else
   {
      camera_update();
   }

   scroll_x = (camera.pos.x*grid_size)/1024-RVR_XRES/2;
   scroll_y = (camera.pos.y*grid_size)/1024-RVR_YRES/2;

   if(RvR_core_key_pressed(RVR_KEY_NP_ADD)&&grid_size<64)
   {
      int scrollx = ((scroll_x+RVR_XRES/2)*1024)/grid_size;
      int scrolly = ((scroll_y+RVR_YRES/2)*1024)/grid_size;

      grid_size+=4;
      scroll_x = (scrollx*grid_size)/1024-RVR_XRES/2;
      scroll_y = (scrolly*grid_size)/1024-RVR_YRES/2;
   }
   if(RvR_core_key_pressed(RVR_KEY_NP_SUB)&&grid_size>4)
   {
      int scrollx = ((scroll_x+RVR_XRES/2)*1024)/grid_size;
      int scrolly = ((scroll_y+RVR_YRES/2)*1024)/grid_size;

      grid_size-=4;
      scroll_x = (scrollx*grid_size)/1024-RVR_XRES/2;
      scroll_y = (scrolly*grid_size)/1024-RVR_YRES/2;
   }
}

void editor2d_draw()
{
   RvR_draw_clear(color_black);

   int sx = scroll_x/grid_size;
   int sy = scroll_y/grid_size;
   for(int y = 0;y<=RVR_YRES/grid_size+1;y++)
   {
      for(int x = 0;x<=RVR_XRES/grid_size+1;x++)
      {
         {
            int tx = (x+sx)*grid_size-scroll_x;
            int ty = (y+sy)*grid_size-scroll_y;

            uint16_t ftex = RvR_ray_map_floor_tex_at(x+sx,y+sy); 
            uint16_t ctex = RvR_ray_map_ceil_tex_at(x+sx,y+sy); 
            RvR_fix22 fheight = RvR_ray_map_floor_height_at(x+sx,y+sy);
            RvR_fix22 cheight = RvR_ray_map_ceiling_height_at(x+sx,y+sy);

            if(!map_tile_comp(ftex,ctex,fheight,cheight,x+sx,y+sy-1))
               RvR_draw_horizontal_line(tx,tx+grid_size,ty,color_light_gray);
            if(!map_tile_comp(ftex,ctex,fheight,cheight,x+sx-1,y+sy))
               RvR_draw_vertical_line(tx,ty,ty+grid_size,color_light_gray);
         }
      }
   }

   //Draw camera
   RvR_vec2 direction = RvR_vec2_rot(camera.direction);
   int dsx = (direction.x*grid_size)/1024;
   int dsy = (direction.y*grid_size)/1024;
   RvR_draw_line(RVR_XRES/2+dsx/2,RVR_YRES/2+dsy/2,RVR_XRES/2-dsx/2,RVR_YRES/2-dsy/2,color_white);
   direction = RvR_vec2_rot(camera.direction+128);
   RvR_draw_line(RVR_XRES/2+dsx/2,RVR_YRES/2+dsy/2,RVR_XRES/2+dsx/2-(direction.x*grid_size/2)/1024,RVR_YRES/2+dsy/2-(direction.y*grid_size/2)/1024,color_white);
   direction = RvR_vec2_rot(camera.direction-128);
   RvR_draw_line(RVR_XRES/2+dsx/2,RVR_YRES/2+dsy/2,RVR_XRES/2+dsx/2-(direction.x*grid_size/2)/1024,RVR_YRES/2+dsy/2-(direction.y*grid_size/2)/1024,color_white);

   //Draw cursor
   int mx,my;
   if(!mouse_scroll)
   {
      RvR_core_mouse_pos(&mx,&my);
   }
   else
   {
      mx = RVR_XRES/2;
      my = RVR_YRES/2;
   }

   RvR_draw_horizontal_line(mx-4,mx-1,my,color_magenta);
   RvR_draw_horizontal_line(mx+1,mx+4,my,color_magenta);
   RvR_draw_vertical_line(mx,my-1,my-4,color_magenta);
   RvR_draw_vertical_line(mx,my+1,my+4,color_magenta);
}
//-------------------------------------
