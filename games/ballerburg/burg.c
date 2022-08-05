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
#include "burg.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Burg_array burgen;
//-------------------------------------

//Function prototypes
static int read_num(RvR_rw *rw, int *out);
static char zeichen(RvR_rw *rw);
static void shape_insert(Burg *b, int n);
//-------------------------------------

//Function implementations

void burgen_add(RvR_rw *rw)
{
   for(;;)
   {
      Burg burg = {0};

      //Width
      if(read_num(rw,&burg.width)) break;

      //Kanonen
      if(read_num(rw,&burg.kanonen[0])) break;
      if(read_num(rw,&burg.kanonen[1])) break;
      if(read_num(rw,&burg.kanonen[2])) break;
      if(read_num(rw,&burg.kanonen[3])) break;
      if(read_num(rw,&burg.kanonen[4])) break;
      if(read_num(rw,&burg.kanonen[5])) break;
      if(read_num(rw,&burg.kanonen[6])) break;
      if(read_num(rw,&burg.kanonen[7])) break;
      if(read_num(rw,&burg.kanonen[8])) break;
      if(read_num(rw,&burg.kanonen[9])) break;
      if(read_num(rw,&burg.kanonen[10])) break;
      if(read_num(rw,&burg.kanonen[11])) break;
      if(read_num(rw,&burg.kanonen[12])) break;
      if(read_num(rw,&burg.kanonen[13])) break;
      if(read_num(rw,&burg.kanonen[14])) break;
      if(read_num(rw,&burg.kanonen[15])) break;
      if(read_num(rw,&burg.kanonen[16])) break;
      if(read_num(rw,&burg.kanonen[17])) break;
      if(read_num(rw,&burg.kanonen[18])) break;
      if(read_num(rw,&burg.kanonen[19])) break;

      //Trohn
      if(read_num(rw,&burg.trohn[0])) break;
      if(read_num(rw,&burg.trohn[1])) break;
      burg.trohn[2] = 30;
      burg.trohn[3] = 25;

      //Fahne
      if(read_num(rw,&burg.fahne[0])) break;
      if(read_num(rw,&burg.fahne[1])) break;
      burg.fahne[2] = 30;
      burg.fahne[3] = 25;

      //Geld pos
      if(read_num(rw,&burg.geld[0])) break;
      if(read_num(rw,&burg.geld[1])) break;

      //Pulver pos
      if(read_num(rw,&burg.pulver[0])) break;
      if(read_num(rw,&burg.pulver[1])) break;

      //Kugeln pos
      if(read_num(rw,&burg.kugeln[0])) break;
      if(read_num(rw,&burg.kugeln[1])) break;

      //Geld dim
      if(read_num(rw,&burg.geld[2])) break;
      if(read_num(rw,&burg.geld[3])) break;

      //Pulver dim
      if(read_num(rw,&burg.pulver[2])) break;
      if(read_num(rw,&burg.pulver[3])) break;

      //Kugeln dim
      if(read_num(rw,&burg.kugeln[2])) break;
      if(read_num(rw,&burg.kugeln[3])) break;

      //Starting values
      if(read_num(rw,&burg.geld_start)) break;
      if(read_num(rw,&burg.pulver_start)) break;
      if(read_num(rw,&burg.kugeln_start)) break;
      if(read_num(rw,&burg.volk_start)) break;

      //Skip reserved
      int tmp;
      if(read_num(rw,&tmp)) break;
      if(read_num(rw,&tmp)) break;
      if(read_num(rw,&tmp)) break;
      if(read_num(rw,&tmp)) break;

      //Burg shape
      int stop = 0;
      int val = 0;
      do
      {
         if(read_num(rw,&val)) { stop = 1; break; }
         shape_insert(&burg,val);
      }while(val!=-1);

      if(stop)
         break;

      burg_add(burg);      
   }
}

void burg_add(Burg b)
{
   if(burgen.data==NULL)
   {
      burgen.data_size = 8;
      burgen.data_used = 0;
      burgen.data = RvR_malloc(sizeof(*burgen.data)*burgen.data_size);
   }

   burgen.data[burgen.data_used++] = b;

   if(burgen.data_used==burgen.data_size)
   {
      burgen.data_size*=2;
      burgen.data = RvR_realloc(burgen.data,sizeof(*burgen.data)*burgen.data_size);
   }
}

static char zeichen(RvR_rw *rw)
{
   char a;
   RvR_rw_read(rw,&a,1,1);
   return a;
}

static int read_num(RvR_rw *rw, int *out)
{
   char a,sign=1,rem=0;    /* wird durch * getoggled, und zeigt damit an, */
                           /* ob man sich in einer Bemerkung befindet */
   int val=0;

   do
      if( (a=zeichen(rw))=='*' ) rem=!rem;
   while( (a!='-' && (a<'0' || a>'9')) || rem );

   if( a=='-' ) { sign=-1; a=zeichen(rw); }
   while( a>='0' && a<='9' )
   {
      val*=10; val+=a-'0';
      a=zeichen(rw);
   }
   *out = sign*val;
   if(*out==-999)
      return -1;
   return 0;
}

static void shape_insert(Burg *b, int n)
{
   if(b->shape==NULL)
   {
      b->shape_size = 16;
      b->shape_used = 0;
      b->shape = RvR_malloc(sizeof(*b->shape)*b->shape_size);
   }

   b->shape[b->shape_used++] = n;

   if(b->shape_used==b->shape_size)
   {
      b->shape_size*=2;
      b->shape = RvR_realloc(b->shape,sizeof(*b->shape)*b->shape_size);
   }
}
//-------------------------------------
