/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <SLK/SLK.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR_error.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static const char *error_file = NULL;
static unsigned error_line = 0;
static RvR_error error_reason = 0;
static char error_string[512] = "";
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void RvR_error_set(const char *file, unsigned line, unsigned reason)
{
   //This makes it possible to error out but not set the error variable
   //This can be used in case a function A calls a function B that errors out
   //and you want A to fail without overriding the error set by B
   if(reason==0x000)
      return;

   error_file = file;
   error_line = line;

   switch(reason)
   {
   case 0x001: error_reason = RVR_ERROR_FAIL_MALLOC; break;
   case 0x002: error_reason = RVR_ERROR_FAIL_REALLOC; break;
   case 0x003: error_reason = RVR_ERROR_FAIL_FWRITE; break;
   case 0x004: error_reason = RVR_ERROR_FAIL_FSEEK; break;
   case 0x005: error_reason = RVR_ERROR_FAIL_FTELL; break;
   case 0x006: error_reason = RVR_ERROR_FAIL_FOPEN; break;
   case 0x007: error_reason = RVR_ERROR_FAIL_FCLOSE; break;
   case 0x008: error_reason = RVR_ERROR_FAIL_FPRINTF; break;
   case 0x100: error_reason = RVR_ERROR_ARG_OOR; break;
   case 0x101: error_reason = RVR_ERROR_ARG_NULL; break;
   case 0x200: error_reason = RVR_ERROR_BUFFER_SHORT; break;
   }
}

void RvR_error_clear()
{
   error_reason = RVR_ERROR_NONE;
   error_file = NULL;
   error_line = 0;
   error_string[0] = '\0';
}

RvR_error RvR_error_get()
{
   return error_reason;
}

const char *RvR_error_get_string()
{
   if(error_reason==RVR_ERROR_NONE)
   {
      strncpy(error_string,"No error encountered",512);
      return error_string;
   }

   const char *desc = "(NULL)";
   switch(error_reason)
   {
   case RVR_ERROR_NONE: break; //Handled above
   case RVR_ERROR_FAIL_MALLOC: desc = "malloc() failed, out of memory"; break;
   case RVR_ERROR_FAIL_REALLOC: desc = "realloc() failed, out of memory"; break;
   case RVR_ERROR_FAIL_FWRITE: desc = "fwrite() failed"; break;
   case RVR_ERROR_FAIL_FSEEK: desc = "fseek() failed"; break;
   case RVR_ERROR_FAIL_FTELL: desc = "ftell() failed"; break;
   case RVR_ERROR_FAIL_FOPEN: desc = "fopen() failed"; break;
   case RVR_ERROR_FAIL_FCLOSE: desc = "fclose() failed"; break;
   case RVR_ERROR_FAIL_FPRINTF: desc = "fprintf() failed"; break;
   case RVR_ERROR_ARG_OOR: desc = "argument out of expected range"; break;
   case RVR_ERROR_ARG_NULL: desc = "argument NULL where non-NULL was expected"; break;
   case RVR_ERROR_BUFFER_SHORT: desc = "buffer too short"; break;
   }

   snprintf(error_string,512,"(%s:%u): %s",error_file,error_line,desc);
   return error_string;
}

void RvR_log(const char *w, ...)
{
   va_list args;
   va_start(args,w);
   vprintf(w,args);
   va_end(args);
}
//-------------------------------------
