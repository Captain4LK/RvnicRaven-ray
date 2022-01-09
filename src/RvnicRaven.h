/*
RvnicRaven retro game engine

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVNICRAVEN_H_

#define _RVNICRAVEN_H_

//Config

//Core
//Framebuffer resolution
#define RVR_XRES 320
#define RVR_YRES 240

//Fps, RvnicRaven must use a fixed timestep
#define RVR_FPS 30

//Size of precalculated rng table
#define RVR_RNG_TABLE_SIZE 512

//Amount of load cycles a unused texture will stay loaded
#define RVR_TEXTURE_TIMEOUT 2

//Unroll some loops
#define RVR_UNROLL 1

//0 for little, 1 for big endian machine
#define RVR_ENDIAN 0
//-------------------------------------

//Constants
#define RvR_fix22_infinity 2000000000
//-------------------------------------

//RvnicRaven raycasting

//Basically render distance
#define RVR_RAY_MAX_STEPS 32

//Field of view, 360deg is 1024
#define RVR_RAY_HORIZONTAL_FOV 256
#define RVR_RAY_VERTICAL_FOV 330

//Texture resolution for walls/ceilings/floors, calculated as 1<<RVR_RAY_TEXTURE
#define RVR_RAY_TEXTURE 6

//Alternative texture resolution for textures higher than 1 block (8 units), calculated as 1<<RVR_RAY_TEXTURE_HIGH
#define RVR_RAY_TEXTURE_HIGH 7

//Width of sky texture, calculated as 1<<RVR_RAY_TEXTURE_SKY_W
#define RVR_RAY_TEXTURE_SKY_W 8

//HEIGHT of sky texture, calculated as 1<<RVR_RAY_TEXTURE_SKY_H
#define RVR_RAY_TEXTURE_SKY_H 7
//-------------------------------------

//Config end
//-------------------------------------

//Collision
//TODO: rename (remove CAMERA prefix)
#define CAMERA_COLL_RADIUS 256
#define CAMERA_COLL_HEIGHT_BELOW 800
#define CAMERA_COLL_STEP_HEIGHT (1024/4)
#define CAMERA_COLL_HEIGHT_ABOVE 200

//RvnicRaven core types

typedef enum 
{
   RVR_KEY_NONE,
   RVR_KEY_A,RVR_KEY_B,RVR_KEY_C,RVR_KEY_D,RVR_KEY_E,RVR_KEY_F,RVR_KEY_G,
   RVR_KEY_H,RVR_KEY_I,RVR_KEY_J,RVR_KEY_K,RVR_KEY_L,RVR_KEY_M,RVR_KEY_N,
   RVR_KEY_O,RVR_KEY_P,RVR_KEY_Q,RVR_KEY_R,RVR_KEY_S,RVR_KEY_T,RVR_KEY_U,
   RVR_KEY_V,RVR_KEY_W,RVR_KEY_X,RVR_KEY_Y,RVR_KEY_Z,
   RVR_KEY_0,RVR_KEY_1,RVR_KEY_2,RVR_KEY_3,RVR_KEY_4,RVR_KEY_5,
   RVR_KEY_6,RVR_KEY_7,RVR_KEY_8,RVR_KEY_9, RVR_KEY_COMMA, RVR_KEY_PERIOD,
   RVR_KEY_F1,RVR_KEY_F2,RVR_KEY_F3,RVR_KEY_F4,RVR_KEY_F5,RVR_KEY_F6,
   RVR_KEY_F7,RVR_KEY_F8,RVR_KEY_F9,RVR_KEY_F10,RVR_KEY_F11,RVR_KEY_F12,
   RVR_KEY_UP,RVR_KEY_DOWN,RVR_KEY_LEFT,RVR_KEY_RIGHT,
   RVR_KEY_SPACE,RVR_KEY_TAB,RVR_KEY_LSHIFT,RVR_KEY_RSHIFT,RVR_KEY_LCTRL,RVR_KEY_RCTRL,
   RVR_KEY_LALT, RVR_KEY_RALT, RVR_KEY_INS,
   RVR_KEY_DEL,RVR_KEY_HOME,RVR_KEY_END,RVR_KEY_PGUP,RVR_KEY_PGDN,
   RVR_KEY_BACK,RVR_KEY_ESCAPE,RVR_KEY_RETURN,RVR_KEY_ENTER,RVR_KEY_PAUSE,RVR_KEY_SCROLL,
   RVR_KEY_NP0,RVR_KEY_NP1,RVR_KEY_NP2,RVR_KEY_NP3,RVR_KEY_NP4,RVR_KEY_NP5,RVR_KEY_NP6,RVR_KEY_NP7,RVR_KEY_NP8,RVR_KEY_NP9,
   RVR_KEY_NP_MUL,RVR_KEY_NP_DIV,RVR_KEY_NP_ADD,RVR_KEY_NP_SUB,RVR_KEY_NP_DECIMAL, RVR_KEY_NP_ENTER,
   RVR_KEY_MAX,
}RvR_key;

typedef enum 
{
   RVR_PAD_A, RVR_PAD_B, RVR_PAD_X, RVR_PAD_Y, RVR_PAD_BACK, RVR_PAD_GUIDE,
   RVR_PAD_START, RVR_PAD_LEFTSTICK, RVR_PAD_RIGHTSTICK, RVR_PAD_LEFTSHOULDER,
   RVR_PAD_RIGHTSHOULDER, RVR_PAD_UP, RVR_PAD_DOWN, RVR_PAD_LEFT, RVR_PAD_RIGHT,
   RVR_PAD_MAX,
}RvR_gamepad_button;

typedef enum 
{
   RVR_BUTTON_LEFT,RVR_BUTTON_RIGHT,RVR_BUTTON_MIDDLE,RVR_BUTTON_X1,RVR_BUTTON_X2,
   RVR_BUTTON_MAX,
}RvR_mouse_button;

typedef enum
{
   RVR_ERROR_NONE = 0x000,              //No error encountered
   RVR_ERROR_FAIL_MALLOC = 0x001,       //malloc() failed, out of memory
   RVR_ERROR_FAIL_REALLOC = 0x002,      //realloc() failed, out of memory
   RVR_ERROR_FAIL_FWRITE = 0x003,       //fwrite() failed
   RVR_ERROR_FAIL_FSEEK = 0x004,        //fseek() failed
   RVR_ERROR_FAIL_FTELL = 0x005,        //ftell() failed
   RVR_ERROR_FAIL_FOPEN = 0x006,        //fopen() failed
   RVR_ERROR_FAIL_FCLOSE = 0x007,       //fclose() failed
   RVR_ERROR_FAIL_FPRINTF = 0x008,      //fprintf() failed
   RVR_ERROR_FAIL_FREAD = 0x009,        //fread() failed
   RVR_ERROR_ARG_OOR = 0x100,           //argument outside expected range
   RVR_ERROR_ARG_NULL = 0x101,          //argument NULL 
   RVR_ERROR_BUFFER_SHORT = 0x200,      //buffer short
}RvR_error;

typedef enum
{
   RVR_CONFIG_INT, RVR_CONFIG_KEY,
}RvR_config_type;

typedef struct
{
   uint8_t r,g,b,a;
}RvR_color;

typedef struct
{
   int width;
   int height;
   uint8_t *data;
}RvR_texture;

typedef int32_t RvR_fix24;
typedef int32_t RvR_fix22;
typedef int32_t RvR_fix16;
typedef char * RvR_config;

typedef struct
{
   RvR_fix22 x;
   RvR_fix22 y;
}RvR_vec2;

typedef struct
{
   RvR_fix22 x;
   RvR_fix22 y;
   RvR_fix22 z;
}RvR_vec3;

typedef struct
{
   int type;
   uint8_t endian;

   union
   {
      FILE *fp;
      struct
      {
         void *mem;
         size_t size;
         long pos;
      }mem;
      struct
      {
         const void *mem;
         size_t size;
         long pos;
      }cmem;
   }file;
}RvR_rw;

//RvnicRaven core types end
//-------------------------------------

//RvnicRaven core functions

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
RvR_config RvR_ini_parse(const char *path);
void RvR_ini_free(RvR_config config);
void RvR_ini_read(RvR_config config, void *dst, RvR_config_type type, const char *ident);

void RvR_core_quit();
void RvR_core_init(char *title, int scale);
void RvR_core_mouse_relative(int relative);
void RvR_core_mouse_show(int show);
void RvR_core_key_repeat(int repeat);
int RvR_core_running();
void RvR_core_update();
void RvR_core_render_present();

int  RvR_core_key_down(RvR_key key);
int  RvR_core_key_pressed(RvR_key key);
int  RvR_core_key_released(RvR_key key);
int  RvR_core_mouse_down(RvR_mouse_button button);
int  RvR_core_mouse_pressed(RvR_mouse_button button);
int  RvR_core_mouse_released(RvR_mouse_button button);
int  RvR_core_mouse_wheel_scroll();
int  RvR_core_gamepad_down(int index, RvR_gamepad_button button);
int  RvR_core_gamepad_pressed(int index, RvR_gamepad_button button);
int  RvR_core_gamepad_released(int index, RvR_gamepad_button button);
void RvR_core_mouse_relative_pos(int *x, int *y);
void RvR_core_mouse_pos(int *x, int *y);
void RvR_core_mouse_set_pos(int x, int y);
void RvR_core_text_input_start(char *text, int max_length);
void RvR_core_text_input_end();

uint8_t *RvR_core_framebuffer();

void RvR_rw_init_file(RvR_rw *rw, FILE *f);
void RvR_rw_init_path(RvR_rw *rw, const char *path, const char *mode);
void RvR_rw_init_mem(RvR_rw *rw, void *mem, size_t len);
void RvR_rw_init_const_mem(RvR_rw *rw, const void *mem, size_t len);
void RvR_rw_endian(RvR_rw *rw, uint8_t endian);
void RvR_rw_close(RvR_rw *rw);
void RvR_rw_flush(RvR_rw *rw);
int RvR_rw_seek(RvR_rw *rw, long offset, int origin);
long RvR_rw_tell(RvR_rw *rw);
size_t RvR_rw_read(RvR_rw *rw, void *buffer, size_t size, size_t count);
size_t RvR_rw_write(RvR_rw *rw, const void *buffer, size_t size, size_t count);

int8_t   RvR_rw_read_i8 (RvR_rw *rw);
uint8_t  RvR_rw_read_u8 (RvR_rw *rw);
int16_t  RvR_rw_read_i16(RvR_rw *rw);
uint16_t RvR_rw_read_u16(RvR_rw *rw);
int32_t  RvR_rw_read_i32(RvR_rw *rw);
uint32_t RvR_rw_read_u32(RvR_rw *rw);
int64_t  RvR_rw_read_i64(RvR_rw *rw);
uint64_t RvR_rw_read_u64(RvR_rw *rw);

void RvR_rw_write_i8 (RvR_rw *rw, int8_t val);
void RvR_rw_write_u8 (RvR_rw *rw, uint8_t val);
void RvR_rw_write_i16(RvR_rw *rw, int16_t val);
void RvR_rw_write_u16(RvR_rw *rw, uint16_t val);
void RvR_rw_write_i32(RvR_rw *rw, int32_t val);
void RvR_rw_write_u32(RvR_rw *rw, uint32_t val);
void RvR_rw_write_i64(RvR_rw *rw, int64_t val);
void RvR_rw_write_u64(RvR_rw *rw, uint64_t val);

uint16_t RvR_endian_swap16(uint16_t n, uint8_t endian);
uint32_t RvR_endian_swap32(uint32_t n, uint8_t endian);
uint64_t RvR_endian_swap64(uint64_t n, uint8_t endian);

void  RvR_compress(FILE *in, FILE *out);
void  RvR_compress_path(const char *path_in, const char *path_out);
void  RvR_mem_compress(void *mem, int32_t length, FILE *out);
void *RvR_decompress(FILE *in, int32_t *length, uint8_t *endian);
void *RvR_decompress_path(const char *path, int32_t *length, uint8_t *endian);
void *RvR_mem_decompress(void *mem, int32_t length_in, int32_t *length_out, uint8_t *endian);

void RvR_draw_clear(uint8_t index);
void RvR_draw_texture(RvR_texture *t, int x, int y);
void RvR_draw_texture2(RvR_texture *t, int x, int y);
void RvR_draw_rectangle(int x, int y, int width, int height, uint8_t index);
void RvR_draw_rectangle_fill(int x, int y, int width, int height, uint8_t index);
void RvR_draw_circle(int x, int y, int radius, uint8_t index);
void RvR_draw_set_font(RvR_texture *t);
void RvR_draw_string(int x, int y, int scale, const char *text, uint8_t index);
void RvR_draw(int x, int y, uint8_t index); //do not use, access framebuffer directly if possible
void RvR_draw_line(int x0, int y0, int x1, int y1, uint8_t index);
void RvR_draw_vertical_line(int x, int y0, int y1, uint8_t index);
void RvR_draw_horizontal_line(int x0, int x1, int y, uint8_t index);

//Set the current error
//Parameters:
//    const char *file - name of the file the error occurred in
//                       usually set using __FILE__
//                       file != NULL
//    unsigned line - line the error occurred in
//                    usually set using __LINE__
//    unsigned reason - reason for error
//                      integer codes can be read from switch statement
//                      in implementation
void RvR_error_set(const char *file, unsigned line, unsigned reason);

//Clear all errors
void RvR_error_clear();

//Returns the last error as a enum
RvR_error RvR_error_get();

//Returns the last error as a string, 
//returned string will only be valid until next HLH_error_get_string() call
const char *RvR_error_get_string();

void RvR_log(const char *w, ...);

#define RvR_log_line(w,...) do { char RvR_log_line_tmp[512]; snprintf(RvR_log_line_tmp,512,__VA_ARGS__); RvR_log(w " (%s:%u): %s\n",__FILE__,__LINE__,RvR_log_line_tmp); } while(0)

#define RvR_error_fail(X) do { RvR_error_set(__FILE__,__LINE__,(X)); goto RvR_err; } while(0)

#define RvR_error_check(X,Y) do { if(!(X)) RvR_error_fail(Y); } while(0)

//Different versions of the Fowler/Noll/Vo hash
//the basic versions (e.g. RvR_fnv64a) perform the hash on a string using their specifc init values as the hashval
//the _str versions (e.g. RvR_fnv64a_str) perform the hash on a string, but allow specifying a custom hashval
//the _buf versions (e.g. RvR_fnv64a_buf) perform the hash on an arbitrary buffer and allow specifying a custom hashval
uint64_t RvR_fnv64a(const char *str);
uint64_t RvR_fnv64a_str(const char *str, uint64_t hval);
uint64_t RvR_fnv64a_buf(const void *buf, size_t len, uint64_t hval);
uint32_t RvR_fnv32a(const char *str);
uint32_t RvR_fnv32a_str(const char *str, uint32_t hval);
uint32_t RvR_fnv32a_buf(const void *buf, size_t len, uint32_t hval);

//RvnicRaven provides a custom memory allocater
//If RvR_malloc_init doesn't get called, all other
//functions will instead call their stdlib equivalents (free, malloc, realloc).
//Even if you don't want to use the custom allocater, it's still
//advised to use these functions instead of the stdlib variants
void  RvR_malloc_init(size_t min, size_t max);
void *RvR_malloc(size_t size);
void  RvR_free(void *ptr);
void *RvR_realloc(void *ptr, size_t size);
void  RvR_malloc_report();

//RvnicRaven stores its palette in a binary format, with the
//colors just being dumped sequentially (768 bytes --> 256 colors --> 1 byte r,g,b each)
void       RvR_palette_load(uint16_t id);
RvR_color *RvR_palette();
uint8_t   *RvR_shade_table(uint8_t light);

unsigned RvR_rand();
int      RvR_rand_get_state();
void     RvR_rand_set_state(unsigned state);

RvR_fix22 RvR_fix22_cos(RvR_fix22 a);
RvR_fix22 RvR_fix22_sin(RvR_fix22 a);
RvR_fix22 RvR_fix22_tan(RvR_fix22 a);
RvR_fix22 RvR_fix22_ctg(RvR_fix22 a);
RvR_fix22 RvR_fix22_sqrt(RvR_fix22 a);
RvR_fix22 RvR_fix22_atan2(RvR_fix22 x, RvR_fix22 y);

RvR_fix16 RvR_fix16_from_int(int a);
int       RvR_fix16_to_int(RvR_fix16 a);
RvR_fix16 RvR_fix16_mul(RvR_fix16 a, RvR_fix16 b);
RvR_fix16 RvR_fix16_div(RvR_fix16 a, RvR_fix16 b);
RvR_fix16 RvR_fix16_round(RvR_fix16 a);
RvR_fix16 RvR_fix16_floor(RvR_fix16 a);
RvR_fix16 RvR_fix16_ceil(RvR_fix16 a);

RvR_fix22 RvR_fix22_from_int(int a);
int       RvR_fix22_to_int(RvR_fix22 a);
RvR_fix22 RvR_fix22_mul(RvR_fix22 a, RvR_fix22 b);
RvR_fix22 RvR_fix22_div(RvR_fix22 a, RvR_fix22 b);
RvR_fix22 RvR_fix22_round(RvR_fix22 a);
RvR_fix22 RvR_fix22_floor(RvR_fix22 a);
RvR_fix22 RvR_fix22_ceil(RvR_fix22 a);

RvR_fix24 RvR_fix24_from_int(int a);
int       RvR_fix24_to_int(RvR_fix24 a);
RvR_fix24 RvR_fix24_mul(RvR_fix24 a, RvR_fix24 b);
RvR_fix24 RvR_fix24_div(RvR_fix24 a, RvR_fix24 b);
RvR_fix24 RvR_fix24_round(RvR_fix24 a);
RvR_fix24 RvR_fix24_floor(RvR_fix24 a);
RvR_fix24 RvR_fix24_ceil(RvR_fix24 a);

RvR_vec2  RvR_vec2_rot(RvR_fix22 angle);
RvR_fix22 RvR_dist2(RvR_vec2 p0, RvR_vec2 p1);
RvR_fix22 RvR_len2(RvR_vec2 v);

int32_t RvR_div_round_down(int32_t a, int32_t b);
int32_t RvR_abs(int32_t a);
int32_t RvR_wrap(int32_t a, int32_t mod);

#define RvR_non_zero(a) ((a)+((a)==0)) ///< To prevent zero divisions.
#define RvR_zero_clamp(x) ((x)*((x)>=0))
#define RvR_min(a,b) ((a)<(b)?(a):(b))
#define RvR_max(a,b) ((a)>(b)?(a):(b))
#define RvR_sign(a) (a<0?-1:1)
#define RvR_clamp(a,min,max) (RvR_max((min),RvR_min((max),(a))))

void        RvR_pak_add(const char *path);
void        RvR_pak_create_from_csv(const char *path_csv, const char *path_pak);
void        RvR_pak_flush();
void        RvR_lump_add(const char *name, const char *path);
void       *RvR_lump_get(const char *name, unsigned *size);
const char *RvR_lump_get_path(const char *name);
int         RvR_lump_exists(const char *name);

RvR_texture *RvR_texture_get(uint16_t id);
void RvR_texture_load_begin();
void RvR_texture_load_end();
void RvR_texture_load(uint16_t id);
void RvR_texture_lock(uint16_t id);
void RvR_texture_unlock(uint16_t id);
void RvR_texture_create(uint16_t id, int width, int height);
void RvR_font_load(uint16_t id);
void RvR_font_unload(uint16_t id);

//RvnicRaven core functions end
//-------------------------------------

//RvnicRaven raycast types

typedef struct
{
   RvR_vec2 start;
   RvR_vec2 direction;
}RvR_ray;

typedef struct
{
   uint16_t type;
   RvR_vec3 pos;
   RvR_fix22 direction;
   int32_t extra0;
   int32_t extra1;
   int32_t extra2;
   int32_t extra3;
}RvR_ray_map_sprite;

typedef struct
{
   uint16_t *wall_ftex;
   uint16_t *wall_ctex;
   uint16_t *floor_tex;
   uint16_t *ceil_tex;
   RvR_fix22 *floor;
   RvR_fix22 *ceiling;
   uint16_t width;
   uint16_t height;
   uint8_t floor_color;
   uint16_t sky_tex;
}RvR_ray_map;

typedef struct
{
   uint16_t *wall_ftex;
   uint16_t *wall_ctex;
   uint16_t *floor_tex;
   uint16_t *ceil_tex;
   int8_t *floor;
   int8_t *ceiling;
   uint16_t width;
   uint16_t height;
   uint8_t floor_color;
   uint16_t sky_tex;

   RvR_ray_map_sprite *sprites;
   uint32_t sprite_count;
}RvR_ray_map_cache;

typedef struct
{
   RvR_vec2 position;
   RvR_vec2 square;
   RvR_fix22 distance;
   uint8_t direction;
   uint16_t wall_ftex;
   uint16_t wall_ctex;
   uint16_t floor_tex;
   uint16_t ceil_tex;
   RvR_fix22 fheight;
   RvR_fix22 cheight;
   RvR_fix22 texture_coord;
}RvR_ray_hit_result;

typedef struct RvR_ray_depth_buffer_entry
{
   RvR_fix22 depth;
   int32_t limit;

   struct RvR_ray_depth_buffer_entry *next;
}RvR_ray_depth_buffer_entry;

typedef struct
{
   RvR_ray_depth_buffer_entry *floor[RVR_XRES];
   RvR_ray_depth_buffer_entry *ceiling[RVR_XRES];
}RvR_ray_depth_buffer;

typedef struct
{
   RvR_vec2 position;
   RvR_vec2 tex_coords;
   RvR_fix22 depth;
   int8_t is_horizon;
   RvR_ray_hit_result hit;
}RvR_ray_pixel_info;

typedef void (*RvR_ray_column_function) (RvR_ray_hit_result *hits, uint16_t x, RvR_ray ray);

//RvnicRaven raycast types end
//-------------------------------------

//RvnicRaven raycast functions

void RvR_ray_cast_multi_hit(RvR_ray ray, RvR_ray_hit_result *hit_results, uint16_t *hit_results_len);
void RvR_rays_cast_multi_hit(RvR_ray_column_function column);
RvR_fix22 RvR_ray_perspective_scale_vertical(RvR_fix22 org_size, RvR_fix22 distance);
RvR_fix22 RvR_ray_perspective_scale_vertical_inverse(RvR_fix22 org_size, RvR_fix22 sc_size);
RvR_fix22 RvR_ray_perspective_scale_horizontal(RvR_fix22 org_size, RvR_fix22 distance);

//TODO: move out of RvnicRaven?
void      RvR_ray_move_with_collision(RvR_vec3 offset, int8_t compute_height, int8_t force, RvR_fix22 *floor_height, RvR_fix22 *ceiling_height);

void      RvR_ray_set_angle(RvR_fix22 angle);
RvR_fix22 RvR_ray_get_angle();
void      RvR_ray_set_shear(int16_t shear);
int16_t   RvR_ray_get_shear();
void      RvR_ray_set_position(RvR_vec3 position);
RvR_vec3  RvR_ray_get_position();

void RvR_ray_map_create(uint16_t width, uint16_t height);
void RvR_ray_map_reset();
void RvR_ray_map_reset_full();
void RvR_ray_map_load_path(const char *path);
void RvR_ray_map_load(uint16_t id);
void RvR_ray_map_load_rw(RvR_rw *rw);
void RvR_ray_map_save(const char *path);
int RvR_ray_map_sprite_count();
RvR_ray_map_sprite *RvR_ray_map_sprite_get(unsigned index);

RvR_ray_map *RvR_ray_map_get();
RvR_ray_map_cache *RvR_ray_map_cache_get();
int       RvR_ray_map_inbounds(int16_t x, int16_t y);
uint16_t  RvR_ray_map_sky_tex();
uint16_t  RvR_ray_map_wall_ftex_at(int16_t x, int16_t y);
uint16_t  RvR_ray_map_wall_ctex_at(int16_t x, int16_t y);
uint16_t  RvR_ray_map_floor_tex_at(int16_t x, int16_t y);
uint16_t  RvR_ray_map_ceil_tex_at(int16_t x, int16_t y);
RvR_fix22 RvR_ray_map_floor_height_at(int16_t x, int16_t y);
RvR_fix22 RvR_ray_map_ceiling_height_at(int16_t x, int16_t y);

void RvR_ray_map_wall_ftex_set(int16_t x, int16_t y, uint16_t tex);
void RvR_ray_map_wall_ctex_set(int16_t x, int16_t y, uint16_t tex);
void RvR_ray_map_floor_tex_set(int16_t x, int16_t y, uint16_t tex);
void RvR_ray_map_ceil_tex_set(int16_t x, int16_t y, uint16_t tex);
void RvR_ray_map_floor_height_set(int16_t x, int16_t y, RvR_fix22 height);
void RvR_ray_map_ceiling_height_set(int16_t x, int16_t y, RvR_fix22 height);

uint16_t  RvR_ray_map_wall_ftex_at_us(int16_t x, int16_t y);
uint16_t  RvR_ray_map_wall_ctex_at_us(int16_t x, int16_t y);
uint16_t  RvR_ray_map_floor_tex_at_us(int16_t x, int16_t y);
uint16_t  RvR_ray_map_ceil_tex_at_us(int16_t x, int16_t y);
RvR_fix22 RvR_ray_map_floor_height_at_us(int16_t x, int16_t y);
RvR_fix22 RvR_ray_map_ceiling_height_at_us(int16_t x, int16_t y);

RvR_ray_depth_buffer *RvR_ray_draw_depth_buffer();
RvR_ray_pixel_info RvR_ray_map_to_screen(RvR_vec3 world_position);

void RvR_ray_draw_sprite(RvR_vec3 pos, int32_t tex);
void RvR_ray_draw();
void RvR_ray_draw_debug(uint8_t index);

//RvnicRaven raycast functions end
//-------------------------------------

#endif
