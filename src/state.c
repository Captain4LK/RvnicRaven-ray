/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <SLK/SLK.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR_math.h"
#include "RvR_raycast.h"
#include "RvR_raycast_draw.h"
#include "RvR_texture.h"
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

   //Graphics
   SLK_layer_set_current(0);
   SLK_draw_pal_sprite(RvR_texture_get(0x4100),0,0);
   SLK_draw_pal_string((XRES-200)/2,YRES/6,5,"Title",8);
   SLK_draw_pal_string((XRES-80)/2,140,2,"Start",8);
   SLK_draw_pal_string((XRES-112)/2,YRES/2+YRES/12,2,"Credits",8);
   SLK_draw_pal_string((XRES-64)/2,YRES/2+YRES/6,2,"Quit",8);
   switch(selected)
   {
   case 0: SLK_draw_pal_string((XRES-112)/2,YRES/2,2,">     <",78); break;
   case 1: SLK_draw_pal_string((XRES-144)/2,YRES/2+YRES/12,2,">       <",78); break;
   case 2: SLK_draw_pal_string((XRES-96)/2,YRES/2+YRES/6,2,">    <",78); break;
   }

   //Input
   if(SLK_key_pressed(SLK_KEY_DOWN)&&selected<2)
   {
      selected++;
      sound_play(SOUND_STOP,0);
   }
   if(SLK_key_pressed(SLK_KEY_UP)&&selected>0)
   {
      selected--;
      sound_play(SOUND_STOP,0);
   }
   if(SLK_key_pressed(SLK_KEY_ENTER))
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
      case 2:
         SLK_core_quit();
         //sound_play(SOUND_PISTOL);
         break;
      }
   }
}

static void mode_credits()
{
   //Graphics
   SLK_layer_set_current(0);
   SLK_draw_pal_sprite(RvR_texture_get(0x4100),0,0);
   SLK_draw_pal_string((XRES-280)/2,YRES/8,5,"Credits",78);
   SLK_draw_pal_string((XRES-560)/2,YRES/3,1,"H. Lukas Holzbeierlein (Captain4LK)",78);

   //Input
   if(SLK_key_pressed(SLK_KEY_ENTER)||SLK_key_pressed(SLK_KEY_ESCAPE))
   {
      mode = STATE_TITLE;
      sound_play(SOUND_SWITCH,0);
   }
}

static void mode_epsel()
{
   static int selected = 0;

   //Graphics
   SLK_layer_set_current(0);
   SLK_draw_pal_clear();

   //Input
   if(SLK_key_pressed(SLK_KEY_ENTER))
   {
      set_episode(selected);
      mode = STATE_DIFFSEL;
   }
}

static void mode_diffsel()
{
   static int selected = 0;

   //Graphics
   SLK_layer_set_current(0);
   SLK_draw_pal_clear();

   //Input
   if(SLK_key_pressed(SLK_KEY_ENTER))
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
