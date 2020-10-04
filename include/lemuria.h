
#include <inttypes.h>

#define LEMURIA_TIME_SAMPLES 512
#define LEMURIA_FREQ_SAMPLES 256

#define LEMURIA_EFFECT_BACKGROUND 0
#define LEMURIA_EFFECT_FOREGROUND 1
#define LEMURIA_EFFECT_TEXTURE    2

typedef struct lemuria_engine_s lemuria_engine_t;

lemuria_engine_t * lemuria_create(void);

void lemuria_set_size(lemuria_engine_t *, int width, int height);

/* The type argument of the functions above is one of the
   LEMURIA_EFFECT_* defines above */

int lemuria_num_effects(lemuria_engine_t *, int type);
const char * lemuria_effect_name(lemuria_engine_t *, int type, int index);
const char * lemuria_effect_label(lemuria_engine_t *, int type, int index);

void lemuria_change_effect(lemuria_engine_t *, int type);
void lemuria_set_effect(lemuria_engine_t *, int type, int index);
void lemuria_next_effect(lemuria_engine_t *, int type);

void lemuria_draw_frame(lemuria_engine_t *);

/* Update display */

void lemuria_wait(lemuria_engine_t *);


/* Add audio samples (can be called from another thread) */

void lemuria_update_audio(lemuria_engine_t * e, int16_t * channels[2]);

/* Destroy engine */

void lemuria_destroy(lemuria_engine_t *);

#define LEMURIA_ANTIALIAS_NONE 0
#define LEMURIA_ANTIALIAS_BEST 2

void lemuria_set_antialiasing(lemuria_engine_t * engine, int antialiasing);

void lemuria_print_help(lemuria_engine_t * e);
