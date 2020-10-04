/*    
 *   Lemuria, an OpenGL music visualization
 *   Copyright (C) 2002 - 2007 Burkhard Plaum
 *
 *   Lemuria is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <inttypes.h>

#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include <config.h>
#include <lemuria_private.h>
#include <effect.h>
#include <utils.h>

#define DEFAULT_WIDTH  640
#define DEFAULT_HEIGHT 480

static void copy_frame_256(int16_t dst[2][256], int16_t src[2][256])
  {
  memcpy(dst[0], src[0], 256 * sizeof(int16_t));
  memcpy(dst[1], src[1], 256 * sizeof(int16_t));
  }

static void copy_frame_512(int16_t dst[2][512], int16_t src[2][512])
  {
  memcpy(dst[0], src[0], 512 * sizeof(int16_t));
  memcpy(dst[1], src[1], 512 * sizeof(int16_t));
  }


lemuria_engine_t * lemuria_create()
  {
  struct timeval currenttime;
  lemuria_engine_t * ret = calloc(1, sizeof(lemuria_engine_t));
  
  ret->analysis = lemuria_analysis_init();
  
  //
  
  
  ret->fullscreen = 0;

  // lemuria_create_window(ret, embed, &ret->width, &ret->height);
  
  //  lemuria_set_glcontext(ret);

  lemuria_init_gl(ret);
  lemuria_texture_create(ret);
  lemuria_goom_create(ret);
  lemuria_xaos_create(ret);
  /* Initialize Font */

  lemuria_font_init(ret);
    
  //  lemuria_unset_glcontext(ret);


  /* Initialize random generator */
  gettimeofday(&currenttime, (struct timezone *)0);
  /*  fprintf(stderr, "srand: %u\n", (unsigned int)currenttime.tv_usec); */
  srand((unsigned int)currenttime.tv_usec);

  //  lemuria_v4l_init();

  ret->fft = fft_init();
  
  return ret;
  }

void lemuria_set_size(lemuria_engine_t * e, int width, int height)
  {
  e->width = width;
  e->height = height;
  }


/* Destroy engine */

void lemuria_destroy(lemuria_engine_t * e)
  {
  //  lemuria_set_glcontext(e);

  /* Shut down all effects */
  if(e->background.data) e->background.effect->cleanup(e->background.data);
  if(e->foreground.data) e->foreground.effect->cleanup(e->foreground.data);
  if(e->texture.data) e->texture.effect->cleanup(e->texture.data);

  e->background.effect = NULL;
  e->foreground.effect = NULL;
  e->texture.effect = NULL;
  //  fprintf(stderr, "Unsetting GLContext...");
  
  lemuria_font_destroy(e);
  

  lemuria_analysis_cleanup(e->analysis);
  
  fft_close(e->fft);
  lemuria_goom_destroy(e);
  lemuria_xaos_destroy(e);
  
  lemuria_texture_destroy(e);
  
  lemuria_destroy_gl(e);
  
  //  lemuria_unset_glcontext(e);
  
  //  lemuria_destroy_window(e);
  
    
  free(e);
  }

void lemuria_init_gl(lemuria_engine_t * e)
  {
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
  
  // Other stuff
  glClearDepth(1.0);                       // Depth Buffer Setup
  glEnable(GL_DEPTH_TEST);                 // Enable Depth Buffer
  
  }

void lemuria_destroy_gl(lemuria_engine_t * e)
  {
  }

/* Add time samples */

static void add_time(lemuria_engine_t * engine,
                      int16_t ** samples)
  {

  engine->time_new_write = 1;

  memcpy(engine->time_buffer_write[0], samples[0], 512 * sizeof(int16_t));
  memcpy(engine->time_buffer_write[1], samples[1], 512 * sizeof(int16_t));
  
  }

/* Add frequency samples */

static void add_freq(lemuria_engine_t * e,
                     int16_t samples[2][256])
  {

  e->freq_new_write = 1;
  copy_frame_256(e->freq_buffer_write, samples);
  
  }

void lemuria_update_audio(lemuria_engine_t * e, int16_t * data[2])
  {
  int i;
  int16_t freq_data[2][256];
  float fft_scratch[257];
  
  add_time(e, data);
  
  fft_perform(data[0], fft_scratch, e->fft);
  for(i = 0; i < 256; i++)
      freq_data[0][i] = ((int)sqrt(fft_scratch[i + 1])) >> 8;
  
  fft_perform(data[1], fft_scratch, e->fft);
  for(i = 0; i < 256; i++)
    freq_data[1][i] = ((int)sqrt(fft_scratch[i + 1])) >> 8;
  
  add_freq(e, freq_data);
  }

void lemuria_draw_frame(lemuria_engine_t * e)
  {
  //  lemuria_set_glcontext(e);

  if(!e->initialized)
    {
    lemuria_print_info(e);
    e->initialized = 1;
    }
  
  /* Check, if we got new time samples */


  if(e->time_new_write)
    {
    copy_frame_512(e->time_buffer_read, e->time_buffer_write);
    e->time_new_write = 0;
    e->time_new_read = 1;
    }
  

  /* Check, if we got new frequency samples */



  if(e->freq_new_write)
    {
    copy_frame_256(e->freq_buffer_read, e->freq_buffer_write);
    e->freq_new_write = 0;
    e->freq_new_read = 1;
    }
  
  lemuria_goom_update(e);
  lemuria_xaos_update(e);
  
  if(e->paused)
    {
    e->beat_detected = 0;
    e->freq_new_read = 0;
    e->time_new_read = 0;
    //    lemuria_check_events(e);
    return;
    }
  
  /* Do the actual drawing */

  if(e->time_new_read)
    lemuria_analysis_perform(e);

  /* manage Effects (must be done before handling events) */
    
  lemuria_manage_effects(e);
  
  lemuria_texture_update(e);

  /* The texture will change the viewport, so we set it here again */
  
  glViewport(0, 0, e->width, e->height);
  
  /* Draw background */
  glClear(GL_DEPTH_BUFFER_BIT);
  e->background.effect->draw(e, e->background.data);
  /* Draw Foreground */
  glClear(GL_DEPTH_BUFFER_BIT);
  e->foreground.effect->draw(e, e->foreground.data);
  
  /* Update text display (if any) */

  lemuria_text_update(e);
  
  glFlush();
  
  /* Cleanup for the next iteration */
  
  e->beat_detected = 0;
  e->freq_new_read = 0;
  e->time_new_read = 0;

  
  //  lemuria_unset_glcontext(e);
  }

void lemuria_print_help(lemuria_engine_t * e)
  {
  lemuria_put_text(e,
#if 0
                   "Keyboard shortcuts:\n\
A:  Next foreground\n\
T:  Next texture\n\
W:  Next world (=background)\n\
CTRL+A:  Random foreground\n\
CTRL+T:  Random texture\n\
CTRL+W:  Random world\n\
H:  Toggle holding effects\n\
Tab: Toogle fullscreen mode\n\
Pause: Toggle pause\n\
L: Change Antialiasing mode\n\
F1: Print this help\n\
I: Print info",
#else
                   "Keyboard shortcuts:\n\
A:  Next foreground\n\
T:  Next texture\n\
W:  Next world (=background)\n\
CTRL+A:  Random foreground\n\
CTRL+T:  Random texture\n\
CTRL+W:  Random world\n\
TAB:  Toggle fullscreen/window\n\
F1: Print this help\n",
#endif
                   0.5,
                   0.5,
                   0.0,
                   0.0,
                   1.5,
                   300);
  }

void lemuria_print_info(lemuria_engine_t * e)
  {
  lemuria_put_text(e,
                   "Lemuria-"VERSION"\nhttp://gmerlin.sf.net/lemuria\nPress F1 for help\nor sit back and relax",
                   0.5,
                   0.5,
                   0.0,
                   0.0,
                   2.0,
                   200);
  }

void lemuria_set_antialiasing(lemuria_engine_t * engine, int antialiasing)
  {
  engine->antialias = antialiasing;
  }

