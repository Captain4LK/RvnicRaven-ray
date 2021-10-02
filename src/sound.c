/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdlib.h>
#include <SDL2/SDL_mixer.h>
//-------------------------------------

//Internal includes
#include "RvR_core.h"
#include "RvR_config.h"
#include "RvR_error.h"
#include "sound.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables

Mix_Chunk *sounds[SOUND_MAX];
int sound_channel[SOUND_MAX] = {-1};
Mix_Music *music;
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

void sound_init()
{
   if(Mix_OpenAudio(22050,MIX_DEFAULT_FORMAT,2,2048 )<0)
        printf("Error: SDL_mixer failed to initialize! SDL_mixer Error: %s\n",Mix_GetError());

   sounds[SOUND_PISTOL] = Mix_LoadWAV("data/dspistol.wav");
   sounds[SOUND_SWITCH] = Mix_LoadWAV("data/dsswtchn.wav");
   sounds[SOUND_STOP] = Mix_LoadWAV("data/dspstop.wav");
   sounds[SOUND_DSG] = Mix_LoadWAV("data/dsdshtgn.wav");
   sounds[SOUND_DSG_OPEN] = Mix_LoadWAV("data/dsdbopn.wav");
   sounds[SOUND_DSG_LOAD] = Mix_LoadWAV("data/dsdbload.wav");
   sounds[SOUND_DSG_CLOSE] = Mix_LoadWAV("data/dsdbcls.wav");

   music = Mix_LoadMUS("data/WeltHerrschererTheme1.ogg");
}

void sound_play(Sound sound, int once)
{
   if(sound==SOUND_MAX)
      return;

   if(once)
   {
      if(sound_channel[sound]==-1||Mix_GetChunk(sound_channel[sound])!=sounds[sound]||!Mix_Playing(sound_channel[sound]))
      {
         sound_channel[sound] = -1;
         sound_channel[sound] = Mix_PlayChannel(-1,sounds[sound],0);
      }
   }
   else
   {
      sound_channel[sound] = -1;
      sound_channel[sound] = Mix_PlayChannel(-1,sounds[sound],0);
   }
}

void music_play()
{
   Mix_PlayMusic(music,-1);
}
//-------------------------------------
