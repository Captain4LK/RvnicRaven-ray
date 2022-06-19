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
//-------------------------------------

//Internal includes
#include "RvnicRaven.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static char *config_ini(const char *s);
static RvR_key config_strtokey(const char *s);
//-------------------------------------

//Function implementations

RvR_config RvR_ini_parse(RvR_rw *rw)
{
   RvR_error_check(rw!=NULL,"RvR_ini_parse","argument 'rw' must be non-NULL\n");

   RvR_rw_seek(rw,0,SEEK_END);
   int32_t size = RvR_rw_tell(rw);
   RvR_rw_seek(rw,0,SEEK_SET);
   char *buffer_in = RvR_malloc(sizeof(*buffer_in)*(size+1));
   RvR_rw_read(rw,buffer_in,sizeof(*buffer_in)*size,1);
   buffer_in[size] = '\0';

   char *kv = config_ini(buffer_in);
   RvR_error_check(kv!=NULL,"RvR_ini_parse","failed to parse ini file\n");

   RvR_free(buffer_in);

   return kv;

   RvR_err:
      return NULL;
}

void RvR_ini_free(RvR_config config)
{
   RvR_error_check(config!=NULL,"RvR_ini_free","argument 'config' must be non-NULL\n");

   RvR_free(config);

RvR_err:
   return;
}

void RvR_ini_read(RvR_config config, void *dst, RvR_config_type type, const char *ident)
{
   RvR_error_check(config!=NULL,"RvR_ini_read","argument 'config' must be non-NULL\n");
   RvR_error_check(dst!=NULL,"RvR_ini_read","argument 'dst' must be non-NULL\n");
   RvR_error_check(ident!=NULL,"RvR_ini_read","argument 'ident' must be non-NULL\n");

   char *iter = NULL;
   for(iter = config;iter[0];)
   {
      if(strcmp(ident,iter)==0)
      {
         while(*iter++);
         
         switch(type)
         {
         case RVR_CONFIG_INT: *((int *)dst) = atoi(iter); break;
         case RVR_CONFIG_KEY: *((RvR_key *)dst) = config_strtokey(iter); break;
         }

         return;
      }

      while(*iter++);
      while(*iter++);
   }

   RvR_error_fail("RvR_ini_read","identifier '%s' not found in ini\n",ident);

RvR_err:
   return;
}

//Ini parser by r-lyeh: https://github.com/r-lyeh/tinybits
//Original header:
// ini+, extended ini format 
// - rlyeh, public domain
//
// # spec
//
//   ; line comment
//   [details]          ; map section name (optional)
//   user=john          ; key and value (mapped here as details.user=john)
//   +surname=doe jr.   ; sub-key and value (mapped here as details.user.surname=doe jr.)
//   color=240          ; key and value |
//   color=253          ; key and value |> array: color[0], color[1] and color[2]
//   color=255          ; key and value |
//   color=             ; remove key/value(s)
//   color=white        ; recreate key; color[1] and color[2] no longer exist
//   []                 ; unmap section
//   -note=keys may start with symbols (except plus and semicolon)
//   -note=linefeeds are either \r, \n or \r\n.
//   -note=utf8 everywhere.
//
static char *config_ini(const char *s)
{
   char *map = NULL;
   int mapcap = 0;
   int maplen = 0;
   enum 
   { 
      DEL, REM, TAG, KEY, SUB, VAL 
   }fsm = DEL;
   const char *cut[6] = {0};
   const char *end[6] = {0};

   while(*s)
   {
      while(*s&&(*s==' '||*s =='\t'||*s=='\r'||*s=='\n')) ++s;

      if(*s ==';') cut[fsm = REM] = ++s;
      else if(*s=='[') cut[fsm = TAG] = ++s;
      else if(*s=='+') cut[fsm = SUB] = ++s;
      else if(*s=='=') cut[fsm = VAL] = ++s;
      else if(*s>' '&&*s<='z'&&*s!=']') cut[fsm = KEY] = cut[SUB] = end[SUB] = s;
      else { ++s; continue; }

      if(fsm==REM) { while(*s&&*s!='\r'&&*s!='\n') ++s; }
      else if(fsm==TAG) { while(*s&&*s!='\r'&&*s!='\n'&&*s!=']') ++s; end[fsm] = s; }
      else if(fsm==KEY) { while(*s&&*s> ' '&&*s<='z'&&*s!='=') ++s; end[fsm] = s; }
      else if(fsm==SUB) { while(*s&&*s> ' '&&*s<='z'&&*s!='=') ++s; end[fsm] = s; }
      else if(fsm==VAL)
      {
         char buf[256] = {0};
         char *key = NULL;
         int reqlen = 0;

         while(*s && *s >= ' ' && *s <= 'z' && *s != ';') ++s;
         end[fsm] = s;
         while(end[fsm][-1]==' ') --end[fsm];
         key = buf;
         if(end[TAG]-cut[TAG]) key+=sprintf(key,"%.*s.", (int)(end[TAG]-cut[TAG]),cut[TAG]);
         if(end[KEY]-cut[KEY]) key+=sprintf(key,"%.*s", (int)(end[KEY]-cut[KEY]),cut[KEY]);
         if(end[SUB]-cut[SUB]) key+=sprintf(key,".%.*s", (int)(end[SUB]-cut[SUB]),cut[SUB]);
         reqlen = (key-buf)+1+(end[VAL]-cut[VAL])+1+1;
         if((reqlen+maplen)>=mapcap) { map = RvR_realloc(map,(mapcap+=reqlen+512)); }
         sprintf(map+maplen,"%.*s%c%.*s%c%c",(int)(key-buf),buf,0,(int)(end[VAL]-cut[VAL]),cut[VAL],0,0);
         maplen+=reqlen-1;
      }
   }

   return map;
}

static RvR_key config_strtokey(const char *s)
{
   RvR_error_check(s!=NULL,"RvR_ini_read","argument 's' must be non-NULL\n");

   uint32_t hash_key = RvR_fnv32a(s);
   RvR_log("ikey: %s;%u\n",s,hash_key);

   switch(hash_key)
   {
   case 0xe40c292c: return RVR_KEY_A; //'a'
   case 0xe70c2de5: return RVR_KEY_B; //'b'
   case 0xe60c2c52: return RVR_KEY_C; //'c'
   case 0xe10c2473: return RVR_KEY_D; //'d'
   case 0xe00c22e0: return RVR_KEY_E; //'e'
   case 0xe30c2799: return RVR_KEY_F; //'f'
   case 0xe20c2606: return RVR_KEY_G; //'g'
   case 0xed0c3757: return RVR_KEY_H; //'h'
   case 0xec0c35c4: return RVR_KEY_I; //'i'
   case 0xef0c3a7d: return RVR_KEY_J; //'j'
   case 0xee0c38ea: return RVR_KEY_K; //'k'
   case 0xe90c310b: return RVR_KEY_L; //'l'
   case 0xe80c2f78: return RVR_KEY_M; //'m'
   case 0xeb0c3431: return RVR_KEY_N; //'n'
   case 0xea0c329e: return RVR_KEY_O; //'o'
   case 0xf50c43ef: return RVR_KEY_P; //'p'
   case 0xf40c425c: return RVR_KEY_Q; //'q'
   case 0xf70c4715: return RVR_KEY_R; //'r'
   case 0xf60c4582: return RVR_KEY_S; //'s'
   case 0xf10c3da3: return RVR_KEY_T; //'t'
   case 0xf00c3c10: return RVR_KEY_U; //'u'
   case 0xf30c40c9: return RVR_KEY_V; //'v'
   case 0xf20c3f36: return RVR_KEY_W; //'w'
   case 0xfd0c5087: return RVR_KEY_X; //'x'
   case 0xfc0c4ef4: return RVR_KEY_Y; //'y'
   case 0xff0c53ad: return RVR_KEY_Z; //'z'
   case 0x350ca8af: return RVR_KEY_0; //'0'
   case 0x340ca71c: return RVR_KEY_1; //'1'
   case 0x370cabd5: return RVR_KEY_2; //'2'
   case 0x360caa42: return RVR_KEY_3; //'3'
   case 0x310ca263: return RVR_KEY_4; //'4'
   case 0x300ca0d0: return RVR_KEY_5; //'5'
   case 0x330ca589: return RVR_KEY_6; //'6'
   case 0x320ca3f6: return RVR_KEY_7; //'7'
   case 0x3d0cb547: return RVR_KEY_8; //'8'
   case 0x3c0cb3b4: return RVR_KEY_9; //'9'
   case 0x3553e285: return RVR_KEY_SPACE; //'space'
   default: return RVR_KEY_NONE;
   }

RvR_err:
   return RVR_KEY_NONE;
}
//-------------------------------------
