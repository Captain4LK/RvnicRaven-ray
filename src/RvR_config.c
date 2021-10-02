/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdlib.h>
#include <SLK/SLK.h>
//-------------------------------------

//Internal includes
#include "RvR_core.h"
#include "RvR_config.h"
#include "RvR_error.h"
#include "RvR_malloc.h"
#include "RvR_hash.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
int RvR_config_mouse_sensitivity = 128;
int RvR_config_mouse_sensitivity_vertical = 128;
RvR_key RvR_config_move_forward = RVR_KEY_W;
RvR_key RvR_config_move_backward = RVR_KEY_S;
RvR_key RvR_config_strafe_left = RVR_KEY_A;
RvR_key RvR_config_strafe_right = RVR_KEY_D;
RvR_key RvR_config_enable_freelook = RVR_KEY_F;
RvR_key RvR_config_jump = RVR_KEY_SPACE;
unsigned RvR_config_texture_timeout = 1;
int RvR_config_camera_max_shear = 192;
int RvR_config_camera_shear_step = 8;
//-------------------------------------

//Function prototypes
static char *ini(const char *s);
static RvR_key strtokey(const char *s);
static const char *keytostr(RvR_key k);
//-------------------------------------

//Function implementations

int RvR_ini_parse(const char *path)
{
   char *buffer_in = NULL;
   char *kv = NULL;
   char *iter = NULL;
   int32_t size = 0;
   uint64_t hash_key = 0;
   FILE *in = NULL;

   RvR_error_check(path!=NULL,0x101);
   in = fopen(path,"rb");
   if(in==NULL)
   {
      RvR_ini_write(path);
      RvR_error_check(0,0x006);
   }

   RvR_error_check(fseek(in,0,SEEK_END)==0,0x004);
   size = ftell(in);
   RvR_error_check(size!=EOF,0x005);
   RvR_error_check(fseek(in,0,SEEK_SET)==0,0x004);
   buffer_in = RvR_malloc(size+1);
   RvR_error_check(buffer_in!=NULL,0x001);
   RvR_error_check(fread(buffer_in,size,1,in)==1,0x003);
   buffer_in[size] = '\0';
   RvR_error_check(fclose(in)!=EOF,0x007);
   in = NULL;

   kv = ini(buffer_in);
   RvR_error_check(kv!=NULL,0x000);
   for(iter = kv;iter[0];)
   {
      //Read key
      hash_key = RvR_fnv64a(iter);
      //printf("key: %s;%luU\n",iter,hash_key);

      while(*iter++);
      switch(hash_key)
      {
      case 16969830980003191976U: RvR_config_mouse_sensitivity = atoi(iter); break;             //"mouse_sensitivity"
      case  6023395255864463179U: RvR_config_mouse_sensitivity_vertical = atoi(iter); break;    //"mouse_sensitivity_vertical"
      case  3560386037893974558U: RvR_config_move_forward = strtokey(iter); break;              //"move_forward"
      case  2211778712401880640U: RvR_config_move_backward = strtokey(iter); break;             //"move_backward"
      case  9406806077120427738U: RvR_config_strafe_left = strtokey(iter); break;               //"strafe_left"
      case  5573416641251682759U: RvR_config_strafe_right = strtokey(iter); break;              //"strafe_right"
      case  6604522056233544978U: RvR_config_enable_freelook = strtokey(iter); break;           //"enable_freelook"
      case 16940139219101328589U: RvR_config_jump = strtokey(iter); break;                      //"jump"
      case 11034882399129047948U: RvR_config_texture_timeout = atoi(iter); break;               //"texture_timeout"
      case  7868638888548120077U: RvR_config_camera_max_shear = atoi(iter); break;              //"camera_max_shear"
      case 11836815913816774343U: RvR_config_camera_shear_step = atoi(iter); break;             //"camera_shear_step"
      }

      //printf("val: '%s'\n", iter);

      while(*iter++);
   }
   
   RvR_free(kv);
   RvR_free(buffer_in);

   return 0;

RvR_err:

   if(in!=NULL&&RvR_error_get()!=RVR_ERROR_FAIL_FCLOSE)
      fclose(in);

   if(kv!=NULL)
      RvR_free(kv);

   if(buffer_in!=NULL)
      RvR_free(buffer_in);

   RvR_log("RvR error %s\n",RvR_error_get_string());

   return 1;
}

int RvR_ini_write(const char *path)
{
   FILE *f = NULL;

   RvR_error_check(path!=NULL,0x101);
   f = fopen(path,"w");
   RvR_error_check(f!=NULL,0x006);

   RvR_error_check(fprintf(f,";Mouse input\n")>=0,0x008);
   RvR_error_check(fprintf(f,"mouse_sensitivity=%d\n",RvR_config_mouse_sensitivity)>=0,0x008);
   RvR_error_check(fprintf(f,"mouse_sensitivity_vertical=%d\n",RvR_config_mouse_sensitivity_vertical)>=0,0x008);

   RvR_error_check(fprintf(f,"\n;Keyboard input\n")>=0,0x008);
   RvR_error_check(fprintf(f,"move_forward=%s\n",keytostr(RvR_config_move_forward))>=0,0x008);
   RvR_error_check(fprintf(f,"move_backward=%s\n",keytostr(RvR_config_move_backward))>=0,0x008);
   RvR_error_check(fprintf(f,"strafe_left=%s\n",keytostr(RvR_config_strafe_left))>=0,0x008);
   RvR_error_check(fprintf(f,"strafe_right=%s\n",keytostr(RvR_config_strafe_right))>=0,0x008);
   RvR_error_check(fprintf(f,"enable_freelook=%s\n",keytostr(RvR_config_enable_freelook))>=0,0x008);
   RvR_error_check(fprintf(f,"jump=%s\n",keytostr(RvR_config_jump))>=0,0x008);

   RvR_error_check(fprintf(f,"\n;Engine settings\n")>=0,0x008);
   RvR_error_check(fprintf(f,"texture_timeout=%d\n",RvR_config_texture_timeout)>=0,0x008);
   RvR_error_check(fprintf(f,"camera_max_shear=%d\n",RvR_config_camera_max_shear)>=0,0x008);
   RvR_error_check(fprintf(f,"camera_shear_step=%d\n",RvR_config_camera_shear_step)>=0,0x008);

   RvR_error_check(fclose(f)!=EOF,0x007);

   return 0;

RvR_err:

   if(f!=NULL&&RvR_error_get()!=RVR_ERROR_FAIL_FCLOSE)
      fclose(f);

   RvR_log("RvR error %s\n",RvR_error_get_string());

   return 1;
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
static char *ini(const char *s)
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
         if((reqlen+maplen)>=mapcap) { map = RvR_realloc(map,(mapcap+=reqlen+512)); RvR_error_check(map!=NULL,0x002); }
         sprintf(map+maplen,"%.*s%c%.*s%c%c",(int)(key-buf),buf,0,(int)(end[VAL]-cut[VAL]),cut[VAL],0,0);
         maplen+=reqlen-1;
      }
   }

   return map;

RvR_err:

   return NULL;
}

static RvR_key strtokey(const char *s)
{
   uint64_t hash_key = RvR_fnv64a(s);
   //printf("ikey: %s;%luU\n",s,hash_key);

   switch(hash_key)
   {
   case 12638187200555641996U: return RVR_KEY_A;
   case 12638190499090526629U: return RVR_KEY_B;
   case 12638189399578898418U: return RVR_KEY_C;
   case 12638183902020757363U: return RVR_KEY_D;
   case 12638182802509129152U: return RVR_KEY_E;
   case 12638186101044013785U: return RVR_KEY_F;
   case 12638185001532385574U: return RVR_KEY_G;
   case 12638197096160295895U: return RVR_KEY_H;
   case 12638195996648667684U: return RVR_KEY_I;
   case 12638199295183552317U: return RVR_KEY_J;
   case 12638198195671924106U: return RVR_KEY_K;
   case 12638192698113783051U: return RVR_KEY_L;
   case 12638191598602154840U: return RVR_KEY_M;
   case 12638194897137039473U: return RVR_KEY_N;
   case 12638193797625411262U: return RVR_KEY_O;
   case 12638205892253321583U: return RVR_KEY_P;
   case 12638204792741693372U: return RVR_KEY_Q;
   case 12638208091276578005U: return RVR_KEY_R;
   case 12638206991764949794U: return RVR_KEY_S;
   case 12638201494206808739U: return RVR_KEY_T;
   case 12638200394695180528U: return RVR_KEY_U;
   case 12638203693230065161U: return RVR_KEY_V;
   case 12638202593718436950U: return RVR_KEY_W;
   case 12638214688346347271U: return RVR_KEY_X;
   case 12638213588834719060U: return RVR_KEY_Y;
   case 12638216887369603693U: return RVR_KEY_Z;
   case 12638135523509116079U: return RVR_KEY_0;
   case 12638134423997487868U: return RVR_KEY_1;
   case 12638137722532372501U: return RVR_KEY_2;
   case 12638136623020744290U: return RVR_KEY_3;
   case 12638131125462603235U: return RVR_KEY_4;
   case 12638130025950975024U: return RVR_KEY_5;
   case 12638133324485859657U: return RVR_KEY_6;
   case 12638132224974231446U: return RVR_KEY_7;
   case 12638144319602141767U: return RVR_KEY_8;
   case 12638143220090513556U: return RVR_KEY_9;
   case  4858524775536387045U: return RVR_KEY_SPACE;
   default: return RVR_KEY_NONE;
   }
}

static const char *keytostr(RvR_key k)
{
   switch(k)
   {
      case RVR_KEY_A: return "a";
      case RVR_KEY_B: return "b";
      case RVR_KEY_C: return "c";
      case RVR_KEY_D: return "d";
      case RVR_KEY_E: return "e";
      case RVR_KEY_F: return "f";
      case RVR_KEY_G: return "g";
      case RVR_KEY_H: return "h";
      case RVR_KEY_I: return "i";
      case RVR_KEY_J: return "j";
      case RVR_KEY_K: return "k";
      case RVR_KEY_L: return "l";
      case RVR_KEY_M: return "m";
      case RVR_KEY_N: return "n";
      case RVR_KEY_O: return "o";
      case RVR_KEY_P: return "p";
      case RVR_KEY_Q: return "q";
      case RVR_KEY_R: return "r";
      case RVR_KEY_S: return "s";
      case RVR_KEY_T: return "t";
      case RVR_KEY_U: return "u";
      case RVR_KEY_V: return "v";
      case RVR_KEY_W: return "w";
      case RVR_KEY_X: return "x";
      case RVR_KEY_Y: return "y";
      case RVR_KEY_Z: return "z";
      case RVR_KEY_0: return "0";
      case RVR_KEY_1: return "1";
      case RVR_KEY_2: return "2";
      case RVR_KEY_3: return "3";
      case RVR_KEY_4: return "4";
      case RVR_KEY_5: return "5";
      case RVR_KEY_6: return "6";
      case RVR_KEY_7: return "7";
      case RVR_KEY_8: return "8";
      case RVR_KEY_9: return "9";
      case RVR_KEY_SPACE: return "space";
      default: return "NONE";
   }
}
//-------------------------------------
