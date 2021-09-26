/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_ERROR_H_

#define _RVR_ERROR_H_

typedef enum
{
   RVR_ERROR_NONE = 0x000,              //No error encountered
   RVR_ERROR_FAIL_MALLOC = 0x001,       //malloc() failed, out of memory
   RVR_ERROR_FAIL_REALLOC = 0x002,      //realloc() failed, out of memory
   RVR_ERROR_FAIL_FWRITE = 0x003,       //fwrite() failed
   RVR_ERROR_FAIL_FSEEK = 0x004,        //fseek() failed
   RVR_ERROR_FAIL_FTELL = 0x005,        //ftell() failed
   RVR_ERROR_FAIL_FOPEN = 0x006,        //fopen() failed
   RVR_ERROR_FAIL_FCLOSE = 0x007,       //fclose() failed
   RVR_ERROR_FAIL_FPRINTF = 0x008,      //fprintf() failed
   RVR_ERROR_ARG_OOR = 0x100,           //argument outside expected range
   RVR_ERROR_ARG_NULL = 0x101,          //argument NULL 
   RVR_ERROR_BUFFER_SHORT = 0x200,      //buffer short
}RvR_error;

//Set the current error
//Parameters:
//    const char *file - name of the file the error occurred in
//                       usually set using __FILE__
//                       file != NULL
//    unsigned line - line the error occurred in
//                    usually set using __LINE__
//    unsigned reason - reason for error
//                      integer codes can be read from switch statement
//                      in implementation
void RvR_error_set(const char *file, unsigned line, unsigned reason);

//Clear all errors
void RvR_error_clear();

//Returns the last error as a enum
RvR_error RvR_error_get();

//Returns the last error as a string, 
//returned string will only be valid until next HLH_error_get_string() call
const char *RvR_error_get_string();

void RvR_log(const char *w, ...);

#define RVR_ERROR_FAIL(X) do { RvR_error_set(__FILE__,__LINE__,(X)); goto RvR_err; } while(0)

#define RVR_ERROR_CHECK(X,Y) do { if(!(X)) RVR_ERROR_FAIL(Y); } while(0)

#endif
