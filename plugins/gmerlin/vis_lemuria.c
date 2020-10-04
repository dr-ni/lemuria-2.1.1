/*****************************************************************
 
  vis_lemuria.c
 
  Copyright (c) 2007 by Burkhard Plaum - plaum@ipf.uni-stuttgart.de
 
  http://gmerlin.sourceforge.net
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 
*****************************************************************/

#include <string.h>
#include <math.h>

#include <config.h>
#include <gmerlin/bg_version.h>

#include <gmerlin/translation.h>
#include <gmerlin/plugin.h>
#include <gmerlin/utils.h>
#include <gmerlin/log.h>
#include <gmerlin/keycodes.h>
#include <gmerlin/accelerator.h>

#include <gmerlin/x11/x11.h>

#include <lemuria.h>

#include <GL/glx.h>

#define LOG_DOMAIN "vis_lemuria"

typedef struct
  {
  lemuria_engine_t * e;
  bg_x11_window_t * w;
  bg_x11_window_callbacks_t window_callbacks;
  int fullscreen;
  int width, height;
  bg_accelerator_map_t * accel_map;
  int antialias;

  bg_ov_callbacks_t * cb;
  } lemuria_priv_t;

/* Window callbacks */

/* Accelerators */

#define ACCEL_TOGGLE_FULLSCREEN 0
#define ACCEL_EXIT_FULLSCREEN   1
#define ACCEL_SET_FOREGROUND    2
#define ACCEL_NEXT_FOREGROUND   3
#define ACCEL_SET_BACKGROUND    4
#define ACCEL_NEXT_BACKGROUND   5
#define ACCEL_SET_TEXTURE       6
#define ACCEL_NEXT_TEXTURE      7
#define ACCEL_HELP              8

static const bg_accelerator_t accels[] =
{
  { BG_KEY_TAB,    0,                   ACCEL_TOGGLE_FULLSCREEN },
  { BG_KEY_f,      0,                   ACCEL_TOGGLE_FULLSCREEN },
  { BG_KEY_ESCAPE, 0,                   ACCEL_EXIT_FULLSCREEN   },
  { BG_KEY_a,      0,                   ACCEL_NEXT_FOREGROUND   },
  { BG_KEY_a,      BG_KEY_CONTROL_MASK, ACCEL_SET_FOREGROUND    },
  { BG_KEY_w,      0,                   ACCEL_NEXT_BACKGROUND   },
  { BG_KEY_w,      BG_KEY_CONTROL_MASK, ACCEL_SET_BACKGROUND    },
  { BG_KEY_t,      0,                   ACCEL_NEXT_TEXTURE      },
  { BG_KEY_t,      BG_KEY_CONTROL_MASK, ACCEL_SET_TEXTURE       },
  { BG_KEY_F1,     0,                   ACCEL_HELP              },
  { BG_KEY_NONE,   0,                0 },
};

static int accel_callback(void * data, int id)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)data;
  switch(id)
    {
    case ACCEL_TOGGLE_FULLSCREEN:
      if(vp->fullscreen)
        bg_x11_window_set_fullscreen(vp->w, 0);
      else
        bg_x11_window_set_fullscreen(vp->w, 1);
      return 1;
      break;
    case ACCEL_EXIT_FULLSCREEN:
      if(vp->fullscreen)
        bg_x11_window_set_fullscreen(vp->w, 0);
      return 1;
      break;
    case ACCEL_SET_FOREGROUND:
      lemuria_change_effect(vp->e, LEMURIA_EFFECT_FOREGROUND);
      return 1;
      break;
    case ACCEL_NEXT_FOREGROUND:
      lemuria_next_effect(vp->e, LEMURIA_EFFECT_FOREGROUND);
      return 1;
      break;
    case ACCEL_SET_BACKGROUND:
      lemuria_change_effect(vp->e, LEMURIA_EFFECT_BACKGROUND);
      return 1;
      break;
    case ACCEL_NEXT_BACKGROUND:
      lemuria_next_effect(vp->e, LEMURIA_EFFECT_BACKGROUND);
      return 1;
      break;
    case ACCEL_SET_TEXTURE:
      lemuria_change_effect(vp->e, LEMURIA_EFFECT_TEXTURE);
      return 1;
      break;
    case ACCEL_NEXT_TEXTURE:
      lemuria_next_effect(vp->e, LEMURIA_EFFECT_TEXTURE);
      return 1;
      break;
    case ACCEL_HELP:
      lemuria_print_help(vp->e);
      return 1;
      break;

    }
  return 0;
  }

static void set_fullscreen(void * data, int fullscreen)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)data;
  vp->fullscreen = fullscreen;
  }


static void size_changed(void * data, int width, int height)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)data;
  vp->width = width;
  vp->height = height;
  
  if(vp->e)
    lemuria_set_size(vp->e, width, height);
  }

static int motion_callback(void * data, int x, int y, int mask)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)data;

  if(vp->cb && vp->cb->motion_callback)
    {
    vp->cb->motion_callback(vp->cb->data, x, y, mask);
    return 1;
    }
  return 0;
  }

static int button_callback(void * data, int x, int y, int button, int mask)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)data;
  
  if(vp->cb && vp->cb->button_callback)
    {
    vp->cb->button_callback(vp->cb->data, x, y, button, mask);
    return 1;
    }
  return 0;
  }

static int button_release_callback(void * data, int x, int y,
                                   int button, int mask)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)data;
  
  if(vp->cb && vp->cb->button_release_callback)
    {
    vp->cb->button_release_callback(vp->cb->data, x, y, button, mask);
    return 1;
    }
  return 0;
  }

static void * create_lemuria()
  {
  lemuria_priv_t * ret;
  ret = calloc(1, sizeof(*ret));
  ret->accel_map = bg_accelerator_map_create();
  
  bg_accelerator_map_append_array(ret->accel_map, accels);
  
  /* Initialize callbacks */
  ret->window_callbacks.accel_map = ret->accel_map;
  //  ret->window_callbacks.key_callback = key_callback;

  ret->window_callbacks.accel_callback = accel_callback;
  
  ret->window_callbacks.size_changed = size_changed;
  ret->window_callbacks.set_fullscreen = set_fullscreen;
  ret->window_callbacks.motion_callback = motion_callback;
  ret->window_callbacks.button_callback = button_callback;
  ret->window_callbacks.button_release_callback = button_release_callback;
  ret->window_callbacks.data = ret;
  
  return ret;
  }

static void destroy_lemuria(void * priv)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)priv;
  bg_accelerator_map_destroy(vp->accel_map);
  free(vp);
  }

static const bg_parameter_info_t parameters[] =
  {
    {
      .name = "antialias",
      .long_name = "Antialiasing",
      .type = BG_PARAMETER_SLIDER_INT,
      .flags = BG_PARAMETER_SYNC,
      .val_min = { .val_i = LEMURIA_ANTIALIAS_NONE },
      .val_max = { .val_i = LEMURIA_ANTIALIAS_BEST },
      .help_string = "Antialiasing level (0 = off, highest is best)\n",
    },
    { /* End of parameters */ },
  };

static const bg_parameter_info_t * get_parameters_lemuria(void * priv)
  {
  return parameters;
  }

static void
set_parameter_lemuria(void * priv, const char * name,
                    const bg_parameter_value_t * val)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)priv;
  
  if(!name)
    return;
  if(!strcmp(name, "antialias"))
    {
    if(vp->e)
      lemuria_set_antialiasing(vp->e, val->val_i);
    vp->antialias = val->val_i;
    }
  }

#if 0
static int attr_list[] = {
  GLX_RGBA,
  GLX_DEPTH_SIZE, 16,
  GLX_ALPHA_SIZE, 8,
  GLX_DOUBLEBUFFER,
  None
};
#endif

static int
open_lemuria(void * priv, gavl_audio_format_t * audio_format,
             const char * window_id)
  {
  lemuria_priv_t * vp;

  vp = (lemuria_priv_t *)priv;
  vp->w = bg_x11_window_create(window_id);
  
  audio_format->interleave_mode = GAVL_INTERLEAVE_NONE;
  audio_format->num_channels = 2;
  audio_format->channel_locations[0] = GAVL_CHID_NONE;
  gavl_set_channel_setup(audio_format);
  audio_format->sample_format = GAVL_SAMPLE_S16;
  audio_format->samples_per_frame = LEMURIA_TIME_SAMPLES;

  /* Initialize Window */
  bg_x11_window_set_size(vp->w, 640, 480);
  bg_x11_window_set_callbacks(vp->w, &vp->window_callbacks);

  bg_x11_window_set_gl_attribute(vp->w, BG_GL_ATTRIBUTE_RGBA, 1);
  bg_x11_window_set_gl_attribute(vp->w, BG_GL_ATTRIBUTE_ALPHA_SIZE, 8);
  bg_x11_window_set_gl_attribute(vp->w, BG_GL_ATTRIBUTE_DEPTH_SIZE, 16);
  bg_x11_window_set_gl_attribute(vp->w, BG_GL_ATTRIBUTE_DOUBLEBUFFER, 1);
  
  bg_x11_window_realize(vp->w);

  bg_x11_window_start_gl(vp->w);
  
  bg_x11_window_set_gl(vp->w);
  vp->e = lemuria_create();
  
  lemuria_set_antialiasing(vp->e, vp->antialias);
  bg_x11_window_unset_gl(vp->w);
  
  lemuria_set_size(vp->e, vp->width, vp->height);
  
  bg_x11_window_show(vp->w, 1);
  return 1;
  }

static void draw_frame_lemuria(void * priv, gavl_video_frame_t * frame)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)priv;
  bg_x11_window_set_gl(vp->w);
  lemuria_draw_frame(vp->e);
  bg_x11_window_unset_gl(vp->w);
  }

static void update_lemuria(void * priv, gavl_audio_frame_t * frame)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)priv;
  lemuria_update_audio(vp->e, frame->channels.s_16);
  }

static void close_lemuria(void * priv)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)priv;
  bg_x11_window_set_gl(vp->w);
  lemuria_destroy(vp->e);
  bg_x11_window_unset_gl(vp->w);
  bg_x11_window_stop_gl(vp->w);
  }

static void show_frame_lemuria(void * priv)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)priv;
  bg_x11_window_swap_gl(vp->w);
  /* Plugin might do OpenGL stuff here */
  bg_x11_window_set_gl(vp->w);
  bg_x11_window_handle_events(vp->w, 0);
  bg_x11_window_unset_gl(vp->w);
  }

static void set_callbacks_lemuria(void * priv, bg_ov_callbacks_t * cb)
  {
  lemuria_priv_t * vp;
  vp = (lemuria_priv_t *)priv;
  vp->cb = cb;
  }

const bg_visualization_plugin_t the_plugin = 
  {
    .common =
    {
      BG_LOCALE,
      .name =      "vis_lemuria",
      .long_name = TRS("Lemuria"),
      .description = TRS("OpenGL visualization with many effects"),
      .type =     BG_PLUGIN_VISUALIZATION,
      .flags =    BG_PLUGIN_VISUALIZE_GL,
      .create =   create_lemuria,
      .destroy =   destroy_lemuria,
      .get_parameters =   get_parameters_lemuria,
      .set_parameter =    set_parameter_lemuria,
      .priority =         1,
    },
    .set_callbacks = set_callbacks_lemuria,
    .open_win = open_lemuria,
    .update = update_lemuria,
    .draw_frame = draw_frame_lemuria,

    .show_frame = show_frame_lemuria,
    
    .close = close_lemuria
  };

/* Include this into all plugin modules exactly once
   to let the plugin loader obtain the API version */
BG_GET_PLUGIN_API_VERSION;
