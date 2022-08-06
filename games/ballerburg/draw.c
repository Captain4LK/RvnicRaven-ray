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
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "draw.h"
#include "color.h"
//-------------------------------------

//#defines
#define PATTERN_MAX 16
//-------------------------------------

//Typedefs
typedef struct
{
   uint16_t planes;
   uint16_t pattern[PATTERN_MAX];
}Pattern;
//-------------------------------------

//Variables
static int draw_interior;
static int draw_style;
static int draw_write = 1;
static int draw_flip = 1;
static uint8_t draw_color = 1;
static uint8_t text_color = 1;
static int text_write = 1;

static uint16_t pattern_line;

static uint8_t *buffer;
static int buffer_width;
static int buffer_height;

static const Pattern pattern_pattern[25] = 
{
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 4, .pattern = {0x1111,0x0000,0x4444,0x0000}}, 
   {.planes = 2, .pattern = {0x5555,0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 2, .pattern = {0x5555,0xaaaa}},
   {.planes = 2, .pattern = {0x5555,0x0000}},
   {.planes = 2, .pattern = {0x5555,0xffff}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 8, .pattern = {0xffff,0x1010,0x1010,0x1010,0xffff,0x0101,0x0101,0x0101}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 8, .pattern = {0x0a0a,0x0000,0x0000,0x4040,0xa0a0,0x0000,0x0000,0x0404}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
   {.planes = 1, .pattern = {0x0000}},
};

static const uint64_t font[] = {
   UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0xBD8181A5817E0000), UINT64_C(0x000000007E818199), UINT64_C(0xC3FFFFDBFF7E0000), UINT64_C(0x000000007EFFFFE7), UINT64_C(0x7F7F7F3600000000), UINT64_C(0x00000000081C3E7F), 
   UINT64_C(0x7F3E1C0800000000), UINT64_C(0x0000000000081C3E), UINT64_C(0xE7E73C3C18000000), UINT64_C(0x000000003C1818E7), UINT64_C(0xFFFF7E3C18000000), UINT64_C(0x000000003C18187E), UINT64_C(0x3C18000000000000), UINT64_C(0x000000000000183C), 
   UINT64_C(0xC3E7FFFFFFFFFFFF), UINT64_C(0xFFFFFFFFFFFFE7C3), UINT64_C(0x42663C0000000000), UINT64_C(0x00000000003C6642), UINT64_C(0xBD99C3FFFFFFFFFF), UINT64_C(0xFFFFFFFFFFC399BD), UINT64_C(0x331E4C5870780000), UINT64_C(0x000000001E333333), 
   UINT64_C(0x3C666666663C0000), UINT64_C(0x0000000018187E18), UINT64_C(0x0C0C0CFCCCFC0000), UINT64_C(0x00000000070F0E0C), UINT64_C(0xC6C6C6FEC6FE0000), UINT64_C(0x0000000367E7E6C6), UINT64_C(0xE73CDB1818000000), UINT64_C(0x000000001818DB3C), 
   UINT64_C(0x1F7F1F0F07030100), UINT64_C(0x000000000103070F), UINT64_C(0x7C7F7C7870604000), UINT64_C(0x0000000040607078), UINT64_C(0x1818187E3C180000), UINT64_C(0x0000000000183C7E), UINT64_C(0x6666666666660000), UINT64_C(0x0000000066660066), 
   UINT64_C(0xD8DEDBDBDBFE0000), UINT64_C(0x00000000D8D8D8D8), UINT64_C(0x6363361C06633E00), UINT64_C(0x0000003E63301C36), UINT64_C(0x0000000000000000), UINT64_C(0x000000007F7F7F7F), UINT64_C(0x1818187E3C180000), UINT64_C(0x000000007E183C7E), 
   UINT64_C(0x1818187E3C180000), UINT64_C(0x0000000018181818), UINT64_C(0x1818181818180000), UINT64_C(0x00000000183C7E18), UINT64_C(0x7F30180000000000), UINT64_C(0x0000000000001830), UINT64_C(0x7F060C0000000000), UINT64_C(0x0000000000000C06), 
   UINT64_C(0x0303000000000000), UINT64_C(0x0000000000007F03), UINT64_C(0xFF66240000000000), UINT64_C(0x0000000000002466), UINT64_C(0x3E1C1C0800000000), UINT64_C(0x00000000007F7F3E), UINT64_C(0x3E3E7F7F00000000), UINT64_C(0x0000000000081C1C), 
   UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x18183C3C3C180000), UINT64_C(0x0000000018180018), UINT64_C(0x0000002466666600), UINT64_C(0x0000000000000000), UINT64_C(0x36367F3636000000), UINT64_C(0x0000000036367F36), 
   UINT64_C(0x603E0343633E1818), UINT64_C(0x000018183E636160), UINT64_C(0x1830634300000000), UINT64_C(0x000000006163060C), UINT64_C(0x3B6E1C36361C0000), UINT64_C(0x000000006E333333), UINT64_C(0x000000060C0C0C00), UINT64_C(0x0000000000000000), 
   UINT64_C(0x0C0C0C0C18300000), UINT64_C(0x0000000030180C0C), UINT64_C(0x30303030180C0000), UINT64_C(0x000000000C183030), UINT64_C(0xFF3C660000000000), UINT64_C(0x000000000000663C), UINT64_C(0x7E18180000000000), UINT64_C(0x0000000000001818), 
   UINT64_C(0x0000000000000000), UINT64_C(0x0000000C18181800), UINT64_C(0x7F00000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x0000000018180000), UINT64_C(0x1830604000000000), UINT64_C(0x000000000103060C), 
   UINT64_C(0xDBDBC3C3663C0000), UINT64_C(0x000000003C66C3C3), UINT64_C(0x1818181E1C180000), UINT64_C(0x000000007E181818), UINT64_C(0x0C183060633E0000), UINT64_C(0x000000007F630306), UINT64_C(0x603C6060633E0000), UINT64_C(0x000000003E636060), 
   UINT64_C(0x7F33363C38300000), UINT64_C(0x0000000078303030), UINT64_C(0x603F0303037F0000), UINT64_C(0x000000003E636060), UINT64_C(0x633F0303061C0000), UINT64_C(0x000000003E636363), UINT64_C(0x18306060637F0000), UINT64_C(0x000000000C0C0C0C), 
   UINT64_C(0x633E6363633E0000), UINT64_C(0x000000003E636363), UINT64_C(0x607E6363633E0000), UINT64_C(0x000000001E306060), UINT64_C(0x0000181800000000), UINT64_C(0x0000000000181800), UINT64_C(0x0000181800000000), UINT64_C(0x000000000C181800), 
   UINT64_C(0x060C183060000000), UINT64_C(0x000000006030180C), UINT64_C(0x00007E0000000000), UINT64_C(0x000000000000007E), UINT64_C(0x6030180C06000000), UINT64_C(0x00000000060C1830), UINT64_C(0x18183063633E0000), UINT64_C(0x0000000018180018), 
   UINT64_C(0x7B7B63633E000000), UINT64_C(0x000000003E033B7B), UINT64_C(0x7F6363361C080000), UINT64_C(0x0000000063636363), UINT64_C(0x663E6666663F0000), UINT64_C(0x000000003F666666), UINT64_C(0x03030343663C0000), UINT64_C(0x000000003C664303), 
   UINT64_C(0x66666666361F0000), UINT64_C(0x000000001F366666), UINT64_C(0x161E1646667F0000), UINT64_C(0x000000007F664606), UINT64_C(0x161E1646667F0000), UINT64_C(0x000000000F060606), UINT64_C(0x7B030343663C0000), UINT64_C(0x000000005C666363), 
   UINT64_C(0x637F636363630000), UINT64_C(0x0000000063636363), UINT64_C(0x18181818183C0000), UINT64_C(0x000000003C181818), UINT64_C(0x3030303030780000), UINT64_C(0x000000001E333333), UINT64_C(0x1E1E366666670000), UINT64_C(0x0000000067666636), 
   UINT64_C(0x06060606060F0000), UINT64_C(0x000000007F664606), UINT64_C(0xC3DBFFFFE7C30000), UINT64_C(0x00000000C3C3C3C3), UINT64_C(0x737B7F6F67630000), UINT64_C(0x0000000063636363), UINT64_C(0x63636363633E0000), UINT64_C(0x000000003E636363), 
   UINT64_C(0x063E6666663F0000), UINT64_C(0x000000000F060606), UINT64_C(0x63636363633E0000), UINT64_C(0x000070303E7B6B63), UINT64_C(0x363E6666663F0000), UINT64_C(0x0000000067666666), UINT64_C(0x301C0663633E0000), UINT64_C(0x000000003E636360), 
   UINT64_C(0x18181899DBFF0000), UINT64_C(0x000000003C181818), UINT64_C(0x6363636363630000), UINT64_C(0x000000003E636363), UINT64_C(0xC3C3C3C3C3C30000), UINT64_C(0x00000000183C66C3), UINT64_C(0xDBC3C3C3C3C30000), UINT64_C(0x000000006666FFDB), 
   UINT64_C(0x18183C66C3C30000), UINT64_C(0x00000000C3C3663C), UINT64_C(0x183C66C3C3C30000), UINT64_C(0x000000003C181818), UINT64_C(0x0C183061C3FF0000), UINT64_C(0x00000000FFC38306), UINT64_C(0x0C0C0C0C0C3C0000), UINT64_C(0x000000003C0C0C0C), 
   UINT64_C(0x1C0E070301000000), UINT64_C(0x0000000040607038), UINT64_C(0x30303030303C0000), UINT64_C(0x000000003C303030), UINT64_C(0x0000000063361C08), UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000), UINT64_C(0x0000FF0000000000), 
   UINT64_C(0x0000000000180C0C), UINT64_C(0x0000000000000000), UINT64_C(0x3E301E0000000000), UINT64_C(0x000000006E333333), UINT64_C(0x66361E0606070000), UINT64_C(0x000000003E666666), UINT64_C(0x03633E0000000000), UINT64_C(0x000000003E630303), 
   UINT64_C(0x33363C3030380000), UINT64_C(0x000000006E333333), UINT64_C(0x7F633E0000000000), UINT64_C(0x000000003E630303), UINT64_C(0x060F0626361C0000), UINT64_C(0x000000000F060606), UINT64_C(0x33336E0000000000), UINT64_C(0x001E33303E333333), 
   UINT64_C(0x666E360606070000), UINT64_C(0x0000000067666666), UINT64_C(0x18181C0018180000), UINT64_C(0x000000003C181818), UINT64_C(0x6060700060600000), UINT64_C(0x003C666660606060), UINT64_C(0x1E36660606070000), UINT64_C(0x000000006766361E), 
   UINT64_C(0x18181818181C0000), UINT64_C(0x000000003C181818), UINT64_C(0xDBFF670000000000), UINT64_C(0x00000000DBDBDBDB), UINT64_C(0x66663B0000000000), UINT64_C(0x0000000066666666), UINT64_C(0x63633E0000000000), UINT64_C(0x000000003E636363), 
   UINT64_C(0x66663B0000000000), UINT64_C(0x000F06063E666666), UINT64_C(0x33336E0000000000), UINT64_C(0x007830303E333333), UINT64_C(0x666E3B0000000000), UINT64_C(0x000000000F060606), UINT64_C(0x06633E0000000000), UINT64_C(0x000000003E63301C), 
   UINT64_C(0x0C0C3F0C0C080000), UINT64_C(0x00000000386C0C0C), UINT64_C(0x3333330000000000), UINT64_C(0x000000006E333333), UINT64_C(0xC3C3C30000000000), UINT64_C(0x00000000183C66C3), UINT64_C(0xC3C3C30000000000), UINT64_C(0x0000000066FFDBDB), 
   UINT64_C(0x3C66C30000000000), UINT64_C(0x00000000C3663C18), UINT64_C(0x6363630000000000), UINT64_C(0x001F30607E636363), UINT64_C(0x18337F0000000000), UINT64_C(0x000000007F63060C), UINT64_C(0x180E181818700000), UINT64_C(0x0000000070181818), 
   UINT64_C(0x1800181818180000), UINT64_C(0x0000000018181818), UINT64_C(0x18701818180E0000), UINT64_C(0x000000000E181818), UINT64_C(0x000000003B6E0000), UINT64_C(0x0000000000000000), UINT64_C(0x63361C0800000000), UINT64_C(0x00000000007F6363), 
};
//-------------------------------------

//Function prototypes
static void draw(int x, int y, int mode);
static void draw_line(int x0, int y0, int x1, int y1, int mode);
static void draw_lines(int count, int *pos);
static void fill_lines(int count, int *pos);

static int point_on_line(int x0, int y0, int x1, int y1, int x, int y);
//-------------------------------------

//Function implementations

void draw_buffer_set(uint8_t *buff, int width, int height)
{
   buffer = buff;
   buffer_width = width;
   buffer_height = height;
}

void draw_buffer_set_color(uint8_t c)
{
   draw_color = c;
}

void draw_buffer_set_text_color(uint8_t c)
{
   text_color = c;
}

void draw_buffer_set_text_write(int m)
{
   text_write = m;
}

void draw_buffer_set_write(int m)
{
   draw_write = RvR_min(4,RvR_max(1,m));
}

void draw_buffer_set_flip(int f)
{
   draw_flip = f;
}

void draw_buffer_set_pattern(int pattern)
{
   draw_interior = 2;
   draw_style = pattern;
   if(pattern<=0)
      draw_interior = 1;
}

void draw_buffer_shape(int x, int y, const int *shape)
{
   int pos[256];
   int pos_count;
   int fill = 1;

   draw_interior = 2;
   draw_style = 9;

   while(*shape!=-1)
   {
      if(*shape==-2)
      {
         draw_interior = *++shape;
         draw_style = *++shape;
      }
      else if(*shape==-4)
      {
         fill = !fill;
      }
      else
      {
         pos_count = 0;
         while(*shape>-1)
         {
            pos[pos_count++] = x+(*shape++)*draw_flip;
            pos[pos_count++] = y-*shape++;
         }
         pos[pos_count++] = pos[0];
         pos[pos_count++] = pos[1];

         if(fill)
            fill_lines(pos_count/2,pos);
         else
            draw_lines(pos_count/2-1,pos);
      }

      shape++;
   }
}

void draw_buffer_clear(void)
{
   memset(buffer,color_white,sizeof(*buffer)*buffer_width*buffer_height);
}

void draw_buffer(int x, int y)
{
   int draw_start_y = 0;
   int draw_start_x = 0;
   int draw_end_x = buffer_width;
   int draw_end_y = buffer_height;
   if(x<0)
      draw_start_x = -x;
   if(y<0)
      draw_start_y = -y;
   if(x+draw_end_x>RVR_XRES)
      draw_end_x = buffer_width+(RVR_XRES-x-draw_end_x);
   if(y+draw_end_y>RVR_YRES)
      draw_end_y = buffer_height+(RVR_YRES-y-draw_end_y);

   //Clip dst sprite
   x = x<0?0:x;
   y = y<0?0:y;

   const uint8_t *src = &buffer[draw_start_x+draw_start_y*buffer_width];
   uint8_t *dst = &RvR_core_framebuffer()[x+y*RVR_XRES];
   int src_step = -(draw_end_x-draw_start_x)+buffer_width;
   int dst_step = RVR_XRES-(draw_end_x-draw_start_x);
    
   for(int y1 = draw_start_y;y1<draw_end_y;y1++,dst+=dst_step,src+=src_step)
      for(int x1 = draw_start_x;x1<draw_end_x;x1++,src++,dst++)
         *dst = *src?*src:*dst;
}

void draw_buffer_line(int x0, int y0, int x1, int y1, uint16_t pattern)
{
   pattern_line = pattern;
   draw_line(x0,y0,x1,y1,2);
}

void draw_buffer_rectangle(int x, int y, int width, int height)
{
   for(int iy = y;iy<y+height;iy++)
      for(int ix = x;ix<x+width;ix++)
         draw(ix,iy,1);
   /*//Clip src rect
   int draw_start_y = 0;
   int draw_start_x = 0;
   int draw_end_x = width;
   int draw_end_y = height;

   if(x<0)
      draw_start_x = -x;
   if(y<0)
      draw_start_y = -y;
   if(x+draw_end_x>buffer_width)
      draw_end_x = width+(buffer_width-x-draw_end_x);
   if(y+draw_end_y>buffer_height)
      draw_end_y = height+(buffer_height-y-draw_end_y);
    
   //Clip dst rect
   x = x<0?0:x;
   y = y<0?0:y;

   uint8_t *dst = &buffer[x+y*buffer_width];
   int dst_step = buffer_width-(draw_end_x-draw_start_x);
    
   for(int y1 = draw_start_y;y1<draw_end_y;y1++,dst+=dst_step)
      for(int x1 = draw_start_x;x1<draw_end_x;x1++,dst++)
         *dst = draw_color?color_black:color_white;*/
}

void draw_buffer_text(int x, int y, const char *str)
{
   int write = draw_write;
   draw_write = text_write;
   y-=13;
   uint8_t color = draw_color;
   int len = strlen(str);
   for(int i = 0;i<len;i++)
   {
      uint8_t c = str[i];
      if(c>127)
         c = '?';

      const uint8_t *data = (const uint8_t *)font+c*16;
      for(int iy = y;iy<y+16;iy++)
      {
         uint8_t byte = data[iy-y];
         for(int ix = x;ix<x+8;ix++)
         {
            if(byte&(1<<(ix-x)))
               draw_color = text_color;
            else
               draw_color = !text_color;
            draw(ix,iy,0);
         }
      }
      x+=8;
   }

   draw_color = color;
   draw_write = write;
}

static void draw(int x, int y, int mode)
{
   if(x<0||x>=RVR_XRES)
      return;
   if(y<0||y>=RVR_YRES)
      return;

   int col = draw_color;
   if(mode==1)
   {
      switch(draw_interior)
      {
      case 0: //blank
         return;
      case 1: //solid
         break;
      case 2: //pattern
         if(!(pattern_pattern[draw_style].pattern[y%pattern_pattern[draw_style].planes]&(1<<(x&15))))
            col = !col;
         break;
      default:
         printf("mode %d %d unimplemented\n",draw_interior,draw_style);
         break;
      }
   }
   else if(mode==2)
   {
      if(!(pattern_line&(1<<(x&15))))
         col = !col;
   }

   unsigned mask = col;
   unsigned new = 0;
   unsigned fg = 1;
   unsigned bg = 0;
   unsigned c = buffer[y*RVR_XRES+x]==color_black;

   switch(draw_write)
   {
   case 1:
      new = (fg&mask)|(bg&~mask);
      break;
   case 2:
      new = (fg&mask)|(c&~mask);
      break;
   case 3:
      new = c^mask;
      break;
   case 4:
      new = (c&mask)|(fg&~mask);
      break;
   }

   buffer[y*RVR_XRES+x] = new?color_black:color_white;
}

void draw_line(int x0, int y0, int x1, int y1, int mode)
{
   if(x0>x1||y0>y1)
   {
      int t = x0;
      x0 = x1;
      x1 = t;
      t = y0;
      y0 = y1;
      y1 = t;
   }
   int dx = x1-x0;
   int ix = (dx>0)-(dx<0);
   dx = abs(dx)<<1;
   int dy = y1-y0;
   int iy = (dy>0)-(dy<0);
   dy = abs(dy)<<1;
   draw(x0,y0,mode);

   if(dx>=dy)
   {
      int error = dy-(dx>>1);
      while(x0!=x1)
      {
         if(error>0||(!error&&ix>0))
         {
            error-=dx;
            y0+=iy;
         }

         error+=dy;
         x0+=ix;

         draw(x0,y0,mode);
      }
   }
   else
   {
      int error = dx-(dy>>1);

      while(y0!=y1)
      {
         if(error>0||(!error&&iy>0))
         {
            error-=dy;
            x0+=ix;
         }

         error+=dx;
         y0+=iy;

         draw(x0,y0,mode);
      }
   }
}

static void draw_lines(int count, int *pos)
{
   for(int i = 0;i<count-1;i++)
   {
      draw_line(pos[2*i],pos[2*i+1],pos[2*i+2],pos[2*i+3],0);
   }
}

static void fill_lines(int count, int *pos)
{
   //It's stupid but it works
   int xmin = pos[0];
   int xmax = pos[0];
   int ymin = pos[1];
   int ymax = pos[1];
   for(int i = 0;i<count;i++)
   {
      xmin = RvR_min(pos[2*i],xmin);
      xmax = RvR_max(pos[2*i],xmax);
      ymin = RvR_min(pos[2*i+1],ymin);
      ymax = RvR_max(pos[2*i+1],ymax);
   }

   for(int y = ymin;y<=ymax;y++)
   {
      for(int x = xmin;x<=xmax;x++)
      {
         int inside = 0;
         int crossed = 0;

         for(int i = 0;i<count-1;i++)
         {
            int x0 = pos[2*i]-x;
            int y0 = pos[2*i+1]-y;
            int x1 = pos[2*i+2]-x;
            int y1 = pos[2*i+3]-y;

            //Check if point on line --> abort and set inside
            if(point_on_line(pos[2*i],pos[2*i+1],pos[2*i+2],pos[2*i+3],x,y))
            {
               crossed = 1;
               break;
            }

            if(!RvR_sign_equal(y0,y1))
            {
               if(RvR_sign_equal(x0,x1))
                  crossed+=RvR_negative(x1);
               else
                  crossed+=!RvR_sign_equal(x0*y1-x1*y0,y1);
            }
         }

         inside = RvR_odd(crossed);

         if(inside)
            draw(x,y,1);
      }
   }

   //Outline
   for(int i = 0;i<count-1;i++)
   {
      draw_line(pos[2*i],pos[2*i+1],pos[2*i+2],pos[2*i+3],0);
   }
}

static int point_on_line(int x0, int y0, int x1, int y1, int x, int y)
{
   int dx0 = x-x0;
   int dy0 = y-y0;
   int dx1 = x1-x0;
   int dy1 = y1-y0;

   int cross = dx0*dy1-dy0*dx1;
   if(cross!=0)
      return 0;

   if(RvR_abs(dx1)>=RvR_abs(dy1))
      return dx1>0?x0<=x&&x<=x1:x1<=x&&x<=x0;
   else
      return dy1>0?y0<=y&&y<=y1:y1<=y&&y<=y0;
}
//-------------------------------------
