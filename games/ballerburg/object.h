/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _OBJECT_H_

#define _OBJECT_H_

typedef struct
{
   int16_t next;
   int16_t head;
   int16_t tail;
   uint16_t type;
   uint16_t flags;
   uint16_t state;
   uint32_t spec;
   int16_t x;
   int16_t y;
   int16_t width;
   int16_t height;

   union
   {  
      struct
      {
         uint8_t color_border;
         uint8_t color_text;
         uint8_t color_inside;
         uint8_t writing_mode;
         uint8_t pattern;
      }box;
      char *str;
      struct
      {
         char *str;
         char *template;
         char *validate;
         int16_t font;
         int16_t font_id;
         int16_t justify;
         int16_t color;
         int16_t font_size;
         int16_t thickness;
         int16_t str_len;
         int16_t template_len;
      }text;
   }as;
}Object;

void objects_get(void);
void object_print(Object *o);

extern Object *a_opt, *a_ein, * a_men, *a_inf, *a_sch, *a_brg, *a_nam, *a_dra, *a_sta, *a_sie, *a_com, *a_re1, *a_re2, *a_re3;

#endif
