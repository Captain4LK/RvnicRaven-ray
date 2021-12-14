/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _EDITOR_H_

#define _EDITOR_H_

typedef struct 
{
   RvR_vec3 pos;
   RvR_fix22 direction;
   int16_t shear;
   RvR_fix22 vertical_speed;
}Camera;
extern Camera camera;

void editor_init();
void editor_update();
void editor_draw();
void editor_undo();
void editor_redo();
void camera_update();

void editor_undo_reset();

//Editing
//Height
void editor_ed_floor(int16_t x, int16_t y, int fac);
void editor_ed_ceiling(int16_t x, int16_t y, int fac);
void editor_ed_flood_floor(int16_t x, int16_t y, int fac);
void editor_ed_flood_ceiling(int16_t x, int16_t y, int fac);

//Textures
void editor_ed_floor_tex(int16_t x, int16_t y, uint16_t tex);
void editor_ed_ceiling_tex(int16_t x, int16_t y, uint16_t tex);
void editor_ed_floor_wall_tex(int16_t x, int16_t y, uint16_t tex);
void editor_ed_ceiling_wall_tex(int16_t x, int16_t y, uint16_t tex);

void editor_ed_flood_floor_tex(int16_t x, int16_t y, uint16_t tex);
void editor_ed_flood_ceiling_tex(int16_t x, int16_t y, uint16_t tex);
void editor_ed_flood_floor_wall_tex(int16_t x, int16_t y, uint16_t tex);
void editor_ed_flood_ceiling_wall_tex(int16_t x, int16_t y, uint16_t tex);

#endif
