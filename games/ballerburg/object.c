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
#include "baller.h"
#include "object.h"
#include "ressource.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
Object *a_opt, *a_ein, * a_men, *a_inf, *a_sch, *a_brg, *a_nam, *a_dra, *a_sta, *a_sie, *a_com, *a_re1, *a_re2, *a_re3;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void objects_get(void)
{
   a_men = ressource_get(0,MENUE);
   a_inf = ressource_get(0,INFOTREE);
   a_sch = ressource_get(0,SCHUSS);
   a_brg = ressource_get(0,BURG);
   a_nam = ressource_get(0,NAMEN);
   a_dra = ressource_get(0,DRAN);
   a_sta = ressource_get(0,STATUS);
   a_sie = ressource_get(0,SIEGER);
   a_com = ressource_get(0,COMPUTER);
   a_re1 = ressource_get(0,REGEL1);
   a_re2 = ressource_get(0,REGEL2);
   a_re3 = ressource_get(0,REGEL3);
   a_opt = ressource_get(0,OPTION);
   a_ein = ressource_get(0,EINTRAG);
}

void object_print(Object *o)
{
   printf("next %d\n",o->next);
   printf("first child %d\n",o->head);
   printf("last child %d\n",o->tail);
   printf("type %d\n",o->type);
   printf("flags %d\n",o->flags);
   printf("state %d\n",o->state);
   printf("spec %d\n",o->spec);
   printf("x %d\n",o->x);
   printf("y %d\n",o->y);
   printf("width %d\n",o->width);
   printf("height %d\n",o->height);
}

void object_offset(const Object *tree, const Object *o, int *x, int *y)
{
   *x = 0;
   *y = 0;
   do
   {
      *x+=o->x;
      *y+=o->y;
      o = object_parent(tree,o);
   }while(o!=NULL);
}

const Object *object_parent(const Object *tree, const Object *o)
{
   if(o==tree)
      return NULL;

   int next;
   for(;;)
   {
      next = o->next;
      if(tree+tree[next].tail==o)
         break;
      o = tree+next;
   }

   //Unreachable
   return next<0?NULL:tree+next;
}
//-------------------------------------
