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
static int menu = 0;
static char menu_input[512] = {0};
static uint16_t menu_new_width = 0;
static uint16_t menu_new_height = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void editor2d_update()
{
   if(RvR_core_key_pressed(RVR_KEY_ESCAPE))
      menu = !menu;

   //Big mess
   //Probably the worst code I've written this year...
   if(menu!=0)
   {
      switch(menu)
      {
      case -2:
         if(RvR_core_key_pressed(RVR_KEY_ESCAPE))
            menu = 0;
         break;
      case -1:
         if(RvR_core_key_pressed(RVR_KEY_ESCAPE))
            menu = 0;
         break;
      case 1:
         if(RvR_core_key_pressed(RVR_KEY_N))
         {
            menu = 2;
         }
         else if(RvR_core_key_pressed(RVR_KEY_S))
         {
            map_save();
         }
         else if(RvR_core_key_pressed(RVR_KEY_A))
         {
            menu = 5;
            menu_input[0] = '\0';
            RvR_core_text_input_start(menu_input,9);
         }
         break;
      case 2:
         if(RvR_core_key_pressed(RVR_KEY_ESCAPE)||RvR_core_key_pressed(RVR_KEY_N))
         {
            menu = 0;
         }

         if(RvR_core_key_pressed(RVR_KEY_Y))
         {
            menu_input[0] = '\0';
            RvR_core_text_input_start(menu_input,512);
            menu = 3;
         }
         break;
      case 3:
         if(RvR_core_key_pressed(RVR_KEY_ESCAPE))
         {
            RvR_core_text_input_end();
            menu = 0;
         }

         if(RvR_core_key_pressed(RVR_KEY_ENTER))
         {
            RvR_core_text_input_end();
            menu_new_width = atoi(menu_input);
            if(menu_new_width<=0)
            {
               menu = -1;
            }
            else
            {
               menu_input[0] = '\0';
               RvR_core_text_input_start(menu_input,512);
               menu = 4;
            }
         }
         break;
      case 4:
         if(RvR_core_key_pressed(RVR_KEY_ESCAPE))
         {
            RvR_core_text_input_end();
            menu = 0;
         }

         if(RvR_core_key_pressed(RVR_KEY_ENTER))
         {
            RvR_core_text_input_end();
            menu_new_height = atoi(menu_input);
            if(menu_new_height<=0)
            {
               menu = -1;
            }
            else
            {
               map_new(menu_new_width,menu_new_height);
               menu = 0;
            }
         }
         break;
      case 5:
         if(RvR_core_key_pressed(RVR_KEY_ESCAPE))
         {
            RvR_core_text_input_end();
            menu = 0;
         }

         if(RvR_core_key_pressed(RVR_KEY_ENTER))
         {
            RvR_core_text_input_end();
            map_set_name(menu_input);
            map_save();
            menu = -2;
         }
         break;
      }

      return;
   }

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

      RvR_draw_rectangle_fill(0,RVR_YRES-12,RVR_XRES,12,color_dark_gray);
      char tmp[512];
      
   switch(menu)
   {
   case -2: snprintf(tmp,512,"Saved map to %s.rvr",menu_input); RvR_draw_string(5,RVR_YRES-10,1,tmp,color_white); break;
   case -1: RvR_draw_string(5,RVR_YRES-10,1,"Invalid input",color_white); break;
   case 0: snprintf(tmp,512,"x: %d y:%d ang:%d",camera.pos.x,camera.pos.y,camera.direction); RvR_draw_string(5,RVR_YRES-10,1,tmp,color_white); break;
   case 1: RvR_draw_string(5,RVR_YRES-10,1,"(N)ew, (L)oad, (S)ave , save (A)s, (Q)uit",color_white); break;
   case 2: RvR_draw_string(5,RVR_YRES-10,1,"Are you sure you want to start a new map? (Y/N)",color_white); break;
   case 3: snprintf(tmp,512,"Map width: %s",menu_input); RvR_draw_string(5,RVR_YRES-10,1,tmp,color_white); break;
   case 4: snprintf(tmp,512,"Map height: %s",menu_input); RvR_draw_string(5,RVR_YRES-10,1,tmp,color_white); break;
   case 5: snprintf(tmp,512,"Save as: %s",menu_input); RvR_draw_string(5,RVR_YRES-10,1,tmp,color_white); break;
   }
}
//-------------------------------------
