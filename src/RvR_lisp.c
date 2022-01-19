/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
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
static int lisp_trace_level = 0;
static int lisp_trace_print_level = 0;
static RvR_rw *lisp_current_print_file = NULL;
static RvR_lisp_object *lisp_symbol_root = NULL;
static size_t lisp_symbol_count = 0;
static int lisp_break_level = 0;
static RvR_lisp_space lisp_current_space;

static uint8_t *lisp_free_space[4] = {0};
static uint8_t *lisp_space[4] = {0};
static size_t lisp_space_size[4] = {0};

static RvR_lisp_object *lisp_undefined = NULL;
static RvR_lisp_object *lisp_true_symbol = NULL;
static RvR_lisp_object *lisp_list_symbol = NULL;
static RvR_lisp_object *lisp_string_symbol = NULL;
static RvR_lisp_object *lisp_quote_symbol = NULL;
static RvR_lisp_object *lisp_backquote_symbol = NULL;
static RvR_lisp_object *lisp_comma_symbol = NULL;
static RvR_lisp_object *lisp_in_symbol = NULL;
static RvR_lisp_object *lisp_do_symbol = NULL;
static RvR_lisp_object *lisp_aref_symbol = NULL;
static RvR_lisp_object *lisp_colon_initial_contents = NULL;
static RvR_lisp_object *lisp_colon_initial_element = NULL;
static RvR_lisp_object *lisp_if1_progn = NULL;
static RvR_lisp_object *lisp_if2_progn = NULL;
static RvR_lisp_object *lisp_if12_progn = NULL;
static RvR_lisp_object *lisp_if_symbol = NULL;
static RvR_lisp_object *lisp_progn_symbol = NULL;
static RvR_lisp_object *lisp_not_symbol = NULL;
static RvR_lisp_object *lisp_eq_symbol = NULL;
static RvR_lisp_object *lisp_zero_symbol = NULL;
static RvR_lisp_object *lisp_eq0_symbol = NULL;
static RvR_lisp_object *lisp_car_symbol = NULL;
static RvR_lisp_object *lisp_cdr_symbol = NULL;
static RvR_lisp_object *lisp_load_warning = NULL;

static int lisp_total_user_functions = 0;
static char lisp_n[RVR_LISP_MAX_TOKEN_LEN] = {0};

static uint8_t *lisp_cstart, *lisp_cend, *lisp_collected_start, *lisp_collected_end;

static RvR_lisp_cinit_func lisp_cinit = NULL;
static RvR_lisp_ccall_func lisp_ccall = NULL;
static RvR_lisp_lcall_func lisp_lcall = NULL;
static RvR_lisp_lset_func lisp_lset = NULL;
static RvR_lisp_lget_func lisp_lget = NULL;
static RvR_lisp_lprint_func lisp_lprint = NULL;

struct
{
   RvR_lisp_object ***data;
   unsigned data_size;
   unsigned data_used;
}lisp_ptr_ref = {0};

struct
{
   RvR_lisp_object **data;
   unsigned data_size;
   unsigned data_used;
}lisp_user_stack = {0};
//-------------------------------------

//Function prototypes
static void lisp_print(RvR_lisp_object *block);
static void lisp_print_string(const char *st);
static void lisp_where_print(int max_level);
static void lisp_ptr_ref_push(RvR_lisp_object **ref);
static RvR_lisp_object **lisp_ptr_ref_pop(size_t total);
static void lisp_need_perm_space(const char *why);
static size_t lisp_get_free_size(int which_space);
static void *lisp_malloc(size_t size, int which_space);
static void lisp_collect_space(int which_space, int grow);
static void lisp_collect_symbols(RvR_lisp_object *root);
static void lisp_collect_stacks();
static RvR_lisp_object *lisp_collect_object(RvR_lisp_object *x);
static RvR_lisp_object *lisp_collect_array(RvR_lisp_object *x);
static RvR_lisp_object *lisp_collect_list(RvR_lisp_object *x);
static char *lisp_error(const char *loc, const char *cause);
static void lisp_delete_all_symbols(RvR_lisp_object *root);
static RvR_lisp_object *lisp_pairlis(RvR_lisp_object *list1, RvR_lisp_object *list2, RvR_lisp_object *list3);
static RvR_lisp_object *lisp_add_sys_function(const char *name, int min_args, int max_args, int number);

static void lisp_skip_c_comment(const char **s);
static long lisp_str_token_len(const char *st);

static RvR_lisp_object *lisp_comp_optimize(RvR_lisp_object *list);

static void lisp_pro_print(RvR_rw *out, RvR_lisp_object *p);
static void lisp_preport(char *fn);

static RvR_lisp_object *lisp_mapcar(RvR_lisp_object *arg_list);
static RvR_lisp_object *lisp_concatenate(RvR_lisp_object *prog_list);

static RvR_lisp_object *lisp_backquote_eval(RvR_lisp_object *args);

static void lisp_user_stack_push(RvR_lisp_object *data);
static RvR_lisp_object *lisp_user_stack_pop(size_t total);
//-------------------------------------

//Function implementations

//Get type of item
uint32_t RvR_lisp_item_type(RvR_lisp_object *x)
{
   if(x!=NULL)
      return x->type;
   return RVR_L_CONS_CELL;
}

void RvR_lisp_object_print(RvR_lisp_object *o)
{
   if(o==NULL)
   {
      lisp_print_string("nil");
      return;
   }

   char buf[32] = {0};
   lisp_print_level++;

   switch(RvR_lisp_item_type(o))
   {
   case RVR_L_CONS_CELL:
   {
      RvR_lisp_object *cs = o;
      lisp_print_string("(");
      for(;cs!=NULL;cs = RvR_lisp_cdr(cs))
      {
         if(RvR_lisp_item_type(cs)==RVR_L_CONS_CELL)
         {
            RvR_lisp_object_print(cs->obj.list.car);
            if(cs->obj.list.cdr!=NULL)
               lisp_print_string(" ");
         }
         else
         {
            lisp_print_string(". ");
            RvR_lisp_object_print(cs);
            cs = NULL;
         }
      }
      lisp_print_string(")");
      break;
   }
   case RVR_L_NUMBER:
      snprintf(buf,32,"%"PRId32,o->obj.num.num);
      lisp_print_string(buf);
      break;
   case RVR_L_SYMBOL:
      lisp_print_string(RvR_lisp_string(o->obj.sym.name));
      break;
   case RVR_L_USER_FUNCTION:
   case RVR_L_SYS_FUNCTION:
      lisp_print_string("err... function?");
      break;
   case RVR_L_C_FUNCTION:
      lisp_print_string("C function, returns number");
      break;
   case RVR_L_C_BOOL:
      lisp_print_string("C boolean function");
      break;
   case RVR_L_L_FUNCTION:
      lisp_print_string("External lisp function");
      break;
   case RVR_L_STRING:
      if(lisp_current_print_file!=NULL)
         lisp_print_string(RvR_lisp_string(o));
      else
         RvR_log("\"%s\"",RvR_lisp_string(o));
      break;
   case RVR_L_POINTER:
      snprintf(buf,32,"%p",RvR_lisp_pointer(o));
      lisp_print_string(buf);
      break;
   case RVR_L_FIXED_POINT:
      snprintf(buf,32,"%g",(RvR_lisp_fixed_point(o)>>16)+((RvR_lisp_fixed_point(o)&0xffff))/(double)(0x10000));
      lisp_print_string(buf);
      break;
   case RVR_L_CHARACTER:
      if(lisp_current_print_file!=NULL)
      {
         uint8_t ch = o->obj.ch.ch;
         RvR_rw_write_u8(lisp_current_print_file,ch);
      }
      else
      {
         uint16_t ch = o->obj.ch.ch;
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
      lisp_lprint(o->obj.var.index);
      break;
   case RVR_L_1D_ARRAY:
      {
         RvR_lisp_object *a = o;
         RvR_lisp_object **data = o->obj.arr.data;
         RvR_log("#(");
         for(size_t j = 0;j<o->obj.arr.len;j++)
         {
            RvR_lisp_object_print(data[j]);
            if(j!=a->obj.arr.len-1)
               RvR_log(" ");
         }
         RvR_log(")");
      }
      break;
   case RVR_L_COLLECTED_OBJECT:
      lisp_print_string("GC_reference->");
      RvR_lisp_object_print(o->obj.ref.ref);
      break;
   default:
      RvR_log("Shouldn't happen\n");
      break;
   }

   lisp_print_level--;
   if(!lisp_print_level&&lisp_current_print_file==NULL)
      RvR_log("\n");
}

RvR_lisp_object *RvR_lisp_cdr(RvR_lisp_object *o)
{
   if(o==NULL)
      return NULL;

   if(RvR_lisp_item_type(o)==RVR_L_CONS_CELL)
      return o->obj.list.cdr;
   return NULL;
}

RvR_lisp_object *RvR_lisp_car(RvR_lisp_object *o)
{
   if(o==NULL)
      return NULL;

   if(RvR_lisp_item_type(o)==RVR_L_CONS_CELL)
      return o->obj.list.car;
   return NULL;
}

char *RvR_lisp_string(RvR_lisp_object *o)
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

   return o->obj.str.str;
}

void *RvR_lisp_pointer(RvR_lisp_object *o)
{
   if(o==NULL)
      return NULL;

#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(o)!=RVR_L_POINTER)
   {
      RvR_lisp_object_print(o);
      RvR_lisp_break(" is not a pointer\n");
      exit(0);
   }
#endif

   return o->obj.addr.addr;
}

int32_t RvR_lisp_fixed_point(RvR_lisp_object *o)
{
   if(o==NULL)
      return 0;

#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(o)!=RVR_L_FIXED_POINT)
   {
      RvR_lisp_object_print(o);
      RvR_lisp_break(" is not a fixed point number\n");
      exit(0);
   }
#endif

   return o->obj.fix.x;
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
            RvR_lisp_object *prog = RvR_lisp_compile(&s);
            lisp_ptr_ref_push(&prog);
            while(*s==' '||*s=='\t'||*s=='\r'||*s=='\n')
               s++;

            RvR_lisp_object_print(RvR_lisp_eval(prog));

            lisp_ptr_ref_pop(1); //prog
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

RvR_lisp_object *RvR_lisp_eval_block(RvR_lisp_object *list)
{
   lisp_ptr_ref_push(&list);
   void *ret = NULL;
   while(list!=NULL)
   {
      ret = RvR_lisp_eval(list->obj.list.car);
      list = list->obj.list.cdr;
   }

   lisp_ptr_ref_pop(1); //list
   return ret;
}

RvR_lisp_object *RvR_lisp_eval(RvR_lisp_object *o)
{
   lisp_ptr_ref_push(&o);

   int tstart = lisp_trace_level;

   if(lisp_trace_level)
   {
      if(lisp_trace_level<=lisp_trace_print_level)
      {
         RvR_log("%d (%d, %d, %d) TRACE : ",lisp_trace_level,lisp_get_free_size(RVR_LISP_PERM_SPACE),lisp_get_free_size(RVR_LISP_TMP_SPACE),lisp_ptr_ref.data_used);
         RvR_lisp_object_print(o);
         RvR_log("\n");

      }
      lisp_trace_level++;
   }

   RvR_lisp_object *ret = NULL;

   if(o!=NULL)
   {
      switch(RvR_lisp_item_type(o))
      {
      case RVR_L_BAD_CELL:
         RvR_lisp_break("error: eval on bad cell\n");
         exit(0);
         break;
      case RVR_L_CHARACTER:
      case RVR_L_STRING:
      case RVR_L_NUMBER:
      case RVR_L_POINTER:
      case RVR_L_FIXED_POINT:
         ret = o;
         break;
      case RVR_L_SYMBOL:
         if(o==lisp_true_symbol)
         {
            ret = o;
         }
         else
         {
            ret = o->obj.sym.value;
            if(RvR_lisp_item_type(ret)==RVR_L_OBJECT_VAR)
               ret = lisp_lget(ret->obj.var.index);
         }
         break;
      case RVR_L_CONS_CELL:
         ret = RvR_lisp_eval_function(o->obj.list.car,o->obj.list.cdr);
         break;
      default:
         fprintf(stderr,"shouldn't happen\n");
         break;
      }
   }

   if(tstart)
   {
      lisp_trace_level--;
      if(lisp_trace_level<=lisp_trace_print_level)
         RvR_log("%d (%d, %d, %d) TRACE ==> ",lisp_trace_level,lisp_get_free_size(RVR_LISP_PERM_SPACE),lisp_get_free_size(RVR_LISP_TMP_SPACE),lisp_ptr_ref.data_used);
      RvR_lisp_object_print(ret);
      RvR_log("\n");
   }

   lisp_ptr_ref_pop(1); //o

   return ret;
}

RvR_lisp_object *RvR_lisp_array_create(size_t len, RvR_lisp_object *rest)
{
   lisp_ptr_ref_push(&rest);

   size_t size = sizeof(RvR_lisp_object)+len*sizeof(RvR_lisp_object *);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->obj.arr.data = (void *)(p+1);
   p->type = RVR_L_1D_ARRAY;
   p->obj.arr.len = len;
   RvR_lisp_object **data = p->obj.arr.data;

   lisp_ptr_ref_push(&p);

   if(rest!=NULL)
   {
      RvR_lisp_object *x = RvR_lisp_eval(rest->obj.list.car);
      if(x==lisp_colon_initial_contents)
      {
         x = RvR_lisp_eval(rest->obj.list.cdr->obj.list.car);
         data = p->obj.arr.data;

         for(size_t i = 0;i<len;i++,x = x->obj.list.cdr)
         {
            if(x==NULL)
            {
               RvR_lisp_object_print(rest);
               RvR_lisp_break("(make-array) incorrect list length\n");
               exit(0);
            }
            data[i] = x->obj.list.car;
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
         x = RvR_lisp_eval(rest->obj.list.cdr->obj.list.car);
         data = p->obj.arr.data;
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
   p->obj.fix.x = x;

   return p;
}

RvR_lisp_object *RvR_lisp_object_var_create(int index)
{
   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_OBJECT_VAR;
   p->obj.var.index = index;

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
   p->obj.addr.addr = addr;

   return p;
}

RvR_lisp_object *RvR_lisp_char_create(uint16_t ch)
{
   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_CHARACTER;
   p->obj.ch.ch = ch;

   return p;
}

RvR_lisp_object *RvR_lisp_strings_create(const char *string)
{
   RvR_lisp_object *p = RvR_lisp_stringl_create(strlen(string)+1);
   strcpy(p->obj.str.str,string);

   return p;
}

RvR_lisp_object *RvR_lisp_stringsl_create(const char *string, int length)
{
   RvR_lisp_object *p = RvR_lisp_stringl_create(length+1);
   memcpy(p->obj.str.str,string,length);
   p->obj.str.str[length] = '\0';

   return p;
}

RvR_lisp_object *RvR_lisp_stringl_create(int length)
{
   size_t size = sizeof(RvR_lisp_object)+length;
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->obj.str.str = (void *)(p+1);
   p->type = RVR_L_STRING;
   p->obj.str.str[0] = '\0';

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
   p->obj.usr.arg_list = arg_list;
   p->obj.usr.block_list = block_list;

   lisp_ptr_ref_pop(1); //block_list
   lisp_ptr_ref_pop(1); //arg_list

   return p;
}

RvR_lisp_object *RvR_lisp_sys_function_create(int min_args, int max_args, int fun_number)
{
   size_t size = sizeof(RvR_lisp_object);
   int space = (lisp_current_space==RVR_LISP_GC_SPACE)?RVR_LISP_GC_SPACE:RVR_LISP_PERM_SPACE;
   RvR_lisp_object *p = lisp_malloc(size,space);
   memset(p,0,size);
   p->type = RVR_L_SYS_FUNCTION;
   p->obj.sys.min_args = min_args;
   p->obj.sys.max_args = max_args;
   p->obj.sys.fun_number = fun_number;

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
   p->obj.sym.name = RvR_lisp_strings_create(name);
   p->obj.sym.value = lisp_undefined;
   p->obj.sym.function = lisp_undefined;

#ifndef RVR_LISP_PROFLE
   p->obj.sym.time_taken = 0;
#endif

   lisp_ptr_ref_pop(1); //p

   return p;
}

RvR_lisp_object *RvR_lisp_number_create(int32_t num)
{
   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_NUMBER;
   p->obj.num.num = num;

   return p;
}

RvR_lisp_object *RvR_lisp_list_create()
{
   size_t size = sizeof(RvR_lisp_object);
   RvR_lisp_object *p = lisp_malloc(size,lisp_current_space);
   memset(p,0,size);
   p->type = RVR_L_CONS_CELL;
   p->obj.list.car = NULL;
   p->obj.list.cdr = NULL;

   return p;
}

RvR_lisp_object *RvR_lisp_nth(int num, RvR_lisp_object *list)
{
   if(num<0)
   {
      RvR_lisp_break("NTH: %d is not a nonnegative fixnum and therefore not a valid index\n", num);
      exit(1);
   }

   while(list!=NULL&&num)
   {
      list = list->obj.list.cdr;
      num--;
   }

   if(list==NULL)
      return NULL;
   return list->obj.list.car;
}

void *RvR_lisp_pointer_value(RvR_lisp_object *pointer)
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

   return pointer->obj.addr.addr;
}

int32_t RvR_lisp_number_value(RvR_lisp_object *number)
{
   switch(RvR_lisp_item_type(number))
   {
   case RVR_L_NUMBER: return number->obj.num.num;
   case RVR_L_FIXED_POINT: return (number->obj.fix.x)>>16;
   case RVR_L_STRING: return *RvR_lisp_string(number);
   case RVR_L_CHARACTER: return RvR_lisp_character_value(number);
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

uint16_t RvR_lisp_character_value(RvR_lisp_object *c)
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

   return c->obj.ch.ch;
}

int32_t RvR_lisp_fixed_point_value(RvR_lisp_object *c)
{
   switch(RvR_lisp_item_type(c))
   {
   case RVR_L_NUMBER: return (c->obj.num.num)<<16;
   case RVR_L_FIXED_POINT: return c->obj.fix.x;
   default:
      RvR_lisp_object_print(c);
      RvR_lisp_break(" is not a number\n");
      exit(0);
   }

   return 0;
}

RvR_lisp_object *RvR_lisp_eq(RvR_lisp_object *n1, RvR_lisp_object *n2)
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
      return n1->obj.num.num==n2->obj.num.num?lisp_true_symbol:NULL;
   if(t1==RVR_L_CHARACTER)
      return n1->obj.ch.ch==n2->obj.ch.ch?lisp_true_symbol:NULL;
   if(n1==n2)
      return lisp_true_symbol;

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

   if(x>=o->obj.arr.len||x<0)
   {
      RvR_lisp_break("array reference out of bounds (%d)\n",x);
      exit(0);
   }

   return o->obj.arr.data[x];
}

RvR_lisp_object *RvR_lisp_equal(RvR_lisp_object *n1, RvR_lisp_object *n2)
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
      if(!strcmp(RvR_lisp_string(n1),RvR_lisp_string(n2)))
         return lisp_true_symbol;
      return NULL;
   case RVR_L_CONS_CELL :
      while(n1!=NULL&&n2!=NULL) //loop through the list and compare each element
      {
         if(!RvR_lisp_equal(n1->obj.list.car,n2->obj.list.car))
            return NULL;
         n1 = n1->obj.list.cdr;
         n2 = n2->obj.list.cdr;
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
      int cmp = strcmp(name,RvR_lisp_string(p->obj.sym.name));
      if(cmp==0)
         return p;
      p = (cmp<0)?p->obj.sym.left:p->obj.sym.right;
   }
   return NULL;
}

RvR_lisp_object *RvR_lisp_symbol_find_or_create(const char *name)
{
   RvR_lisp_object *p = lisp_symbol_root;
   RvR_lisp_object **parent = &lisp_symbol_root;

   while(p!=NULL)
   {
      int cmp = strcmp(name,RvR_lisp_string(p->obj.sym.name));
      if(cmp==0)
         return p;
      parent = (cmp<0)?&p->obj.sym.left:&p->obj.sym.right;
      p = *parent;
   }

   //Make sure all symbols get defined in permanant space
   int sp = lisp_current_space;
   if(lisp_current_space!=RVR_LISP_GC_SPACE)
      lisp_current_space = RVR_LISP_PERM_SPACE;

   p = RvR_malloc(sizeof(*p));
   p->type = RVR_L_SYMBOL;
   p->obj.sym.name = RvR_lisp_strings_create(name);

   //If constant, set the value to ourself
   p->obj.sym.value = (name[0] ==':')?p:lisp_undefined;
   p->obj.sym.function = lisp_undefined;
#ifdef L_PROFILE
   p->time_taken = 0;
#endif
   p->obj.sym.left = p->obj.sym.right = NULL;
   *parent = p;

   lisp_symbol_count++;
   lisp_current_space = sp;

   return p;
}

RvR_lisp_object *RvR_lisp_assoc(RvR_lisp_object *item, RvR_lisp_object *list)
{
   if(RvR_lisp_item_type(list)!=RVR_L_CONS_CELL)
      return NULL;

   while(list!=NULL)
   {
      if(RvR_lisp_eq(list->obj.list.car->obj.list.car,item))
         return RvR_lisp_car(list);
      list = list->obj.list.cdr;
   }

   return NULL;
}

size_t RvR_lisp_list_length(RvR_lisp_object *list)
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

   for(RvR_lisp_object *p = list;p!=NULL;p = p->obj.list.cdr)
      ret++;

   return ret;
}

void RvR_lisp_symbol_function_set(RvR_lisp_object *sym, RvR_lisp_object *fun)
{
   sym->obj.sym.function = fun;
}

RvR_lisp_object *RvR_lisp_add_c_object(RvR_lisp_object *symbol, int index)
{
   lisp_need_perm_space("RvR_lisp_add_c_object");
   RvR_lisp_object *s = symbol;
   if(s->obj.sym.value!=lisp_undefined)
   {
      RvR_lisp_break("RvR_lisp_add_c_object -> symbol %s already has a value\n",RvR_lisp_string(s->obj.sym.name));
      exit(0);
   }

   s->obj.sym.value = RvR_lisp_object_var_create(index);
   return NULL;
}

RvR_lisp_object *RvR_lisp_add_c_function(const char *name, int min_args, int max_args, int number)
{
   lisp_total_user_functions++;
   lisp_need_perm_space("RvR_lisp_add_c_function");
   RvR_lisp_object *s = RvR_lisp_symbol_find_or_create(name);
   if(s->obj.sym.function!=lisp_undefined)
   {
      RvR_lisp_break("RvR_lisp_add_c_function --> symbol %s already has a function\n",name);
      exit(0);
   }

   s->obj.sym.function = RvR_lisp_c_function_create(min_args,max_args,number);

   return s;
}

RvR_lisp_object *RvR_lisp_add_c_bool_function(const char *name, int min_args, int max_args, int number)
{
   lisp_total_user_functions++;
   lisp_need_perm_space("RvR_lisp_add_c_bool_function");
   RvR_lisp_object *s = RvR_lisp_symbol_find_or_create(name);
   if(s->obj.sym.function!=lisp_undefined)
   {
      RvR_lisp_break("RvR_lisp_add_c_bool_function --> symbol %s already has a function\n",name);
      exit(0);
   }

   s->obj.sym.function = RvR_lisp_c_bool_create(min_args,max_args,number);

   return s;
}

RvR_lisp_object *RvR_lisp_add_lisp_function(const char *name, int min_args, int max_args, int number)
{
   lisp_total_user_functions++;
   lisp_need_perm_space("RvR_lisp_add_lisp_function");
   RvR_lisp_object *s = RvR_lisp_symbol_find_or_create(name);
   if(s->obj.sym.function!=lisp_undefined)
   {
      RvR_lisp_break("RvR_lisp_add_lisp_function --> symbol %s already has a function\n",name);
      exit(0);
   }

   s->obj.sym.function = RvR_lisp_user_lisp_function_create(min_args,max_args,number);

   return s;
}

int RvR_lisp_read_token(const char **s, char *buffer)
{
   //skip space
   while (**s==' '||**s=='\t'||**s=='\n'||**s=='\r'||**s==26)
      (*s)++;
   if(**s==';')  //comment
   {
      while (**s&&**s!='\n'&&**s!='\r'&&**s!=26)
         (*s)++;
      return RvR_lisp_read_token(s,buffer);
   }
   else if(**s=='/'&&*((*s)+1)=='*')   //c style comment
   {
      lisp_skip_c_comment(s);
      return RvR_lisp_read_token(s,buffer);
   }
   else if(**s==0)
      return 0;
   else if(**s==')'||**s=='('||**s=='\''||**s=='`'||**s==','||**s==26)
   {
      *(buffer++)=*((*s)++);
      *buffer = 0;
   }
   else if(**s=='"')    //string
   {
      *(buffer++)=*((*s)++);          //don't read off the string because it
                                   //may be to long to fit in the token buffer
                                   //so just read the '"' so the compiler knows to scan the rest.
      *buffer = 0;
   }
   else if(**s=='#')
   {
      *(buffer++) = *((*s)++);
      if(**s!='\'')
         *(buffer++) = *((*s)++);
      *buffer = 0;
   }
   else
   {
      while(**s&&**s!=')'&&**s!='('&&**s!=' '&&**s!='\n'&&**s!='\r'&&**s!='\t'&&**s!=';'&&**s!=26)
         *(buffer++) = *((*s)++);
      *buffer = 0;
   }

   return 1;
}

int RvR_lisp_end_of_program(const char *s)
{
   return !RvR_lisp_read_token(&s,lisp_n);
}

void     RvR_lisp_push_onto_list(RvR_lisp_object *object, RvR_lisp_object **list)
{
   lisp_ptr_ref_push(&object);
   lisp_ptr_ref_push(list);

   RvR_lisp_object *c = RvR_lisp_list_create();
   c->obj.list.car = object;
   c->obj.list.cdr = *list;
   *list = c;

   lisp_ptr_ref_pop(1); //list
   lisp_ptr_ref_pop(1); //object
}

RvR_lisp_object *RvR_lisp_compile(const char **s)
{
   RvR_lisp_object *ret = NULL;

   if(!RvR_lisp_read_token(s,lisp_n))
      lisp_error(NULL,"unexpected end of program");

   if(!strcmp(lisp_n,"nil"))
      return NULL;
   if(toupper(lisp_n[0]=='T'&&!lisp_n[1]))
      return lisp_true_symbol;
   if(lisp_n[0]=='\'')
   {
      RvR_lisp_object *cs = RvR_lisp_list_create();
      RvR_lisp_object *c2 = NULL;
      RvR_lisp_object *tmp = NULL;

      lisp_ptr_ref_push(&cs);
      lisp_ptr_ref_push(&c2);

      cs->obj.list.car = lisp_quote_symbol;
      c2 = RvR_lisp_list_create();
      tmp = RvR_lisp_compile(s);
      c2->obj.list.car = tmp;
      c2->obj.list.cdr = NULL;
      cs->obj.list.cdr = c2;
      ret = cs;

      lisp_ptr_ref_pop(1); //c2
      lisp_ptr_ref_pop(1); //cs
   }
   else if(lisp_n[0]=='`')
   {
      RvR_lisp_object *cs = RvR_lisp_list_create();
      RvR_lisp_object *c2 = NULL;
      RvR_lisp_object *tmp = NULL;

      lisp_ptr_ref_push(&cs);
      lisp_ptr_ref_push(&c2);

      cs->obj.list.car = lisp_backquote_symbol;
      c2 = RvR_lisp_list_create();
      tmp = RvR_lisp_compile(s);
      c2->obj.list.car = tmp;
      c2->obj.list.cdr = NULL;
      cs->obj.list.cdr = c2;
      ret = cs;

      lisp_ptr_ref_pop(1); //c2
      lisp_ptr_ref_pop(1); //cs
   }
   else if(lisp_n[0]==',')
   {
      RvR_lisp_object *cs = RvR_lisp_list_create();
      RvR_lisp_object *c2 = NULL;
      RvR_lisp_object *tmp = NULL;

      lisp_ptr_ref_push(&cs);
      lisp_ptr_ref_push(&c2);

      cs->obj.list.car = lisp_comma_symbol;
      c2 = RvR_lisp_list_create();
      tmp = RvR_lisp_compile(s);
      c2->obj.list.car = tmp;
      c2->obj.list.cdr = NULL;
      cs->obj.list.cdr = c2;
      ret = cs;

      lisp_ptr_ref_pop(1); //c2
      lisp_ptr_ref_pop(1); //cs
   }
   else if(lisp_n[0]=='(')
   {
      RvR_lisp_object *first = NULL;
      RvR_lisp_object *cur = NULL;
      RvR_lisp_object *last = NULL;

      lisp_ptr_ref_push(&first);
      lisp_ptr_ref_push(&cur);
      lisp_ptr_ref_push(&last);

      int done = 0;
      do
      {
         const char *tmp = *s;
         if(!RvR_lisp_read_token(&tmp,lisp_n))
            lisp_error(NULL,"unexpeted end of program");
         if(lisp_n[0]==')')
         {
            done = 1;
            RvR_lisp_read_token(s,lisp_n);
         }
         else
         {
            if(lisp_n[0]=='.'&&!lisp_n[1])
            {
               if(first==NULL)
               {
                  lisp_error(*s,"token '.' not allowed here\n");
               }
               else
               {
                  RvR_lisp_object *tmpo = NULL;
                  RvR_lisp_read_token(s,lisp_n);
                  tmpo = RvR_lisp_compile(s);
                  last->obj.list.cdr = tmpo;
                  last = NULL;
               }
            }
            else if(last==NULL&&first!=NULL)
            {
               lisp_error(*s,"illegal end of dotted list\n");
            }
            else
            {
               RvR_lisp_object *tmpo = NULL;
               cur = RvR_lisp_list_create();

               lisp_ptr_ref_push(&cur);

               if(first==NULL)
                  first = cur;
               tmpo = RvR_lisp_compile(s);
               cur->obj.list.car = tmpo;
               if(last!=NULL)
                  last->obj.list.cdr = cur;
               last = cur;

               lisp_ptr_ref_pop(1); //cur
            }
         }
      }while(!done);
      ret = lisp_comp_optimize(first);

      lisp_ptr_ref_pop(1); //last
      lisp_ptr_ref_pop(1); //cur
      lisp_ptr_ref_pop(1); //first
   }
   else if(lisp_n[0]==')')
   {
      lisp_error(*s,"mismatched )");
   }
   else if(isdigit(lisp_n[0])||(lisp_n[0]=='-'&&isdigit(lisp_n[1])))
   {
      RvR_lisp_object *num = RvR_lisp_number_create(0);
      sscanf(lisp_n,"%d",&num->obj.num.num);
      ret = num;
   }
   else if(lisp_n[0]=='"')
   {
      ret = RvR_lisp_stringl_create(lisp_str_token_len(*s));
      char *start = ret->obj.str.str;
      for(;**s&&(**s!='"'||*((*s)+1)=='"');(*s)++,start++)
      {
         if(**s=='\\')
         {
            (*s)++;
            if(**s=='n') *start = '\n';
            if(**s=='r') *start = '\r';
            if(**s=='t') *start = '\t';
            if(**s=='\\') *start = '\\';
         }
         else
            *start = **s;
         if(**s=='"')
            (*s)++;
      }
      *start = '\0';
      (*s)++;
   }
   else if(lisp_n[0]=='#')
   {
      if(lisp_n[1]=='\\')
      {
         //Read character name
         RvR_lisp_read_token(s,lisp_n);
         if(!strcmp(lisp_n,"newline"))
            ret = RvR_lisp_char_create('\n');
         else if(!strcmp(lisp_n,"space"))
            ret = RvR_lisp_char_create(' ');
         else
            ret = RvR_lisp_char_create(lisp_n[0]);
      }
      else if(lisp_n[1]=='\0') //short hand for function
      {
         RvR_lisp_object *cs = RvR_lisp_list_create();
         RvR_lisp_object *c2 = NULL;
         RvR_lisp_object *tmp = NULL;

         lisp_ptr_ref_push(&cs);
         lisp_ptr_ref_push(&c2);

         tmp = RvR_lisp_symbol_find_or_create("function");
         cs->obj.list.car = tmp;
         c2 = RvR_lisp_list_create();
         tmp = RvR_lisp_compile(s);
         c2->obj.list.car = tmp;
         cs->obj.list.cdr = c2;
         ret = cs;

         lisp_ptr_ref_pop(1); //c2
         lisp_ptr_ref_pop(1); //cs
      }
      else
      {
         RvR_lisp_break("Unknown #\\ notation: %s\n",lisp_n);
         exit(0);
      }
   }
   else
   {
      ret = RvR_lisp_symbol_find_or_create(lisp_n);
   }

   return ret;
}

RvR_lisp_object *RvR_lisp_eval_function(RvR_lisp_object *sym, RvR_lisp_object *arg_list)
{
#if RVR_LISP_TYPE_CHECK
   int args = 0;
   int req_min = 0;
   int req_max = 0;
   if(RvR_lisp_item_type(sym)!=RVR_L_SYMBOL)
   {
      RvR_lisp_object_print(sym);
      RvR_lisp_break("EVAL: is not a function name (not a symbol either)");
      exit(0);
   }
#endif

   RvR_lisp_object *fun = sym->obj.sym.function;

   lisp_ptr_ref_push(&fun);
   lisp_ptr_ref_push(&arg_list);

   uint32_t t = RvR_lisp_item_type(fun);

#if RVR_LISP_TYPE_CHECK
   switch(t)
   {
   case RVR_L_SYS_FUNCTION:
   case RVR_L_C_FUNCTION:
   case RVR_L_C_BOOL:
   case RVR_L_L_FUNCTION:
      req_min = fun->obj.sys.min_args;
      req_max = fun->obj.sys.max_args;
      break;
   case RVR_L_USER_FUNCTION:
      lisp_ptr_ref_pop(1); //arg_list
      lisp_ptr_ref_pop(1); //fun
      return RvR_lisp_eval_user_function(sym,arg_list);
   default:
      RvR_lisp_object_print(sym);
      RvR_lisp_break(" is not a function name");
      exit(0);
   }

   if(req_min!=-1)
   {
      RvR_lisp_object *a = arg_list;
      for(args = 0;a!=NULL;a = a->obj.list.cdr)
         args++;

      if(args<req_min)
      {
         RvR_lisp_object_print(arg_list);
         RvR_lisp_object_print(sym->obj.sym.name);
         RvR_lisp_break("\nToo few parameters to function\n");
         exit(0);
      }
      else if(req_max!=-1&&args>req_max)
      {
         RvR_lisp_object_print(arg_list);
         RvR_lisp_object_print(sym->obj.sym.name);
         RvR_lisp_break("\nToo many parameters to function\n");
         exit(0);
      }
   }
#endif

#if RVR_LISP_PROFILE
   //TODO
#endif

   RvR_lisp_object *ret = NULL;
   switch(t)
   {
   case RVR_L_SYS_FUNCTION:
      ret = RvR_lisp_eval_sys_function(fun,arg_list);
      break;
   case RVR_L_L_FUNCTION:
      ret = lisp_lcall(fun->obj.sys.fun_number,arg_list);
      break;
   case RVR_L_USER_FUNCTION:
      lisp_ptr_ref_pop(1); //arg_list
      lisp_ptr_ref_pop(1); //fun
      return RvR_lisp_eval_user_function(sym,arg_list);
   case RVR_L_C_FUNCTION:
   case RVR_L_C_BOOL:
   {
      RvR_lisp_object *first = NULL;
      RvR_lisp_object *cur = NULL;

      lisp_ptr_ref_push(&first);
      lisp_ptr_ref_push(&cur);
      lisp_ptr_ref_push(&arg_list);

      while(arg_list!=NULL)
      {
         RvR_lisp_object *tmp = RvR_lisp_list_create();
         if(first!=NULL)
            cur->obj.list.cdr = tmp;
         else
            first = tmp;
         cur = tmp;

         RvR_lisp_object *val = RvR_lisp_eval(arg_list->obj.list.car);
         cur->obj.list.car = val;
         arg_list = RvR_lisp_cdr(arg_list);
      }

      if(t==RVR_L_C_FUNCTION)
         ret = RvR_lisp_number_create(lisp_ccall(fun->obj.sys.fun_number,first));
      else if(lisp_ccall(fun->obj.sys.fun_number,first))
         ret = lisp_true_symbol;
      else
         ret = NULL;

      lisp_ptr_ref_pop(1); //arg_list
      lisp_ptr_ref_pop(1); //cur
      lisp_ptr_ref_pop(1); //first
      break;
   }
   default:
      fprintf(stderr,"not a fun, shouldn't happen\n");
   }

#if RVR_LISP_PROFILE
   //TODO
#endif

   lisp_ptr_ref_pop(1); //arg_list
   lisp_ptr_ref_pop(1); //fun

   return ret;
}

RvR_lisp_object *RvR_lisp_eval_user_function(RvR_lisp_object *sym, RvR_lisp_object *arg_list)
{
   RvR_lisp_object *ret = NULL;

   lisp_ptr_ref_push(&ret);

#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(sym)!=RVR_L_SYMBOL)
   {
      RvR_lisp_object_print(sym);
      RvR_lisp_break("EVAL: is not a function name (not symbol either)");
      exit(0);
   }
#endif

#if RVR_LISP_PROFILE
   //TODO
#endif

   RvR_lisp_object *fun = sym->obj.sym.function;

#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(fun)!=RVR_L_USER_FUNCTION)
   {
      RvR_lisp_object_print(fun);
      RvR_lisp_break("is not a user defined function\n");
   }
#endif

   RvR_lisp_object *fun_arg_list = fun->obj.usr.arg_list;
   RvR_lisp_object *block_list = fun->obj.usr.block_list;
   lisp_ptr_ref_push(&fun_arg_list);
   lisp_ptr_ref_push(&block_list);

   //mark the stack start, so we can restore when done
   long stack_start = lisp_user_stack.data_used;

   RvR_lisp_object *f_arg = NULL;
   lisp_ptr_ref_push(&f_arg);
   lisp_ptr_ref_push(&arg_list);

   for(f_arg = fun_arg_list;f_arg!=NULL;f_arg = f_arg->obj.list.cdr)
   {
      RvR_lisp_object *s = f_arg->obj.list.car;
      lisp_user_stack_push(s->obj.sym.value);
   }

   //open block so that local vars aren't saved on the stack
   {
      int new_start = lisp_user_stack.data_used;
      int i = new_start;

      for(f_arg = fun_arg_list;f_arg!=NULL;f_arg = f_arg->obj.list.cdr)
      {
         if(arg_list==NULL)
         {
            RvR_lisp_object_print(sym);
            RvR_lisp_break("too few parameters to function\n");
            exit(0);
         }

         lisp_user_stack_push(RvR_lisp_eval(arg_list->obj.list.car));
         arg_list = arg_list->obj.list.cdr;
      }

      //now store all the values and put them into the symbols
      for(f_arg = fun_arg_list;f_arg!=NULL;f_arg = f_arg->obj.list.cdr)
         f_arg->obj.list.car->obj.sym.value = lisp_user_stack.data[i++];

      lisp_user_stack.data_used = new_start;
   }

   if(f_arg!=NULL)
   {
      RvR_lisp_object_print(sym);
      RvR_lisp_break("too many parameters to function\n");
      exit(0);
   }

   //now evaluate the function block
   while(block_list!=NULL)
   {
      ret = RvR_lisp_eval(block_list->obj.list.car);
      block_list = block_list->obj.list.cdr;
   }

   long cur_stack = stack_start;
   for(f_arg = fun_arg_list;f_arg!=NULL;f_arg = f_arg->obj.list.cdr)
      f_arg->obj.list.car->obj.sym.value = lisp_user_stack.data[cur_stack++];

   lisp_user_stack.data_used = stack_start;

#if RVR_LISP_PROFILE
   //TODO
#endif

   lisp_ptr_ref_pop(1); //arg_list
   lisp_ptr_ref_pop(1); //f_arg
   lisp_ptr_ref_pop(1); //block_list
   lisp_ptr_ref_pop(1); //fun_arg_list
   lisp_ptr_ref_pop(1); //ret

   return ret;
}

RvR_lisp_object *RvR_lisp_eval_sys_function(RvR_lisp_object *sym, RvR_lisp_object *arg_list)
{
   RvR_lisp_object *ret = NULL;

   lisp_ptr_ref_push(&arg_list);

   switch(sym->obj.sys.fun_number)
   {
   case SYS_FUNC_PRINT:
      while(arg_list!=NULL)
      {
         ret = RvR_lisp_eval(arg_list->obj.list.car);
         arg_list = arg_list->obj.list.cdr;
         RvR_lisp_object_print(ret);
      }
      break;
   case SYS_FUNC_CAR:
      ret = RvR_lisp_car(RvR_lisp_eval(arg_list->obj.list.car));
      break;
   case SYS_FUNC_CDR:
      ret = RvR_lisp_cdr(RvR_lisp_eval(arg_list->obj.list.car));
      break;
   case SYS_FUNC_LENGTH:
   {
      RvR_lisp_object *v = RvR_lisp_eval(arg_list->obj.list.car);
      switch(RvR_lisp_item_type(v))
      {
      case RVR_L_STRING:
         ret = RvR_lisp_number_create(strlen(v->obj.str.str));
         break;
      case RVR_L_CONS_CELL:
         ret = RvR_lisp_number_create(RvR_lisp_list_length(v));
         break;
      default:
         RvR_lisp_object_print(v);
         RvR_lisp_break("length: type not supported\n");
         break;
      }
      break;
   }
   case SYS_FUNC_LIST:
   {
      RvR_lisp_object *cur = NULL;
      RvR_lisp_object *first = NULL;
      RvR_lisp_object *last = NULL;

      lisp_ptr_ref_push(&cur);
      lisp_ptr_ref_push(&first);
      lisp_ptr_ref_push(&last);

      while(arg_list!=NULL)
      {
         cur = RvR_lisp_list_create();
         RvR_lisp_object *val = RvR_lisp_eval(arg_list->obj.list.car);
         cur->obj.list.car = val;
         if(last!=NULL)
            last->obj.list.cdr = cur;
         else
            first = cur;
         last = cur;
         arg_list = arg_list->obj.list.cdr;
      }
      ret = first;

      lisp_ptr_ref_pop(1); //last
      lisp_ptr_ref_pop(1); //first
      lisp_ptr_ref_pop(1); //cur
      break;
   }
   case SYS_FUNC_CONS:
   {
      RvR_lisp_object *c = RvR_lisp_list_create();

      lisp_ptr_ref_push(&c);

      RvR_lisp_object *val = RvR_lisp_eval(arg_list->obj.list.car);
      c->obj.list.car = val;
      val = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car);
      c->obj.list.cdr = val;
      ret = c;

      lisp_ptr_ref_pop(1); //c

      break;
   }
   case SYS_FUNC_QUOTE:
      ret = arg_list->obj.list.car;
      break;
   case SYS_FUNC_EQ:
      lisp_user_stack_push(RvR_lisp_eval(arg_list->obj.list.car));
      lisp_user_stack_push(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      ret = RvR_lisp_eq(lisp_user_stack_pop(1),lisp_user_stack_pop(1));
      break;
   case SYS_FUNC_EQUAL:
      lisp_user_stack_push(RvR_lisp_eval(arg_list->obj.list.car));
      lisp_user_stack_push(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      ret = RvR_lisp_equal(lisp_user_stack_pop(1),lisp_user_stack_pop(1));
      break;
   case SYS_FUNC_PLUS:
   {
      int32_t sum = 0;
      while(arg_list!=NULL)
      {
         sum+=RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
         arg_list = arg_list->obj.list.cdr;
      }
      ret = RvR_lisp_number_create(sum);
      break;
   }
   case SYS_FUNC_TIMES:
   {
      int32_t prod = 1;
      RvR_lisp_object *first = RvR_lisp_eval(arg_list->obj.list.car);

      lisp_ptr_ref_push(&first);

      //TODO: fixed point mul (maybe use seperate function/operator?)
      do
      {
         prod*=RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
         arg_list = arg_list->obj.list.cdr;
         if(arg_list!=NULL)
            first = RvR_lisp_eval(arg_list->obj.list.car);
      }while(arg_list!=NULL);
      ret = RvR_lisp_number_create(prod);

      lisp_ptr_ref_pop(1); //first

      break;
   }
   case SYS_FUNC_SLASH:
   {
      int32_t quot = 0;
      int first = 1;

      while(arg_list!=NULL)
      {
         RvR_lisp_object *i = RvR_lisp_eval(arg_list->obj.list.car);
         if(RvR_lisp_item_type(i)!=RVR_L_NUMBER)
         {
            RvR_lisp_object_print(i);
            RvR_lisp_break("/ only defined for numbers, cannot divide\n");
            exit(0);
         }

         if(first)
         {
            quot = i->obj.num.num;
            first = 0;
         }
         else
         {
            quot/=i->obj.num.num;
         }

         arg_list = arg_list->obj.list.cdr;
      }
      ret = RvR_lisp_number_create(quot);
      
      break;
   }
   case SYS_FUNC_MINUS:
   {
      int32_t sub = RvR_lisp_eval(arg_list->obj.list.car)->obj.num.num;
      arg_list = arg_list->obj.list.cdr;

      while(arg_list!=NULL)
      {
         sub-=RvR_lisp_eval(arg_list->obj.list.car)->obj.num.num;
         arg_list = arg_list->obj.list.cdr;
      }
      ret = RvR_lisp_number_create(sub);

      break;
   }
   case SYS_FUNC_IF:
      if(RvR_lisp_eval(arg_list->obj.list.car)!=NULL)
      {
         ret = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car);
      }
      else
      {
         arg_list = arg_list->obj.list.cdr->obj.list.cdr;
         if(arg_list!=NULL)
            ret = RvR_lisp_eval(arg_list->obj.list.car);
         else
            ret = NULL;
      }
      break;
   case SYS_FUNC_SETQ:
   case SYS_FUNC_SETF:
   {
      RvR_lisp_object *set_to = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car);
      RvR_lisp_object *i = NULL;

      lisp_ptr_ref_push(&set_to);
      lisp_ptr_ref_push(&i);

      i = arg_list->obj.list.car;
      uint32_t x = RvR_lisp_item_type(set_to);
      switch(RvR_lisp_item_type(i))
      {
      case RVR_L_SYMBOL:
         switch(RvR_lisp_item_type(i->obj.sym.value))
         {
         case RVR_L_NUMBER:
            if(x==RVR_L_NUMBER&&i->obj.sym.value!=lisp_undefined)
               RvR_lisp_symbol_number_set(i,set_to->obj.num.num);
            else
               RvR_lisp_symbol_value_set(i,set_to);
            break;
         case RVR_L_OBJECT_VAR:
            lisp_lset(i->obj.sym.value->obj.var.index,set_to);
            break;
         default:
            RvR_lisp_symbol_value_set(i,set_to);
         }
         ret = i->obj.sym.value;
         break;
      case RVR_L_CONS_CELL:
      {
#if RVR_LISP_TYPE_CHECK
         RvR_lisp_object *car = i->obj.list.car;
         if(car==lisp_car_symbol)
         {
            car = RvR_lisp_eval(i->obj.list.cdr->obj.list.car);
            if(car==NULL||RvR_lisp_item_type(car)!=RVR_L_CONS_CELL)
            {
               RvR_lisp_object_print(car);
               RvR_lisp_break("setq car: evaled object is not a cons cell\n");
               exit(0);
            }
            car->obj.list.car = set_to;
         }
         else if(car==lisp_cdr_symbol)
         {
            car = RvR_lisp_eval(i->obj.list.cdr->obj.list.car);
            if(car==NULL||RvR_lisp_item_type(car)!=RVR_L_CONS_CELL)
            {
               RvR_lisp_object_print(car);
               RvR_lisp_break("setq cdr: evaled object is not a cons cell\n");
               exit(0);
            }
            car->obj.list.cdr = set_to;
         }
         else if(car!=lisp_aref_symbol)
         {
            RvR_lisp_break("expected (aref, car, cdr or symbol) in setq\n");
            exit(0);
         }
         else
         {
#endif
            RvR_lisp_object *a = RvR_lisp_eval(i->obj.list.cdr->obj.list.car);

            lisp_ptr_ref_push(&a);

#if RVR_LISP_TYPE_CHECK
            if(RvR_lisp_item_type(a)!=RVR_L_1D_ARRAY)
            {
               RvR_lisp_object_print(a);
               RvR_lisp_break("is not an array (aref)\n");
               exit(0);
            }
#endif
            int num = RvR_lisp_number_value(RvR_lisp_eval(i->obj.list.cdr->obj.list.cdr->obj.list.car));

#if RVR_LISP_TYPE_CHECK
            if(num>=a->obj.arr.len||num<0)
            {
               RvR_lisp_break("aref: value out of bounds (%d)\n",num);
               exit(0);
            }
#endif
            a->obj.arr.data[num] = set_to;

            lisp_ptr_ref_pop(1); //a

#if RVR_LISP_TYPE_CHECK
         }
#endif
         ret = set_to;
         break;
      }
      default:
         RvR_lisp_object_print(i);
         RvR_lisp_break("setq/setf only defined for symbols and arrays now...\n");
         exit(0);
         break;
      }

      lisp_ptr_ref_pop(1); //i
      lisp_ptr_ref_pop(1); //set_to

      break;
   }
   case SYS_FUNC_SYMBOL_LIST:
      ret = NULL;
      break;
   case SYS_FUNC_ASSOC:
   {
      RvR_lisp_object *item = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&item);

      RvR_lisp_object *list = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car);
      lisp_ptr_ref_push(&list);

      ret = RvR_lisp_assoc(item,list);

      lisp_ptr_ref_pop(1); //list
      lisp_ptr_ref_pop(1); //item

      break;
   }
   case SYS_FUNC_NOT:
   case SYS_FUNC_NULL:
      if(RvR_lisp_eval(arg_list->obj.list.car)==NULL)
         ret = lisp_true_symbol;
      else
         ret = NULL;
      break;
   case SYS_FUNC_ACONS:
   {
      RvR_lisp_object *i1 = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&i1);

      RvR_lisp_object *i2 = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car);
      lisp_ptr_ref_push(&i2);

      RvR_lisp_object *cs = RvR_lisp_list_create();
      cs->obj.list.car = i1;
      cs->obj.list.cdr = i2;
      ret = cs;

      lisp_ptr_ref_pop(1); //i2
      lisp_ptr_ref_pop(1); //i1

      break;
   }
   case SYS_FUNC_PAIRLIS:
   {
      lisp_user_stack_push(RvR_lisp_eval(arg_list->obj.list.car));
      arg_list = arg_list->obj.list.cdr;
      lisp_user_stack_push(RvR_lisp_eval(arg_list->obj.list.car));
      arg_list = arg_list->obj.list.cdr;

      RvR_lisp_object *n3 = RvR_lisp_eval(arg_list->obj.list.car);
      RvR_lisp_object *n2 = lisp_user_stack_pop(1);
      RvR_lisp_object *n1 = lisp_user_stack_pop(1);

      ret = lisp_pairlis(n1,n2,n3);

      break;
   }
   case SYS_FUNC_LET:
   {
      //make an a-list of new variable names and new values
      RvR_lisp_object *var_list = arg_list->obj.list.car;
      RvR_lisp_object *block_list = arg_list->obj.list.cdr;

      lisp_ptr_ref_push(&var_list);
      lisp_ptr_ref_push(&block_list);

      long stack_start = lisp_user_stack.data_used;

      while(var_list!=NULL)
      {
         RvR_lisp_object *var_name = var_list->obj.list.car->obj.list.car;
         RvR_lisp_object *tmp = NULL;

#if RVR_LISP_TYPE_CHECK
         if(RvR_lisp_item_type(var_name)!=RVR_L_SYMBOL)
         {
            RvR_lisp_object_print(var_name);
            RvR_lisp_break("should be a symbol (let)\n");
            exit(0);
         }
#endif

         lisp_user_stack_push(var_name->obj.sym.value);
         tmp = RvR_lisp_eval(var_list->obj.list.car->obj.list.cdr->obj.list.car);
         RvR_lisp_symbol_value_set(var_name,tmp);
         var_list = var_list->obj.list.cdr;
      }

      //now evaluate each of the blocks with the new environment and
      //return value from the last block
      while(block_list!=NULL)
      {
         ret = RvR_lisp_eval(block_list->obj.list.car);
         block_list = block_list->obj.list.cdr;
      }

      long cur_stack = stack_start;
      var_list = arg_list->obj.list.car;

      while(var_list!=NULL)
      {
         RvR_lisp_object *var_name = var_list->obj.list.car->obj.list.car;
         RvR_lisp_symbol_value_set(var_name,lisp_user_stack.data[cur_stack++]);
         var_list = var_list->obj.list.cdr;
      }

      lisp_user_stack.data_used = stack_start;

      lisp_ptr_ref_pop(1); //block_list
      lisp_ptr_ref_pop(1); //var_list

      break;
   }
   case SYS_FUNC_DEFUN:
   {
      RvR_lisp_object *symbol = arg_list->obj.list.car;
      lisp_ptr_ref_push(&symbol);

#if RVR_LISP_TYPE_CHECK
      if(RvR_lisp_item_type(symbol)!=RVR_L_SYMBOL)
      {
         RvR_lisp_object_print(symbol);
         RvR_lisp_break("is no a symbol! (defun)\n");
         exit(0);
      }

      if(RvR_lisp_item_type(arg_list)!=RVR_L_CONS_CELL)
      {
         RvR_lisp_object_print(arg_list);
         RvR_lisp_break("is no a lambda list! (defun)\n");
         exit(0);
      }
#endif

      RvR_lisp_object *block_list = arg_list->obj.list.cdr->obj.list.cdr;

      RvR_lisp_object *ufun = RvR_lisp_user_function_create(RvR_lisp_car(RvR_lisp_cdr(arg_list)),block_list);
      symbol->obj.sym.function = ufun;
      ret = symbol;

      lisp_ptr_ref_pop(1); //symbol

      break;
   }
   case SYS_FUNC_ATOM:
      ret = RvR_lisp_atom(RvR_lisp_eval(arg_list->obj.list.car));
      break;
   case SYS_FUNC_AND:
   {
      RvR_lisp_object *l = arg_list;
      lisp_ptr_ref_push(&l);

      ret = lisp_true_symbol;
      while(l!=NULL)
      {
         if(RvR_lisp_eval(l->obj.list.car)==NULL)
         {
            ret = NULL;
            l = NULL; //short-circuit
         }
         else
         {
            l = l->obj.list.cdr;
         }
      }

      lisp_ptr_ref_pop(1); //l

      break;
   }
   case SYS_FUNC_OR:
   {
      RvR_lisp_object *l = arg_list;
      lisp_ptr_ref_push(&l);

      ret = NULL;
      while(l!=NULL)
      {
         if(RvR_lisp_eval(l->obj.list.car)!=NULL)
         {
            ret = lisp_true_symbol;
            l = NULL; //short-circuit
         }
         else
         {
            l = l->obj.list.cdr;
         }
      }

      lisp_ptr_ref_pop(1); //l

      break;
   }
   case SYS_FUNC_PROGN:
      ret = RvR_lisp_eval_block(arg_list);
      break;
   case SYS_FUNC_CONCATENATE:
      ret = lisp_concatenate(arg_list);
      break;
   case SYS_FUNC_CHAR_CODE:
   {
      RvR_lisp_object *i = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&i);

      ret = NULL;
      switch(RvR_lisp_item_type(i))
      {
      case RVR_L_CHARACTER:
         ret = RvR_lisp_number_create(i->obj.ch.ch);
         break;
      case RVR_L_STRING:
         ret = RvR_lisp_number_create(*RvR_lisp_string(i));
         break;
      default:
         RvR_lisp_object_print(i);
         RvR_lisp_break("is not character type\n");
         exit(0);
         break;
      }

      lisp_ptr_ref_pop(1); //i

      break;
   }
   case SYS_FUNC_CODE_CHAR:
   {
      RvR_lisp_object *i = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&i);

      if(RvR_lisp_item_type(i)!=RVR_L_NUMBER)
      {
         RvR_lisp_object_print(i);
         RvR_lisp_break("is not a number type\n");
         exit(0);
      }
      ret = RvR_lisp_char_create(i->obj.num.num);

      lisp_ptr_ref_pop(1); //i

      break;
   }
   case SYS_FUNC_COND:
   {
      RvR_lisp_object *block_list = arg_list->obj.list.car;
      lisp_ptr_ref_push(&block_list);

      ret = NULL;
      lisp_ptr_ref_push(&ret); //Required to protect from the last Eval call

      while(block_list!=NULL)
      {
         if(RvR_lisp_eval(RvR_lisp_car(block_list->obj.list.car))!=NULL)
            ret = RvR_lisp_eval(block_list->obj.list.car->obj.list.cdr->obj.list.car);
         block_list = block_list->obj.list.cdr;
      }

      lisp_ptr_ref_pop(1); //ret
      lisp_ptr_ref_pop(1); //block_list

      break;
   }
   case SYS_FUNC_SELECT:
   {
      RvR_lisp_object *selector = RvR_lisp_eval(arg_list->obj.list.car);
      RvR_lisp_object *sel = arg_list->obj.list.cdr;
      lisp_ptr_ref_push(&selector);
      lisp_ptr_ref_push(&sel);

      ret = NULL;
      lisp_ptr_ref_push(&ret); //Required to protect from the last Eval call

      while(sel!=NULL)
      {
         if(RvR_lisp_equal(selector,RvR_lisp_eval(sel->obj.list.car->obj.list.car)))
         {
            sel = sel->obj.list.car->obj.list.cdr;
            while(sel!=NULL)
            {
               ret = RvR_lisp_eval(sel->obj.list.car);
               sel = sel->obj.list.cdr;
            }
         }
         else
         {
            sel = sel->obj.list.cdr;
         }
      }

      lisp_ptr_ref_pop(1); //ret
      lisp_ptr_ref_pop(1); //sel
      lisp_ptr_ref_pop(1); //selector

      break;
   }
   case SYS_FUNC_FUNCTION:
      ret = RvR_lisp_eval(arg_list->obj.list.car)->obj.sym.function;
      break;
   case SYS_FUNC_MAPCAR:
      ret = lisp_mapcar(arg_list);
      break;
   case SYS_FUNC_FUNCALL:
   {
      RvR_lisp_object *n1 = RvR_lisp_eval(arg_list->obj.list.car);
      ret = RvR_lisp_eval_function(n1,arg_list->obj.list.cdr);

      break;
   }
   case SYS_FUNC_GT:
   {
      int32_t n1 = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      int32_t n2 = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      ret = n1>n2?lisp_true_symbol:NULL;

      break;
   }
   case SYS_FUNC_LT:
   {
      int32_t n1 = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      int32_t n2 = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      ret = n1<n2?lisp_true_symbol:NULL;

      break;
   }
   case SYS_FUNC_GE:
   {
      int32_t n1 = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      int32_t n2 = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      ret = n1>=n2?lisp_true_symbol:NULL;

      break;
   }
   case SYS_FUNC_LE:
   {
      int32_t n1 = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      int32_t n2 = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      ret = n1<=n2?lisp_true_symbol:NULL;

      break;
   }
   case SYS_FUNC_TMP_SPACE:
      RvR_lisp_tmp_space();
      ret = lisp_true_symbol;
      break;
   case SYS_FUNC_PERM_SPACE:
      RvR_lisp_perm_space();
      ret = lisp_true_symbol;
      break;
   case SYS_FUNC_SYMBOL_NAME:
   {
      RvR_lisp_object *symb = RvR_lisp_eval(arg_list->obj.list.car);

#if RVR_LISP_TYPE_CHECK
      if(RvR_lisp_item_type(symb)!=RVR_L_SYMBOL)
      {
         RvR_lisp_object_print(symb);
         RvR_lisp_break("is not a symbol (symbol-name)\n");
         exit(0);
      }
#endif

      ret = symb->obj.sym.name;

      break;
   }
   case SYS_FUNC_TRACE:
      lisp_trace_level++;
      if(arg_list!=NULL)
         lisp_trace_print_level = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      ret = lisp_true_symbol;
      break;
   case SYS_FUNC_UNTRACE:
      if(lisp_trace_level>0)
      {
         lisp_trace_level--;
         ret = lisp_true_symbol;
      }
      else
      {
         ret = NULL;
      }
      break;
   case SYS_FUNC_DIGSTR:
   {
      char tmp[50];
      char *tp = NULL;
      int32_t num = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      int32_t dig = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));

      tp = tmp+49;
      *(tp--) = '\0';
      while(num)
      {
         *(tp--) = '0'+(num%10);
         num/=10;
         dig--;
      }
      while(dig--)
         *(tp--) = '0';
      ret = RvR_lisp_strings_create(tp+1);

      break;
   }
   case SYS_FUNC_LOCAL_LOAD:
   case SYS_FUNC_LOAD:
   case SYS_FUNC_COMPILE_FILE:
   {
      //TODO: local-load? what does it do? It doesn't seem to be used by abuse?
      RvR_lisp_object *fn = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&fn);

      //Read file
      char *st = RvR_lisp_string(fn);
      RvR_rw fp;
      RvR_rw_init_path(&fp,st,"rb");
      RvR_rw_seek(&fp,0,SEEK_END);
      size_t l = RvR_rw_tell(&fp);
      RvR_rw_seek(&fp,0,SEEK_SET);
      char *s = RvR_malloc(l+1);
      RvR_rw_read(&fp,s,l,1);
      RvR_rw_close(&fp);

      const char *cs = s;
      RvR_lisp_object *compiled_form = NULL;
      lisp_ptr_ref_push(&compiled_form);

      while(!RvR_lisp_end_of_program(cs))
      {
         void *m = RvR_lisp_mark_heap(RVR_LISP_TMP_SPACE);
         compiled_form = RvR_lisp_compile(&cs);
         RvR_lisp_eval(compiled_form);
         compiled_form = NULL;
         RvR_lisp_restore_heap(m,RVR_LISP_TMP_SPACE);
      }

      RvR_free(s);
      ret = fn;

      lisp_ptr_ref_pop(1); //compiled_form
      lisp_ptr_ref_pop(1); //fn

      break;
   }
   case SYS_FUNC_ABS:
      ret = RvR_lisp_number_create(RvR_abs(RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car))));
      break;
   case SYS_FUNC_MIN:
   {
      int32_t a = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      int32_t b = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      ret = RvR_lisp_number_create(RvR_min(a,b));

      break;
   }
   case SYS_FUNC_MAX:
   {
      int32_t a = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      int32_t b = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      ret = RvR_lisp_number_create(RvR_max(a,b));

      break;
   }
   case SYS_FUNC_BACKQUOTE:
      ret = lisp_backquote_eval(arg_list->obj.list.car);
      break;
   case SYS_FUNC_COMMA:
      RvR_lisp_object_print(arg_list);
      RvR_lisp_break("comma is illegal outside of backquote\n");
      exit(0);
      break;
   case SYS_FUNC_NTH:
   {
      int32_t x = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      ret = RvR_lisp_nth(x,RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));

      break;
   }
   case SYS_FUNC_RESIZE_TMP:
      //TODO: remove
      break;
   case SYS_FUNC_RESIZE_PERM:
      //TODO: remove
      break;
   case SYS_FUNC_COS:
      //TODO
      break;
   case SYS_FUNC_SIN:
      //TODO
      break;
   case SYS_FUNC_ATAN2:
      //TODO
      break;
   case SYS_FUNC_ENUM:
   {
      int sp = lisp_current_space;
      lisp_current_space = RVR_LISP_PERM_SPACE;
      int32_t x = 0;

      while(arg_list!=NULL)
      {
         RvR_lisp_object *symbol = RvR_lisp_eval(arg_list->obj.list.car);
         lisp_ptr_ref_push(&symbol);

         switch(RvR_lisp_item_type(symbol))
         {
         case RVR_L_SYMBOL:
         {
            RvR_lisp_object *tmp = RvR_lisp_number_create(x);
            symbol->obj.sym.value = tmp;

            break;
         }
         case RVR_L_CONS_CELL:
         {
            RvR_lisp_object *s = RvR_lisp_eval(symbol->obj.list.car);
            lisp_ptr_ref_push(&s);

#if RVR_LISP_TYPE_CHECK
            if(RvR_lisp_item_type(s)!=RVR_L_SYMBOL)
            {
               RvR_lisp_object_print(arg_list);
               RvR_lisp_break("expecting (symbol value) for enum\n");
               exit(0);
            }
#endif

            x = RvR_lisp_number_value(RvR_lisp_eval(symbol->obj.list.cdr->obj.list.car));
            RvR_lisp_object *tmp = RvR_lisp_number_create(x);
            symbol->obj.sym.value = tmp;

            lisp_ptr_ref_pop(1); //s

            break;
         }
         default:
            RvR_lisp_object_print(arg_list);
            RvR_lisp_break("expecting symbol or (symbol value) in enum\n");
            exit(0);
            break;
         }

         arg_list = arg_list->obj.list.cdr;
         x++;

         lisp_ptr_ref_pop(1); //symbol
      }

      lisp_current_space = sp;

      break;
   }
   case SYS_FUNC_QUIT:
      exit(0);
      break;
   case SYS_FUNC_EVAL:
      ret = RvR_lisp_eval(RvR_lisp_eval(arg_list->obj.list.car));
      break;
   case SYS_FUNC_BREAK:
      RvR_lisp_break("User break\n");
      break;
   case SYS_FUNC_MOD:
   {
      int32_t x = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      int32_t y = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      if(y==0)
      {
         RvR_lisp_break("mod: div by zero\n");
         y = 1;
      }

      ret = RvR_lisp_number_create(x%y);

      break;
   }
   case SYS_FUNC_FOR:
   {
      RvR_lisp_object *bind_var = arg_list->obj.list.car;
      lisp_ptr_ref_push(&bind_var);

      if(RvR_lisp_item_type(bind_var)!=RVR_L_SYMBOL)
      {
         RvR_lisp_break("expecting for iterator to be a symbol\n");
         exit(1);
      }
      arg_list = arg_list->obj.list.cdr;

      if(arg_list->obj.list.car!=lisp_in_symbol)
      {
         RvR_lisp_break("expecting in after 'for iterator'\n");
         exit(1);
      }
      arg_list = arg_list->obj.list.cdr;

      RvR_lisp_object *ilist = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&ilist);

      arg_list = arg_list->obj.list.cdr;

      if(arg_list->obj.list.car!=lisp_do_symbol)
      {
         RvR_lisp_break("expecting do after for iterator in list\n");
         exit(1);
      }
      arg_list = arg_list->obj.list.cdr;

      RvR_lisp_object *block = NULL;
      lisp_ptr_ref_push(&block);
      lisp_ptr_ref_push(&ret); //Required to protect from the last SetValue call

      lisp_user_stack_push(bind_var->obj.sym.value); //save old symbol value
      while(ilist!=NULL)
      {
         RvR_lisp_symbol_value_set(bind_var,ilist->obj.list.car);
         for(block = arg_list;block!=NULL;block = block->obj.list.cdr)
            ret = RvR_lisp_eval(block->obj.list.car);
         ilist = ilist->obj.list.cdr;
      }
      RvR_lisp_symbol_value_set(bind_var,lisp_user_stack_pop(1)); //restore value

      lisp_ptr_ref_pop(1); //ret
      lisp_ptr_ref_pop(1); //block
      lisp_ptr_ref_pop(1); //ilist
      lisp_ptr_ref_pop(1); //bind_var

      break;
   }
   case SYS_FUNC_OPEN_FILE:
   {
      RvR_lisp_object *str1 = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&str1);

      RvR_lisp_object *str2 = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car);
      RvR_rw *old_file = lisp_current_print_file;
      lisp_current_print_file = RvR_malloc(sizeof(*lisp_current_print_file));
      RvR_rw_init_path(lisp_current_print_file,RvR_lisp_string(str1),RvR_lisp_string(str2));

      while(arg_list!=NULL)
      {
         ret = RvR_lisp_eval(arg_list->obj.list.car);
         arg_list = arg_list->obj.list.cdr;
      }

      RvR_rw_close(lisp_current_print_file);
      RvR_free(lisp_current_print_file);
      lisp_current_print_file = old_file;

      lisp_ptr_ref_pop(1); //str1

      break;
   }
   case SYS_FUNC_BIT_AND:
   {
      int32_t first = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      arg_list = arg_list->obj.list.cdr;

      while(arg_list!=NULL)
      {
         first&=RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
         arg_list = arg_list->obj.list.cdr;
      }
      ret = RvR_lisp_number_create(first);

      break;
   }
   case SYS_FUNC_BIT_OR:
   {
      int32_t first = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      arg_list = arg_list->obj.list.cdr;

      while(arg_list!=NULL)
      {
         first|=RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
         arg_list = arg_list->obj.list.cdr;
      }
      ret = RvR_lisp_number_create(first);

      break;
   }
   case SYS_FUNC_BIT_XOR:
   {
      int32_t first = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      arg_list = arg_list->obj.list.cdr;

      while(arg_list!=NULL)
      {
         first^=RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
         arg_list = arg_list->obj.list.cdr;
      }
      ret = RvR_lisp_number_create(first);

      break;
   }
   case SYS_FUNC_MAKE_ARRAY:
   {
      int32_t l = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      if(l>=2<<16||l<=0)
      {
         RvR_lisp_break("bad array size %d\n",l);
         exit(0);
      }
      ret = RvR_lisp_array_create(l,arg_list->obj.list.cdr);

      break;
   }
   case SYS_FUNC_AREF:
   {
      int32_t x = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      ret = RvR_lisp_array_get(RvR_lisp_eval(arg_list->obj.list.car),x);

      break;
   }
   case SYS_FUNC_IF_1PROGN:
      if(RvR_lisp_eval(arg_list->obj.list.car)!=NULL)
         ret = RvR_lisp_eval_block(arg_list->obj.list.cdr->obj.list.car);
      else
         ret = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car);
      break;
   case SYS_FUNC_IF_2PROGN:
      if(RvR_lisp_eval(arg_list->obj.list.car)!=NULL)
         ret = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car);
      else
         ret = RvR_lisp_eval_block(arg_list->obj.list.cdr->obj.list.cdr->obj.list.car);
      break;
   case SYS_FUNC_IF_12PROGN:
      if(RvR_lisp_eval(arg_list->obj.list.car)!=NULL)
         ret = RvR_lisp_eval_block(arg_list->obj.list.cdr->obj.list.car);
      else
         ret = RvR_lisp_eval_block(arg_list->obj.list.cdr->obj.list.cdr->obj.list.car);
      break;
   case SYS_FUNC_EQ0:
   {
      RvR_lisp_object *v = RvR_lisp_eval(arg_list->obj.list.car);
      if(RvR_lisp_item_type(v)!=RVR_L_NUMBER||v->obj.num.num!=0)
         ret = NULL;
      else
         ret = lisp_true_symbol;

      break;
   }
   case SYS_FUNC_PREPORT:
   {
#if RVR_LISP_PROFILE
      char *s = RvR_lisp_string(RvR_lisp_eval(arg_list->obj.list.car));
      lisp_preport(s);
#endif
      break;
   }
   case SYS_FUNC_SEARCH:
   {
      RvR_lisp_object *arg1 = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&arg1);

      arg_list = arg_list->obj.list.cdr;
      char *haystack = RvR_lisp_string(RvR_lisp_eval(arg_list->obj.list.car));
      char *needle = RvR_lisp_string(arg1);
      char *find = strstr(haystack,needle);
      ret = find!=NULL?RvR_lisp_number_create(find-haystack):NULL;

      lisp_ptr_ref_pop(1); //arg1

      break;
   }
   case SYS_FUNC_ELT:
   {
      RvR_lisp_object *arg1 = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&arg1);

      arg_list = arg_list->obj.list.cdr;
      int32_t x = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      char *st = RvR_lisp_string(arg1);

      if(x<0||x>=strlen(st))
      {
         RvR_lisp_break("elt: out of range of string\n");
         exit(0);
      }

      ret = RvR_lisp_char_create(st[x]);

      lisp_ptr_ref_pop(1); //arg1

      break;
   }
   case SYS_FUNC_LISTP:
   {
      RvR_lisp_object *tmp = RvR_lisp_eval(arg_list->obj.list.car);
      ret = (RvR_lisp_item_type(tmp)==RVR_L_CONS_CELL)?lisp_true_symbol:NULL;

      break;
   }
   case SYS_FUNC_NUMBERP:
   {
      RvR_lisp_object *tmp = RvR_lisp_eval(arg_list->obj.list.car);
      ret = (RvR_lisp_item_type(tmp)==RVR_L_CONS_CELL||RvR_lisp_item_type(tmp)==RVR_L_FIXED_POINT)?lisp_true_symbol:NULL;
      
      break;
   }
   case SYS_FUNC_DO:
   {
      RvR_lisp_object *init_var = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&init_var);

      int ustack_start = lisp_user_stack.data_used;
      RvR_lisp_object *symbol = NULL;
      lisp_ptr_ref_push(&symbol);

      for(init_var = arg_list->obj.list.car;init_var!=NULL;init_var = init_var->obj.list.cdr)
      {
         symbol = init_var->obj.list.car->obj.list.car;
         if(RvR_lisp_item_type(symbol)!=RVR_L_SYMBOL)
         {
            RvR_lisp_break("expecting symbol name for iteration var\n");
            exit(0);
         }
         lisp_user_stack_push(symbol->obj.sym.value);
      }

      RvR_lisp_object **do_evaled = lisp_user_stack.data+lisp_user_stack.data_used;

      //push all of the init forms, so we can set the symbol
      for(init_var = arg_list->obj.list.car;init_var!=NULL;init_var = init_var->obj.list.cdr)
         lisp_user_stack_push(RvR_lisp_eval(init_var->obj.list.car->obj.list.cdr->obj.list.car));

      //now set all the symbols
      for(init_var = arg_list->obj.list.car;init_var!=NULL;init_var = init_var->obj.list.cdr)
      {
         symbol = init_var->obj.list.car->obj.list.car;
         RvR_lisp_symbol_value_set(symbol,*do_evaled);
         do_evaled++;
      }

      for(int i = 0;!i;) //set i to 1 when terminate conditions are met
      {
         i = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car->obj.list.car)!=NULL;
         if(!i)
         {
            RvR_lisp_eval_block(arg_list->obj.list.cdr->obj.list.cdr);
            for(init_var = arg_list->obj.list.car;init_var!=NULL;init_var = init_var->obj.list.cdr)
               RvR_lisp_eval(init_var->obj.list.car->obj.list.cdr->obj.list.cdr->obj.list.car);
         }
      }

      ret = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car->obj.list.cdr->obj.list.car);

      //restore old values for symbols
      do_evaled = lisp_user_stack.data+ustack_start;
      for(init_var = arg_list->obj.list.car;init_var!=NULL;init_var = init_var->obj.list.cdr)
      {
         symbol = init_var->obj.list.car->obj.list.car;
         RvR_lisp_symbol_value_set(symbol,*do_evaled);
         do_evaled++;
      }

      lisp_user_stack.data_used = ustack_start;

      lisp_ptr_ref_pop(1); //symbol
      lisp_ptr_ref_pop(1); //init_var

      break;
   }
   case SYS_FUNC_GC:
      lisp_collect_space(lisp_current_space,0);
      break;
   case SYS_FUNC_SCHAR:
   {
      char *s = RvR_lisp_string(RvR_lisp_eval(arg_list->obj.list.car));
      arg_list = arg_list->obj.list.cdr;
      int32_t x = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));

      if(x<0||x>=strlen(s))
      {
         RvR_lisp_break("SCHAR: index %d out of bounds\n",x);
         exit(0);
      }

      ret = RvR_lisp_char_create(s[x]);

      break;
   }
   case SYS_FUNC_SYMBOLP:
   {
      RvR_lisp_object *tmp = RvR_lisp_eval(arg_list->obj.list.car);
      ret = (RvR_lisp_item_type(tmp)==RVR_L_SYMBOL)?lisp_true_symbol:NULL;

      break;
   }
   case SYS_FUNC_NUM2STR:
   {
      char str[20];
      snprintf(str,20,"%d",RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car)));
      ret = RvR_lisp_strings_create(str);

      break;
   }
   case SYS_FUNC_NCONC:
   {
      RvR_lisp_object *l1 = RvR_lisp_eval(arg_list->obj.list.car);
      lisp_ptr_ref_push(&l1);

      arg_list = arg_list->obj.list.cdr;
      RvR_lisp_object *first = l1;
      RvR_lisp_object *next = NULL;
      lisp_ptr_ref_push(&first);

      if(l1==NULL)
      {
         l1 = first = RvR_lisp_eval(arg_list->obj.list.car);
         arg_list = arg_list->obj.list.cdr;
      }

      if(RvR_lisp_item_type(l1)!=RVR_L_CONS_CELL)
      {
         RvR_lisp_object_print(l1);
         RvR_lisp_break("first arg should be a list\n");
         exit(0);
      }

      do
      {
         next = l1;
         while(next!=NULL)
         {
            l1 = next;
            next = RvR_lisp_cdr(next);
         }

         RvR_lisp_object *tmp = RvR_lisp_eval(arg_list->obj.list.car);
         l1->obj.list.cdr = tmp;
         arg_list = arg_list->obj.list.cdr;
      }
      while(arg_list!=NULL);

      ret = first;

      lisp_ptr_ref_pop(1); //first
      lisp_ptr_ref_pop(1); //l1

      break;
   }
   case SYS_FUNC_FIRST:
      ret = (RvR_lisp_eval(arg_list->obj.list.car))->obj.list.car;
      break;
   case SYS_FUNC_SECOND:
      ret = (RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car))->obj.list.car;
      break;
   case SYS_FUNC_THIRD:
      ret = (RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.cdr->obj.list.car))->obj.list.car;
      break;
   case SYS_FUNC_FOURTH:
      ret = (RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.car))->obj.list.car;
      break;
   case SYS_FUNC_FIFTH:
      ret = (RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.car))->obj.list.car;
      break;
   case SYS_FUNC_SIXTH:
      ret = (RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.car))->obj.list.car;
      break;
   case SYS_FUNC_SEVENTH:
      ret = (RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.car))->obj.list.car;
      break;
   case SYS_FUNC_EIGHTH:
      ret = (RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.car))->obj.list.car;
      break;
   case SYS_FUNC_NINTH:
      ret = (RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.car))->obj.list.car;
      break;
   case SYS_FUNC_TENTH:
      ret = (RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.cdr->obj.list.car))->obj.list.car;
      break;
   case SYS_FUNC_SUBSTR:
   {
      int32_t x1 = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.car));
      int32_t x2 = RvR_lisp_number_value(RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car));
      RvR_lisp_object *st = RvR_lisp_eval(arg_list->obj.list.cdr->obj.list.car->obj.list.car);
      lisp_ptr_ref_push(&st);

      if(x1<0||x1>x2||x2>=strlen(RvR_lisp_string(st)))
      {
         RvR_lisp_break("substr: bad x1 or x2 value\n");
         exit(0);
      }

      RvR_lisp_object *s = RvR_lisp_stringl_create(x2-x1+2);
      if(x2-x1)
         memcpy(RvR_lisp_string(s),RvR_lisp_string(st)+x1,x2-x1+1);

      RvR_lisp_string(s)[x2-x1+1] = '\0';
      ret = s;

      lisp_ptr_ref_pop(1); //st

      break;
   }
   case 99: //TODO: whats that?
   {
      RvR_lisp_object *r = NULL;
      RvR_lisp_object *rstart = NULL;
      lisp_ptr_ref_push(&r);
      lisp_ptr_ref_push(&rstart);

      while(arg_list!=NULL)
      {
         RvR_lisp_object *q = RvR_lisp_eval(arg_list->obj.list.car);
         if(rstart==NULL)
            rstart = q;
         while(r!=NULL&&r->obj.list.cdr!=NULL)
            r = r->obj.list.cdr;
         r->obj.list.cdr = q;
         arg_list = arg_list->obj.list.cdr;
      }

      ret = rstart;

      lisp_ptr_ref_pop(1); //rstart
      lisp_ptr_ref_pop(1); //r

      break;
   }
   default:
      RvR_log("Undefined system function number %d\n",sym->obj.sys.fun_number);
      break;
   }

   lisp_ptr_ref_pop(1); //arg_list

   return ret;
}

void RvR_lisp_tmp_space()
{
   lisp_current_space = RVR_LISP_TMP_SPACE;
}

void RvR_lisp_perm_space()
{
   lisp_current_space = RVR_LISP_PERM_SPACE;
}

void RvR_lisp_use_user_space(void *addr, long size)
{
   lisp_current_space = RVR_LISP_USER_SPACE;
   lisp_free_space[RVR_LISP_USER_SPACE] = lisp_space[RVR_LISP_USER_SPACE] = addr;
   lisp_space_size[RVR_LISP_USER_SPACE] = size;
}

void RvR_lisp_init(RvR_lisp_cinit_func init, RvR_lisp_ccall_func ccall, RvR_lisp_lcall_func lcall, RvR_lisp_lset_func lset, RvR_lisp_lget_func lget, RvR_lisp_lprint_func lprint)
{
   //Set function pointers
   lisp_cinit = init;
   lisp_ccall = ccall;
   lisp_lcall = lcall;
   lisp_lset = lset;
   lisp_lget = lget;
   lisp_lprint = lprint;

   lisp_symbol_root = NULL;
   lisp_total_user_functions = 0;

   lisp_free_space[0] = lisp_space[0] = RvR_malloc(0x1000);
   lisp_space_size[0] = 0x1000;

   lisp_free_space[1] = lisp_space[1] = RvR_malloc(0x1000);
   lisp_space_size[1] = 0x1000;

   lisp_current_space = RVR_LISP_PERM_SPACE;

   //lcomp_init
   //This needs to be defined first
   lisp_undefined = RvR_lisp_symbol_find_or_create(":UNDEFINED");

   //Collection problems result if we don't do this
   lisp_undefined->obj.sym.function = NULL;
   lisp_undefined->obj.sym.value = NULL;

   lisp_true_symbol = RvR_lisp_symbol_find_or_create("T");
   lisp_list_symbol = RvR_lisp_symbol_find_or_create("list");
   lisp_string_symbol = RvR_lisp_symbol_find_or_create("string");
   lisp_quote_symbol = RvR_lisp_symbol_find_or_create("quote");
   lisp_backquote_symbol = RvR_lisp_symbol_find_or_create("backquote");
   lisp_comma_symbol = RvR_lisp_symbol_find_or_create("comma");
   lisp_in_symbol = RvR_lisp_symbol_find_or_create("in");
   lisp_do_symbol = RvR_lisp_symbol_find_or_create("do");
   lisp_aref_symbol = RvR_lisp_symbol_find_or_create("aref");
   lisp_colon_initial_contents = RvR_lisp_symbol_find_or_create(":initial-contents");
   lisp_colon_initial_element = RvR_lisp_symbol_find_or_create(":initial-element");

   lisp_if1_progn = RvR_lisp_symbol_find_or_create("if-1progn");
   lisp_if2_progn = RvR_lisp_symbol_find_or_create("if-2progn");
   lisp_if12_progn = RvR_lisp_symbol_find_or_create("if-12progn");
   lisp_if_symbol = RvR_lisp_symbol_find_or_create("if");
   lisp_progn_symbol = RvR_lisp_symbol_find_or_create("progn");
   lisp_not_symbol = RvR_lisp_symbol_find_or_create("not");
   lisp_eq_symbol = RvR_lisp_symbol_find_or_create("eq");
   lisp_zero_symbol = RvR_lisp_symbol_find_or_create("0");
   lisp_eq0_symbol = RvR_lisp_symbol_find_or_create("eq0");
   lisp_car_symbol = RvR_lisp_symbol_find_or_create("car");
   lisp_cdr_symbol = RvR_lisp_symbol_find_or_create("cdr");
   lisp_load_warning = RvR_lisp_symbol_find_or_create("load_warning");
   //lcomp_init end

   for(size_t i = 0;i<sizeof(lisp_sys_funcs)/sizeof(*lisp_sys_funcs);i++)
      lisp_add_sys_function(lisp_sys_funcs[i].name,lisp_sys_funcs[i].min_args,lisp_sys_funcs[i].max_args,i);

   lisp_cinit();

   lisp_current_space = RVR_LISP_TMP_SPACE;
   RvR_log("Lisp: %d symbols defined, %d system functions, ""%d pre-compiled functions\n",lisp_symbol_count,sizeof(lisp_sys_funcs)/sizeof(*lisp_sys_funcs),lisp_total_user_functions);
}

void RvR_lisp_uninit()
{
   RvR_free(lisp_space[0]);
   RvR_free(lisp_space[1]);

   lisp_delete_all_symbols(lisp_symbol_root);
   lisp_symbol_root = NULL;
   lisp_symbol_count = 0;
}

void RvR_lisp_clear_tmp()
{
   lisp_free_space[RVR_LISP_TMP_SPACE] = lisp_space[RVR_LISP_TMP_SPACE];
}

void RvR_lisp_symbol_number_set(RvR_lisp_object *sym, int32_t num)
{
#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(sym)!=RVR_L_SYMBOL)
   {
      RvR_lisp_object_print(sym);
      RvR_lisp_break("is not a symbol\n");
      exit(0);
   }
#endif

   if(sym->obj.sym.value!=lisp_undefined&&RvR_lisp_item_type(sym->obj.sym.value)==RVR_L_NUMBER)
      sym->obj.sym.value->obj.num.num = num;
   else
      sym->obj.sym.value = RvR_lisp_number_create(num);
}

void RvR_lisp_symbol_value_set(RvR_lisp_object *sym, RvR_lisp_object *val)
{
#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(sym)!=RVR_L_SYMBOL)
   {
      RvR_lisp_object_print(sym);
      RvR_lisp_break("is not a symbol\n");
      exit(0);
   }
#endif

   sym->obj.sym.value = val;
}

static void lisp_print(RvR_lisp_object *block)
{
   if(block==NULL||RvR_lisp_item_type(block)!=RVR_L_CONS_CELL)
   {
      RvR_lisp_object_print(block);
      return;
   }

   RvR_log("(");
   for(;block!=NULL&&RvR_lisp_item_type(block)==RVR_L_CONS_CELL;block = block->obj.list.cdr)
   {
      RvR_lisp_object *a =block->obj.list.car;
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

static void lisp_print_string(const char *st)
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

static void lisp_ptr_ref_push(RvR_lisp_object **ref)
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

static RvR_lisp_object **lisp_ptr_ref_pop(size_t total)
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

   void *ret = lisp_free_space[which_space];
   lisp_free_space[which_space] += size;

   return ret;
}

static void lisp_collect_space(int which_space, int grow)
{
   int old_space = lisp_current_space;
   lisp_cstart = lisp_space[which_space];
   lisp_cend = lisp_free_space[which_space];

   lisp_space_size[RVR_LISP_GC_SPACE] = lisp_space_size[which_space];
   if(grow)
   {
      lisp_space_size[RVR_LISP_GC_SPACE]+=lisp_space_size[which_space]>>1;
      lisp_space_size[RVR_LISP_GC_SPACE]-=(lisp_space_size[RVR_LISP_GC_SPACE]&7);
   }

   uint8_t *new_space = RvR_malloc(lisp_space_size[RVR_LISP_GC_SPACE]);
   lisp_current_space = RVR_LISP_GC_SPACE;
   lisp_free_space[RVR_LISP_GC_SPACE] = lisp_space[RVR_LISP_GC_SPACE] = new_space;

   lisp_collected_start = new_space;
   lisp_collected_end = new_space+lisp_space_size[RVR_LISP_GC_SPACE];

   lisp_collect_symbols(lisp_symbol_root);

   lisp_collect_stacks();

   //for debuging clear it out
   memset(lisp_space[which_space],0,lisp_space_size[which_space]);
   RvR_free(lisp_space[which_space]);

   lisp_space[which_space] = new_space;
   lisp_space_size[which_space] = lisp_space_size[RVR_LISP_GC_SPACE];
   lisp_free_space[which_space] = new_space+(lisp_free_space[RVR_LISP_GC_SPACE]-lisp_space[RVR_LISP_GC_SPACE]);

   lisp_current_space = old_space;
}

static void lisp_collect_symbols(RvR_lisp_object *root)
{
   if(root==NULL)
      return;

   root->obj.sym.value = lisp_collect_object(root->obj.sym.value);
   root->obj.sym.function = lisp_collect_object(root->obj.sym.function);
   root->obj.sym.name = lisp_collect_object(root->obj.sym.name);
   lisp_collect_symbols(root->obj.sym.left);
   lisp_collect_symbols(root->obj.sym.right);
}

static void lisp_collect_stacks()
{
   RvR_lisp_object **d = lisp_user_stack.data;
   for(size_t i = 0;i<lisp_user_stack.data_used;i++,d++)
      *d = lisp_collect_object(*d);

   RvR_lisp_object ***d2 = lisp_ptr_ref.data;
   for(size_t i = 0;i<lisp_ptr_ref.data_used;i++,d2++)
   {
      RvR_lisp_object **ptr = *d2;
      *ptr = lisp_collect_object(*ptr);
   }
}

static RvR_lisp_object *lisp_collect_object(RvR_lisp_object *x)
{
   RvR_lisp_object *ret = x;

   if((uint8_t *)x>=lisp_cstart&&(uint8_t *)x<lisp_cend)
   {
      switch(RvR_lisp_item_type(x))
      {
      case RVR_L_BAD_CELL:
         RvR_lisp_break("error: collecting corrupted cell\n");
         break;
      case RVR_L_NUMBER:
         ret = RvR_lisp_number_create(x->obj.num.num);
         break;
      case RVR_L_SYS_FUNCTION:
         ret = RvR_lisp_sys_function_create(x->obj.sys.min_args,x->obj.sys.max_args,x->obj.sys.fun_number);
         break;
      case RVR_L_USER_FUNCTION:
         ret = RvR_lisp_user_function_create(lisp_collect_object(x->obj.usr.arg_list),lisp_collect_object(x->obj.usr.block_list));
         break;
      case RVR_L_STRING:
         ret = RvR_lisp_strings_create(x->obj.str.str);
         break;
      case RVR_L_CHARACTER:
         ret = RvR_lisp_char_create(x->obj.ch.ch);
         break;
      case RVR_L_C_FUNCTION:
         ret = RvR_lisp_c_function_create(x->obj.sys.min_args,x->obj.sys.max_args,x->obj.sys.fun_number);
         break;
      case RVR_L_C_BOOL:
         ret = RvR_lisp_c_bool_create(x->obj.sys.min_args,x->obj.sys.max_args,x->obj.sys.fun_number);
         break;
      case RVR_L_L_FUNCTION:
         ret = RvR_lisp_user_lisp_function_create(x->obj.sys.min_args,x->obj.sys.max_args,x->obj.sys.fun_number);
         break;
      case RVR_L_POINTER:
         ret = RvR_lisp_pointer_create(x->obj.addr.addr);
         break;
      case RVR_L_1D_ARRAY:
         ret = lisp_collect_array(x);
         break;
      case RVR_L_FIXED_POINT:
         ret = RvR_lisp_fixed_point_create(x->obj.fix.x);
         break;
      case RVR_L_CONS_CELL:
         ret = lisp_collect_list(x);
         break;
      case RVR_L_OBJECT_VAR:
         ret = RvR_lisp_object_var_create(x->obj.var.index);
         break;
      case RVR_L_COLLECTED_OBJECT:
         ret = x->obj.ref.ref;
         break;
      default:
         RvR_lisp_break("error: collecting bad object 0x%x\n",RvR_lisp_item_type(x));
         break;
      }

      x->type = RVR_L_COLLECTED_OBJECT;
      x->obj.ref.ref = ret;
   }
   else if((uint8_t *)x<lisp_collected_start||(uint8_t *)x>=lisp_collected_end)
   {
      //Still need to remap cons_cells lying outside of space, for
      //instance on the stack.
      for(RvR_lisp_object *cell = NULL;x!=NULL;cell = x,x = x->obj.list.cdr)
      {
         if(RvR_lisp_item_type(x)!=RVR_L_CONS_CELL)
         {
            if(cell!=NULL)
               cell->obj.list.cdr = lisp_collect_object(cell->obj.list.cdr);
            break;
         }
         x->obj.list.car = lisp_collect_object(x->obj.list.car);
      }
   }

   return ret;
}

static RvR_lisp_object *lisp_collect_array(RvR_lisp_object *x)
{
   size_t s = x->obj.arr.len;
   RvR_lisp_object *a = RvR_lisp_array_create(s,NULL);
   RvR_lisp_object **src = x->obj.arr.data;
   RvR_lisp_object **dst = a->obj.arr.data;

   for(size_t i = 0;i<s;i++)
      dst[i] = lisp_collect_object(src[i]);

   return a;
}

static RvR_lisp_object *lisp_collect_list(RvR_lisp_object *x)
{
   RvR_lisp_object *first = NULL;
   RvR_lisp_object *last = NULL;

   for(;x!=NULL&&RvR_lisp_item_type(x)==RVR_L_CONS_CELL;)
   {
      RvR_lisp_object *p = RvR_lisp_list_create();
      RvR_lisp_object *old_car = x->obj.list.car;
      RvR_lisp_object *old_cdr = x->obj.list.cdr;
      RvR_lisp_object *old_x = x;
      x = x->obj.list.cdr;
      old_x->type = RVR_L_COLLECTED_OBJECT;
      old_x->obj.ref.ref = p;

      p->obj.list.car = lisp_collect_object(old_car);
      p->obj.list.cdr = lisp_collect_object(old_cdr);

      if(last!=NULL)
         last->obj.list.cdr = p;
      else
         first = p;
      last = p;
   }
   if(x!=NULL)
      last->obj.list.cdr = lisp_collect_object(x);

   return first;
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
      lisp_delete_all_symbols(root->obj.sym.left);
      lisp_delete_all_symbols(root->obj.sym.right);
      RvR_free(root);
   }
}

static RvR_lisp_object *lisp_pairlis(RvR_lisp_object *list1, RvR_lisp_object *list2, RvR_lisp_object *list3)
{
   if(RvR_lisp_item_type(list1)!=RVR_L_CONS_CELL||RvR_lisp_item_type(list1)!=RvR_lisp_item_type(list2))
      return NULL;

   void *ret = NULL;
   size_t l1 = RvR_lisp_list_length(list1);
   size_t l2 = RvR_lisp_list_length(list2);

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
            last->obj.list.cdr = cur;
         last = cur;

         RvR_lisp_object *cell = RvR_lisp_list_create();
         tmp = RvR_lisp_car(list1);
         cell->obj.list.car = tmp;
         tmp = RvR_lisp_car(list2);
         cell->obj.list.cdr = tmp;
         cur->obj.list.car = cell;

         list1 = ((RvR_lisp_object *)list1)->obj.list.cdr;
         list2 = ((RvR_lisp_object *)list2)->obj.list.cdr;
      }

      cur->obj.list.cdr = list3;
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
   if(s->obj.sym.function!=lisp_undefined)
   {
      RvR_lisp_break("add_sys_function -> symbol %s already has a function\n",name);
      exit(0);
   }

   s->obj.sym.function = RvR_lisp_sys_function_create(min_args, max_args, number);

   return s;
}

static void lisp_skip_c_comment(const char **s)
{
   (*s)+=2;
   while(**s&&(**s!='*'||*((*s)+1)!='/'))
   {
      if(**s=='/'&&*((*s)+1)=='*')
         lisp_skip_c_comment(s);
      else
         (*s)++;
   }

   if(**s)
      (*s)+=2;
}

static long lisp_str_token_len(const char *st)
{
   long x = 1;
   while(*st&&(*st!='"'||st[1]=='"'))
   {
      if(*st=='\\'||*st=='"')
         st++;
      st++;
      x++;
   }

   return x;
}

static RvR_lisp_object *lisp_comp_optimize(RvR_lisp_object *list)
{
   RvR_lisp_object *return_value = list;

   lisp_ptr_ref_push(&list);

   if(list!=NULL)
   {
      if(list->obj.list.car==lisp_if_symbol)
      {
         RvR_lisp_object *eval1 = RvR_lisp_car(RvR_lisp_cdr(RvR_lisp_cdr(list)));
         RvR_lisp_object *eval2 = RvR_lisp_car(RvR_lisp_cdr(RvR_lisp_cdr(RvR_lisp_cdr(list))));

         lisp_ptr_ref_push(&eval1);
         lisp_ptr_ref_push(&eval2);

         RvR_lisp_object *ret = NULL;
         lisp_ptr_ref_push(&ret);

         if(RvR_lisp_car(list)==lisp_eq_symbol&&(RvR_lisp_car(RvR_lisp_cdr(list))==lisp_zero_symbol)) //simplify (eq 0 x) -> (eq0 x)
         {
            RvR_lisp_push_onto_list(RvR_lisp_car(RvR_lisp_cdr(RvR_lisp_cdr(list))),&ret);
            RvR_lisp_push_onto_list(lisp_eq0_symbol,&ret);
            return_value = lisp_comp_optimize(ret);
         }
         else if(RvR_lisp_car(list)==lisp_eq_symbol&&(RvR_lisp_car(RvR_lisp_cdr(RvR_lisp_cdr(list)))==lisp_zero_symbol)) //simplify (eq x 0)-> (eq0 x)
         {
            RvR_lisp_push_onto_list(RvR_lisp_car(RvR_lisp_cdr(list)),&ret);
            RvR_lisp_push_onto_list(lisp_eq0_symbol,&ret);
            return_value = lisp_comp_optimize(ret);
         }
         else if(RvR_lisp_car(RvR_lisp_car(RvR_lisp_cdr(list)))==lisp_not_symbol) //simplify (if (not y) x z) -> (if y z x)
         {
            RvR_lisp_push_onto_list(RvR_lisp_car(RvR_lisp_cdr(RvR_lisp_cdr(list))),&ret);
            RvR_lisp_push_onto_list(RvR_lisp_car(RvR_lisp_cdr(RvR_lisp_cdr(RvR_lisp_cdr(list)))),&ret);
            RvR_lisp_push_onto_list(RvR_lisp_car(RvR_lisp_cdr(RvR_lisp_cdr(RvR_lisp_cdr(list)))),&ret);
            RvR_lisp_push_onto_list(lisp_if_symbol,&ret);
            return_value = lisp_comp_optimize(ret);
         }
         else if(RvR_lisp_car(eval1)==lisp_progn_symbol&&(eval2==NULL||RvR_lisp_item_type(eval2)!=RVR_L_CONS_CELL))
         {
            RvR_lisp_push_onto_list(eval2,&ret);
            RvR_lisp_push_onto_list(RvR_lisp_cdr(eval1),&ret);
            RvR_lisp_push_onto_list(RvR_lisp_car(RvR_lisp_cdr(list)),&ret);
            RvR_lisp_push_onto_list(lisp_if1_progn,&ret);
            return_value = lisp_comp_optimize(ret);
         }
         else if(RvR_lisp_car(eval1)==lisp_progn_symbol&&RvR_lisp_car(eval2)==lisp_progn_symbol)
         {
            RvR_lisp_push_onto_list(RvR_lisp_cdr(eval2),&ret);
            RvR_lisp_push_onto_list(RvR_lisp_cdr(eval1),&ret);
            RvR_lisp_push_onto_list(RvR_lisp_car(RvR_lisp_cdr(list)),&ret);
            RvR_lisp_push_onto_list(lisp_if12_progn,&ret);
            return_value = lisp_comp_optimize(ret);
         }
         else if(RvR_lisp_car(eval2)==lisp_progn_symbol)
         {
            RvR_lisp_push_onto_list(RvR_lisp_cdr(eval2),&ret);
            RvR_lisp_push_onto_list(eval1,&ret);
            RvR_lisp_push_onto_list(RvR_lisp_car(RvR_lisp_cdr(list)),&ret);
            RvR_lisp_push_onto_list(lisp_if2_progn,&ret);
            return_value = lisp_comp_optimize(ret);
         }

         lisp_ptr_ref_pop(1); //ret
         lisp_ptr_ref_pop(1); //eval2
         lisp_ptr_ref_pop(1); //eval1
      }
   }

   lisp_ptr_ref_pop(1); //list
   return return_value;
}

static void lisp_pro_print(RvR_rw *out, RvR_lisp_object *p)
{
   if(p!=NULL)
   {
      lisp_pro_print(out,p->obj.sym.right);
      {
         char st[100];
         snprintf(st,100,"%20s %f\n",p->obj.sym.name->obj.str.str,p->obj.sym.time_taken);
         RvR_rw_write(out,st,strlen(st),1);
      }
      lisp_pro_print(out,p->obj.sym.left);
   }
}

static void lisp_preport(char *fn)
{
   RvR_rw fp;
   RvR_rw_init_path(&fp,"preport.out","wb");

   lisp_pro_print(&fp,lisp_symbol_root);

   RvR_rw_close(&fp);
}

static RvR_lisp_object *lisp_mapcar(RvR_lisp_object *arg_list)
{
   lisp_ptr_ref_push(&arg_list);
   RvR_lisp_object *sym = RvR_lisp_eval(arg_list->obj.list.car);

   switch(RvR_lisp_item_type(sym))
   {
   case RVR_L_SYS_FUNCTION:
   case RVR_L_USER_FUNCTION:
   case RVR_L_SYMBOL:
      break;
   default:
      RvR_lisp_object_print(sym);
      RvR_lisp_break(" is not a function\n");
      exit(0);
   }

   int i = 0;
   int stop = 0;
   int num_args  = RvR_lisp_list_length(arg_list->obj.list.cdr);
   if(!num_args)
      return NULL;

   RvR_lisp_object **arg_on = RvR_malloc(sizeof(*arg_on)*num_args);
   RvR_lisp_object *list_on = arg_list->obj.list.cdr;
   long old_ptr_son = lisp_ptr_ref.data_used;

   for(i = 0;i<num_args;i++)
   {
      arg_on[i] = RvR_lisp_eval(list_on->obj.list.car);

      lisp_ptr_ref_push(&arg_on[i]);

      list_on = list_on->obj.list.cdr;
      if(arg_on[i]==NULL)
         stop = 1;
   }

   if(stop)
   {
      RvR_free(arg_on);
      return NULL;
   }

   RvR_lisp_object *na_list = NULL;
   RvR_lisp_object *return_list = NULL;
   RvR_lisp_object *last_return = NULL;

   do
   {
      na_list = NULL;

      RvR_lisp_object *first = NULL;
      for(i = 0;!stop&&i<num_args;i++)
      {
         if(na_list==NULL)
         {
            first = na_list = RvR_lisp_list_create();
         }
         else
         {
            na_list->obj.list.cdr = RvR_lisp_list_create();
            na_list = na_list->obj.list.cdr;
         }

         if(arg_on[i]!=NULL)
         {
            na_list->obj.list.car = arg_on[i]->obj.list.car;
            arg_on[i] = arg_on[i]->obj.list.cdr;
         }
         else
         {
            stop = 1;
         }
      }
      if(!stop)
      {
         RvR_lisp_object *c = RvR_lisp_list_create();
         c->obj.list.car = RvR_lisp_eval_function(sym,first);
         if(return_list!=NULL)
            last_return->obj.list.cdr = c;
         else
            return_list = c;
         last_return = c;
      }
   }while(!stop);

   lisp_ptr_ref.data_used = old_ptr_son;

   RvR_free(arg_on);
   lisp_ptr_ref_pop(1); //arg_list

   return return_list;
}

static RvR_lisp_object *lisp_concatenate(RvR_lisp_object *prog_list)
{
   RvR_lisp_object *el_list = prog_list->obj.list.cdr;

   lisp_ptr_ref_push(&prog_list);
   lisp_ptr_ref_push(&el_list);

   RvR_lisp_object *ret = NULL;
   RvR_lisp_object *rtype = RvR_lisp_eval(prog_list->obj.list.car);

   long len = 0;
   if(rtype==lisp_string_symbol) //Determine length of resulting string
   {
      int elements = RvR_lisp_list_length(el_list);
      if(!elements)
      {
         ret = RvR_lisp_strings_create("");
      }
      else
      {
         RvR_lisp_object **str_eval = RvR_malloc(sizeof(*str_eval)*elements);
         int i = 0;
         int old_ptr_stack_start = lisp_ptr_ref.data_used;

         for(i = 0;i<elements;i++,el_list = el_list->obj.list.cdr)
         {
            str_eval[i] = RvR_lisp_eval(el_list->obj.list.car);
            lisp_ptr_ref_push(&str_eval[i]);

            switch(RvR_lisp_item_type(str_eval[i]))
            {
            case RVR_L_CONS_CELL:
            {
               RvR_lisp_object *char_list = str_eval[i];
               while(char_list!=NULL)
               {
                  if(RvR_lisp_item_type(char_list->obj.list.car)==RVR_L_CHARACTER)
                  {
                     len++;
                  }
                  else
                  {
                     RvR_lisp_object_print(str_eval[i]);
                     RvR_lisp_break(" is not a character\n");
                     exit(0);
                  }

                  char_list = char_list->obj.list.cdr;
               }
               break;
            }
            case RVR_L_STRING: len+=strlen(str_eval[i]->obj.str.str); break;
            default:
               RvR_lisp_object_print(prog_list);
               RvR_lisp_break("type not supported\n");
               exit(0);
               break;
            }
         }

         RvR_lisp_object *st = RvR_lisp_stringl_create(len+1);
         char *s = st->obj.str.str;

         for(i = 0;i<elements;i++)
         {
            switch(RvR_lisp_item_type(str_eval[i]))
            {
            case RVR_L_CONS_CELL:
            {
               RvR_lisp_object *char_list = str_eval[i];
               while(char_list!=NULL)
               {
                  if(RvR_lisp_item_type(char_list->obj.list.car)==RVR_L_CHARACTER)
                     *(s++) = char_list->obj.list.car->obj.ch.ch;
                  char_list = char_list->obj.list.cdr;
               }
               break;
            }
            case RVR_L_STRING:
               memcpy(s,str_eval[i]->obj.str.str,strlen(str_eval[i]->obj.str.str));
               s+=strlen(str_eval[i]->obj.str.str);
               break;
            default:;
            }
         }

         RvR_free(str_eval);
         lisp_ptr_ref.data_used = old_ptr_stack_start;
         *s = '\0';
         ret = st;
      }
   }
   else
   {
      RvR_lisp_object_print(prog_list);
      RvR_lisp_break("concat operation not supported, try 'string\n");
      exit(0);
   }

   lisp_ptr_ref_pop(1); //el_list
   lisp_ptr_ref_pop(1); //prog_list

   return ret;
}

static RvR_lisp_object *lisp_backquote_eval(RvR_lisp_object *args)
{
   if(RvR_lisp_item_type(args)!=RVR_L_CONS_CELL)
      return args;

   if(args==NULL)
      return NULL;
   if(args->obj.list.car==lisp_comma_symbol)
      return RvR_lisp_eval(args->obj.list.cdr->obj.list.car);

   RvR_lisp_object *first = NULL;
   RvR_lisp_object *last = NULL;
   RvR_lisp_object *cur = NULL;
   RvR_lisp_object *tmp = NULL;

   lisp_ptr_ref_push(&first);
   lisp_ptr_ref_push(&last);
   lisp_ptr_ref_push(&cur);
   lisp_ptr_ref_push(&args);

   while(args!=NULL)
   {
      if(RvR_lisp_item_type(args)==RVR_L_CONS_CELL)
      {
         if(args->obj.list.car==lisp_comma_symbol)
         {
            tmp = RvR_lisp_eval(args->obj.list.cdr->obj.list.car);
            last->obj.list.cdr = tmp;
            args = NULL;
         }
         else
         {
            cur = RvR_lisp_list_create();
            if(first!=NULL)
               last->obj.list.cdr = cur;
            else
               first = cur;
            last = cur;
            tmp = lisp_backquote_eval(args);
            cur->obj.list.car = tmp;
            args = args->obj.list.cdr;
         }
      }
      else
      {
         tmp = lisp_backquote_eval(args);
         last->obj.list.cdr = tmp;
         args = NULL;
      }
   }

   lisp_ptr_ref_pop(1); //args
   lisp_ptr_ref_pop(1); //cur
   lisp_ptr_ref_pop(1); //last
   lisp_ptr_ref_pop(1); //first

   return first;
}

static void lisp_user_stack_push(RvR_lisp_object *data)
{
   if(lisp_user_stack.data==NULL)
   {
      lisp_user_stack.data_size = 1024;
      lisp_user_stack.data_used = 0;
      lisp_user_stack.data = RvR_malloc(sizeof(*lisp_user_stack.data)*lisp_user_stack.data_size);
   }

   lisp_user_stack.data[lisp_user_stack.data_used++] = data;

   if(lisp_user_stack.data_used==lisp_user_stack.data_size)
   {
      RvR_lisp_break("error: stack overflow (%d>=%d)\n",lisp_user_stack.data_used,lisp_user_stack.data_size);
      exit(1);
   }
}

static RvR_lisp_object *lisp_user_stack_pop(size_t total)
{
   if(total>lisp_user_stack.data_used)
   {
      RvR_lisp_break("error: stack underflow\n");
      exit(1);
   }

   lisp_user_stack.data_used-=total;
   return lisp_user_stack.data[lisp_user_stack.data_used];
}
//-------------------------------------
