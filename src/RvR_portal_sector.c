/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
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

int RvR_port_sector_inside(int16_t sector, RvR_fix22 x, RvR_fix22 y)
{
   RvR_port_map *map = RvR_port_map_get();

   if(sector>=map->num_sectors||sector<0)
      return 0;

   RvR_port_wall *wall = &map->walls[map->sectors[sector].first_wall];
   RvR_fix22 x0 = 0;
   RvR_fix22 y0 = 0;
   RvR_fix22 x1 = 0;
   RvR_fix22 y1 = 0;
   int crossed = 0;

   for(int i = 0;i<map->sectors[sector].num_walls;i++,wall++)
   {
      //Subtract targeted point from coordinates
      //The targeted point is now at 0/0 and the coordinates
      //are relative to that
      x0 = wall->x-x;
      y0 = wall->y-y;
      x1 = map->walls[wall->p2].x-x;
      y1 = map->walls[wall->p2].y-y;

      //If both integers have the same sign (i.e. both are negative/positive)
      //the line is completely above/below the targeted point and does not need to
      //be checked
      if(!RvR_sign_equal(y0,y1))
      {
         //If both integers have the same sign, the line defined by the two points
         //is completely to the right or to the left of the targeted point.
         //If both are negative, add one to the number of lines crossed
         if(RvR_sign_equal(x0,x1))
            crossed+=RvR_negative(x1);
         //If the integers have different signs, the line defined by the two points
         //intersects the y-axis. 
         //To know whether the line is to the left or to the right of the
         //point we use the perp dot product.
         //if p1 is to the left of p2 the perp dot product is negative,
         //if p1 is to the right of p2 the perp dot product is positive,
         //if p1 and p2 are parallel the perp dot product is zero
         //For the line to be to the left of the point, the signs
         //of the perp dot product and y2 must be unequal. 
         //Draw it on a sheet of paper, it will become more
         //apparent why that's the case
         else
            crossed+=!RvR_sign_equal(x0*y1-x1*y0,y1);
      }
   }

   //If an odd number of lines was crossed, the point lies in the polygon/sector according to 
   //the even-odd rule
   return RvR_odd(crossed);
}

int16_t RvR_port_sector_update(int16_t last_sector, RvR_fix22 x, RvR_fix22 y)
{
   //Early out: still in last sector
   if(RvR_port_sector_inside(last_sector,x,y))
      return last_sector;

   RvR_port_map *map = RvR_port_map_get();

   //Check sectors adjacent to last one
   if(last_sector>=0&&last_sector<map->num_sectors)
   {
      RvR_port_wall *wall = &map->walls[map->sectors[last_sector].first_wall];

      for(int i = 0;i<map->sectors[last_sector].num_walls;i++,wall++)
      {
         int portal = wall->portal; 
         if(RvR_port_sector_inside(portal,x,y))
            return portal;
      }
   }

   //Worst case: need to check all sectors
   for(int i = 0;i<map->num_sectors;i++)
      if(RvR_port_sector_inside(i,x,y))
         return i;

   return -1;
}
//-------------------------------------
