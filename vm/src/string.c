/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/*
 *	NMH's Simple C Compiler, 2011,2014
 *	ctype functions
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

void *memchr(const void *ptr, int c, size_t count)
{
   register int a0 asm("a0") = (intptr_t)ptr;
   register int a1 asm("a1") = c;
   register int a2 asm("a2") = count;
   register int syscall_id asm("a7") = 1;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(a2),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

int memcmp(const void *lhs, const void *rhs, size_t count)
{
   register int a0 asm("a0") = (intptr_t)lhs;
   register int a1 asm("a1") = (intptr_t)rhs;
   register int a2 asm("a2") = count;
   register int syscall_id asm("a7") = 2;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(a2),"r"(syscall_id));
   return a0;
}

void *memcpy(void * restrict dest, const void * restrict src, size_t count)
{
   register int a0 asm("a0") = (intptr_t)dest;
   register int a1 asm("a1") = (intptr_t)src;
   register int a2 asm("a2") = count;
   register int syscall_id asm("a7") = 3;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(a2),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

void *memmove(void *dest, const void *src, size_t count)
{
   register int a0 asm("a0") = (intptr_t)dest;
   register int a1 asm("a1") = (intptr_t)src;
   register int a2 asm("a2") = count;
   register int syscall_id asm("a7") = 4;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(a2),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

void *memset(void *dest, int c, size_t count)
{
   register int a0 asm("a0") = (intptr_t)dest;
   register int a1 asm("a1") = c;
   register int a2 asm("a2") = count;
   register int syscall_id asm("a7") = 5;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(a2),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

char *strcat(char * restrict dest, const char * restrict src)
{
   register int a0 asm("a0") = (intptr_t)dest;
   register int a1 asm("a1") = (intptr_t)src;
   register int syscall_id asm("a7") = 6;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

char *strchr(const char *str, int c)
{
   register int a0 asm("a0") = (intptr_t)str;
   register int a1 asm("a1") = c;
   register int syscall_id asm("a7") = 7;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

int strcmp(const char *lhs, const char *rhs)
{
   register int a0 asm("a0") = (intptr_t)lhs;
   register int a1 asm("a1") = (intptr_t)rhs;
   register int syscall_id asm("a7") = 8;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(syscall_id));
   return a0;
}

char *strcpy(char * restrict dest, const char * restrict src)
{
   register int a0 asm("a0") = (intptr_t)dest;
   register int a1 asm("a1") = (intptr_t)src;
   register int syscall_id asm("a7") = 9;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

size_t strcspn(const char *dest, const char *src)
{
   register int a0 asm("a0") = (intptr_t)dest;
   register int a1 asm("a1") = (intptr_t)src;
   register int syscall_id asm("a7") = 10;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(syscall_id));
   return a0;
}

char *strerror(int errnum)
{
   //TODO: can't be implemented as syscall, since it returns a const string
   return NULL;
}

size_t strlen(const char *str)
{
   register int a0 asm("a0") = (intptr_t)str;
   register int syscall_id asm("a7") = 11;
   asm volatile ("ecall" : "+r"(a0) : "r"(syscall_id));
   return a0;
}

char *strncat(char * restrict dest, const char * restrict src, size_t count)
{
   register int a0 asm("a0") = (intptr_t)dest;
   register int a1 asm("a1") = (intptr_t)src;
   register int a2 asm("a2") = count;
   register int syscall_id asm("a7") = 12;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(a2),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

int strncmp(const char *lhs, const char *rhs, size_t count)
{
   register int a0 asm("a0") = (intptr_t)lhs;
   register int a1 asm("a1") = (intptr_t)rhs;
   register int a2 asm("a2") = count;
   register int syscall_id asm("a7") = 13;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(a2),"r"(syscall_id));
   return a0;
}

char *strncpy(char * restrict dest, const char * restrict src, size_t count)
{
   register int a0 asm("a0") = (intptr_t)dest;
   register int a1 asm("a1") = (intptr_t)src;
   register int a2 asm("a2") = count;
   register int syscall_id asm("a7") = 14;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(a2),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

char *strpbrk(const char *dest, const char *breakset)
{
   register int a0 asm("a0") = (intptr_t)dest;
   register int a1 asm("a1") = (intptr_t)breakset;
   register int syscall_id asm("a7") = 15;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

char *strrchr(const char *str, int c)
{
   register int a0 asm("a0") = (intptr_t)str;
   register int a1 asm("a1") = c;
   register int syscall_id asm("a7") = 16;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(syscall_id));
   return (void *)(intptr_t)a0;
}

size_t strspn(const char *dest, const char *src)
{
   register int a0 asm("a0") = (intptr_t)dest;
   register int a1 asm("a1") = (intptr_t)src;
   register int syscall_id asm("a7") = 17;
   asm volatile ("ecall":"+r"(a0):"r"(a1),"r"(syscall_id));
   return a0;
}

char *strtok(char * restrict src, const char * restrict delim)
{
   static char	*s = NULL;
   char *p;

   if(src==NULL)
      src = s;

   while(*src&&strchr(delim,*src))
      src++;

   if(!*src)
      return NULL;

   for(p = src;*src&&!strchr(delim,*src);src++);

   if(*src&&src[1])
      *src++ = 0;
   s = src;

   return p;
}
//-------------------------------------
