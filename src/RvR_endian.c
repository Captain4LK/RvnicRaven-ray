/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
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

uint16_t RvR_endian_swap16(uint16_t n, uint8_t endian)
{
   if(endian!=RVR_ENDIAN)
      return (n>>8)|(n<<8);
   return n;
}

uint32_t RvR_endian_swap32(uint32_t n, uint8_t endian)
{
   if(endian!=RVR_ENDIAN)
      return (n>>24)|(n<<24)|((n>>8)&0xff00)|((n&0xff00)<<8);
   return n;
}

uint64_t RvR_endian_swap64(uint64_t n, uint8_t endian)
{
   if(endian!=RVR_ENDIAN)
   {
      uint32_t lo = n&0xFFFFFFFF;
      uint32_t hi = (n>>32);
      n = RvR_endian_swap32(lo,endian);
      n<<=32;
      n|=RvR_endian_swap32(hi,endian);

      return n;
   }
   return n;
}
//-------------------------------------
