/*
RvnicRaven retro game engine

Written in 2021,2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _RVNICRAVEN_H_

#define _RVNICRAVEN_H_

//Config

//Core
//Framebuffer resolution
#define RVR_XRES 640
#define RVR_YRES 480

//Fps, RvnicRaven must use a fixed timestep
#define RVR_FPS 30

//Size of precalculated rng table
#define RVR_RNG_TABLE_SIZE 512

//Maximum amount of textures loaded at once
#define RVR_TEXTURE_MAX 256

//If set to 1, logs all texture load/unload/etc operations
#define RVR_TEXTURE_DEBUG 1

//Unroll some loops
#define RVR_UNROLL 1

//0 for little, 1 for big endian machine
#define RVR_ENDIAN 0
//-------------------------------------

//Constants
#define RvR_fix22_infinity 2000000000
//-------------------------------------

//RvnicRaven vm

#define RVR_VM_STACK_SIZE 1024

#define RVR_VM_COMPUTED_GOTO 0
//-------------------------------------

//RvnicRaven raycasting

//Basically render distance
#define RVR_RAY_MAX_STEPS 64

//Debug features

//0 -> no planes
//1 -> DEBUG single colored planes (different color for each plane), WARNING FLICKERING COLORS!!
//2 -> textured planes
#define RVR_RAY_DRAW_PLANES 2
//-------------------------------------

//Macro functions
//The beautiful abomination
//Stack/dynamic array implementation
#define RvR_stack_type(type,name) typedef struct { type *data; unsigned data_size; unsigned data_used; }name
#define RvR_stack_function_prototype(type,name,prefix) prefix type name##_pop(name *st); prefix void name##_push(name *st, type v); prefix void name##_clear(name *st); prefix void name##_free(name *st); prefix int name##_empty(const name *st)
//TODO: handle non-growing stacks in _push
#define RvR_stack_function(type,name,grow,min_size,prefix) prefix type name##_pop(name *st) { if(st->data==NULL||st->data_used==0) return (type){0}; return st->data[--st->data_used]; } prefix void name##_push(name *st, type v) { if(st->data==NULL) { st->data_size = min_size; st->data = RvR_malloc(sizeof(*st->data)*st->data_size); } st->data[st->data_used++] = v; if(st->data_used==st->data_size) { st->data_size+=grow; st->data = RvR_realloc(st->data,sizeof(*st->data)*st->data_size); } } prefix void name##_clear(name *st) { st->data_used = 0; } prefix void name##_free(name *st) { if(st->data==NULL) return; RvR_free(st->data); st->data = NULL; st->data_used = 0; st->data_size = 0; } prefix int name##_empty(const name *st) { return st->data_used==0; }

//-------------------------------------

//RvnicRaven raycasting

#define RVR_PORT_MAX_SECTORS 4096
#define RVR_PORT_MAX_WALLS 16384
//-------------------------------------

//Config end
//-------------------------------------

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
   RVR_RW_STD_FILE = 0,
   RVR_RW_STD_FILE_PATH = 1,
   RVR_RW_MEM = 2,
   RVR_RW_CONST_MEM = 3,
   RVR_RW_DYN_MEM = 4,
}RvR_rw_type;

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
   RvR_rw_type type;
   uint8_t endian;

   union
   {
      FILE *fp;
      struct
      {
         void *mem;
         long size;
         long pos;
      }mem;
      struct
      {
         const void *mem;
         long size;
         long pos;
      }cmem;
      struct
      {
         void *mem;
         long size;
         long pos;
         long min_grow;
      }dmem;
   }as;
}RvR_rw;

//RvnicRaven core types end
//-------------------------------------

//RvnicRaven core functions

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
int RvR_core_frametime();
int RvR_core_frametime_average();

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
uint32_t RvR_core_frame();

void RvR_rw_init_file(RvR_rw *rw, FILE *f);
void RvR_rw_init_path(RvR_rw *rw, const char *path, const char *mode);
void RvR_rw_init_mem(RvR_rw *rw, void *mem, size_t len);
void RvR_rw_init_const_mem(RvR_rw *rw, const void *mem, size_t len);
void RvR_rw_init_dyn_mem(RvR_rw *rw, size_t base_len, size_t min_grow);

void RvR_rw_endian(RvR_rw *rw, uint8_t endian);
void RvR_rw_close(RvR_rw *rw);
void RvR_rw_flush(RvR_rw *rw);
int RvR_rw_seek(RvR_rw *rw, long offset, int origin);
long RvR_rw_tell(RvR_rw *rw);
int RvR_rw_eof(RvR_rw *rw);
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
void RvR_draw_font_set(uint16_t id);
void RvR_draw_string(int x, int y, int scale, const char *text, uint8_t index);
void RvR_draw(int x, int y, uint8_t index); //do not use, access framebuffer directly if possible
void RvR_draw_line(int x0, int y0, int x1, int y1, uint8_t index);
void RvR_draw_vertical_line(int x, int y0, int y1, uint8_t index);
void RvR_draw_horizontal_line(int x0, int x1, int y, uint8_t index);

void RvR_log(const char *w, ...);

#define RvR_log_line(w,...) do { char RvR_log_line_tmp[1024]; snprintf(RvR_log_line_tmp,1024,__VA_ARGS__); RvR_log(w " (%s:%u): %s\n",__FILE__,__LINE__,RvR_log_line_tmp); } while(0)

#define RvR_error_fail(w,...) do { RvR_log_line(w,__VA_ARGS__); goto RvR_err; } while(0)

#define RvR_error_check(X,w,...) do { if(!(X)) RvR_error_fail(w,__VA_ARGS__); } while(0)

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
void             RvR_palette_load(uint16_t id);
const RvR_color *RvR_palette();
uint8_t         *RvR_shade_table(uint8_t light);
uint8_t         RvR_blend(uint8_t c0, uint8_t c1);

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

int32_t  RvR_div_round_down(int32_t a, int32_t b);
int32_t  RvR_abs(int32_t a);
int32_t  RvR_wrap(int32_t a, int32_t mod);
uint32_t RvR_log2(uint32_t a);

#define RvR_non_zero(a) ((a)+((a)==0)) ///< To prevent zero divisions.
#define RvR_zero_clamp(x) ((x)*((x)>=0))
#define RvR_min(a,b) ((a)<(b)?(a):(b))
#define RvR_max(a,b) ((a)>(b)?(a):(b))
#define RvR_sign(a) (a<0?-1:1)
#define RvR_clamp(a,min,max) (RvR_max((min),RvR_min((max),(a))))

//These are here to make some code more understandable
#define RvR_sign_equal(a,b) (((a)^(b))>=0)
#define RvR_negative(a) ((a)<0)
#define RvR_positive(a) ((a)>0)
#define RvR_even(a) (!((a)&1))
#define RvR_odd(a) ((a)&1)

void        RvR_pak_add(const char *path);
void        RvR_pak_create_from_csv(const char *path_csv, const char *path_pak);
void        RvR_pak_flush();
void        RvR_lump_add(const char *name, const char *path);
void       *RvR_lump_get(const char *name, unsigned *size);
const char *RvR_lump_get_path(const char *name);
int         RvR_lump_exists(const char *name);

RvR_texture *RvR_texture_get(uint16_t id); //Pointer returned is only valid until next RvR_texture_get() call (unless the texture has been manually created)
void RvR_texture_create(uint16_t id, int width, int height); //These textures need to be manually managed
void RvR_texture_create_free(uint16_t id);



//RvnicRaven core functions end
//-------------------------------------

//RvnicRaven vm types
typedef struct RvR_vm RvR_vm;
typedef intptr_t (*RvR_vm_func_call) (RvR_vm *vm, intptr_t *args);

struct RvR_vm
{
   int32_t *code;
   RvR_vm_func_call callback;

   //Code
   int instruction_count;
   intptr_t *instruction_pointers;

   //Data
   int32_t *data;
   int data_len;
   int data_mask;
};
//RvnicRaven vm types end
//-------------------------------------

//RvnicRaven vm functions
void RvR_vm_create(RvR_vm *vm, RvR_rw *code, RvR_vm_func_call callback);
void RvR_vm_free(RvR_vm *vm);
void RvR_vm_call(RvR_vm *vm, uint8_t opcode, ...);
//RvnicRaven vm functions end
//-------------------------------------

//RvnicRaven raycast types

typedef struct
{
   RvR_vec2 start;
   RvR_vec2 direction;
}RvR_ray;

typedef struct
{
   RvR_vec3 pos;
   RvR_fix22 direction;
   uint16_t texture;
   uint32_t flags;
   int32_t extra0;
   int32_t extra1;
   int32_t extra2;
}RvR_ray_map_sprite;

typedef struct
{
   uint16_t sky_tex;
   uint16_t width;
   uint16_t height;
   uint32_t sprite_count;
   RvR_fix22 *floor;
   RvR_fix22 *ceiling;
   uint16_t *floor_tex;
   uint16_t *ceil_tex;
   uint16_t *wall_ftex;
   uint16_t *wall_ctex;
   RvR_ray_map_sprite *sprites;
}RvR_ray_map;

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

typedef struct
{
   RvR_vec2 position;
   RvR_fix22 depth;
}RvR_ray_pixel_info;

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

typedef void (*RvR_ray_column_function) (RvR_ray_hit_result *hits, int hits_len, uint16_t x, RvR_ray ray);

//RvnicRaven raycast types end
//-------------------------------------

//RvnicRaven raycast functions

void RvR_ray_cast_multi_hit(RvR_ray ray, RvR_ray_hit_result *hit_results, uint16_t *hit_results_len);
void RvR_rays_cast_multi_hit(RvR_ray_column_function column);

void      RvR_ray_set_angle(RvR_fix22 angle);
RvR_fix22 RvR_ray_get_angle();
void      RvR_ray_set_shear(int16_t shear);
int16_t   RvR_ray_get_shear();
void      RvR_ray_set_position(RvR_vec3 position);
RvR_vec3  RvR_ray_get_position();
void      RvR_ray_set_fov(RvR_fix22 fov);
RvR_fix22 RvR_ray_get_fov();

void RvR_ray_map_create(uint16_t width, uint16_t height);
void RvR_ray_map_load_path(const char *path);
void RvR_ray_map_load(uint16_t id);
void RvR_ray_map_load_rw(RvR_rw *rw);
void RvR_ray_map_save(const char *path);
int RvR_ray_map_sprite_count();
RvR_ray_map_sprite *RvR_ray_map_sprite_get(unsigned index);
RvR_ray_map *RvR_ray_map_get();

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

void                  RvR_ray_cast_multi_hit_draw(RvR_ray ray, RvR_ray_hit_result *hit_results, uint16_t *hit_results_len);
void                  RvR_rays_cast_multi_hit_draw(RvR_ray_column_function column);
void                  RvR_ray_draw_begin();
void                  RvR_ray_draw_end();
//Only valid between RvR_ray_draw_begin() and RvR_ray_draw_end():
void                  RvR_ray_draw_map();
void                  RvR_ray_draw_sprite(RvR_vec3 pos, RvR_fix22 angle, uint16_t tex, uint32_t flags);
void                  RvR_ray_draw_debug(uint8_t index);
RvR_ray_depth_buffer *RvR_ray_draw_depth_buffer();
RvR_ray_pixel_info    RvR_ray_map_to_screen(RvR_vec3 world_position);

//RvnicRaven raycast functions end
//-------------------------------------

//RvnicRaven portal types

typedef struct
{
   RvR_fix22 x;
   RvR_fix22 y;
   int16_t p2;
   int16_t portal;
   uint16_t tex;
}RvR_port_wall;

typedef struct
{
   int16_t num_walls;
   int16_t first_wall;

   RvR_fix22 floor_height;
   RvR_fix22 ceiling_height;

   uint16_t floor_tex;
   uint16_t ceiling_tex;
}RvR_port_sector;

typedef struct
{

}RvR_port_sprite;

typedef struct
{
   int16_t num_sectors;
   int16_t num_walls;

   RvR_port_sector sectors[RVR_PORT_MAX_SECTORS];
   RvR_port_wall walls[RVR_PORT_MAX_WALLS];
}RvR_port_map;

//RvnicRaven portal types end
//-------------------------------------

//RvnicRaven portal functions

void      RvR_port_set_angle(RvR_fix22 angle);
RvR_fix22 RvR_port_get_angle();
void      RvR_port_set_shear(int16_t shear);
int16_t   RvR_port_get_shear();
void      RvR_port_set_position(RvR_vec3 position);
RvR_vec3  RvR_port_get_position();
void      RvR_port_set_sector(int16_t sector);
int16_t   RvR_port_get_sector();

RvR_fix22 RvR_port_perspective_scale_vertical_inverse(RvR_fix22 org_size, RvR_fix22 sc_size);

void      RvR_port_set_fov(RvR_fix22 fov);
RvR_fix22 RvR_port_get_fov();

void RvR_port_map_create();
void RvR_port_map_load_path();
void RvR_port_map_load(uint16_t id);
void RvR_port_map_load_rw(RvR_rw *rw);
void RvR_port_map_save(const char *path);

RvR_port_map *RvR_port_map_get();

int RvR_port_sector_inside(int16_t sector, RvR_fix22 x, RvR_fix22 y);
int16_t RvR_port_sector_update(int16_t last_sector, RvR_fix22 x, RvR_fix22 y);

void RvR_port_draw_2d();
void RvR_port_draw();

//RvnicRaven portal functions end
//-------------------------------------

#endif
