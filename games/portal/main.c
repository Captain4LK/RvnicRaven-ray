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
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
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

int main(int argc, char **argv)
{
   //Init RvnicRaven core
   RvR_core_init("Portal",0);
   RvR_core_mouse_relative(1);

   RvR_pak_add("test.pak");

   RvR_palette_load(0);
   RvR_draw_font_set(0xF000);

   RvR_port_map_create();
   RvR_port_map *map = RvR_port_map_get();

   map->num_sectors = 3;
   map->num_walls = 16;
   map->sectors[0].num_walls = 8;
   map->sectors[0].first_wall = 0;
   map->sectors[1].num_walls = 4;
   map->sectors[1].first_wall = 8;
   map->sectors[2].num_walls = 4;
   map->sectors[2].first_wall = 12;

   map->walls[0].x = 32*256;
   map->walls[0].y = 32*256;
   map->walls[0].p2 = 1;
   map->walls[0].portal = -1;

   map->walls[1].x = 160*256;
   map->walls[1].y = 32*256;
   map->walls[1].p2 = 2;
   map->walls[1].portal = 1;

   map->walls[2].x = 160*256;
   map->walls[2].y = 160*256;
   map->walls[2].p2 = 3;
   map->walls[2].portal = -1;

   map->walls[3].x = 32*256;
   map->walls[3].y = 200*256;
   map->walls[3].p2 = 0;
   map->walls[3].portal = -1;

   map->walls[4].x = 64*256;
   map->walls[4].y = 64*256;
   map->walls[4].p2 = 5;
   map->walls[4].portal = -1;

   map->walls[5].x = 64*256;
   map->walls[5].y = 128*256;
   map->walls[5].p2 = 6;
   map->walls[5].portal = -1;

   map->walls[6].x = 128*256;
   map->walls[6].y = 128*256;
   map->walls[6].p2 = 7;
   map->walls[6].portal = -1;

   map->walls[7].x = 128*256;
   map->walls[7].y = 64*256;
   map->walls[7].p2 = 4;
   map->walls[7].portal = -1;

   map->walls[8].x = 160*256;
   map->walls[8].y = 32*256;
   map->walls[8].p2 = 9;
   map->walls[8].portal = -1;

   map->walls[9].x = 200*256;
   map->walls[9].y = 32*256;
   map->walls[9].p2 = 10;
   map->walls[9].portal = -1;

   map->walls[10].x = 200*256;
   map->walls[10].y = 200*256;
   map->walls[10].p2 = 11;
   map->walls[10].portal = 2;

   map->walls[11].x = 160*256;
   map->walls[11].y = 160*256;
   map->walls[11].p2 = 8;
   map->walls[11].portal = 0;

   map->walls[12].x = 160*256;
   map->walls[12].y = 160*256;
   map->walls[12].p2 = 13;
   map->walls[12].portal = 1;

   map->walls[13].x = 200*256;
   map->walls[13].y = 200*256;
   map->walls[13].p2 = 14;
   map->walls[13].portal = -1;

   map->walls[14].x = 200*256;
   map->walls[14].y = 230*256;
   map->walls[14].p2 = 15;
   map->walls[14].portal = -1;

   map->walls[15].x = 32*256;
   map->walls[15].y = 200*256;
   map->walls[15].p2 = 11;
   map->walls[15].portal = -1;

   RvR_vec3 pos = {0};
   RvR_fix22 dir = 0;
   int16_t sector = 0;
   RvR_fix22 fov = 256;

   while(RvR_core_running())
   {
      RvR_core_update();

      RvR_vec2 direction = RvR_vec2_rot(dir);
      direction.x/=4;
      direction.y/=4;
      if(RvR_core_key_down(RVR_KEY_UP))
      {
         pos.x+=direction.x;
         pos.y+=direction.y;
      }
      if(RvR_core_key_down(RVR_KEY_DOWN))
      {
         pos.x-=direction.x;
         pos.y-=direction.y;
      }
      if(RvR_core_key_down(RVR_KEY_LEFT))
         dir+=10;      
      if(RvR_core_key_down(RVR_KEY_RIGHT))
         dir-=10;

      int16_t sector_n = RvR_port_sector_update(sector,pos.x,pos.y);
      sector = sector_n<0?sector:sector_n;

      if(RvR_core_key_down(RVR_KEY_NP_ADD))
         fov++;
      else if(RvR_core_key_down(RVR_KEY_NP_SUB))
         fov--;

      RvR_port_set_position(pos);
      RvR_port_set_angle(dir);
      RvR_port_set_sector(sector);
      RvR_port_set_fov(fov);

      RvR_draw_clear(1);
      RvR_port_draw();

      RvR_core_render_present();
   }

   return 0;
}
//-------------------------------------
