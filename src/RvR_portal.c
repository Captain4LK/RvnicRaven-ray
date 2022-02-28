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
static RvR_fix22 port_cam_angle = 0;
static RvR_vec3 port_cam_position = {0};
static int16_t port_cam_shear = 0;
static int16_t port_cam_sector = 0;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_port_set_angle(RvR_fix22 angle)
{
   port_cam_angle = angle;
}

RvR_fix22 RvR_port_get_angle()
{
   return port_cam_angle;
}

void RvR_port_set_shear(int16_t shear)
{
   port_cam_shear = shear;
}

int16_t RvR_port_get_shear()
{
   return port_cam_shear;
}

void RvR_port_set_position(RvR_vec3 position)
{
   port_cam_position = position;
}

RvR_vec3 RvR_port_get_position()
{
   return port_cam_position;
}

void RvR_port_set_sector(int16_t sector)
{
   port_cam_sector = sector;
}

int16_t RvR_port_get_sector()
{
   return port_cam_sector;
}
//-------------------------------------
