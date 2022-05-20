/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_LIBC_H_

#define _RVR_LIBC_H_

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

//ctype.h
int isalnum(int c);
int isalpha(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);

//string.h
void  *memchr(const void *ptr, int c, size_t count);
int    memcmp(const void *lhs, const void *rhs, size_t count);
void  *memcpy(void * restrict dest, const void * restrict src, size_t count);
void  *memmove(void *dest, const void *src, size_t count);
void  *memset(void *dest, int c, size_t count);
char  *strcat(char * restrict dest, const char * restrict src);
char  *strchr(const char *str, int c);
int    strcmp(const char *lhs, const char *rhs);
char  *strcpy(char * restrict dest, const char * restrict src);
size_t strcspn(const char *dest, const char *src);
char  *strerror(int errnum);
size_t strlen(const char *str);
char  *strncat(char * restrict dest, const char * restrict src, size_t count);
int    strncmp(const char *lhs, const char *rhs, size_t count);
char  *strncpy(char * restrict dest, const char * restrict src, size_t count);
char  *strpbrk(const char *dest, const char *breakset);
char  *strrchr(const char *str, int c);
size_t strspn(const char *dest, const char *src);
char  *strtok(char * restrict src, const char * restrict delim);

//stdlib.h
void   abort();
int    abs(int n);
int    atoi(const char *s);
void  *bsearch(const void *key, const void *ptr, size_t count, size_t size, int (*cmp)(const void *, const void *));
void  *calloc(size_t num, size_t size);
void   exit(int exit_code);
void   free(void *ptr);
void  *malloc(size_t size);
void   qsort(void *array, size_t count, size_t size, int (*cmp)(const void *, const void *));
int    rand();
void  *realloc(void *ptr, size_t new_size);

//stdio.h
int puts(const char *str);
int putchar(int ch);

#endif
