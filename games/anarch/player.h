/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _PLAYER_H_

#define _PLAYER_H_

typedef struct
{
   AI_ent *entity;
   int16_t shear;
   RvR_fix22 vertical_speed;
   uint8_t key;
   uint8_t key_needed;

   int32_t ammo_bull;
   int32_t ammo_rckt;
   int32_t ammo_cell;
   int32_t weapon;
}Player;

extern Player player;

void player_update();
int32_t player_weapon_ammo(int32_t weapon);
void player_weapon_rotate(int direction);
void player_weapon_switch(int32_t weapon);

#endif
