/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
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

static const RvR_fix22 math_cos_table22[1024] =
{
   1024,1023,1023,1023,1023,1023,1023,1023,1022,1022,1022,1021,1021,1020,1020,1019,
   1019,1018,1017,1017,1016,1015,1014,1013,1012,1011,1010,1009,1008,1007,1006,1005,
   1004,1003,1001,1000,999,997,996,994,993,991,990,988,986,985,983,981,
   979,978,976,974,972,970,968,966,964,962,959,957,955,953,950,948,
   946,943,941,938,936,933,930,928,925,922,920,917,914,911,908,906,
   903,900,897,894,890,887,884,881,878,875,871,868,865,861,858,854,
   851,847,844,840,837,833,829,826,822,818,814,811,807,803,799,795,
   791,787,783,779,775,771,767,762,758,754,750,745,741,737,732,728,
   724,719,715,710,706,701,696,692,687,683,678,673,668,664,659,654,
   649,644,639,634,629,625,620,615,609,604,599,594,589,584,579,574,
   568,563,558,553,547,542,537,531,526,521,515,510,504,499,493,488,
   482,477,471,466,460,454,449,443,437,432,426,420,414,409,403,397,
   391,386,380,374,368,362,356,350,344,339,333,327,321,315,309,303,
   297,291,285,279,273,267,260,254,248,242,236,230,224,218,212,205,
   199,193,187,181,175,168,162,156,150,144,137,131,125,119,112,106,
   100,94,87,81,75,69,62,56,50,43,37,31,25,18,12,6,
   0,-6,-12,-18,-25,-31,-37,-43,-50,-56,-62,-69,-75,-81,-87,-94,
   -100,-106,-112,-119,-125,-131,-137,-144,-150,-156,-162,-168,-175,-181,-187,-193,
   -199,-205,-212,-218,-224,-230,-236,-242,-248,-254,-260,-267,-273,-279,-285,-291,
   -297,-303,-309,-315,-321,-327,-333,-339,-344,-350,-356,-362,-368,-374,-380,-386,
   -391,-397,-403,-409,-414,-420,-426,-432,-437,-443,-449,-454,-460,-466,-471,-477,
   -482,-488,-493,-499,-504,-510,-515,-521,-526,-531,-537,-542,-547,-553,-558,-563,
   -568,-574,-579,-584,-589,-594,-599,-604,-609,-615,-620,-625,-629,-634,-639,-644,
   -649,-654,-659,-664,-668,-673,-678,-683,-687,-692,-696,-701,-706,-710,-715,-719,
   -724,-728,-732,-737,-741,-745,-750,-754,-758,-762,-767,-771,-775,-779,-783,-787,
   -791,-795,-799,-803,-807,-811,-814,-818,-822,-826,-829,-833,-837,-840,-844,-847,
   -851,-854,-858,-861,-865,-868,-871,-875,-878,-881,-884,-887,-890,-894,-897,-900,
   -903,-906,-908,-911,-914,-917,-920,-922,-925,-928,-930,-933,-936,-938,-941,-943,
   -946,-948,-950,-953,-955,-957,-959,-962,-964,-966,-968,-970,-972,-974,-976,-978,
   -979,-981,-983,-985,-986,-988,-990,-991,-993,-994,-996,-997,-999,-1000,-1001,-1003,
   -1004,-1005,-1006,-1007,-1008,-1009,-1010,-1011,-1012,-1013,-1014,-1015,-1016,-1017,-1017,-1018,
   -1019,-1019,-1020,-1020,-1021,-1021,-1022,-1022,-1022,-1023,-1023,-1023,-1023,-1023,-1023,-1023,
   -1024,-1023,-1023,-1023,-1023,-1023,-1023,-1023,-1022,-1022,-1022,-1021,-1021,-1020,-1020,-1019,
   -1019,-1018,-1017,-1017,-1016,-1015,-1014,-1013,-1012,-1011,-1010,-1009,-1008,-1007,-1006,-1005,
   -1004,-1003,-1001,-1000,-999,-997,-996,-994,-993,-991,-990,-988,-986,-985,-983,-981,
   -979,-978,-976,-974,-972,-970,-968,-966,-964,-962,-959,-957,-955,-953,-950,-948,
   -946,-943,-941,-938,-936,-933,-930,-928,-925,-922,-920,-917,-914,-911,-908,-906,
   -903,-900,-897,-894,-890,-887,-884,-881,-878,-875,-871,-868,-865,-861,-858,-854,
   -851,-847,-844,-840,-837,-833,-829,-826,-822,-818,-814,-811,-807,-803,-799,-795,
   -791,-787,-783,-779,-775,-771,-767,-762,-758,-754,-750,-745,-741,-737,-732,-728,
   -724,-719,-715,-710,-706,-701,-696,-692,-687,-683,-678,-673,-668,-664,-659,-654,
   -649,-644,-639,-634,-629,-625,-620,-615,-609,-604,-599,-594,-589,-584,-579,-574,
   -568,-563,-558,-553,-547,-542,-537,-531,-526,-521,-515,-510,-504,-499,-493,-488,
   -482,-477,-471,-466,-460,-454,-449,-443,-437,-432,-426,-420,-414,-409,-403,-397,
   -391,-386,-380,-374,-368,-362,-356,-350,-344,-339,-333,-327,-321,-315,-309,-303,
   -297,-291,-285,-279,-273,-267,-260,-254,-248,-242,-236,-230,-224,-218,-212,-205,
   -199,-193,-187,-181,-175,-168,-162,-156,-150,-144,-137,-131,-125,-119,-112,-106,
   -100,-94,-87,-81,-75,-69,-62,-56,-50,-43,-37,-31,-25,-18,-12,-6,
   0,6,12,18,25,31,37,43,50,56,62,69,75,81,87,94,
   100,106,112,119,125,131,137,144,150,156,162,168,175,181,187,193,
   199,205,212,218,224,230,236,242,248,254,260,267,273,279,285,291,
   297,303,309,315,321,327,333,339,344,350,356,362,368,374,380,386,
   391,397,403,409,414,420,426,432,437,443,449,454,460,466,471,477,
   482,488,493,499,504,510,515,521,526,531,537,542,547,553,558,563,
   568,574,579,584,589,594,599,604,609,615,620,625,629,634,639,644,
   649,654,659,664,668,673,678,683,687,692,696,701,706,710,715,719,
   724,728,732,737,741,745,750,754,758,762,767,771,775,779,783,787,
   791,795,799,803,807,811,814,818,822,826,829,833,837,840,844,847,
   851,854,858,861,865,868,871,875,878,881,884,887,890,894,897,900,
   903,906,908,911,914,917,920,922,925,928,930,933,936,938,941,943,
   946,948,950,953,955,957,959,962,964,966,968,970,972,974,976,978,
   979,981,983,985,986,988,990,991,993,994,996,997,999,1000,1001,1003,
   1004,1005,1006,1007,1008,1009,1010,1011,1012,1013,1014,1015,1016,1017,1017,1018,
   1019,1019,1020,1020,1021,1021,1022,1022,1022,1023,1023,1023,1023,1023,1023,1023,
};
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

RvR_fix22 RvR_fix22_from_int(int a)
{
   return (RvR_fix22)a*1024;
}

int RvR_fix22_to_int(RvR_fix22 a)
{
   return (int)(a/1024);
}

RvR_fix22 RvR_fix22_mul(RvR_fix22 a, RvR_fix22 b)
{
   int64_t p = (int64_t)a*(int64_t)b;

   return (RvR_fix22)(p/1024);
}

RvR_fix22 RvR_fix22_div(RvR_fix22 a, RvR_fix22 b)
{
   int64_t p = (int64_t)a*1024;

   return (RvR_fix22)(p/(int64_t)b);
}

RvR_fix22 RvR_fix22_cos(RvR_fix22 a)
{
   return math_cos_table22[a&1023];
}

RvR_fix22 RvR_fix22_sin(RvR_fix22 a)
{
   return RvR_fix22_cos(a-256);
}

RvR_fix22 RvR_fix22_tan(RvR_fix22 a)
{
   return (RvR_fix22_sin(a)*1024)/RvR_non_zero(RvR_fix22_cos(a));
}

RvR_fix22 RvR_fix22_ctg(RvR_fix22 a)
{
   return (RvR_fix22_cos(a)*1024)/RvR_fix22_sin(a);
}

RvR_fix22 RvR_fix22_sqrt(RvR_fix22 a)
{
   RvR_fix22 result = 0;
   RvR_fix22 b = 1u<<30;

   while(b>a)
      b>>=2;

   while(b!=0)
   {
      if(a>=result+b)
      {
         a-=result+b;
         result = result+2*b;
      }

      b>>=2;
      result>>=1;
   }

   return result;
}

int32_t RvR_div_round_down(int32_t a, int32_t b)
{
  return a/b-((a>=0)?0:1);
}

RvR_vec2 RvR_vec2_rot(RvR_fix22 angle)
{
   RvR_vec2 result;

   result.x = RvR_fix22_cos(angle);
   result.y = -RvR_fix22_sin(angle);

   return result;
}

RvR_fix22 RvR_dist2(RvR_vec2 p0, RvR_vec2 p1)
{
   RvR_fix22 dx = p1.x-p0.x;
   RvR_fix22 dy = p1.y-p0.y;

  dx = dx*dx;
  dy = dy*dy;

  return RvR_fix22_sqrt(dx+dy);
}

RvR_fix22 RvR_len2(RvR_vec2 v)
{
   RvR_vec2 zero = {0};

   return RvR_dist2(zero,v);
}

int32_t RvR_abs(int32_t a)
{
   return a*(((a>=0)<<1)-1);
}

//Like mod, but behaves differently for negative values.
int32_t RvR_wrap(int32_t a, int32_t mod)
{
   int32_t cmp = a<0;
   return cmp*mod+(a%mod)-cmp;
}

int32_t RvR_clamp(int32_t a, int32_t min, int32_t max)
{
   if(a>=min)
   {
      if(a<=max)
         return a;
      else
         return max;
   }
   else
   {
      return min;
   }
}
//-------------------------------------
