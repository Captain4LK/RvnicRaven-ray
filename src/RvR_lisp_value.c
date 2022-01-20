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
#include "RvnicRaven.h"
#include "RvR_lisp_value.h"
#include "RvR_lisp_chunk.h"
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

void lisp_value_array_init(Lisp_value_array *array)
{
   array->values = NULL;
   array->values_used = 0;
   array->values_size = 0;
}

void lisp_value_array_write(Lisp_value_array *array, int32_t value)
{
   if(array->values==NULL)
   {
      array->values_used = 0;
      array->values_size = 256;
      array->values = RvR_malloc(sizeof(*array->values)*array->values_size);
   }

   array->values[array->values_used++] = value;

   if(array->values_used>=array->values_size)
   {
      array->values_size+=256;
      array->values = RvR_realloc(array->values,sizeof(*array->values)*array->values_size);
   }
}

void lisp_value_array_free(Lisp_value_array *array)
{
   if(array->values==NULL)
      return;

   RvR_free(array->values);
   lisp_value_array_init(array);
}
//-------------------------------------
