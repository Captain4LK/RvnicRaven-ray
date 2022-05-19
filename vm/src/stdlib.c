/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/*
 * NMH's Simple C Compiler, 2011,2014
 * ctype functions
 */

//External includes
#include <libc.h>
//-------------------------------------

//Internal includes
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

void abort()
{
   register int a0 asm("a0") = 0;
   register int syscall_id asm("a7") = 18;
   asm volatile ("ecall":"+r"(a0):"r"(syscall_id));

   __builtin_unreachable();
}

int abs(int n)
{
   return n<0?-n:n;
}

int atoi(const char *s)
{
   register int a0 asm("a0") = (intptr_t)s;
   register int syscall_id asm("a7") = 19;
   asm volatile ("ecall":"+r"(a0):"r"(syscall_id));

   return a0;
}

void *bsearch(const void *key, const void *ptr, size_t count, size_t size, int (*cmp)(const void *, const void *))
{
   uint8_t *p, *end;
   int r,r2,delta;

   end = (uint8_t *)ptr+size*count;
   delta = count/2;
   p = (uint8_t *)ptr+delta*size;
   while((r = cmp(p,key))!=0&& delta)
   {
      if(r<0)
         p+=delta*size;
      else
         p-=delta*size;
      delta/=2;
      if(p<(uint8_t *)ptr)
         p = ptr;
      else if(p>=end)
         p = end-size;
   }

   if(r==0)
      return p;
   r2 = r;

   while((r2<0)==(r<0)&&p>=(uint8_t *)ptr&&p<end)
   {
      if(r==0)
         return p;
      else if(r<0)
         p+=size;
      else
         p-=size;
      r = cmp(p,key);
   }

   return r?NULL:p;
}

void *calloc(size_t num, size_t size)
{
   void *mem = malloc(num*size);
   memset(mem,0,num*size);
   return mem;
}

void exit(int exit_code)
{
   register int a0 asm("a0") = exit_code;
   register int syscall_id asm("a7") = 0;
   asm volatile ("ecall":"+r"(a0):"r"(syscall_id));

   __builtin_unreachable();
}

void free(void *ptr)
{
   register int a0 asm("a0") = (intptr_t)ptr;
   register int syscall_id asm("a7") = 20;
   asm volatile ("ecall":"+r"(a0):"r"(syscall_id));
}

void *malloc(size_t size)
{
   register int a0 asm("a0") = size;
   register int syscall_id asm("a7") = 21;
   asm volatile ("ecall":"+r"(a0):"r"(syscall_id));

   return (void *)(intptr_t)a0;
}

void qsort(void *array, size_t count, size_t size, int (*cmp)(const void *, const void *))
{
   int gap,i,j,k,tmp;
   uint8_t *p,*x1,*x2;

   p = array;
   for(gap = count/2;gap>0;gap/=2)
   {
      for(i = gap;i<count;i++)
      {
         for(j = i-gap;j>=0;j-=gap)
         {
            if(cmp(p+j*size,p+(j+gap)*size)<=0)
               break;
            x1 = p+j*size;
            x2 = p+(j+gap)*size;
            for(k = 0;k<size;k++)
            {
               tmp = x1[k];
               x1[k] = x2[k];
               x2[k] = tmp;
            }
         }
      }
   }
}

int rand()
{
   register int a0 asm("a0") = 0;
   register int syscall_id asm("a7") = 22;
   asm volatile ("ecall":"+r"(a0):"r"(syscall_id));

   return a0;
}

void *realloc(void *ptr, size_t new_size)
{
   register int a0 asm("a0") = (intptr_t)ptr;
   register int a1 asm("a1") = new_size;
   register int syscall_id asm("a7") = 23;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}
//-------------------------------------
/*#include <stddef.h>
#include <stdint.h>
#include <string.h>

void exit(int exit_code)
{
   register int a0 asm("a0") = exit_code;
   register int syscall_id asm("a7") = 0;
   asm volatile ("ecall":"+r"(a0):"r"(syscall_id));

   __builtin_unreachable();
}

void *malloc(size_t size)
{
   register int a0 asm("a0") = size;
   register int syscall_id asm("a7") = 1;
   asm volatile ("ecall":"+r"(a0):"r"(syscall_id));

   return (void *)(intptr_t)a0;
}

void *calloc(size_t num, size_t size)
{
   void *mem = malloc(num*size);
   memset(mem,0,num*size);
   return mem;
}

void *realloc(void *ptr, size_t new_size)
{
   register int a0 asm("a0") = (intptr_t)ptr;
   register int a1 asm("a1") = new_size;
   register int syscall_id asm("a7") = 2;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

void free(void *ptr)
{
   register int a0 asm("a0") = (intptr_t)ptr;
   register int syscall_id asm("a7") = 3;
   asm volatile ("ecall":"+r"(a0):"r"(syscall_id));
}*/
