/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _SOUND_H_

#define _SOUND_H_

typedef enum
{
   SOUND_PISTOL,
   SOUND_SWITCH,
   SOUND_STOP,
   SOUND_DSG,
   SOUND_DSG_OPEN,
   SOUND_DSG_LOAD,
   SOUND_DSG_CLOSE,
   SOUND_MAX,
}Sound;


void sound_init();
void sound_play(Sound sound, int once);
void music_play();

#endif
