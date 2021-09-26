/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <string.h>
#include <SLK/SLK.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR_error.h"
#include "RvR_malloc.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct Memory_node
{
  int32_t size;
  struct Memory_node *next;
}Memory_node;

typedef struct
{
   int32_t block_size;
   void *addr;
   Memory_node *sfirst, *slast;
}Block_manager;
//-------------------------------------

//Variables
static int bmanage_total = 0;
static int m_instance = 0;
static int mem_break = -1;
static Block_manager bmanage[5] = {0};
//-------------------------------------

//Function prototypes
static void block_manager_init(Block_manager *b, void *block, long block_size);
static void *block_manager_alloc(Block_manager *b, int32_t size);
static void block_manager_free(Block_manager *b, void *ptr);
static long block_manager_pointer_size(void *ptr);
static void block_manager_report(Block_manager *b);
//-------------------------------------

//Function implementations

void RvR_malloc_init(int min, int max)
{
   if(bmanage_total)
   {
      RvR_log("RvR_malloc: already initialized\n");
      return;
   }

   void *mem;
   int32_t size = max;
   for(mem = NULL;!mem&&size>=min;)
   {
      mem = SLK_system_malloc(size);
      if(!mem) 
         size-=0x100;        
   }

   if(mem)
   {
      block_manager_init(&bmanage[bmanage_total],mem,size);
      bmanage_total++; 
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

   m_instance++;
   if(m_instance==mem_break)
      RvR_log("RvR_malloc: mem break\n");

   if(!bmanage_total) 
   {
      return SLK_system_malloc(size);
   }

   size = (size+3)&(0xffffffff-3);

   for(int i = 0;i<bmanage_total;i++)
   {
      void *a = block_manager_alloc(&bmanage[i],size);
      if(a) 
         return a;
   }

   //TODO: try to free some unnecessary allocations in this case
   //      free currently unused textures?
   RvR_log("RvR_malloc: Allocation of size %zu failed, out of memory\n",size);
   RvR_malloc_report();

   return NULL;
}

void RvR_free(void *ptr)
{
   if(!bmanage_total) 
   { 
      SLK_system_free(ptr); 
      return ; 
   }

   for(int i = 0;i<bmanage_total;i++)
   {
      if(ptr>=(void *)bmanage[i].sfirst)  //is the pointer in this block?
      {
         if(ptr<=(void *)bmanage[i].slast)  //is it in static space?
         {
            block_manager_free(&bmanage[i],ptr);
            return ;
         } 
      }
   }

   RvR_log("RvR_malloc: free() bad pointer\n");
}

void *RvR_realloc(void *ptr, size_t size)
{
   if(!ptr) 
      return RvR_malloc(size);

   if(!bmanage_total) 
   {
      void *d = SLK_system_realloc(ptr,size); 
      return d;
   }

   if(size==0) 
   { 
      RvR_free(ptr); 
      return NULL; 
   }

   int32_t old_size = 0;
   for(int i = 0;i<bmanage_total;i++)
   {
      if(ptr>=(void *)bmanage[i].sfirst && 
         ptr<=(void *)(((char *)bmanage[i].sfirst)+bmanage[i].block_size))
      {
         old_size = block_manager_pointer_size(ptr);  

         if(ptr<=(void *)bmanage[i].slast)
         {
            void *nptr = RvR_malloc(size);
            if((int32_t)size>old_size)
               memcpy(nptr,ptr,old_size);
            else
               memcpy(nptr,ptr,size);

            block_manager_free(&bmanage[i],ptr);

            return nptr;
         }
      }
   }

   RvR_log("RvR_malloc: realloc() bad pointer\n");
   return NULL;
}

void RvR_malloc_report()
{
   if(!bmanage_total)
      return;

   for(int i = 0;i<bmanage_total;i++)
   {
      block_manager_report(&bmanage[i]);
   }
}

static void block_manager_init(Block_manager *b, void *block, long block_size)
{
   b->block_size = block_size;
   b->addr = block;

   b->sfirst=(Memory_node *)(((char *)block));   
   b->slast = b->sfirst;
   b->sfirst->size=-(block_size-(int32_t)sizeof(Memory_node));
   b->sfirst->next=NULL;
}

static void *block_manager_alloc(Block_manager *b, int32_t size)
{
   Memory_node *s = b->sfirst;
   if(!s) 
      return NULL;
   for(;s&&-s->size<size;s = s->next);
   if(!s)
      return NULL;
   s->size = -s->size;

   if(s->size-size>(int32_t)sizeof(Memory_node)+4)  //is there enough space to split the block?
   {    
      Memory_node *p = (Memory_node *)((char *)s+sizeof(Memory_node)+size);
      if(s==b->slast)
         b->slast = p;
      p->size = -(s->size-size-(int32_t)sizeof(Memory_node));
      p->next=s->next;
      s->next=p;
      s->size=size;
   }

   return (void *)(((char *)s)+sizeof(Memory_node));
}

static void block_manager_free(Block_manager *b, void *ptr)
{
   Memory_node *o=(Memory_node *)(((char *)ptr)-sizeof(Memory_node)),*last = NULL;

   if(o->next&&o->next->size<0)   //see if we can add into next block
   {
      if (o->next==b->slast)
         b->slast = o;
      o->size+=-o->next->size+sizeof(Memory_node);
      o->next = o->next->next;
   }

   Memory_node *n = b->sfirst;
   for(;n&&n!=o;n=n->next)
      last = n;

   if(last&&last->size<0)
   {
      if(o==b->slast)
         b->slast = last;
      last->next=o->next;
      last->size-=o->size+sizeof(Memory_node);	
   }
   else
      o->size=-o->size;            
}

static long block_manager_pointer_size(void *ptr)
{
   return ((Memory_node *)(((char *)ptr)-sizeof(Memory_node)))->size;
}

static void block_manager_report(Block_manager *b)
{
   RvR_log("************** Block size = %d ***************\n",b->block_size);
   RvR_log("Index\tBase\t\t(Offset)\t      Size\n");
   int i = 0;
   Memory_node * f = b->sfirst;
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
