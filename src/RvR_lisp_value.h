/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_LISP_VALUE_H_

#define _RVR_LISP_VALUE_H_

typedef struct
{
   int32_t *values;
   int values_used;
   int values_size;
}Lisp_value_array;

void lisp_value_array_init(Lisp_value_array *array);
void lisp_value_array_write(Lisp_value_array *array, int32_t value);
void lisp_value_array_free(Lisp_value_array *array);

#endif
