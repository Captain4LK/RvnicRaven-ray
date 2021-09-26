/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_FIX22_H_

#define _RVR_FIX22_H_

typedef int32_t RvR_fix22;

typedef struct
{
   RvR_fix22 x;
   RvR_fix22 y;
}RvR_vec2;

typedef struct
{
   RvR_fix22 x;
   RvR_fix22 y;
   RvR_fix22 z;
}RvR_vec3;

RvR_fix22 RvR_fix22_from_int(int a);
int       RvR_fix22_to_int(RvR_fix22 a);
RvR_fix22 RvR_fix22_mul(RvR_fix22 a, RvR_fix22 b);
RvR_fix22 RvR_fix22_div(RvR_fix22 a, RvR_fix22 b);
RvR_fix22 RvR_fix22_cos(RvR_fix22 a);
RvR_fix22 RvR_fix22_sin(RvR_fix22 a);
RvR_fix22 RvR_fix22_tan(RvR_fix22 a);
RvR_fix22 RvR_fix22_ctg(RvR_fix22 a);
RvR_fix22 RvR_fix22_sqrt(RvR_fix22 a);

RvR_vec2  RvR_vec2_rot(RvR_fix22 angle);
RvR_fix22 RvR_dist2(RvR_vec2 p0, RvR_vec2 p1);
RvR_fix22 RvR_len2(RvR_vec2 v);

int32_t RvR_div_round_down(int32_t a, int32_t b);
int32_t RvR_abs(int32_t a);
int32_t RvR_wrap(int32_t a, int32_t mod);
int32_t RvR_clamp(int32_t a, int32_t min, int32_t max);

#define RvR_non_zero(a) ((a)+((a)==0)) ///< To prevent zero divisions.
#define RvR_zero_clamp(x) ((x)*((x)>=0))
#define RvR_min(a,b) ((a)<(b)?(a):(b))
#define RvR_max(a,b) ((a)>(b)?(a):(b))

#endif
