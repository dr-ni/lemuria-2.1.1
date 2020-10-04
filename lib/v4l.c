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

#include <lemuria_private.h>
#include <effect.h>
#include <utils.h>

#include "gradients/lemuria_v4l.incl"
#include <gmerlin/plugin.h>
#include <gmerlin/pluginregistry.h>
#include <gmerlin/utils.h>

/* We make everything static to disturb the code only little */

static bg_plugin_handle_t * plugin_handle  = (bg_plugin_handle_t*)0;
static bg_rv_plugin_t * plugin           = (bg_rv_plugin_t*)0;
static bg_cfg_registry_t * cfg_reg       = (bg_cfg_registry_t *)0;
static bg_plugin_registry_t * plugin_reg = (bg_plugin_registry_t *)0;

static gavl_video_format_t in_format;
static gavl_video_format_t out_format_1;
static gavl_video_format_t out_format_2;

static gavl_video_converter_t * cnv;

gavl_video_frame_t * in_frame_1 = (gavl_video_frame_t *)0;
gavl_video_frame_t * in_frame_2 = (gavl_video_frame_t *)0;

gavl_video_frame_t * out_frame_1 = (gavl_video_frame_t *)0;
gavl_video_frame_t * out_frame_2 = (gavl_video_frame_t *)0;

static int new_frame = 1;
static int is_open = 0;


pthread_mutex_t frame_mutex;

pthread_mutex_t state_mutex;
int do_stop = 0;


pthread_t thread;



static void * thread_func(void * data);

float texture_coords[4][2];

void lemuria_v4l_init()
  {
  pthread_attr_t attr;
  gavl_video_options_t opt;
  bg_cfg_section_t * cfg_section;
  const bg_plugin_info_t * info;
  char * tmp_path;

  cfg_reg = bg_cfg_registry_create();
  tmp_path = bg_search_file_read("generic", "config.xml");
  bg_cfg_registry_load(cfg_reg, tmp_path);
  if(tmp_path)
    free(tmp_path);
  cfg_section = bg_cfg_registry_find_section(cfg_reg, "plugins");
  plugin_reg = bg_plugin_registry_create(cfg_section);

  info = bg_plugin_find_by_name(plugin_reg, "i_v4l");

  plugin_handle = bg_plugin_load(plugin_reg, info);

  plugin = (bg_rv_plugin_t*)(plugin_handle->plugin);
  
  if(!plugin->open(plugin_handle->priv, &in_format))
    {
    fprintf(stderr, "Cannot open v4l device\n");
    return;
    }
  is_open = 1;
  
  cnv = gavl_video_converter_create();
  gavl_video_default_options(&opt);
    
  gavl_video_format_copy(&out_format_1, &in_format);
  gavl_video_format_copy(&out_format_2, &in_format);
  
  out_format_1.colorspace = GAVL_RGB_24;
  out_format_2.colorspace = GAVL_RGBA_32;

  out_format_1.frame_width  = 1;
  out_format_1.frame_height = 1;

  while(out_format_1.frame_width < out_format_1.image_width)
    out_format_1.frame_width <<= 1;

  while(out_format_1.frame_height < out_format_1.image_height)
    out_format_1.frame_height <<= 1;

  out_format_2.frame_width  = out_format_1.frame_width;
  out_format_2.frame_height = out_format_1.frame_height;
  
  gavl_video_converter_init(cnv, &opt,
                            &in_format,
                            &out_format_1);

  in_frame_1 = gavl_video_frame_create(&in_format);
  in_frame_2 = gavl_video_frame_create(&in_format);

  out_frame_1 = gavl_video_frame_create(&out_format_1);
  out_frame_2 = gavl_video_frame_create(&out_format_2);

  gavl_video_frame_clear(in_frame_1, &in_format);
  gavl_video_frame_clear(in_frame_2, &in_format);

  gavl_video_frame_clear(out_frame_1, &out_format_1);
  gavl_video_frame_clear(out_frame_2, &out_format_2);
  
  pthread_mutex_init(&frame_mutex, (pthread_mutexattr_t*)0);
  pthread_mutex_init(&state_mutex, (pthread_mutexattr_t*)0);

  pthread_attr_init(&attr);
  pthread_create(&(thread), &attr, &thread_func,
                 (void*)0);
  pthread_attr_destroy (&attr);
  
  texture_coords[0][0] = 0.0;
  texture_coords[0][1] = (float)(out_format_2.image_height) / (float)(out_format_2.frame_height);

  texture_coords[1][0] = (float)(out_format_2.image_width) / (float)(out_format_2.frame_width);
  texture_coords[1][1] = (float)(out_format_2.image_height) / (float)(out_format_2.frame_height);

  texture_coords[2][0] = (float)(out_format_2.image_width) / (float)(out_format_2.frame_width);
  texture_coords[2][1] = 0.0;

  texture_coords[3][0] = 0.0;
  texture_coords[3][1] = 0.0;
  
#if 0
  fprintf(stderr, "Input format:\n");
  gavl_video_format_dump(&in_format);

  fprintf(stderr, "Output format:\n");
  gavl_video_format_dump(&out_format_1);
#endif

  }


void lemuria_v4l_cleanup()
  {
  pthread_mutex_lock(&state_mutex);
  do_stop = 1;
  pthread_mutex_unlock(&state_mutex);

  pthread_join(thread, NULL);

  gavl_video_frame_destroy(in_frame_1);
  gavl_video_frame_destroy(in_frame_2);
  gavl_video_frame_destroy(out_frame_1);
  gavl_video_frame_destroy(out_frame_2);
  
  bg_plugin_unref(plugin_handle);
  gavl_video_converter_destroy(cnv);

  bg_plugin_registry_destroy(plugin_reg);
  bg_cfg_registry_destroy(cfg_reg);

  }

static void * thread_func(void * data)
  {
  gavl_time_t delay_time;

  delay_time = GAVL_TIME_SCALE / 30;
  
  while(1)
    {
    pthread_mutex_lock(&state_mutex);
    if(do_stop)
      {
      pthread_mutex_unlock(&state_mutex);
      break;
      }
    pthread_mutex_unlock(&state_mutex);
       
    if(!is_open)
      {
      /* Try to reopen */
      if(plugin->open(plugin_handle->priv, &in_format))
        {
        is_open = 1;
        fprintf(stderr, "Camera back again :-)\n");
        }
      }

    if(is_open)
      {
      if(!plugin->read_frame(plugin_handle->priv, in_frame_1))
        {
        fprintf(stderr, "Cannot read frame (Camera unplugged?)\n");
        plugin->close(plugin_handle->priv);
        is_open = 0;
        }
      else
        {
        pthread_mutex_lock(&frame_mutex);
        new_frame = 1;
        gavl_video_frame_copy(&in_format, in_frame_2, in_frame_1);
        pthread_mutex_unlock(&frame_mutex);
        }
      }
    gavl_time_delay(&delay_time);
    }
  return NULL;
  }

/* Drawing specific stuff */

static float bg_colors[][4] = 
  {
    { 1.0, 1.0, 1.0, 1.0 },
    { 0.5, 1.0, 1.0, 1.0 },
    { 1.0, 0.5, 1.0, 1.0 },
    { 1.0, 1.0, 0.5, 1.0 },
  };

#define NUM_BG_COLORS (sizeof(bg_colors)/sizeof(bg_colors[0]))
#define TEXTURE_ADVANCE_MAX 0.005

typedef struct
  {
  unsigned int texture;
  lemuria_range_t color_range;
  lemuria_range_t texture_advance_range;
  int start_color;
  int end_color;
  int frame_count;

  lemuria_background_t background;

  float texture_advance_start;
  float texture_advance_end;

  float texture_start;
  int texture_flip;
  } v4l_data;

static lemuria_background_data_t background =
  {
    .texture_mode = LEMURIA_TEXTURE_CLOUDS_ROTATE,
    gradient_lemuria_v4l,
    clouds_size : 1,
  };
 

static void * init_v4l(lemuria_engine_t * e)
  {
  v4l_data * d;

#ifdef DEBUG
  fprintf(stderr, "init_v4l...");
#endif

  //  fprintf(stderr, "Frame size: %d %d\n", out_format.frame_width, out_format.frame_width);
  
  d = calloc(1, sizeof(*d));
  
  glGenTextures(1, &(d->texture));
  glBindTexture(GL_TEXTURE_2D, d->texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, 4,
               out_format_2.frame_width,
               out_format_2.frame_height,
               0, GL_RGBA, GL_UNSIGNED_BYTE,
               out_frame_2->planes[0]);

  d->start_color = lemuria_random_int(0, NUM_BG_COLORS-1);
  d->end_color = lemuria_random_int(0, NUM_BG_COLORS-2);

  d->texture_advance_start = lemuria_random(-TEXTURE_ADVANCE_MAX, TEXTURE_ADVANCE_MAX);
  d->texture_advance_end = lemuria_random(-TEXTURE_ADVANCE_MAX, TEXTURE_ADVANCE_MAX);
  lemuria_range_init(&(d->texture_advance_range), 1, 50, 100);
    
  if(d->end_color >= d->start_color)
    d->end_color++;
  lemuria_range_init(&(d->color_range), 4, 50, 100);

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif

  lemuria_background_init(e, &(d->background), &background);

  return d;
  }

#define ALPHA 64

#define R_TO_LUM 76
#define G_TO_LUM 150
#define B_TO_LUM 29

static void update_frame()
  {
  int i, j, tmp;
  uint8_t * src, *dst;
  
  pthread_mutex_lock(&frame_mutex);

  if(new_frame)
    {
    gavl_video_convert(cnv, in_frame_2, out_frame_1);
    new_frame = 0;
    }
  pthread_mutex_unlock(&frame_mutex);
  
  for(i = 0; i < out_format_2.image_height; i++)
    {
    src = out_frame_1->planes[0] + i * out_frame_1->strides[0];
    dst = out_frame_2->planes[0] + i * out_frame_2->strides[0];

    for(j = 0; j < out_format_2.image_width; j++)
      {
      tmp = ALPHA * src[0] + (0xff - ALPHA) * dst[0];
      dst[0] = tmp >> 8;

      tmp = ALPHA * src[1] + (0xff - ALPHA) * dst[1];
      dst[1] = tmp >> 8;

      tmp = ALPHA * src[2] + (0xff - ALPHA) * dst[2];
      dst[2] = tmp >> 8;

      /* Alpha is equal to luminance */

      tmp = dst[0] * R_TO_LUM + dst[1] * G_TO_LUM + dst[2] * B_TO_LUM;
      dst[3] = 0xff - (tmp >> 8);
      
      src+=3;
      dst+=4;
      }
    }
  
  glTexImage2D(GL_TEXTURE_2D, 0, 4,
               out_format_2.frame_width,
               out_format_2.frame_height,
               0, GL_RGBA, GL_UNSIGNED_BYTE,
               out_frame_2->planes[0]);
  
  }

#define MAX_Y 1.25
#define MAX_X ((MAX_Y*4.0)/3.0)

static float bg_coords[4][3] =
  {
    { -MAX_X, -MAX_Y, 0.0 },
    { MAX_X, -MAX_Y,  0.0 },
    { MAX_X,  MAX_Y,  0.0 },
    { -MAX_X,  MAX_Y, 0.0 },
  };

static void draw_v4l(lemuria_engine_t * e, void * user_data)
  {
  float texture_advance;
  float color[4];
    
  v4l_data * d = (v4l_data*)(user_data);

  d->frame_count++;
  if(d->frame_count == 5)
    {
    d->frame_count = 0;
    glBindTexture(GL_TEXTURE_2D, d->texture);
    update_frame();
    }
  lemuria_background_update(&(d->background));
  
  if(e->beat_detected)
    {
    if(lemuria_range_done(&(d->color_range)) && lemuria_decide(0.2))
      {
      d->start_color = d->end_color;
      d->end_color = lemuria_random_int(0, NUM_BG_COLORS-2);
      if(d->end_color >= d->start_color)
        d->end_color++;
      
      lemuria_range_init(&(d->color_range), 4, 50, 100);
      }

    if(lemuria_range_done(&(d->texture_advance_range)) && lemuria_decide(0.2))
      {
      d->texture_advance_start = d->texture_advance_end;
      d->texture_advance_end = lemuria_random(-TEXTURE_ADVANCE_MAX, TEXTURE_ADVANCE_MAX);
      
      lemuria_range_init(&(d->texture_advance_range), 1, 50, 100);
      }
    }
  
  lemuria_range_update(&(d->color_range));
  lemuria_range_update(&(d->texture_advance_range));
  
  lemuria_range_get(&(d->color_range),
                    bg_colors[d->start_color],
                    bg_colors[d->end_color],
                    color);

  lemuria_range_get(&(d->texture_advance_range),
                    &(d->texture_advance_start),
                    &(d->texture_advance_end),
                    &texture_advance);
  
  glColor4fv(color);
  
  //  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  lemuria_set_perspective(e, 1, 1000.0);

  
  
  glEnable(GL_TEXTURE_2D);

  lemuria_background_set(&(d->background));
  lemuria_background_draw(&(d->background),
                          bg_coords,
                          1,
                          1,
                          &(d->texture_start),
                          texture_advance,
                          &(d->texture_flip));
#if 1
  glDisable(GL_DEPTH_TEST);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  
  glBindTexture(GL_TEXTURE_2D, d->texture);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  
  glBegin(GL_QUADS);

  glTexCoord2f(texture_coords[0][0], texture_coords[0][1]);
  glVertex3fv(bg_coords[0]);

  glTexCoord2f(texture_coords[1][0], texture_coords[1][1]);
  glVertex3fv(bg_coords[1]);

  glTexCoord2f(texture_coords[2][0], texture_coords[2][1]);
  glVertex3fv(bg_coords[2]);

  glTexCoord2f(texture_coords[3][0], texture_coords[3][1]);
  glVertex3fv(bg_coords[3]);
  
  glEnd();

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

#endif
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glDisable(GL_TEXTURE_2D);
  }


static void delete_v4l(void * data)
  {
  v4l_data * d = (v4l_data*)(data);
  glDeleteTextures(1, &(d->texture));
  lemuria_background_delete(&(d->background));
  free(d);
  }


effect_plugin_t v4l_effect =
  {
    .init =    init_v4l,
    .draw =    draw_v4l,
    .cleanup = delete_v4l,
  };
