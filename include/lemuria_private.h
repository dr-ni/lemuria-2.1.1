#include <lemuria.h>
#include <fft.h>

typedef struct effect_plugin_s effect_plugin_t;

// Transitions of foreground effects

#define EFFECT_RUNNING       0 // Nothing happening
#define EFFECT_FINISH        1 // Plugin should finish now
#define EFFECT_FINISHING     2 // Plugin is finishing
#define EFFECT_STARTING      3 // Plugin just starts
#define EFFECT_DONE          4 // Plugin is done

#define CAMERA_X 0.0
#define CAMERA_Y 0.0
#define CAMERA_Z 3.0

typedef struct lemuria_effect_s
  {
  effect_plugin_t * effect;
  void * data;
  int frame_counter;
  int mode;
  int index;
  int min_frames;
  
  int next_index;
  } lemuria_effect_t;

typedef struct lemuria_xaos_s lemuria_xaos_t;
typedef struct lemuria_texture_s lemuria_texture_t;

struct lemuria_engine_s
  {
  lemuria_effect_t foreground;
  lemuria_effect_t background;
  lemuria_effect_t texture;
  
  int paused;
  int antialias;
  
  lemuria_texture_t * lemuria_texture;
  
  /* Audio buffers */

  int16_t time_buffer_write[2][512];
  int16_t time_buffer_read[2][512];
  
  int16_t freq_buffer_write[2][256];
  int16_t freq_buffer_read[2][256];

  int time_new_read;
  int time_new_write;

  int freq_new_read;
  int freq_new_write;
  
  int is_initialized;
  
  /* Actual render size */
  
  int width, height;
  
  /* Stuff related to the window system */

  //  void * window_data;
  
  int fullscreen;
    
  /* For the beat detection routine */

  float bass_power;
  unsigned int beat_counter;

  /* For making the analysis */
  
  void * analysis;

  /* Values describing the music */
  
  int loudness, thickness, quiet;
  
  int beat_detected;

  /* These are the variables for the transformations of the textures: */

  //  void * texture_transform;

  /* Goom */

  void * goom;

  /* Xaos */

  lemuria_xaos_t * xaos;
  
  int32_t last_frame_time;
  
  /* Font */
    
  void * font;

  fft_state * fft;
  
  int initialized;
  
  };

// This is for all background textures and the foreground texture

#define TEXTURE_SIZE 256

/*
 *  Texture stuff
 *
 */

void lemuria_texture_update(lemuria_engine_t *);
void lemuria_texture_create(lemuria_engine_t *);
void lemuria_texture_destroy(lemuria_engine_t *);

/* Num is 0 for normal texture, 1 for wrong color texture */
void lemuria_texture_bind(lemuria_engine_t *, int num);

/* We do the texture stuff only if needed */

void lemuria_texture_ref(lemuria_engine_t *, int num);
void lemuria_texture_unref(lemuria_engine_t *, int num);

/* Window system related functions */

#if 0
void lemuria_create_window(lemuria_engine_t * e,
                           const char * embed, int * width, int * height);

void lemuria_destroy_window(lemuria_engine_t * e);

void lemuria_set_glcontext(lemuria_engine_t * e);
void lemuria_unset_glcontext(lemuria_engine_t * e);
#endif

void lemuria_init_gl(lemuria_engine_t * e);
void lemuria_destroy_gl(lemuria_engine_t * e);

void lemuria_font_init(lemuria_engine_t * e);
/* Goom stuff */

void lemuria_goom_create(lemuria_engine_t * e);
void lemuria_goom_update(lemuria_engine_t * e);

void lemuria_goom_bindtexture(lemuria_engine_t * e);
void lemuria_goom_destroy(lemuria_engine_t * e);

void lemuria_goom_bind(lemuria_engine_t * e);

void lemuria_goom_ref(lemuria_engine_t * e);
void lemuria_goom_unref(lemuria_engine_t * e);

int lemuria_goom_refcount(lemuria_engine_t * e);

/* Xaos */

void lemuria_xaos_create(lemuria_engine_t * e);
void lemuria_xaos_destroy(lemuria_engine_t * e);
void lemuria_xaos_update(lemuria_engine_t * e);
void lemuria_xaos_ref(lemuria_engine_t * e);
void lemuria_xaos_unref(lemuria_engine_t * e);
void lemuria_xaos_bind(lemuria_engine_t * e);

/* Font */

void lemuria_font_init(lemuria_engine_t * e);
void lemuria_font_destroy(lemuria_engine_t * e);

void lemuria_put_text(lemuria_engine_t * e,
                      char * text,
                      float justify_left,
                      float justify_right,
                      float pos_x,
                      float pos_y,
                      float size,
                      int frames_to_live);

void lemuria_text_update(lemuria_engine_t * e);

void lemuria_print_info(lemuria_engine_t * e);

