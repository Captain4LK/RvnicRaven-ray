/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
#include "RvnicRaven.h"
#include "RvR_backend.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef struct
{
   const char *name;
   int min_args, max_args;
}lisp_func;

typedef enum 
{
   SYS_FUNC_PRINT = 0,
   SYS_FUNC_CAR = 1,
   SYS_FUNC_CDR = 2,
   SYS_FUNC_LENGTH = 3,
   SYS_FUNC_LIST = 4,
   SYS_FUNC_CONS = 5,
   SYS_FUNC_QUOTE = 6,
   SYS_FUNC_EQ = 7,
   SYS_FUNC_PLUS = 8,
   SYS_FUNC_MINUS = 9,
   SYS_FUNC_IF = 10,
   SYS_FUNC_SETF = 11,
   SYS_FUNC_SYMBOL_LIST = 12,
   SYS_FUNC_ASSOC = 13,
   SYS_FUNC_NULL = 14,
   SYS_FUNC_ACONS = 15,
   SYS_FUNC_PAIRLIS = 16,
   SYS_FUNC_LET = 17,
   SYS_FUNC_DEFUN = 18,
   SYS_FUNC_ATOM = 19,
   SYS_FUNC_NOT = 20,
   SYS_FUNC_AND = 21,
   SYS_FUNC_OR = 22,
   SYS_FUNC_PROGN = 23,
   SYS_FUNC_EQUAL = 24,
   SYS_FUNC_CONCATENATE = 25,
   SYS_FUNC_CHAR_CODE = 26,
   SYS_FUNC_CODE_CHAR = 27,
   SYS_FUNC_TIMES = 28,
   SYS_FUNC_SLASH = 29,
   SYS_FUNC_COND = 30,
   SYS_FUNC_SELECT = 31,
   SYS_FUNC_FUNCTION = 32,
   SYS_FUNC_MAPCAR = 33,
   SYS_FUNC_FUNCALL = 34,
   SYS_FUNC_GT = 35,
   SYS_FUNC_LT = 36,
   SYS_FUNC_TMP_SPACE = 37,
   SYS_FUNC_PERM_SPACE = 38,
   SYS_FUNC_SYMBOL_NAME = 39,
   SYS_FUNC_TRACE = 40,
   SYS_FUNC_UNTRACE = 41,
   SYS_FUNC_DIGSTR = 42,
   SYS_FUNC_COMPILE_FILE = 43,
   SYS_FUNC_ABS = 44,
   SYS_FUNC_MIN = 45,
   SYS_FUNC_MAX = 46,
   SYS_FUNC_GE = 47,
   SYS_FUNC_LE = 48,
   SYS_FUNC_BACKQUOTE = 49,
   SYS_FUNC_COMMA = 50,
   SYS_FUNC_NTH = 51,
   SYS_FUNC_RESIZE_TMP = 52,
   SYS_FUNC_RESIZE_PERM = 53,
   SYS_FUNC_COS = 54,
   SYS_FUNC_SIN = 55,
   SYS_FUNC_ATAN2 = 56,
   SYS_FUNC_ENUM = 57,
   SYS_FUNC_QUIT = 58,
   SYS_FUNC_EVAL = 59,
   SYS_FUNC_BREAK = 60,
   SYS_FUNC_MOD = 61,
   SYS_FUNC_WRITE_PROFILE = 62,
   SYS_FUNC_SETQ = 63,
   SYS_FUNC_FOR = 64,
   SYS_FUNC_OPEN_FILE = 65,
   SYS_FUNC_LOAD = 66,
   SYS_FUNC_BIT_AND = 67,
   SYS_FUNC_BIT_OR = 68,
   SYS_FUNC_BIT_XOR = 69,
   SYS_FUNC_MAKE_ARRAY = 70,
   SYS_FUNC_AREF = 71,
   SYS_FUNC_IF_1PROGN = 72,
   SYS_FUNC_IF_2PROGN = 73,
   SYS_FUNC_IF_12PROGN = 74,
   SYS_FUNC_EQ0 = 75,
   SYS_FUNC_PREPORT = 76,
   SYS_FUNC_SEARCH = 77,
   SYS_FUNC_ELT = 78,
   SYS_FUNC_LISTP = 79,
   SYS_FUNC_NUMBERP = 80,
   SYS_FUNC_DO = 81,
   SYS_FUNC_GC = 82,
   SYS_FUNC_SCHAR = 83,
   SYS_FUNC_SYMBOLP = 84,
   SYS_FUNC_NUM2STR = 85,
   SYS_FUNC_NCONC = 86,
   SYS_FUNC_FIRST = 87,
   SYS_FUNC_SECOND = 88,
   SYS_FUNC_THIRD = 89,
   SYS_FUNC_FOURTH = 90,
   SYS_FUNC_FIFTH = 91,
   SYS_FUNC_SIXTH = 92,
   SYS_FUNC_SEVENTH = 93,
   SYS_FUNC_EIGHTH = 94,
   SYS_FUNC_NINTH = 95,
   SYS_FUNC_TENTH = 96,
   SYS_FUNC_SUBSTR = 97,
   SYS_FUNC_LOCAL_LOAD = 98,
}lisp_sys_func_index;
//-------------------------------------

//Variables
const lisp_func lisp_sys_funcs[] =
{
    { "print", 1, -1 }, /* 0 */
    { "car", 1, 1 }, /* 1 */
    { "cdr", 1, 1 }, /* 2 */
    { "length", 0, -1 }, /* 3 */
    { "list", 0, -1 }, /* 4 */
    { "cons", 2, 2 }, /* 5 */
    { "quote", 1, 1 }, /* 6 */
    { "eq", 2, 2 }, /* 7 */
    { "+", 0, -1 }, /* 8 */
    { "-", 1, -1 }, /* 9 */
    { "if", 2, 3 }, /* 10 */
    { "setf", 2, 2 }, /* 11 */
    { "symbol-list", 0, 0 }, /* 12 */
    { "assoc", 2, 2 }, /* 13 */
    { "null", 1, 1 }, /* 14 */
    { "acons", 2, 2 }, /* 15 */
    { "pairlis", 2, 2 }, /* 16 */
    { "let", 1, -1 }, /* 17 */
    { "defun", 2, -1 }, /* 18 */
    { "atom", 1, 1 }, /* 19 */
    { "not", 1, 1 }, /* 20 */
    { "and", -1, -1 }, /* 21 */
    { "or", -1, -1 }, /* 22 */
    { "progn", -1, -1 }, /* 23 */
    { "equal", 2, 2 }, /* 24 */
    { "concatenate", 1, -1 }, /* 25 */
    { "char-code", 1, 1 }, /* 26 */
    { "code-char", 1, 1 }, /* 27 */
    { "*", -1, -1 }, /* 28 */
    { "/", 1, -1 }, /* 29 */
    { "cond", -1, -1 }, /* 30 */
    { "select", 1, -1 }, /* 31 */
    { "function", 1, 1 }, /* 32 */
    { "mapcar", 2, -1 }, /* 33 */
    { "funcall", 1, -1 }, /* 34 */
    { ">", 2, 2 }, /* 35 */
    { "<", 2, 2 }, /* 36 */
    { "tmp-space", 0, 0 }, /* 37 */
    { "perm-space", 0, 0 }, /* 38 */
    { "symbol-name", 1, 1 }, /* 39 */
    { "trace", 0, -1 }, /* 40 */
    { "untrace", 0, -1 }, /* 41 */
    { "digstr", 2, 2 }, /* 42 */
    { "compile-file", 1, 1 }, /* 43 */
    { "abs", 1, 1 }, /* 44 */
    { "min", 2, 2 }, /* 45 */
    { "max", 2, 2 }, /* 46 */
    { ">=", 2, 2 }, /* 47 */
    { "<=", 2, 2 }, /* 48 */
    { "backquote", 1, 1 }, /* 49 */
    { "comma", 1, 1 }, /* 50 */
    { "nth", 2, 2 }, /* 51 */
    { "resize-tmp", 1, 1 }, /* 52 */
    { "resize-perm", 1, 1 }, /* 53 */
    { "cos", 1, 1 }, /* 54 */
    { "sin", 1, 1 }, /* 55 */
    { "atan2", 2, 2 }, /* 56 */
    { "enum", 1, -1 }, /* 57 */
    { "quit", 0, 0 }, /* 58 */
    { "eval", 1, 1 }, /* 59 */
    { "break", 0, 0 }, /* 60 */
    { "mod", 2, 2 }, /* 61 */
    { "write_profile", 1, 1 }, /* 62 */
    { "setq", 2, 2 }, /* 63 */
    { "for", 4, -1 }, /* 64 */
    { "open_file", 2, -1 }, /* 65 */
    { "load", 1, 1 }, /* 66 */
    { "bit-and", 1, -1 }, /* 67 */
    { "bit-or", 1, -1 }, /* 68 */
    { "bit-xor", 1, -1 }, /* 69 */
    { "make-array", 1, -1 }, /* 70 */
    { "aref", 2, 2 }, /* 71 */
    { "if-1progn", 2, 3 }, /* 72 */
    { "if-2progn", 2, 3 }, /* 73 */
    { "if-12progn", 2, 3 }, /* 74 */
    { "eq0", 1, 1 }, /* 75 */
    { "preport", 1, 1 }, /* 76 */
    { "search", 2, 2 }, /* 77 */
    { "elt", 2, 2 }, /* 78 */
    { "listp", 1, 1 }, /* 79 */
    { "numberp", 1, 1 }, /* 80 */
    { "do", 2, 3 }, /* 81 */
    { "gc", 0, 0 }, /* 82 */
    { "schar", 2, 2 }, /* 83 */
    { "symbolp", 1, 1 }, /* 84 */
    { "num2str", 1, 1 }, /* 85 */
    { "nconc", 2, -1 }, /* 86 */
    { "first", 1, 1 }, /* 87 */
    { "second", 1, 1 }, /* 88 */
    { "third", 1, 1 }, /* 89 */
    { "fourth", 1, 1 }, /* 90 */
    { "fifth", 1, 1 }, /* 91 */
    { "sixth", 1, 1 }, /* 92 */
    { "seventh", 1, 1 }, /* 93 */
    { "eighth", 1, 1 }, /* 94 */
    { "ninth", 1, 1 }, /* 95 */
    { "tenth", 1, 1 }, /* 96 */
    { "substr", 3, 3 }, /* 97 */
    { "local_load", 1, 1 }, /* 98 */
};

static int lisp_print_level = 0;
static RvR_rw *lisp_current_print_file = NULL;
static RvR_lisp_object *lisp_symbol_root = NULL;
static size_t lisp_symbol_count = 0;
static int lisp_break_level = 0;
static RvR_lisp_space lisp_current_space;
static uint8_t *lisp_free_space[4] = {0};
static uint8_t *lisp_space[4] = {0};
static size_t lisp_space_size[4] = {0};
static void *lisp_colon_initial_contents = NULL;
static void *lisp_colon_initial_element = NULL;
static void *lisp_undefined = NULL;
static void *lisp_true_symbol = NULL;

struct
{
   void ***data;
   unsigned data_size;
   unsigned data_used;
}lisp_ptr_ref = {0};
//-------------------------------------

//Function prototypes
static void lisp_l1print(void *block);
static void lisp_lprint_string(const char *st);
static void lisp_where_print(int max_level);
static void lisp_ptr_ref_push(void **ref);
static void **lisp_ptr_ref_pop(size_t total);
static void lisp_need_perm_space(const char *why);
static size_t lisp_get_free_size(int which_space);
static void *lisp_malloc(size_t size, int which_space);
static void lisp_collect_space(int which_space, int grow);
static char *lisp_error(const char *loc, const char *cause);
static void lisp_delete_all_symbols(RvR_lisp_object *root);
static void *lisp_pairlis(void *list1, void *list2, void *list3);
static RvR_lisp_object *lisp_add_sys_function(const char *name, int min_args, int max_args, int number);
//-------------------------------------

//Function implementations

uint32_t RvR_lisp_item_type(void *x)
{
   if(x!=NULL)
      return ((RvR_lisp_object *)x)->type;
   return RVR_L_CONS_CELL;
}

void RvR_lisp_object_print(RvR_lisp_object *o)
{
   char buf[32];

   lisp_print_level++;

   if(o==NULL)
   {
      lisp_lprint_string("nil");
      return;
   }

   switch(RvR_lisp_item_type(o))
   {
   case RVR_L_CONS_CELL:
      {
         RvR_lisp_object *cs = o;
         lisp_lprint_string("(");
         for(;cs!=NULL;cs = RvR_lisp_cdr(cs))
         {
            if(RvR_lisp_item_type(cs)==RVR_L_CONS_CELL)
            {
               RvR_lisp_object_print(cs->car);
               if(cs->cdr!=NULL)
                  lisp_lprint_string(" ");
            }
            else
            {
               lisp_lprint_string(". ");
               RvR_lisp_object_print(cs);
               cs = NULL;
            }
         }
         lisp_lprint_string(")");
      }
      break;
   case RVR_L_NUMBER:
      snprintf(buf,32,"%ld",o->num);
      lisp_lprint_string(buf);
      break;
   case RVR_L_SYMBOL:
      lisp_lprint_string(RvR_lisp_string_get(o->name));
      break;
   case RVR_L_USER_FUNCTION:
   case RVR_L_SYS_FUNCTION:
      lisp_lprint_string("err... function?");
      break;
   case RVR_L_C_FUNCTION:
      lisp_lprint_string("C function, returns number\n");
      break;
   case RVR_L_C_BOOL:
      lisp_lprint_string("C boolean function\n");
      break;
   case RVR_L_L_FUNCTION:
      lisp_lprint_string("External lisp function\n");
      break;
   case RVR_L_STRING:
      if(lisp_current_print_file!=NULL)
         lisp_lprint_string(RvR_lisp_string_get(o));
      else
         RvR_log("\"%s\"",RvR_lisp_string_get(o));
      break;
   case RVR_L_POINTER:
      snprintf(buf,32,"%p",RvR_lisp_pointer_get(o));
      lisp_lprint_string(buf);
      break;
   case RVR_L_FIXED_POINT:
      snprintf(buf,32,"%g",(RvR_lisp_fixed_point_get(o)>>16)+((RvR_lisp_fixed_point_get(o)&0xffff))/(double)(0x10000));
      lisp_lprint_string(buf);
      break;
   case RVR_L_CHARACTER:
      if(lisp_current_print_file!=NULL)
      {
         uint8_t ch = o->ch;
         RvR_rw_write_u8(lisp_current_print_file,ch);
      }
      else
      {
         uint16_t ch = o->ch;
         RvR_log("#\\");
         switch(ch)
         {
         case '\n': RvR_log("newline"); break;
         case ' ': RvR_log("space"); break;
         default: RvR_log("%c",ch); break;
         }
      }
      break;
   case RVR_L_OBJECT_VAR:
      //TODO: l_obj_print
      break;
   case RVR_L_1D_ARRAY:
      {
         RvR_lisp_object *a = o;
         RvR_lisp_object **data = o->data;
         RvR_log("#(");
         for(size_t j = 0;j<o->len;j++)
         {
            RvR_lisp_object_print(data[j]);
            if(j!=o->len-1)
               RvR_log(" ");
         }
         RvR_log(")");
      }
      break;
   case RVR_L_COLLECTED_OBJECT:
      lisp_lprint_string("GC_reference->");
      RvR_lisp_object_print(o->ref);
      break;
   default:
      RvR_log("Shouldn't happen\n");
      break;
   }

   lisp_print_level--;
   if(!lisp_print_level&&lisp_current_print_file==NULL)
      RvR_log("\n");
}

RvR_lisp_object *RvR_lisp_cdr(void *c)
{
   if(c==NULL)
      return NULL;

   if(RvR_lisp_item_type(c)==RVR_L_CONS_CELL)
      return ((RvR_lisp_object *)c)->cdr;
   return NULL;
}

RvR_lisp_object *RvR_lisp_car(void *c)
{
   if(c==NULL)
      return NULL;

   if(RvR_lisp_item_type(c)==RVR_L_CONS_CELL)
      return ((RvR_lisp_object *)c)->car;
   return NULL;
}

char *RvR_lisp_string_get(RvR_lisp_object *o)
{
   if(o==NULL)
      return NULL;

#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(o)!=RVR_L_STRING)
   {
      RvR_lisp_object_print(o);
      RvR_lisp_break(" is not a string\n");
      exit(0);
   }
#endif

   return o->str;
}

void *RvR_lisp_pointer_get(RvR_lisp_object *o)
{
   if(o==NULL||RvR_lisp_item_type(o)!=RVR_L_POINTER)
      return NULL;
   return o->addr;
}

uint32_t RvR_lisp_fixed_point_get(RvR_lisp_object *o)
{
   if(o==NULL||RvR_lisp_item_type(o)!=RVR_L_FIXED_POINT)
      return 0;
   return o->x;
}

void RvR_lisp_break(const char *format, ...)
{
   lisp_break_level++;
   RvR_rw *old_file = lisp_current_print_file;
   lisp_current_print_file = NULL;
   char st[300];

   va_list ap;
   va_start(ap,format);
   vsnprintf(st,300,format,ap);
   va_end(ap);
   RvR_log("%s\n",st);
   
   int cont = 0;
   do
   {
      RvR_log("type q to quit\n");
      RvR_log("%d. Break> ",lisp_break_level);
      fgets(st,300,stdin);

      if(!strcmp(st,"c")||!strcmp(st,"cont")||!strcmp(st,"continue"))
         cont = 1;
      else if(!strcmp(st,"w")||!strcmp(st,"where"))
         lisp_where_print(-1);
      else if(!strcmp(st,"q")||!strcmp(st,"quit"))
         exit(1);
      else if (!strcmp(st, "e") || !strcmp(st, "env") || !strcmp(st, "environment"))
         RvR_log("Enviorment : \nnot supported right now\n");
      else if (!strcmp(st, "h") || !strcmp(st, "help") || !strcmp(st, "?"))
      {
         RvR_log("CLIVE Debugger\n");
         RvR_log(" w, where : show calling parents\n"
         " e, env   : show environment\n"
         " c, cont  : continue if possible\n"
         " q, quit  : quits the program\n"
         " h, help  : this\n");
      }
      else
      {
         const char *s = st;
         do
         {
            //TODO
            //RvR_lisp_object *prog = LObject::Compile(s);
            //PtrRef r1(prog);
            while(*s==' '||*s=='\t'||*s=='\r'||*s=='\n')
               s++;
            //prog->Eval()->Print();
         }while(*s);
      }
   }while(!cont);
   
   lisp_current_print_file = old_file;
   lisp_break_level--;
}

void RvR_lisp_print_trace_stack(int max_levels)
{
   lisp_where_print(max_levels);
}

void *RvR_lisp_mark_heap(int heap)
{
   return lisp_free_space[heap];
}

void RvR_lisp_restore_heap(void *val, int heap)
{
   lisp_free_space[heap] = val;
}

void *RvR_lisp_eval_block(void *list)
{
   lisp_ptr_ref_push(&list);
   void *ret = NULL;
   while(list!=NULL)
   {
      ret = RvR_lisp_eval(((RvR_lisp_object *)list)->car);
      list = ((RvR_lisp_object *)list)->cdr;
   }

   lisp_ptr_ref_pop(1);
   return ret;
}

RvR_lisp_object *RvR_lisp_eval(RvR_lisp_object *o)
{
   //TODO
}

RvR_lisp_object *RvR_lisp_array_create(size_t len, void *rest)
{
   lisp_ptr_ref_push(&rest);

   size_t size = sizeof(RvR_lisp_object)+len*sizeof(RvR_lisp_object *);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->data = (void *)(p+1);
   p->type = RVR_L_1D_ARRAY;
   p->len = len;
   RvR_lisp_object **data = p->data;

   lisp_ptr_ref_push(&p);

   if(rest!=NULL)
   {
      RvR_lisp_object *x = RvR_lisp_eval(((RvR_lisp_object *)rest)->car);
      if(x==lisp_colon_initial_contents)
      {
         x = RvR_lisp_eval(((RvR_lisp_object *)rest)->cdr->car);
         RvR_lisp_object **data = p->data;

         for(size_t i = 0;i<len;i++,x = ((RvR_lisp_object *)x)->cdr)
         {
            if(x==NULL)
            {
               RvR_lisp_object_print(rest);
               RvR_lisp_break("(make-array) incorrect list length\n");
               exit(0);
            }
            data[i] = ((RvR_lisp_object *)x)->car;
         }

         if(x!=NULL)
         {
            RvR_lisp_object_print(rest);
            RvR_lisp_break("(make-array) incorrect list length\n");
            exit(0);
         }
      }
      else if(x==lisp_colon_initial_element)
      {
         x = RvR_lisp_eval(((RvR_lisp_object *)rest)->cdr->car);
         data = p->data;
         for(size_t i = 0;i<len;i++)
            data[i] = x;
      }
      else
      {
         RvR_lisp_object_print(x);
         RvR_lisp_break("Bad option argument to make-array\n");
         exit(0);
      }
   }

   lisp_ptr_ref_pop(1); //p
   lisp_ptr_ref_pop(1); //rest

   return p;
}

RvR_lisp_object *RvR_lisp_fixed_point_create(int32_t x)
{
   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_FIXED_POINT;
   p->x = x;

   return p;
}

RvR_lisp_object *RvR_lisp_object_var_create(int index)
{
   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_OBJECT_VAR;
   p->index = index;

   return p;
}

RvR_lisp_object *RvR_lisp_pointer_create(void *addr)
{
   if(addr==NULL)
      return NULL;

   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_POINTER;
   p->addr = addr;

   return p;
}

RvR_lisp_object *RvR_lisp_char_create(uint16_t ch)
{
   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_CHARACTER;
   p->ch = ch;

   return p;
}

RvR_lisp_object *RvR_lisp_strings_create(const char *string)
{
   RvR_lisp_object *p = RvR_lisp_stringl_create(strlen(string)+1);
   strcpy(p->str,string);

   return p;
}

RvR_lisp_object *RvR_lisp_stringsl_create(const char *string, int length)
{
   RvR_lisp_object *p = RvR_lisp_stringl_create(length+1);
   memcpy(p->str,string,length);
   p->str[length] = '\0';

   return p;
}

RvR_lisp_object *RvR_lisp_stringl_create(int length)
{
   size_t size = sizeof(RvR_lisp_object)+length;
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->str = (void *)(p+1);
   p->type = RVR_L_STRING;
   p->str[0] = '\0';

   return p;
}

RvR_lisp_object *RvR_lisp_user_function_create(RvR_lisp_object *arg_list, RvR_lisp_object *block_list)
{
   lisp_ptr_ref_push(&arg_list);
   lisp_ptr_ref_push(&block_list);

   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_USER_FUNCTION;
   p->arg_list = arg_list;
   p->block_list = block_list;

   lisp_ptr_ref_pop(1); //block_list
   lisp_ptr_ref_pop(1); //arg_list

   return p;
}

RvR_lisp_object *RvR_lisp_sys_function_create(int min_args, int max_args, int fun_number)
{
   size_t size = sizeof(RvR_lisp_object);
   int space = (lisp_current_space==RVR_LISP_GC_SPACE)?RVR_LISP_GC_SPACE:RVR_LISP_PERM_SPACE;
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_SYS_FUNCTION;
   p->min_args = min_args;
   p->max_args = max_args;
   p->fun_number = fun_number;

   return p;
}

RvR_lisp_object *RvR_lisp_c_function_create(int min_args, int max_args, int fun_number)
{
   RvR_lisp_object *p = RvR_lisp_sys_function_create(min_args,max_args,fun_number);
   p->type = RVR_L_C_FUNCTION;

   return p;
}

RvR_lisp_object *RvR_lisp_c_bool_create(int min_args, int max_args, int fun_number)
{
   RvR_lisp_object *p = RvR_lisp_sys_function_create(min_args,max_args,fun_number);
   p->type = RVR_L_C_BOOL;

   return p;
}

RvR_lisp_object *RvR_lisp_user_lisp_function_create(int min_args, int max_args, int fun_number)
{
   RvR_lisp_object *p = RvR_lisp_sys_function_create(min_args,max_args,fun_number);
   p->type = RVR_L_L_FUNCTION;

   return p;
}

RvR_lisp_object *RvR_lisp_symbol_create(char *name)
{
   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);

   lisp_ptr_ref_push(&p);

   p->type = RVR_L_SYMBOL;
   p->name = RvR_lisp_strings_create(name);
   p->value = lisp_undefined;
   p->function = lisp_undefined;

#ifndef RVR_LISP_PROFLE
   p->time_taken = 0;
#endif

   lisp_ptr_ref_pop(1);

   return p;
}

RvR_lisp_object *RvR_lisp_number_create(uint32_t num)
{
   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_NUMBER;
   p->num = num;

   return p;
}

RvR_lisp_object *RvR_lisp_list_create()
{
   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_CONS_CELL;
   p->car = NULL;
   p->cdr = NULL;

   return p;
}

void *RvR_lisp_nth(int num, void *list)
{
   if(num<0)
   {
      RvR_lisp_break("NTH: %d is not a nonnegative fixnum and therefore not a valid index\n", num);
      exit(1);
   }

   while(list!=NULL&&num)
   {
      list = ((RvR_lisp_object *)list)->cdr;
      num--;
   }

   if(list==NULL)
      return NULL;
   return ((RvR_lisp_object *)list)->car;
}

void *RvR_lisp_pointer_value(void *pointer)
{
   if(pointer==NULL)
      return NULL;

#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(pointer)!=RVR_L_POINTER)
   {
      RvR_lisp_object_print(pointer);
      RvR_lisp_break(" is not a pointer\n");
      exit(0);
   }
#endif

   return ((RvR_lisp_object *)pointer)->addr;
}

int32_t RvR_lisp_number_value(void *number)
{
   switch(RvR_lisp_item_type(number))
   {
   case RVR_L_NUMBER: return ((RvR_lisp_object *)number)->num;
   case RVR_L_FIXED_POINT: return (((RvR_lisp_object *)number)->x)>>16;
   //TODO
   default:
      RvR_lisp_object_print(number);
      RvR_lisp_break(" is not a number\n");
      exit(0);
   }

   return 0;
}

void *RvR_lisp_atom(void *i)
{
   if(RvR_lisp_item_type(i)==RVR_L_CONS_CELL)
      return NULL;
   return lisp_true_symbol;
}

uint16_t RvR_lisp_character_value(void *c)
{
   if(c==NULL)
      return 0;

#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(c)!=RVR_L_CHARACTER)
   {
      RvR_lisp_object_print(c);
      RvR_lisp_break(" is not a character\n");
      exit(0);
   }
#endif

   return ((RvR_lisp_object *)c)->ch;
}

uint32_t RvR_lisp_fixed_point_value(void *c)
{
   switch(RvR_lisp_item_type(c))
   {
   case RVR_L_NUMBER: return (((RvR_lisp_object *)c)->num)<<16;
   case RVR_L_FIXED_POINT: return ((RvR_lisp_object *)c)->x;
   default:
      RvR_lisp_object_print(c);
      RvR_lisp_break(" is not a number\n");
      exit(0);
   }

   return 0;
}

void *RvR_lisp_eq(void *n1, void *n2)
{
   if(n1==NULL&&n2==NULL)
      return lisp_true_symbol;
   if((n1==NULL&&n2!=NULL)||(n1!=NULL&&n2==NULL))
      return NULL;

   uint32_t t1 = RvR_lisp_item_type(n1);
   uint32_t t2 = RvR_lisp_item_type(n2);
   if(t1!=t2)
      return NULL;
   if(t1==RVR_L_NUMBER)
      return ((RvR_lisp_object *)n1)->num==((RvR_lisp_object *)n2)->num?lisp_true_symbol:NULL;
   if(t1==RVR_L_CHARACTER)
      return ((RvR_lisp_object *)n1)->ch==((RvR_lisp_object *)n2)->ch?lisp_true_symbol:NULL;
   if(n1==n2)
      return lisp_true_symbol;
   //if(t1==RVR_L_POINTER)

   return NULL;
}

RvR_lisp_object *RvR_lisp_array_get(RvR_lisp_object *o, int x)
{
#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(o)!=RVR_L_1D_ARRAY)
   {
      RvR_lisp_object_print(o);
      RvR_lisp_break(" is not an array\n");
      exit(0);
   }
#endif

   if(x>=((RvR_lisp_object *)o)->len||x<0)
   {
      RvR_lisp_break("array reference out of bounds (%d)\n",x);
      exit(0);
   }

   return ((RvR_lisp_object *)o)->data[x];
}

void *RvR_lisp_equal(void *n1, void *n2)
{
   if(n1==NULL&&n2==NULL) //if both nil, then equal
      return lisp_true_symbol;

   if(n1==NULL||n2==NULL) //one nil, nope
      return NULL;

   uint32_t t1 = RvR_lisp_item_type(n1), t2 = RvR_lisp_item_type(n2);
   if(t1!=t2)
      return NULL;

   switch(t1)
   {
   case RVR_L_STRING :
      if(!strcmp(RvR_lisp_string_get(n1),RvR_lisp_string_get(n2)))
         return lisp_true_symbol;
      return NULL;
   case RVR_L_CONS_CELL :
      while(n1!=NULL&&n2!=NULL) //loop through the list and compare each element
      {
         if(!RvR_lisp_equal(((RvR_lisp_object *)n1)->car,((RvR_lisp_object *)n2)->car))
            return NULL;
         n1 = ((RvR_lisp_object *)n1)->cdr;
         n2 = ((RvR_lisp_object *)n2)->cdr;
         if(n1&&RvR_lisp_item_type(n1)!=RVR_L_CONS_CELL)
            return RvR_lisp_equal(n1,n2);
      }
      if(n1!=NULL||n2!=NULL)
         return NULL;   //if one is longer than the other
      return lisp_true_symbol;
   default :
      return RvR_lisp_eq(n1, n2);
   }
}

RvR_lisp_object *RvR_lisp_symbol_find(const char *name)
{
   RvR_lisp_object *p = lisp_symbol_root;
   while(p!=NULL)
   {
      int cmp = strcmp(name,RvR_lisp_string_get(p->name));
      if(cmp==0)
         return p;
      p = (cmp<0)?p->left:p->right;
   }
   return NULL;
}

RvR_lisp_object *RvR_lisp_symbol_find_or_create(const char *name)
{
   RvR_lisp_object *p = lisp_symbol_root;
   RvR_lisp_object **parent = &lisp_symbol_root;

   while(p!=NULL)
   {
      int cmp = strcmp(name,RvR_lisp_string_get(p->name));
      if(cmp==0)
         return p;
      parent = (cmp<0)?&p->left:&p->right;
      p = *parent;
   }

   //Make sure all symbols get defined in permanant space
   int sp = lisp_current_space;
   if(lisp_current_space!=RVR_LISP_GC_SPACE)
      lisp_current_space = RVR_LISP_PERM_SPACE;

   p = RvR_malloc(sizeof(*p));
   p->type = RVR_L_SYMBOL;
   p->name = RvR_lisp_strings_create(name);

   //If constant, set the value to ourself
   p->value = (name[0] ==':')?p:lisp_undefined;
   p->function = lisp_undefined;
#ifdef L_PROFILE
   p->time_taken = 0;
#endif
   p->left = p->right = NULL;
   *parent = p;

   lisp_symbol_count++;
   lisp_current_space = sp;

   return p;
}

void *RvR_lisp_assoc(void *item, void *list)
{
   if(RvR_lisp_item_type(list)!=RVR_L_CONS_CELL)
      return NULL;

   while(list!=NULL)
   {
      if(RvR_lisp_eq(((RvR_lisp_object *)list)->car->car,item))
         return RvR_lisp_car(list);
      list = ((RvR_lisp_object *)list)->cdr;
   }

   return NULL;
}

size_t RvR_lisp_list_get_length(RvR_lisp_object *list)
{
   size_t ret = 0;

#ifdef RVR_LISP_TYPE_CHECK
   if(list!=NULL&&RvR_lisp_item_type(list)!=RVR_L_CONS_CELL)
   {
      RvR_lisp_object_print(list);
      RvR_lisp_break(" is not a sequence\n");
      exit(0);
   }
#endif

   for(RvR_lisp_object *p = list;p!=NULL;p = p->cdr)
      ret++;

   return ret;
}

void RvR_lisp_symbol_function_set(RvR_lisp_object *sym, RvR_lisp_object *fun)
{
   sym->function = fun;
}

static void lisp_l1print(void *block)
{
   if(block==NULL||RvR_lisp_item_type(block)!=RVR_L_CONS_CELL)
   {
      RvR_lisp_object_print(block);
      return;
   }

   RvR_log("(");
   for(;block!=NULL&&RvR_lisp_item_type(block)==RVR_L_CONS_CELL;block = ((RvR_lisp_object *)block)->cdr)
   {
      void *a = ((RvR_lisp_object *)block)->car;
      if(RvR_lisp_item_type(a)==RVR_L_CONS_CELL)
         RvR_log("[...]");
      else
         RvR_lisp_object_print(a);
   }

   if(block!=NULL)
   {
      RvR_log(" . ");
      RvR_lisp_object_print(block);
   }

   RvR_log("(");
}

static void lisp_lprint_string(const char *st)
{
   if(lisp_current_print_file!=NULL)
   {
      for(const char *s = st;*s;s++)
         RvR_rw_write_u8(lisp_current_print_file,(uint8_t)*s);
   }
   else
   {
      RvR_log(st);
   }
}

static void lisp_where_print(int max_level)
{
   RvR_log("Main program\n");
   if(max_level==-1)
      max_level = lisp_ptr_ref.data_used;
   else if(max_level>=lisp_ptr_ref.data_used)
      max_level = lisp_ptr_ref.data_used-1;

   for(int i = 0;i<max_level;i++)
   {
      RvR_log("%d> ",i);
      RvR_lisp_object_print(*lisp_ptr_ref.data[i]);
   }
}

static void lisp_ptr_ref_push(void **ref)
{
   if(lisp_ptr_ref.data==NULL)
   {
      lisp_ptr_ref.data_size = 1024;
      lisp_ptr_ref.data_used = 0;
      lisp_ptr_ref.data = RvR_malloc(sizeof(*lisp_ptr_ref.data)*lisp_ptr_ref.data_size);
   }

   lisp_ptr_ref.data[lisp_ptr_ref.data_used++] = ref;

   if(lisp_ptr_ref.data_used==lisp_ptr_ref.data_size)
   {
      RvR_lisp_break("error: stack overflow (%d>=%d)\n",lisp_ptr_ref.data_used,lisp_ptr_ref.data_size);
      exit(1);
   }
}

static void **lisp_ptr_ref_pop(size_t total)
{
   if(total>lisp_ptr_ref.data_used)
   {
      RvR_lisp_break("error: stack underflow\n");
      exit(1);
   }

   lisp_ptr_ref.data_used-=total;
   return lisp_ptr_ref.data[lisp_ptr_ref.data_used];
}

static void lisp_need_perm_space(const char *why)
{
   if(lisp_current_space!=RVR_LISP_PERM_SPACE&&lisp_current_space!=RVR_LISP_GC_SPACE)
   {
      RvR_lisp_break("%s: action requires permanent space\n",why);
      exit(0);
   }
}

static size_t lisp_get_free_size(int which_space)
{
   size_t used = lisp_free_space[which_space]-lisp_space[which_space];
   return lisp_space_size[which_space]>used?lisp_space_size[which_space]-used:0;
}

static void *lisp_malloc(size_t size, int which_space)
{
   //Align allocation
   size = (size+sizeof(intptr_t)-1)&~(sizeof(intptr_t)-1);

   //Collect garbage if necessary
   if(size>lisp_get_free_size(which_space))
   {
      if(which_space==RVR_LISP_PERM_SPACE||which_space==RVR_LISP_TMP_SPACE)
         lisp_collect_space(which_space, 0);

      if(size>lisp_get_free_size(which_space))
         lisp_collect_space(which_space, 1);

      if(size>lisp_get_free_size(which_space))
      {
         RvR_lisp_break("lisp: cannot find %d bytes in space #%d\n",size,which_space);
         exit(0);
      }
   }

   void *ret = (void *)lisp_free_space[which_space];
   lisp_free_space[which_space] += size;

   return ret;
}

static void lisp_collect_space(int which_space, int grow)
{
   //TODO
}

static char *lisp_error(const char *loc, const char *cause)
{
   int lines = 0;
   if(loc!=NULL)
   {
      for(lines = 0;*loc&&lines<10;loc++)
      {
         if(*loc=='\n') lines++;
         RvR_log("%c",*loc);
      }
      RvR_log("\nPROGRAM LOCATION : \n");
   }
   if(cause!=NULL)
      RvR_log("ERROR MESSAGE : %s\n", cause);
   RvR_lisp_break("");
   exit(0);

   return NULL;
}

static void lisp_delete_all_symbols(RvR_lisp_object *root)
{
   if(root!=NULL)
   {
      lisp_delete_all_symbols(root->left);
      lisp_delete_all_symbols(root->right);
      RvR_free(root);
   }
}

static void *lisp_pairlis(void *list1, void *list2, void *list3)
{
   if(RvR_lisp_item_type(list1)!=RVR_L_CONS_CELL||RvR_lisp_item_type(list1)!=RvR_lisp_item_type(list2))
      return NULL;

   void *ret = NULL;
   size_t l1 = RvR_lisp_list_get_length(list1);
   size_t l2 = RvR_lisp_list_get_length(list2);

   if(l1!=l2)
   {
      RvR_lisp_object_print(list1);
      RvR_lisp_object_print(list2);
      RvR_lisp_break("... are not the same length (pairlis)\n");
      exit(0);
   }
   
   if(l1!=0)
   {
      RvR_lisp_object *first = NULL, *last = NULL, *cur = NULL;
      RvR_lisp_object *tmp;

      lisp_ptr_ref_push(&first);
      lisp_ptr_ref_push(&last);
      lisp_ptr_ref_push(&cur);

      while(list1)
      {
         cur = RvR_lisp_list_create();
         if(first==NULL)
            first = cur;
         if(last!=NULL)
            last->cdr = cur;
         last = cur;

         RvR_lisp_object *cell = RvR_lisp_list_create();
         tmp = RvR_lisp_car(list1);
         cell->car = tmp;
         tmp = RvR_lisp_car(list2);
         cell->cdr = tmp;
         cur->car = cell;

         list1 = ((RvR_lisp_object *)list1)->cdr;
         list2 = ((RvR_lisp_object *)list2)->cdr;
      }

      cur->cdr = list3;
      ret=first;

      lisp_ptr_ref_pop(1); //cur
      lisp_ptr_ref_pop(1); //last
      lisp_ptr_ref_pop(1); //first
   }

   return ret;
}

static RvR_lisp_object *lisp_add_sys_function(const char *name, int min_args, int max_args, int number)
{
   lisp_need_perm_space("add_sys_function");
   RvR_lisp_object *s = RvR_lisp_symbol_find_or_create(name);
   if(s->function!=lisp_undefined)
   {
      RvR_lisp_break("add_sys_fucntion -> symbol %s already has a function\n",name);
      exit(0);
   }

   s->function = RvR_lisp_sys_function_create(min_args, max_args, number);

   return s;
}
//-------------------------------------
