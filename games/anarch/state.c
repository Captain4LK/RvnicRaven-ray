/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
//-------------------------------------

//Internal includes
#include "../../src/RvnicRaven.h"
#include "state.h"
#include "game.h"
#include "sprite.h"
#include "ai.h"
#include "player.h"
#include "sound.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef enum
{
   STATE_TITLE, STATE_CREDITS, STATE_EPSEL, STATE_DIFFSEL, STATE_GAME,
}Game_state;
//-------------------------------------

//Variables
static Game_state mode = 0;
//-------------------------------------

//Function prototypes
static void mode_title();
static void mode_credits();
static void mode_epsel();
static void mode_diffsel();
static void mode_game();
//-------------------------------------

//Function implementations

void mode_update()
{
   switch(mode)
   {
   case STATE_TITLE: mode_title(); break;
   case STATE_CREDITS: mode_credits(); break;
   case STATE_EPSEL: mode_epsel(); break;
   case STATE_DIFFSEL: mode_diffsel(); break;
   case STATE_GAME: mode_game(); break;
   }
}

static void mode_title()
{
   static int selected = 0;
   static int scroll = 0;
   scroll = (scroll+1)&63;

   //Graphics
   RvR_texture *tex = RvR_texture_get(0x4100);
   RvR_draw_clear(1);
   for(int i = 0;i<(RVR_XRES/64)+1;i++)
      RvR_draw_texture(tex,i*64-scroll,0);
   tex = RvR_texture_get(0x4101);
   RvR_draw_texture2(tex,(RVR_XRES-tex->width*2)/2,6);

   RvR_draw_rectangle_fill((RVR_XRES-tex->width*2)/2,RVR_YRES/2+(RVR_YRES/20)*(selected+2)-1,tex->width*2,9,3);
   RvR_draw_string((RVR_XRES-40)/2,RVR_YRES/2+(RVR_YRES/20)*2,2,"Play",selected==0?8:24);
   RvR_draw_string((RVR_XRES-40)/2,RVR_YRES/2+(RVR_YRES/20)*3,2,"Load",selected==1?8:24);
   RvR_draw_string((RVR_XRES-50)/2,RVR_YRES/2+(RVR_YRES/20)*4,2,"Sound",selected==2?8:24);
   RvR_draw_string((RVR_XRES-40)/2,RVR_YRES/2+(RVR_YRES/20)*5,2,"Look",selected==3?8:24);
   RvR_draw_string((RVR_XRES-40)/2,RVR_YRES/2+(RVR_YRES/20)*6,2,"Exit",selected==4?8:24);
   RvR_draw_string(8,RVR_YRES-16,2,"1.00",5);

   //Input
   if(RvR_core_key_pressed(RVR_KEY_DOWN)&&selected<4)
   {
      selected++;
      sound_play(SOUND_STOP,0);
   }
   if(RvR_core_key_pressed(RVR_KEY_UP)&&selected>0)
   {
      selected--;
      sound_play(SOUND_STOP,0);
   }
   if(RvR_core_key_pressed(RVR_KEY_ENTER))
   {
      switch(selected)
      {
      case 0: 
         mode = STATE_GAME;
         music_play();
         sound_play(SOUND_SWITCH,0);
         game_map_load();
         break;
      case 1:
         mode = STATE_CREDITS;
         sound_play(SOUND_SWITCH,0);
         break;
      case 4:
         RvR_core_quit();
         //sound_play(SOUND_PISTOL);
         break;
      }
   }
   if(RvR_core_key_pressed(RVR_KEY_M))
      RvR_malloc_report();
}

static void mode_credits()
{
   //Graphics
   RvR_draw_texture(RvR_texture_get(0x4100),0,0);
   RvR_draw_string((RVR_XRES-280)/2,RVR_YRES/8,5,"Credits",78);
   RvR_draw_string((RVR_XRES-560)/2,RVR_YRES/3,1,"H. Lukas Holzbeierlein (Captain4LK)",78);

   //Input
   if(RvR_core_key_pressed(RVR_KEY_ENTER)||RvR_core_key_pressed(RVR_KEY_ESCAPE))
   {
      mode = STATE_TITLE;
      sound_play(SOUND_SWITCH,0);
   }
}

static void mode_epsel()
{
   static int selected = 0;

   //Graphics

   //Input
   if(RvR_core_key_pressed(RVR_KEY_ENTER))
   {
      set_episode(selected);
      mode = STATE_DIFFSEL;
   }
}

static void mode_diffsel()
{
   static int selected = 0;

   //Graphics

   //Input
   if(RvR_core_key_pressed(RVR_KEY_ENTER))
   {
      set_difficulty(selected);
      game_map_load();
      mode = STATE_GAME;      
   }
}

static void mode_game()
{
   //Game logic
   game_update();
}
//-------------------------------------
