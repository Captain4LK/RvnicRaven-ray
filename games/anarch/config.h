/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _CONFIG_H_

#define _CONFIG_H_

extern int config_mouse_sensitivity;
extern int config_mouse_sensitivity_vertical;
extern RvR_key config_move_forward;
extern RvR_key config_move_backward;
extern RvR_key config_strafe_left;
extern RvR_key config_strafe_right;
extern RvR_key config_enable_freelook;
extern RvR_key config_jump;
extern int config_camera_max_shear;
extern int config_camera_shear_step;

#define MESSAGE_MAX 5
#define MESSAGE_TIMEOUT (RVR_FPS*4)

#define CAMERA_SHEAR_SPEED 3
#define CAMERA_SHEAR_MAX 512

#define CAMERA_COLL_RADIUS 256
#define CAMERA_COLL_HEIGHT_BELOW 800
#define CAMERA_COLL_HEIGHT_ABOVE 200
#define CAMERA_COLL_STEP_HEIGHT (1024/4)

#endif
