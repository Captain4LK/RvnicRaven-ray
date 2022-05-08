/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct Malloc_memory_node
{
  int32_t size;
  struct Malloc_memory_node *next;
}Malloc_memory_node;

typedef struct
{
   int32_t block_size;
   void *addr;
   Malloc_memory_node *sfirst, *slast;
}Malloc_block_manager;
//-------------------------------------

//Variables
static int malloc_bmanage_total = 0;
static int malloc_instance = 0;
static Malloc_block_manager malloc_bmanage = {0};
//-------------------------------------

//Function prototypes
static void  malloc_block_init(Malloc_block_manager *b, void *block, long block_size);
static void *malloc_block_alloc(Malloc_block_manager *b, int32_t size);
static void  malloc_block_free(Malloc_block_manager *b, void *ptr);
static long  malloc_block_pointer_size(void *ptr);
static void  malloc_block_report(Malloc_block_manager *b);
//-------------------------------------

//Function implementations

void RvR_malloc_init(size_t min, size_t max)
{
   void *mem = NULL;
   size_t size = max;

   if(malloc_bmanage_total)
   {
      RvR_log("RvR_malloc: already initialized\n");
      return;
   }

   //Clamp min to a minimun value to prevent underflow of size_t
   min = RvR_max(0x100,min);

   //This won't work on operating systems where malloc cannont fail
   //and the programm instead just gets killed.
   for(mem = NULL;mem==NULL&&size>=min;)
   {
      mem = malloc(size);

      if(mem==NULL) 
         size-=0x100;        
   }

   if(mem!=NULL)
   {
      malloc_block_init(&malloc_bmanage,mem,size);
      malloc_bmanage_total++; 
      RvR_log("RvR_malloc: allocated %d bytes for allocator\n",size);
   }  
   else
   {
      RvR_log("RvR_malloc: not enough memory, using system malloc()\n");
   }
}

void *RvR_malloc(size_t size)
{
   if(size==0)
      RvR_log("RvR_malloc: tried to malloc 0 bytes\n");

   if(!malloc_bmanage_total) 
   {
      return malloc(size);
   }

   malloc_instance++;
   if(malloc_instance==-1)
      RvR_log("RvR_malloc: mem break\n");

   size = (size+3)&(0xffffffff-3);

   void *mem = malloc_block_alloc(&malloc_bmanage,size);
   if(mem!=NULL) 
      return mem;

   //TODO: try to free some unnecessary allocations in this case
   //      free currently unused textures?
   RvR_log("RvR_malloc: Allocation of size %zu failed, out of memory\n",size);
   RvR_malloc_report();

   return NULL;
}

void *RvR_malloc_base()
{
   return malloc_bmanage.addr;
}

void RvR_free(void *ptr)
{
   if(!malloc_bmanage_total) 
   { 
      free(ptr); 
      return ; 
   }

   if(ptr>=(void *)malloc_bmanage.sfirst)  //is the pointer in this block?
   {
      if(ptr<=(void *)malloc_bmanage.slast)  //is it in static space?
      {
         malloc_block_free(&malloc_bmanage,ptr);
         return ;
      } 
   }

   RvR_log("RvR_malloc: free() bad pointer\n");
}

void *RvR_realloc(void *ptr, size_t size)
{
   if(ptr==NULL) 
      return RvR_malloc(size);

   if(!malloc_bmanage_total) 
   {
      void *d = realloc(ptr,size); 
      return d;
   }

   if(size==0) 
   { 
      RvR_free(ptr); 
      return NULL; 
   }

   int32_t old_size = 0;
   if(ptr>=(void *)malloc_bmanage.sfirst && 
      ptr<=(void *)(((char *)malloc_bmanage.sfirst)+malloc_bmanage.block_size))
   {
      old_size = malloc_block_pointer_size(ptr);  

      if(ptr<=(void *)malloc_bmanage.slast)
      {
         void *nptr = RvR_malloc(size);
         if((int32_t)size>old_size)
            memcpy(nptr,ptr,old_size);
         else
            memcpy(nptr,ptr,size);

         malloc_block_free(&malloc_bmanage,ptr);

         return nptr;
      }
   }

   RvR_log("RvR_malloc: realloc() bad pointer\n");
   return NULL;
}

void RvR_malloc_report()
{
   if(!malloc_bmanage_total)
   {
      RvR_log("RvR_malloc: using system allocator, memory report not possible\n");
      return;
   }

   malloc_block_report(&malloc_bmanage);
}

static void malloc_block_init(Malloc_block_manager *b, void *block, long block_size)
{
   b->block_size = block_size;
   b->addr = block;

   b->sfirst = (Malloc_memory_node *)(((char *)block));   
   b->slast = b->sfirst;
   b->sfirst->size = -(block_size-(int32_t)sizeof(Malloc_memory_node));
   b->sfirst->next = NULL;
}

static void *malloc_block_alloc(Malloc_block_manager *b, int32_t size)
{
   Malloc_memory_node *s = b->sfirst;
   if(s==NULL) 
      return NULL;
   for(;s&&-s->size<size;s = s->next);
   if(s==NULL)
      return NULL;
   s->size = -s->size;

   if(s->size-size>(int32_t)sizeof(Malloc_memory_node)+4)  //is there enough space to split the block?
   {    
      Malloc_memory_node *p = (Malloc_memory_node *)((char *)s+sizeof(Malloc_memory_node)+size);
      if(s==b->slast)
         b->slast = p;
      p->size = -(s->size-size-(int32_t)sizeof(Malloc_memory_node));
      p->next=s->next;
      s->next=p;
      s->size=size;
   }

   return (void *)(((char *)s)+sizeof(Malloc_memory_node));
}

static void malloc_block_free(Malloc_block_manager *b, void *ptr)
{
   Malloc_memory_node *o = (Malloc_memory_node *)(((char *)ptr)-sizeof(Malloc_memory_node)),*last = NULL;

   if(o->next&&o->next->size<0)   //see if we can add into next block
   {
      if(o->next==b->slast)
         b->slast = o;
      o->size+=-o->next->size+sizeof(Malloc_memory_node);
      o->next = o->next->next;
   }

   Malloc_memory_node *n = b->sfirst;
   for(;n&&n!=o;n=n->next)
      last = n;

   if(last&&last->size<0)
   {
      if(o==b->slast)
         b->slast = last;
      last->next=o->next;
      last->size-=o->size+sizeof(Malloc_memory_node);	
   }
   else
   {
      o->size=-o->size;            
   }
}

static long malloc_block_pointer_size(void *ptr)
{
   return ((Malloc_memory_node *)(((char *)ptr)-sizeof(Malloc_memory_node)))->size;
}

static void malloc_block_report(Malloc_block_manager *b)
{
   RvR_log("************** Block size = %d ***************\n",b->block_size);
   RvR_log("Index\tBase\t\t(Offset)\t      Size\n");
   int i = 0;
   Malloc_memory_node * f = b->sfirst;
   int32_t f_total = 0, a_total = 0;

   for(;f;f = f->next,i++)
   {
      RvR_log("%4d\t%p\t(%10ld)\t%10d",i,f,((char *)f-(char *)b->sfirst),f->size);
      if(f->size>0)
      {
         a_total+=f->size;
      }
      else
      {
         f_total+=-f->size;
      }

      RvR_log("\n");
   }

   RvR_log("**************** Block summary : %d free, %d allocated\n",f_total,a_total);
}
//-------------------------------------
