/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _BURG_H_

#define _BURG_H_

typedef struct
{
   int width;
   int kanonen[20]; //max 10
   int trohn[4];
   int fahne[4];
   int geld[4];
   int pulver[4];
   int kugeln[4];
   int geld_start;
   int pulver_start;
   int kugeln_start;
   int volk_start;
   
   int shape_size;
   int shape_used;
   int *shape;
}Burg;

typedef struct
{
   unsigned data_size;
   unsigned data_used;
   Burg *data;
}Burg_array;

extern Burg_array burgen;

void burgen_add(RvR_rw *rw);
void burg_add(Burg b);

#endif
