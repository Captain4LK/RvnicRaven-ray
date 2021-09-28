/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVR_LUMP_H_

#define _RVR_LUMP_H_

//TODO: rename, lowercase v
typedef enum
{
   RVR_LUMP_ERROR, RVR_LUMP_PAL, RVR_LUMP_MUS, RVR_LUMP_JSON, RVR_LUMP_PAK, RVR_LUMP_TEX, RVR_LUMP_WAV, RVR_LUMP_MAP,
}RvR_lump;

void RvR_pak_add(const char *path);
void RvR_pak_flush();
void RvR_lump_add(const char *name, const char *path, RvR_lump type);
void *RvR_lump_get(const char *name, RvR_lump type, unsigned *size);
const char *RvR_lump_get_path(const char *name, RvR_lump type);

#endif
