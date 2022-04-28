/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/*
RvnicRaven makes use of the Fowler/Noll/Vo (FNV) hash.

Some info taken from http://www.isthe.com/chongo/tech/comp/fnv/index.html:

The basis of the FNV hash algorithm was taken from an idea sent as reviewer comments to the IEEE POSIX P1003.2 committee by Glenn Fowler and Phong Vo back in 1991.
In a subsequent ballot round: Landon Curt Noll improved on their algorithm.
Some people tried this hash and found that it worked rather well. In an EMail message to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.

FNV hashes are designed to be fast while maintaining a low collision rate.
The FNV speed allows one to quickly hash lots of data while maintaining a reasonable collision rate.
The high dispersion of the FNV hashes makes them well suited for hashing nearly identical strings such as URLs, hostnames, filenames, text, IP addresses, etc.

FNV hash algorithms and source code have been released into the public domain.
The authors of the FNV algorithmm took deliberate steps to disclose the algorhtm in a public forum soon after it was invented.
More than a year passed after this public disclosure and the authors deliberatly took no steps to patent the FNV algorithm.
Therefore it is safe to say that the FNV authors have no patent claims on the FNV algorithm as published.
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

uint64_t RvR_fnv64a(const char *str)
{
   RvR_error_check(str!=NULL,"RvR_fnv64a","argument 'str' must be non-NULL\n");

   return RvR_fnv64a_str(str,0xcbf29ce484222325ULL);

RvR_err:
   return RVR_FNV_64_PRIME;
}

uint64_t RvR_fnv64a_str(const char *str, uint64_t hval)
{
   RvR_error_check(str!=NULL,"RvR_fnv64a_str","argument 'str' must be non-NULL\n");

   unsigned char *s = (unsigned char *)str;
   while(*s)
   {
      hval^=(uint64_t)*s++;
      hval*=RVR_FNV_64_PRIME;
   }

RvR_err:
   return hval;
}

uint64_t RvR_fnv64a_buf(const void *buf, size_t len, uint64_t hval)
{
   RvR_error_check(buf!=NULL,"RvR_fnv64a_buf","argument 'buf' must be non-NULL\n");

   unsigned char *bs = (unsigned char *)buf;
   unsigned char *be = bs+len;
   while(bs<be)
   {
      hval^=(uint64_t)*bs++;
      hval*=RVR_FNV_64_PRIME;
   }

RvR_err:
   return hval;
}

uint32_t RvR_fnv32a(const char *str)
{
   RvR_error_check(str!=NULL,"RvR_fnv32a","argument 'str' must be non-NULL\n");

   return RvR_fnv32a_str(str,0x811c9dc5);

RvR_err:
   return RVR_FNV_32_PRIME;
}

uint32_t RvR_fnv32a_str(const char *str, uint32_t hval)
{
   RvR_error_check(str!=NULL,"RvR_fnv32a_str","argument 'str' must be non-NULL\n");

   unsigned char *s = (unsigned char *)str;
   while(*s) 
   {
      hval^=(uint32_t)*s++;
      hval *= RVR_FNV_32_PRIME;
   }

RvR_err:
   return hval;
}

uint32_t RvR_fnv32a_buf(const void *buf, size_t len, uint32_t hval)
{
   RvR_error_check(buf!=NULL,"RvR_fnv32a_buf","argument 'buf' must be non-NULL\n");

   unsigned char *bs = (unsigned char *)buf;
   unsigned char *be = bs+len;
   while(bs<be)
   {
      hval^=(uint32_t)*bs++;
      hval *= RVR_FNV_32_PRIME;
   }

RvR_err:
   return hval;
}
//-------------------------------------
