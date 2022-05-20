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

int puts(const char *str)
{
   register int a0 asm("a0") = (intptr_t)str;
   register int syscall_id asm("a7") = 24;
   asm volatile ("ecall" : "+r"(a0) : "r"(syscall_id));
   return a0;
}

int putchar(int ch)
{
   register int a0 asm("a0") = ch;
   register int syscall_id asm("a7") = 25;
   asm volatile ("ecall" : "+r"(a0) : "r"(syscall_id));
   return a0;
}
//-------------------------------------
