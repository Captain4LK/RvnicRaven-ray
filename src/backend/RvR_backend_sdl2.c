/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
//-------------------------------------

//Internal includes
#include "../RvnicRaven.h"
//-------------------------------------

//#defines
#define MAX_CONTROLLERS 4
//-------------------------------------

//Typedefs
typedef struct
{
   SDL_GameController *gamepad;
   SDL_Haptic *haptic;
   SDL_JoystickID id;
   int connected;
   uint8_t new_button_state[RVR_PAD_MAX];
   uint8_t old_button_state[RVR_PAD_MAX];
}Gamepad;
//-------------------------------------

//Variables
static SDL_Window *sdl_window;
static SDL_Renderer *renderer;
static SDL_Texture *layer_texture;
static int pixel_scale;
static int window_width;
static int window_height;
static int view_x;
static int view_y;
static int view_width;
static int view_height;
static int fps;
static int frametime;
static int framedelay;
static int framestart;
static float delta;
static uint8_t key_map[SDL_NUM_SCANCODES];
static uint8_t mouse_map[6];
static uint8_t gamepad_map[SDL_CONTROLLER_BUTTON_MAX];
static uint8_t new_key_state[RVR_KEY_MAX];
static uint8_t old_key_state[RVR_KEY_MAX];
static uint8_t new_mouse_state[RVR_BUTTON_MAX];
static uint8_t old_mouse_state[RVR_BUTTON_MAX];
static Gamepad gamepads[MAX_CONTROLLERS];
static int mouse_x_rel;
static int mouse_y_rel;
static char *text_input;
static int text_input_active;
static unsigned text_input_max;
static int mouse_x;
static int mouse_y;
static int mouse_wheel;
static int key_repeat = 0;

static uint8_t *framebuffer = NULL;
//-------------------------------------

//Function prototypes
static void backend_update_viewport();
static int get_gamepad_index(int which);
//-------------------------------------

//Function implementations

void RvR_backend_init(const char *title, int scale)
{
   pixel_scale = scale;

   Uint32 flags = 
#ifndef __EMSCRIPTEN__
   SDL_INIT_VIDEO|SDL_INIT_EVENTS;
#else
   SDL_INIT_VIDEO|SDL_INIT_EVENTS;
#endif

   if(SDL_Init(flags)<0)
   {
      RvR_log("RvR_backend_sdl2: failed to init sdl2: %s",SDL_GetError());
      exit(-1);
   }

   if(pixel_scale==0)
   {
      SDL_Rect max_size;

      if(SDL_GetDisplayUsableBounds(0,&max_size)<0)
      {
         RvR_log("RvR_backend_sdl2: failed to get max dimensions: %s",SDL_GetError());
      }
      else
      {
         int max_x,max_y;

         max_x = max_size.w/RVR_XRES;
         max_y = max_size.h/RVR_YRES;

         pixel_scale = (max_x>max_y)?max_y:max_x;
      }
      
   }

   if(pixel_scale<=0)
      pixel_scale = 1;

   sdl_window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,RVR_XRES*pixel_scale,RVR_YRES*pixel_scale,0);

   if(sdl_window==NULL)
   {
      RvR_log("RvR_backend_sdl2: failed to create window: %s",SDL_GetError());
      exit(-1);
   }

   renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
   if(!renderer)
   {
      RvR_log("RvR_backend_sdl2: failed to create renderer: %s",SDL_GetError());
      exit(-1);
   }

   SDL_SetRenderDrawColor(renderer,0,0,0,0);

   layer_texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STREAMING,RVR_XRES,RVR_YRES);
   if(layer_texture==NULL)
      RvR_log("RvR_backend_sdl2: failed to create texture for framebuffer %s",SDL_GetError());

   if(SDL_SetTextureBlendMode(layer_texture,SDL_BLENDMODE_BLEND)<0)
      RvR_log("RvR_backend_sdl2: failed to set texture blend mode: %s",SDL_GetError());

   backend_update_viewport();

   key_map[0x00] = RVR_KEY_NONE;
   key_map[SDL_SCANCODE_A] = RVR_KEY_A;
   key_map[SDL_SCANCODE_B] = RVR_KEY_B;
   key_map[SDL_SCANCODE_C] = RVR_KEY_C;
   key_map[SDL_SCANCODE_D] = RVR_KEY_D;
   key_map[SDL_SCANCODE_E] = RVR_KEY_E;
   key_map[SDL_SCANCODE_F] = RVR_KEY_F;
   key_map[SDL_SCANCODE_G] = RVR_KEY_G;
   key_map[SDL_SCANCODE_H] = RVR_KEY_H;
   key_map[SDL_SCANCODE_I] = RVR_KEY_I;
   key_map[SDL_SCANCODE_J] = RVR_KEY_J;
   key_map[SDL_SCANCODE_K] = RVR_KEY_K;
   key_map[SDL_SCANCODE_L] = RVR_KEY_L;
   key_map[SDL_SCANCODE_M] = RVR_KEY_M;
   key_map[SDL_SCANCODE_N] = RVR_KEY_N;
   key_map[SDL_SCANCODE_O] = RVR_KEY_O;
   key_map[SDL_SCANCODE_P] = RVR_KEY_P;
   key_map[SDL_SCANCODE_Q] = RVR_KEY_Q;
   key_map[SDL_SCANCODE_R] = RVR_KEY_R;
   key_map[SDL_SCANCODE_S] = RVR_KEY_S;
   key_map[SDL_SCANCODE_T] = RVR_KEY_T;
   key_map[SDL_SCANCODE_U] = RVR_KEY_U;
   key_map[SDL_SCANCODE_V] = RVR_KEY_V;
   key_map[SDL_SCANCODE_W] = RVR_KEY_W;
   key_map[SDL_SCANCODE_X] = RVR_KEY_X;
   key_map[SDL_SCANCODE_Y] = RVR_KEY_Y;
   key_map[SDL_SCANCODE_Z] = RVR_KEY_Z;

   key_map[SDL_SCANCODE_F1] = RVR_KEY_F1;
   key_map[SDL_SCANCODE_F2] = RVR_KEY_F2;
   key_map[SDL_SCANCODE_F3] = RVR_KEY_F3;
   key_map[SDL_SCANCODE_F4] = RVR_KEY_F4;
   key_map[SDL_SCANCODE_F5] = RVR_KEY_F5;
   key_map[SDL_SCANCODE_F6] = RVR_KEY_F6;
   key_map[SDL_SCANCODE_F7] = RVR_KEY_F7;
   key_map[SDL_SCANCODE_F8] = RVR_KEY_F8;
   key_map[SDL_SCANCODE_F9] = RVR_KEY_F9;
   key_map[SDL_SCANCODE_F10] = RVR_KEY_F10;
   key_map[SDL_SCANCODE_F11] = RVR_KEY_F11;
   key_map[SDL_SCANCODE_F12] = RVR_KEY_F12;

   key_map[SDL_SCANCODE_DOWN] = RVR_KEY_DOWN;
   key_map[SDL_SCANCODE_LEFT] = RVR_KEY_LEFT;
   key_map[SDL_SCANCODE_RIGHT] = RVR_KEY_RIGHT;
   key_map[SDL_SCANCODE_UP] = RVR_KEY_UP;
   key_map[SDL_SCANCODE_RETURN] = RVR_KEY_ENTER;

   key_map[SDL_SCANCODE_BACKSPACE] = RVR_KEY_BACK;
   key_map[SDL_SCANCODE_ESCAPE] = RVR_KEY_ESCAPE;
   key_map[SDL_SCANCODE_TAB] = RVR_KEY_TAB;
   key_map[SDL_SCANCODE_LGUI] = RVR_KEY_HOME;
   key_map[SDL_SCANCODE_END] = RVR_KEY_END;
   key_map[SDL_SCANCODE_PAGEUP] = RVR_KEY_PGUP;
   key_map[SDL_SCANCODE_PAGEDOWN] = RVR_KEY_PGDN;
   key_map[SDL_SCANCODE_INSERT] = RVR_KEY_INS;
   key_map[SDL_SCANCODE_LSHIFT] = RVR_KEY_LSHIFT;
   key_map[SDL_SCANCODE_RSHIFT] = RVR_KEY_RSHIFT;
   key_map[SDL_SCANCODE_LCTRL] = RVR_KEY_LCTRL;
   key_map[SDL_SCANCODE_RCTRL] = RVR_KEY_RCTRL;
   key_map[SDL_SCANCODE_RALT] = RVR_KEY_RALT;
   key_map[SDL_SCANCODE_LALT] = RVR_KEY_LALT;
   key_map[SDL_SCANCODE_SPACE] = RVR_KEY_SPACE;

   key_map[SDL_SCANCODE_0] = RVR_KEY_0;
   key_map[SDL_SCANCODE_1] = RVR_KEY_1;
   key_map[SDL_SCANCODE_2] = RVR_KEY_2;
   key_map[SDL_SCANCODE_3] = RVR_KEY_3;
   key_map[SDL_SCANCODE_4] = RVR_KEY_4;
   key_map[SDL_SCANCODE_5] = RVR_KEY_5;
   key_map[SDL_SCANCODE_6] = RVR_KEY_6;
   key_map[SDL_SCANCODE_7] = RVR_KEY_7;
   key_map[SDL_SCANCODE_8] = RVR_KEY_8;
   key_map[SDL_SCANCODE_9] = RVR_KEY_9;

   key_map[SDL_SCANCODE_KP_0] = RVR_KEY_NP0;
   key_map[SDL_SCANCODE_KP_1] = RVR_KEY_NP1;
   key_map[SDL_SCANCODE_KP_2] = RVR_KEY_NP2;
   key_map[SDL_SCANCODE_KP_3] = RVR_KEY_NP3;
   key_map[SDL_SCANCODE_KP_4] = RVR_KEY_NP4;
   key_map[SDL_SCANCODE_KP_5] = RVR_KEY_NP5;
   key_map[SDL_SCANCODE_KP_6] = RVR_KEY_NP6;
   key_map[SDL_SCANCODE_KP_7] = RVR_KEY_NP7;
   key_map[SDL_SCANCODE_KP_8] = RVR_KEY_NP8;
   key_map[SDL_SCANCODE_KP_9] = RVR_KEY_NP9;
   key_map[SDL_SCANCODE_KP_MULTIPLY] = RVR_KEY_NP_MUL;
   key_map[SDL_SCANCODE_KP_PLUS] = RVR_KEY_NP_ADD;
   key_map[SDL_SCANCODE_KP_DIVIDE] = RVR_KEY_NP_DIV;
   key_map[SDL_SCANCODE_KP_MINUS] = RVR_KEY_NP_SUB;
   key_map[SDL_SCANCODE_KP_PERIOD] = RVR_KEY_NP_DECIMAL;

   mouse_map[SDL_BUTTON_LEFT] = RVR_BUTTON_LEFT;
   mouse_map[SDL_BUTTON_RIGHT] = RVR_BUTTON_RIGHT;
   mouse_map[SDL_BUTTON_MIDDLE] = RVR_BUTTON_MIDDLE;
   mouse_map[SDL_BUTTON_X1] = RVR_BUTTON_X1;
   mouse_map[SDL_BUTTON_X2] = RVR_BUTTON_X2;

   gamepad_map[SDL_CONTROLLER_BUTTON_A] = RVR_PAD_A;
   gamepad_map[SDL_CONTROLLER_BUTTON_B] = RVR_PAD_B;
   gamepad_map[SDL_CONTROLLER_BUTTON_X] = RVR_PAD_X;
   gamepad_map[SDL_CONTROLLER_BUTTON_Y] = RVR_PAD_Y;
   gamepad_map[SDL_CONTROLLER_BUTTON_BACK] = RVR_PAD_BACK;
   gamepad_map[SDL_CONTROLLER_BUTTON_GUIDE] = RVR_PAD_GUIDE;
   gamepad_map[SDL_CONTROLLER_BUTTON_START] = RVR_PAD_START;
   gamepad_map[SDL_CONTROLLER_BUTTON_LEFTSTICK] = RVR_PAD_LEFTSTICK;
   gamepad_map[SDL_CONTROLLER_BUTTON_RIGHTSTICK] = RVR_PAD_RIGHTSTICK;
   gamepad_map[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = RVR_PAD_LEFTSHOULDER;
   gamepad_map[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = RVR_PAD_RIGHTSHOULDER;
   gamepad_map[SDL_CONTROLLER_BUTTON_DPAD_UP] = RVR_PAD_UP;
   gamepad_map[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = RVR_PAD_DOWN;
   gamepad_map[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = RVR_PAD_LEFT;
   gamepad_map[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = RVR_PAD_RIGHT;

   //Clear key states, just in case,
   //should already be empty since known at compile time
   memset(new_key_state,0,sizeof(new_key_state));
   memset(old_key_state,0,sizeof(old_key_state));
   memset(new_mouse_state,0,sizeof(new_mouse_state));
   memset(old_mouse_state,0,sizeof(old_mouse_state));
   for(int i = 0;i<MAX_CONTROLLERS;i++)
   {
      memset(gamepads[i].new_button_state,0,sizeof(gamepads[i].new_button_state));
      memset(gamepads[i].old_button_state,0,sizeof(gamepads[i].old_button_state));
   }

   if(RVR_FPS<1||RVR_FPS>1000)
      fps = 1000;
   else
      fps = RVR_FPS;
   framedelay = 1000/fps;

   framebuffer = RvR_malloc(RVR_XRES*RVR_YRES);
   memset(framebuffer,0,RVR_XRES*RVR_YRES);
}

void RvR_backend_update()
{
   frametime = SDL_GetTicks()-framestart;

   if(framedelay>frametime)
      SDL_Delay(framedelay-frametime);

   delta = (float)(SDL_GetTicks()-framestart)/1000.0f;
   framestart = SDL_GetTicks();

   mouse_wheel = 0;
   memcpy(old_key_state,new_key_state,sizeof(new_key_state));
   memcpy(old_mouse_state,new_mouse_state,sizeof(new_mouse_state));
   for(int i = 0;i<MAX_CONTROLLERS;i++)
      memcpy(gamepads[i].old_button_state,gamepads[i].new_button_state,sizeof(gamepads[0].new_button_state));

   //Event managing
   SDL_Event event;
   while(SDL_PollEvent(&event))
   {
      switch(event.type)
      {
      case SDL_QUIT:
         RvR_core_quit();
         break;
      case SDL_KEYDOWN:
         if(text_input_active&&event.key.keysym.sym==SDLK_BACKSPACE&&text_input[0]!='\0')
            text_input[strlen(text_input)-1] = '\0';
         if(event.key.state==SDL_PRESSED)
         {
            new_key_state[key_map[event.key.keysym.scancode]] = 1;
            if(event.key.repeat&&key_repeat)
               old_key_state[key_map[event.key.keysym.scancode]] = 0;
         }
         break;
      case SDL_KEYUP:
         if(event.key.state==SDL_RELEASED)
            new_key_state[key_map[event.key.keysym.scancode]] = 0;
         break;
      case SDL_MOUSEBUTTONDOWN:
         if(event.button.state==SDL_PRESSED)
            new_mouse_state[mouse_map[event.button.button]] = 1;
         break;
      case SDL_MOUSEBUTTONUP:
         if(event.button.state==SDL_RELEASED)
            new_mouse_state[mouse_map[event.button.button]] = 0;
         break;       
      case SDL_TEXTINPUT:
         if(text_input_active&&strlen(text_input)+strlen(event.text.text)<text_input_max)
            strcat(text_input,event.text.text);
         break;
      case SDL_MOUSEWHEEL:
         mouse_wheel = event.wheel.y;
         break;
      case SDL_CONTROLLERBUTTONDOWN:
         if(event.cbutton.state==SDL_PRESSED)
         {
            int id = get_gamepad_index(event.cbutton.which);
            gamepads[id].new_button_state[gamepad_map[event.cbutton.button]] = 1;
         }
         break;
      case SDL_CONTROLLERBUTTONUP:
         if(event.cbutton.state==SDL_RELEASED)
         {
            int id = get_gamepad_index(event.cbutton.which);
            gamepads[id].new_button_state[gamepad_map[event.cbutton.button]] = 0;
         }
         break;
      case SDL_CONTROLLERDEVICEADDED:
         {
            int which = event.cdevice.which;
            if(which<MAX_CONTROLLERS)
            {
               gamepads[which].gamepad = SDL_GameControllerOpen(which);
               gamepads[which].connected = 1;
               SDL_Joystick *j = SDL_GameControllerGetJoystick(gamepads[which].gamepad);
               gamepads[which].id = SDL_JoystickInstanceID(j);
            }
         }
         break;
      case SDL_CONTROLLERDEVICEREMOVED:
         {
            int which = event.cdevice.which;
            if(which<0)
               break;
            int id = get_gamepad_index(which);
            gamepads[id].connected = 0;
            SDL_GameControllerClose(gamepads[id].gamepad);
         }
         break;
      case SDL_WINDOWEVENT:
         backend_update_viewport();
         break;
      }
   }
   //-------------------------------------------
   
   int x,y;
   SDL_GetMouseState(&x,&y);

   x-=view_x;
   y-=view_y;
   mouse_x = x/pixel_scale;
   mouse_y = y/pixel_scale;

   SDL_GetRelativeMouseState(&mouse_x_rel,&mouse_y_rel);
   mouse_x_rel = mouse_x_rel/pixel_scale;
   mouse_y_rel = mouse_y_rel/pixel_scale;

   if(mouse_x>=RVR_XRES)
     mouse_x = RVR_XRES-1;
   if(mouse_y>=RVR_YRES)
     mouse_y = RVR_YRES-1;

   if(mouse_x<0)
     mouse_x = 0;
   if(mouse_y<0)
     mouse_y = 0;
}

void RvR_backend_render_present()
{
   SDL_RenderClear(renderer);

   float width = (float)RVR_XRES*pixel_scale;
   float height = (float)RVR_YRES*pixel_scale;
   float x = 0.0f;
   float y = 0.0f;
   SDL_Rect dst_rect;
   dst_rect.x = x;
   dst_rect.y = y;
   dst_rect.w = width;
   dst_rect.h = height;

   void *data;
   int stride;
   SDL_LockTexture(layer_texture,NULL,&data,&stride);
   RvR_color *pix = data;
   for(int i = 0;i<RVR_XRES*RVR_YRES;i++)
      pix[i] = RvR_palette()[framebuffer[i]];
   SDL_UnlockTexture(layer_texture);

   SDL_RenderCopy(renderer,layer_texture,NULL,&dst_rect);

   SDL_RenderPresent(renderer);
}

void RvR_backend_mouse_relative(int relative)
{
   if(SDL_SetRelativeMouseMode(relative)<0)
      RvR_log("RvR_backend_sdl2: failed to set relative mouse mode: %s",SDL_GetError());
}

void RvR_backend_mouse_show(int show)
{
   if(SDL_ShowCursor(show?SDL_ENABLE:SDL_DISABLE)<0)
      RvR_log("RvR_backend_sdl2: failed to show/hide cursor: %s",SDL_GetError());
}

static void backend_update_viewport()
{
   SDL_GetWindowSize(sdl_window,&window_width,&window_height);

   view_width = RVR_XRES*pixel_scale;
   view_height = RVR_YRES*pixel_scale;

   view_x = (window_width-view_width)/2;
   view_y = (window_height-view_height)/2;

   SDL_Rect v;
   v.x = view_x;
   v.y = view_y;
   v.w = view_width;
   v.h = view_height;
   if(SDL_RenderSetViewport(renderer,&v)<0)
      RvR_log("RvR_backend_sdl2: failed to set render viewport: %s",SDL_GetError());
}

static int get_gamepad_index(int which)
{

   for(int i = 0;i<MAX_CONTROLLERS;i++)
      if(gamepads[i].connected&&gamepads[i].id==which)
         return i;

   return -1;
}

int RvR_backend_key_down(int key)
{
   return new_key_state[key];
}

int RvR_backend_key_pressed(int key)
{
   return new_key_state[key]&&!old_key_state[key];
}

int RvR_backend_key_released(int key)
{
   return !new_key_state[key]&&old_key_state[key];
}

int RvR_backend_mouse_down(int key)
{
   return new_mouse_state[key];
}

int RvR_backend_mouse_pressed(int key)
{
   return new_mouse_state[key]&&!old_mouse_state[key];
}

int RvR_backend_mouse_released(int key)
{
   return !new_mouse_state[key]&&old_mouse_state[key];
}

int RvR_backend_mouse_wheel_get_scroll()
{
   return mouse_wheel;
}

int RvR_backend_gamepad_down(int index, int key)
{
   return gamepads[index].new_button_state[key];
}

int RvR_backend_gamepad_pressed(int index, int key)
{
   return gamepads[index].new_button_state[key]&&!gamepads[index].old_button_state[key];
}

int RvR_backend_gamepad_released(int index, int key)
{
   return !gamepads[index].new_button_state[key]&&gamepads[index].old_button_state[key];
}

void RvR_backend_mouse_get_relative_pos(int *x, int *y)
{
   *x = mouse_x_rel;
   *y = mouse_y_rel;
}

void RvR_backend_mouse_get_pos(int *x, int *y)
{
   *x = mouse_x;
   *y = mouse_y;
}

void RvR_backend_mouse_set_pos(int x, int y)
{
   mouse_x = x;
   mouse_y = y;
   x*=pixel_scale;
   y*=pixel_scale;

   SDL_WarpMouseInWindow(sdl_window,x,y);
}

void RvR_backend_text_input_start(char *text, int max_length)
{
   text_input = text;
   text_input_active = 1;
   text_input_max = max_length;

   SDL_StartTextInput();
}

void RvR_backend_text_input_end()
{
   text_input_active = 0;
   
   SDL_StopTextInput();
}

uint8_t *RvR_backend_framebuffer()
{
   return framebuffer;
}

void RvR_backend_key_repeat(int repeat)
{
   key_repeat = repeat;
}
//-------------------------------------
