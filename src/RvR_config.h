/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_CONFIG_H_

#define _RVR_CONFIG_H_

//Window config
#define XRES 320
#define YRES 240
#define TITLE "To be determined"
#define FPS 30

//Math config

//DO NOT CHANGE!
#define RvR_fix22_one (1<<10)
#define RvR_INFINITY 2000000000

#define DITHERING 1
#define RNG_TABLE_SIZE 512

#define GRAVITY 16
#define MAX_VERTICAL_SPEED 256
#define JUMP_SPEED 128

//Graphics and raycasting
#define MAX_STEPS 24
#define MAX_HITS 24
#define HORIZONTAL_FOV 256
#define VERTICAL_FOV 330
#define VERTICAL_DEPTH_MULTIPLY 2
#define HORIZON_DEPTH (11*RvR_fix22_one)
#define SCANLINE_BATCH_SIZE 256
#define ANGLE_STEP (((RvR_fix22_one/4)*RvR_fix22_one)/XRES)
#define SKY_TEX_STEP ((RvR_fix22_one*128-1)/YRES)

//Collision
//TODO: rename (remove CAMERA prefix)
#define CAMERA_COLL_RADIUS 256
#define CAMERA_COLL_HEIGHT_BELOW 800
#define CAMERA_COLL_STEP_HEIGHT (RvR_fix22_one/4)
#define CAMERA_COLL_HEIGHT_ABOVE 200

//TODO: affect depth buffer size
//currently needs to be changed manually in RvR_raycast_draw.c
#define DEPTH_BUFFER_PRECISION 512

extern int RvR_config_mouse_sensitivity;
extern int RvR_config_mouse_sensitivity_vertical;
extern SLK_key RvR_config_move_forward;
extern SLK_key RvR_config_move_backward;
extern SLK_key RvR_config_strafe_left;
extern SLK_key RvR_config_strafe_right;
extern SLK_key RvR_config_enable_freelook;
extern SLK_key RvR_config_jump;
extern unsigned RvR_config_texture_timeout;
extern int RvR_config_camera_max_shear;
extern int RvR_config_camera_shear_step;

//Read config variables from ini file
//Returns 0 on success or 1 on failure
//Possible errors:
//    RVR_ERROR_ARG_NULL
//    RVR_ERROR_FAIL_MALLOC
//    RVR_ERROR_FAIL_REALLOC
//    RVR_ERROR_FAIL_FOPEN
//    RVR_ERROR_FAIL_FCLOSE
//    RVR_ERROR_FAIL_FSEEK
//    RVR_ERROR_FAIL_FTELL
//    RVR_ERROR_FAIL_FREAD
//
//Parameters:
//    const char *path - path to ini file
//                       path != NULL
int RvR_ini_parse(const char *path);

//Write config variables to file
//Returns 0 on success or 1 on failure
//Possible errors:
//    RVR_ERROR_ARG_NULL
//    RVR_ERROR_FAIL_FOPEN
//    RVR_ERROR_FAIL_FCLOSE
//    RVR_ERROR_FAIL_FPRINTF
//
//Parameters:
//    const char *path - path to write ini to
//                       path != NULL
int RvR_ini_write(const char *path);

#endif
