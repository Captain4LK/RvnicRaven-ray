## Version 0
----------------------

From: 23.03.2022

To: 

|Data type|Name|Description|
|---|---|---|
|uint16_t|version|Map format version|
|uint16_t|sky_texture|Sky texture index|
|uint16_t|width|Map width|
|uint16_t|height|Map height|
|uint32_t|sprite_count|Number of sprites in map|
|int32_t[width*height]|floor_height|Floor height array|
|int32_t[width*height]|ceiling_height|Ceiling height array|
|int16_t[width*height]|floor_texture|Floor texture array|
|int16_t[width*height]|ceiling_texture|Ceiling texture array|
|int16_t[width*height]|wall_ftexture|Floor wall texture|
|int16_t[width*height]|wall_ctexture|Ceiling wall texture|
|Sprite[sprite_count]|sprites|Array of sprites in map|

### Sprite

|Data type|Name|Description|
|---|---|---|
|int32_t|x|x coordinate of sprite|
|int32_t|y|y coordinate of sprite|
|int32_t|z|z coordinate of sprite|
|int32_t|angle|sprite angle|
|uint16_t|texture|sprite texture|
|uint32_t|flags|See Sprite flags|
|int32_t|extra0|sprite specific|
|int32_t|extra1|sprite specific|
|int32_t|extra2|sprite specific|


### Sprite flags

Meaning describes behaviour if bit is set to 1

|Bit|Meaning|
|---|---|
|0|Invisible|
|1|flipped on y-axis|
|2|flipped on x-axis|
|3|wall-aligned sprite|
|4|floor aligned sprite|
|5|66% Transparent|
|6|33% Transparent|