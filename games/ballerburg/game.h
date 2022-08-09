/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _GAME_H_

#define _GAME_H_

typedef struct
{
   int burg;
   int burg_y;

   int geld;
   int pulver;
   int kugeln;
   int volk;
   int konig;
   int steuern;

   struct
   {
      int x;
      int y;
      int winkel;
      int pulver;
   }kanonen[10];

   int fahne[2];
   struct
   {
      int x;
      int y;
   }forderturme[5];

   char name[64];
}Player;

void game_new(void);
void game_update(void);
void game_draw(void);
int ein_zug(void);

extern int zug;
extern int wind;
extern Player players[2];
extern uint8_t buffer_game[RVR_XRES*RVR_YRES];

#endif
