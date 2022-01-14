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

struct
{
   void ***data;
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
static void lisp_l1print(void *block);
static void lisp_lprint_string(const char *st);
static void lisp_where_print(int max_level);
static void lisp_ptr_ref_push(void **ref);
static void **lisp_ptr_ref_pop(size_t total);
static void lisp_need_perm_space(const char *why);
static size_t lisp_get_free_size(int which_space);
static void *lisp_malloc(size_t size, int which_space);
static void lisp_collect_space(int which_space, int grow);
static void lisp_collect_symbols(RvR_lisp_object *root);
static RvR_lisp_object *lisp_collect_object(RvR_lisp_object *x);
static RvR_lisp_object *lisp_collect_array(RvR_lisp_object *x);
static RvR_lisp_object *lisp_collect_list(RvR_lisp_object *x);
static char *lisp_error(const char *loc, const char *cause);
static void lisp_delete_all_symbols(RvR_lisp_object *root);
static void *lisp_pairlis(void *list1, void *list2, void *list3);
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

int32_t RvR_lisp_fixed_point_get(RvR_lisp_object *o)
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
            ret = o->value;
            if(RvR_lisp_item_type(ret)==RVR_L_OBJECT_VAR)
               ret = NULL; //TODO
         }
         break;
      case RVR_L_CONS_CELL:
         ret = RvR_lisp_eval_function(o->car,o->cdr);
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

RvR_lisp_object *RvR_lisp_number_create(int32_t num)
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

int32_t RvR_lisp_fixed_point_value(void *c)
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

RvR_lisp_object *RvR_lisp_add_c_object(void *symbol, int index)
{
   lisp_need_perm_space("RvR_lisp_add_c_object");
   RvR_lisp_object *s = symbol;
   if(s->value!=lisp_undefined)
   {
      RvR_lisp_break("RvR_lisp_add_c_object -> symbol %s already has a value\n",RvR_lisp_string_get(s->name));
      exit(0);
   }

   s->value = RvR_lisp_object_var_create(index);
   return NULL;
}

RvR_lisp_object *RvR_lisp_add_c_function(const char *name, int min_args, int max_args, int number)
{
   lisp_total_user_functions++;
   lisp_need_perm_space("RvR_lisp_add_c_function");
   RvR_lisp_object *s = RvR_lisp_symbol_find_or_create(name);
   if(s->function!=lisp_undefined)
   {
      RvR_lisp_break("RvR_lisp_add_c_function --> symbol %s already has a function\n",name);
      exit(0);
   }

   s->function = RvR_lisp_c_function_create(min_args,max_args,number);

   return s;
}

RvR_lisp_object *RvR_lisp_add_c_bool_function(const char *name, int min_args, int max_args, int number)
{
   lisp_total_user_functions++;
   lisp_need_perm_space("RvR_lisp_add_c_bool_function");
   RvR_lisp_object *s = RvR_lisp_symbol_find_or_create(name);
   if(s->function!=lisp_undefined)
   {
      RvR_lisp_break("RvR_lisp_add_c_bool_function --> symbol %s already has a function\n",name);
      exit(0);
   }

   s->function = RvR_lisp_c_bool_create(min_args,max_args,number);

   return s;
}

RvR_lisp_object *RvR_lisp_add_lisp_function(const char *name, int min_args, int max_args, int number)
{
   lisp_total_user_functions++;
   lisp_need_perm_space("RvR_lisp_add_lisp_function");
   RvR_lisp_object *s = RvR_lisp_symbol_find_or_create(name);
   if(s->function!=lisp_undefined)
   {
      RvR_lisp_break("RvR_lisp_add_lisp_function --> symbol %s already has a function\n",name);
      exit(0);
   }

   s->function = RvR_lisp_user_lisp_function_create(min_args,max_args,number);

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

void RvR_lisp_push_onto_list(void *object, void **list)
{
   lisp_ptr_ref_push(&object);
   lisp_ptr_ref_push(list);

   RvR_lisp_object *c = RvR_lisp_list_create();
   c->car = object;
   c->cdr = *list;
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

      cs->car = lisp_quote_symbol;
      c2 = RvR_lisp_list_create();
      tmp = RvR_lisp_compile(s);
      c2->car = tmp;
      c2->cdr = NULL;
      cs->cdr = c2;
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

      cs->car = lisp_backquote_symbol;
      c2 = RvR_lisp_list_create();
      tmp = RvR_lisp_compile(s);
      c2->car = tmp;
      c2->cdr = NULL;
      cs->cdr = c2;
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

      cs->car = lisp_comma_symbol;
      c2 = RvR_lisp_list_create();
      tmp = RvR_lisp_compile(s);
      c2->car = tmp;
      c2->cdr = NULL;
      cs->cdr = c2;
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
                  last->cdr = tmpo;
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
               cur->car = tmpo;
               if(last!=NULL)
                  last->cdr = cur;
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
      sscanf(lisp_n,"%d",&num->num);
      ret = num;
   }
   else if(lisp_n[0]=='"')
   {
      ret = RvR_lisp_stringl_create(lisp_str_token_len(*s));
      char *start = ret->str;
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
         cs->car = tmp;
         c2 = RvR_lisp_list_create();
         tmp = RvR_lisp_compile(s);
         c2->car = tmp;
         cs->cdr = c2;
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

   RvR_lisp_object *fun = sym->function;

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
      req_min = fun->min_args;
      req_max = fun->max_args;
      break;
   case RVR_L_USER_FUNCTION:
      return RvR_lisp_eval_user_function(sym,arg_list);
   default:
      RvR_lisp_object_print(sym);
      RvR_lisp_break(" is not a function name");
      exit(0);
   }

   if(req_min!=-1)
   {
      RvR_lisp_object *a = arg_list;
      for(args = 0;a!=NULL;a = a->cdr)
         args++;

      if(args<req_min)
      {
         RvR_lisp_object_print(arg_list);
         RvR_lisp_object_print(sym->name);
         RvR_lisp_break("\nToo few parameters to function\n");
         exit(0);
      }
      else if(req_max!=-1&&args>req_max)
      {
         RvR_lisp_object_print(arg_list);
         RvR_lisp_object_print(sym->name);
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
      ret = RvR_lisp_eval_sys_function(sym,arg_list);
      break;
   case RVR_L_L_FUNCTION:
      //TODO: ret = lcaller
      break;
   case RVR_L_USER_FUNCTION:
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
            cur->cdr = tmp;
         else
            first = tmp;
         cur = tmp;

         RvR_lisp_object *val = RvR_lisp_eval(arg_list->car);
         cur->car = val;
         arg_list = RvR_lisp_cdr(arg_list);
      }

      //TODO: ccaller
      //if(t==RVR_L_C_FUNCTION)
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

   RvR_lisp_object *fun = sym->function;

#if RVR_LISP_TYPE_CHECK
   if(RvR_lisp_item_type(fun)!=RVR_L_USER_FUNCTION)
   {
      RvR_lisp_object_print(fun);
      RvR_lisp_break("is not a user defined function\n");
   }
#endif

   RvR_lisp_object *fun_arg_list = fun->arg_list;
   RvR_lisp_object *block_list = fun->block_list;
   lisp_ptr_ref_push(&fun_arg_list);
   lisp_ptr_ref_push(&block_list);

   //mark the stack start, so we can restore when done
   long stack_start = lisp_user_stack.data_used;

   RvR_lisp_object *f_arg = NULL;
   lisp_ptr_ref_push(&f_arg);
   lisp_ptr_ref_push(&arg_list);

   for(f_arg = fun_arg_list;f_arg!=NULL;f_arg = f_arg->cdr)
   {
      RvR_lisp_object *s = f_arg->car;
      lisp_user_stack_push(s->value);
   }

   //open block so that local vars aren't saved on the stack
   {
      int new_start = lisp_user_stack.data_used;
      int i = new_start;

      for(f_arg = fun_arg_list;f_arg!=NULL;f_arg = f_arg->cdr)
      {
         if(arg_list==NULL)
         {
            RvR_lisp_object_print(sym);
            RvR_lisp_break("too few parameters to function\n");
            exit(0);
         }

         lisp_user_stack_push(RvR_lisp_eval(arg_list->car));
         arg_list = arg_list->cdr;
      }

      //now store all the values and put them into the symbols
      for(f_arg = fun_arg_list;f_arg!=NULL;f_arg = f_arg->cdr)
         f_arg->car->value = lisp_user_stack.data[i++];

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
      ret = RvR_lisp_eval(block_list->car);
      block_list = block_list->cdr;
   }

   long cur_stack = stack_start;
   for(f_arg = fun_arg_list;f_arg!=NULL;f_arg = f_arg->cdr)
      f_arg->car->value = lisp_user_stack.data[cur_stack++];

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

   switch(sym->fun_number)
   {
   case SYS_FUNC_PRINT:
      while(arg_list!=NULL)
      {
         ret = RvR_lisp_eval(arg_list->car);
         arg_list = arg_list->cdr;
         RvR_lisp_object_print(ret);
      }
      break;
   case SYS_FUNC_CAR:
      ret = RvR_lisp_car(RvR_lisp_eval(arg_list->car));
      break;
   case SYS_FUNC_CDR:
      ret = RvR_lisp_cdr(RvR_lisp_eval(arg_list->car));
      break;
   case SYS_FUNC_LENGTH:
   {
      RvR_lisp_object *v = RvR_lisp_eval(arg_list->car);
      switch(RvR_lisp_item_type(v))
      {
      case RVR_L_STRING:
         ret = RvR_lisp_number_create(strlen(v->str));
         break;
      case RVR_L_CONS_CELL:
         ret = RvR_lisp_number_create(RvR_lisp_list_get_length(v));
         break;
      default:
         RvR_lisp_object_print(v);
         RvR_lisp_break("length: type not supported\n");
         break;
      }
      break;
   }
   case SYS_FUNC_LIST:
      //TODO
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

void RvR_lisp_init()
{
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
   lisp_undefined->function = NULL;
   lisp_undefined->value = NULL;

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

   //clisp_init //TODO
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
   int old_space = lisp_current_space;
   lisp_cstart = lisp_space[which_space];
   lisp_cend = lisp_free_space[which_space];

   lisp_space_size[RVR_LISP_GC_SPACE] = lisp_space_size[which_space];
   if(grow)
   {
      lisp_space_size[RVR_LISP_GC_SPACE]+=lisp_space_size[which_space] >> 1;
      lisp_space_size[RVR_LISP_GC_SPACE]-=(lisp_space_size[RVR_LISP_GC_SPACE]&7);
   }

   uint8_t *new_space = RvR_malloc(lisp_space_size[RVR_LISP_GC_SPACE]);
   lisp_current_space = RVR_LISP_GC_SPACE;
   lisp_free_space[RVR_LISP_GC_SPACE] = lisp_space[RVR_LISP_GC_SPACE] = new_space;

   lisp_collected_start = new_space;
   lisp_collected_end = new_space+lisp_space_size[RVR_LISP_GC_SPACE];

   lisp_collect_symbols(lisp_symbol_root);
   //TODO

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

   root->value = lisp_collect_object(root->value);
   root->function = lisp_collect_object(root->value);
   root->name = lisp_collect_object(root->name);
   lisp_collect_symbols(root->left);
   lisp_collect_symbols(root->right);
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
         ret = RvR_lisp_number_create(x->num);
         break;
      case RVR_L_SYS_FUNCTION:
         ret = RvR_lisp_sys_function_create(x->min_args,x->max_args,x->fun_number);
         break;
      case RVR_L_USER_FUNCTION:
         ret = RvR_lisp_user_function_create(lisp_collect_object(x->arg_list),lisp_collect_object(x->block_list));
         break;
      case RVR_L_STRING:
         ret = RvR_lisp_strings_create(x->str);
         break;
      case RVR_L_CHARACTER:
         ret = RvR_lisp_char_create(x->ch);
         break;
      case RVR_L_C_FUNCTION:
         ret = RvR_lisp_c_function_create(x->min_args,x->max_args,x->fun_number);
         break;
      case RVR_L_C_BOOL:
         ret = RvR_lisp_c_bool_create(x->min_args,x->max_args,x->fun_number);
         break;
      case RVR_L_L_FUNCTION:
         ret = RvR_lisp_user_lisp_function_create(x->min_args,x->max_args,x->fun_number);
         break;
      case RVR_L_POINTER:
         ret = RvR_lisp_pointer_create(x->addr);
         break;
      case RVR_L_1D_ARRAY:
         ret = lisp_collect_array(x);
         break;
      case RVR_L_FIXED_POINT:
         ret = RvR_lisp_fixed_point_create(x->x);
         break;
      case RVR_L_CONS_CELL:
         ret = lisp_collect_list(x);
         break;
      case RVR_L_OBJECT_VAR:
         ret = RvR_lisp_object_var_create(x->index);
         break;
      case RVR_L_COLLECTED_OBJECT:
         ret = x->ref;
         break;
      default:
         RvR_lisp_break("error: collecting bad object 0x%x\n",RvR_lisp_item_type(x));
         break;
      }

      x->type = RVR_L_COLLECTED_OBJECT;
      x->ref = ret;
   }
   else if((uint8_t *)x<lisp_collected_start||(uint8_t *)x>=lisp_collected_end)
   {
      //Still need to remap cons_cells lying outside of space, for
      //instance on the stack.
      for(RvR_lisp_object *cell = NULL;x!=NULL;cell = x,x = x->cdr)
      {
         if(RvR_lisp_item_type(x)!=RVR_L_CONS_CELL)
         {
            if(cell!=NULL)
               cell->cdr = lisp_collect_object(cell->cdr);
            break;
         }
         x->car = lisp_collect_object(x->car);
      }
   }

   return ret;
}

static RvR_lisp_object *lisp_collect_array(RvR_lisp_object *x)
{
   size_t s = x->len;
   RvR_lisp_object *a = RvR_lisp_array_create(s,NULL);
   RvR_lisp_object **src = x->data;
   RvR_lisp_object **dst = a->data;

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
      RvR_lisp_object *old_car = x->car;
      RvR_lisp_object *old_cdr = x->cdr;
      RvR_lisp_object *old_x = x;
      x = x->cdr;
      old_x->type = RVR_L_COLLECTED_OBJECT;
      old_x->ref = p;

      p->car = lisp_collect_object(old_car);
      p->cdr = lisp_collect_object(old_cdr);

      if(last!=NULL)
         last->cdr = p;
      else
         first = p;
      last = p;
   }
   if(x!=NULL)
      last->cdr = lisp_collect_object(x);

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
      RvR_lisp_break("add_sys_function -> symbol %s already has a function\n",name);
      exit(0);
   }

   s->function = RvR_lisp_sys_function_create(min_args, max_args, number);

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
      if(list->car==lisp_if_symbol)
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
            RvR_lisp_push_onto_list(lisp_if_symbol,ret);
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
      lisp_pro_print(out,p->right);
      {
         char st[100];
         snprintf(st,100,"%20s %f\n",p->name->str,p->time_taken);
         RvR_rw_write(out,st,strlen(st),1);
      }
      lisp_pro_print(out,p->left);
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
   RvR_lisp_object *sym = RvR_lisp_eval(arg_list->car);

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
   int num_args  = RvR_lisp_list_get_length(arg_list->cdr);
   if(!num_args)
      return NULL;

   RvR_lisp_object **arg_on = RvR_malloc(sizeof(*arg_on)*num_args);
   RvR_lisp_object *list_on = arg_list->cdr;
   long old_ptr_son = lisp_ptr_ref.data_used;

   for(i = 0;i<num_args;i++)
   {
      arg_on[i] = RvR_lisp_eval(list_on->car);

      lisp_ptr_ref_push(&arg_on[i]);

      list_on = list_on->cdr;
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
            na_list->cdr = RvR_lisp_list_create();
            na_list = na_list->cdr;
         }

         if(arg_on[i]!=NULL)
         {
            na_list->car = arg_on[i]->car;
            arg_on[i] = arg_on[i]->cdr;
         }
         else
         {
            stop = 1;
         }
      }
      if(!stop)
      {
         RvR_lisp_object *c = RvR_lisp_list_create();
         c->car = RvR_lisp_eval_function(sym,first);
         if(return_list!=NULL)
            last_return->cdr = c;
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
   RvR_lisp_object *el_list = prog_list->cdr;

   lisp_ptr_ref_push(&prog_list);
   lisp_ptr_ref_push(&el_list);

   RvR_lisp_object *ret = NULL;
   RvR_lisp_object *rtype = RvR_lisp_eval(prog_list->car);

   long len = 0;
   if(rtype==lisp_string_symbol) //Determine length of resulting string
   {
      int elements = RvR_lisp_list_get_length(el_list);
      if(!elements)
      {
         ret = RvR_lisp_strings_create("");
      }
      else
      {
         RvR_lisp_object **str_eval = RvR_malloc(sizeof(*str_eval)*elements);
         int i = 0;
         int old_ptr_stack_start = lisp_ptr_ref.data_used;

         for(i = 0;i<elements;i++,el_list = el_list->cdr)
         {
            str_eval[i] = RvR_lisp_eval(el_list->car);
            lisp_ptr_ref_push(&str_eval[i]);

            switch(RvR_lisp_item_type(str_eval[i]))
            {
            case RVR_L_CONS_CELL:
            {
               RvR_lisp_object *char_list = str_eval[i];
               while(char_list!=NULL)
               {
                  if(RvR_lisp_item_type(char_list->car)==RVR_L_CHARACTER)
                  {
                     len++;
                  }
                  else
                  {
                     RvR_lisp_object_print(str_eval[i]);
                     RvR_lisp_break(" is not a character\n");
                     exit(0);
                  }

                  char_list = char_list->cdr;
               }
               break;
            }
            case RVR_L_STRING: len+=strlen(str_eval[i]->str); break;
            default:
               RvR_lisp_object_print(prog_list);
               RvR_lisp_break("type not supported\n");
               exit(0);
               break;
            }
         }

         RvR_lisp_object *st = RvR_lisp_stringl_create(len+1);
         char *s = st->str;

         for(i = 0;i<elements;i++)
         {
            switch(RvR_lisp_item_type(str_eval[i]))
            {
            case RVR_L_CONS_CELL:
            {
               RvR_lisp_object *char_list = str_eval[i];
               while(char_list!=NULL)
               {
                  if(RvR_lisp_item_type(char_list->car)==RVR_L_CHARACTER)
                     *(s++) = char_list->car->ch;
                  char_list = char_list->cdr;
               }
               break;
            }
            case RVR_L_STRING:
               memcpy(s,str_eval[i]->str,strlen(str_eval[i]->str));
               s+=strlen(str_eval[i]->str);
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
   if(args->car==lisp_comma_symbol)
      return RvR_lisp_eval(args->cdr->car);

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
         if(args->car==lisp_comma_symbol)
         {
            tmp = RvR_lisp_eval(args->cdr->car);
            last->cdr = tmp;
            args = NULL;
         }
         else
         {
            cur = RvR_lisp_list_create();
            if(first!=NULL)
               last->cdr = cur;
            else
               first = cur;
            last = cur;
            tmp = lisp_backquote_eval(args);
            cur->car = tmp;
            args = args->cdr;
         }
      }
      else
      {
         tmp = lisp_backquote_eval(args);
         last->cdr = tmp;
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
