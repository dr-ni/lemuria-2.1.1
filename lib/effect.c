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

#include <inttypes.h>
#include <stdio.h>

#include <lemuria_private.h>
#include <effect.h>
#include <utils.h>

#define MIN_TEXTURE_FRAMES     100
#define MIN_FOREGROUND_FRAMES 2000
//#define MIN_FOREGROUND_FRAMES 500
#define MIN_BACKGROUND_FRAMES 2000

#define FLAG_NEXT             (1<<0)

// #define TEST_MODE

/* Texture effects */

extern effect_plugin_t oscilloscope_effect;
extern effect_plugin_t blur_effect;
extern effect_plugin_t vectorscope_effect;

/* Foreground effects */

extern effect_plugin_t cube_effect;
extern effect_plugin_t tube_effect;
extern effect_plugin_t bubbles_effect;
extern effect_plugin_t icosphere_effect;
extern effect_plugin_t squares_effect;
extern effect_plugin_t fountain_effect;

extern effect_plugin_t oszi3d_effect;
extern effect_plugin_t superellipse_effect;
extern effect_plugin_t hyperbubble_effect;

extern effect_plugin_t swarm_effect;

extern effect_plugin_t tentacle_effect;

/* Background effects */

extern effect_plugin_t psycho_effect;
extern effect_plugin_t boioioing_effect;
extern effect_plugin_t starfield_effect;
extern effect_plugin_t lines_effect;
extern effect_plugin_t monolith_effect;
extern effect_plugin_t archaic_effect;
extern effect_plugin_t tunnel_effect;
extern effect_plugin_t mountains_effect;
extern effect_plugin_t drive_effect;

extern effect_plugin_t deepsea_effect;

extern effect_plugin_t xaos_effect;
extern effect_plugin_t lineobjects_effect;


// extern effect_plugin_t v4l_effect;


//extern effect_plugin_t test_effect;

typedef struct
  {
  effect_plugin_t * effect;
  const char * name;
  const char * label;
  } effect_info;

static effect_info foreground_effects[] = 
  {
    { &bubbles_effect, "bubbles", "Bubbles" },
    { &cube_effect,    "cube",  "Cube" },
    { &fountain_effect, "fountain", "Starchain" },
//    &tentacle_effect,
    { &oszi3d_effect,   "oszi3d", "3D Oscilloscope" },
    { &swarm_effect,    "swarm", "Swarm" },
    { &hyperbubble_effect, "hyperbubble", "Hyperbubble" },
    { &tube_effect,        "tube", "Tube" },
    { &icosphere_effect,   "icosphere", "Icosphere" },
    { &squares_effect,     "squares", "Squares" },
    { &superellipse_effect, "superellipse", "Superellipse" },
  };

static effect_info background_effects[] = 
  {
//    { &xaos_effect, "xaos", "XaoS" },
    { &mountains_effect, "mountains", "Mountains" },
    { &drive_effect, "drive", "Drive" },
    { &deepsea_effect, "deepsea", "Deep sea" },
    { &lineobjects_effect, "lineobjects", "Line objects" },
    //    &v4l_effect,
    { &boioioing_effect, "boioioing", "Boioioing" },
    { &psycho_effect, "psycho", "Psycho" },
    { &starfield_effect, "starfield", "Starfield" },
    { &monolith_effect, "Monolith", "monolith" },

#if 1
    //    &drive_effect,
    { &tunnel_effect, "tunnel", "Tunnel" },
    { &lines_effect, "lines", "Lines" },
    { &archaic_effect, "archaic", "Archaic" }
#endif
  };

static effect_info texture_effects[] = 
  {
    { &blur_effect, "blur", "Blobs" },
    { &oscilloscope_effect, "oscilloscope", "Oscilloscope" },
    { &vectorscope_effect, "vectorsope", "Vectorsope" }
  };

static int num_foreground_effects =
sizeof(foreground_effects)/sizeof(foreground_effects[0]);

static int num_background_effects =
sizeof(background_effects)/sizeof(background_effects[0]);

static int num_texture_effects =
sizeof(texture_effects)/sizeof(texture_effects[0]);

static void manage_effect(lemuria_engine_t * e,
                          lemuria_effect_t * effect,
                          effect_info * plugins,
                          int num_plugins,
                          int min_frames,
                          float probability)
  {
  
  /* Load effect if there wasn't one */
  //  fprintf(stderr, 
  
  if(!effect->effect)
    {
#ifndef TEST_MODE
    effect->index = lemuria_random_int(e, 0, num_plugins-1);
#else
    effect->index = 0;
#endif
    effect->effect = plugins[effect->index].effect;
    effect->mode = EFFECT_RUNNING;
    effect->frame_counter = 0;
    if(effect->effect->init)
      effect->data = effect->effect->init(e);
    else
      fprintf(stderr, "FATAL: Plugin has no init function\n");
    return;
    }
  
  /* Kick out obsolete effects */

  else if((effect->mode == EFFECT_DONE) || (effect->mode == EFFECT_FINISH))
    {
    effect->effect->cleanup(effect->data);
    effect->index = effect->next_index;
    effect->effect = plugins[effect->index].effect;
    effect->mode = EFFECT_RUNNING;
    if(effect->frame_counter >= 0)
      effect->frame_counter = 0;
    effect->data = effect->effect->init(e);
    }

  /* Check wether we should change the effects */

  else if((effect->mode == EFFECT_RUNNING) &&
          e->beat_detected &&
          (effect->frame_counter > min_frames) &&
          (lemuria_decide(e, probability)))
    {
    effect->mode = EFFECT_FINISH;
    effect->next_index = lemuria_random_int(e, 0, num_plugins-2);
    if(effect->next_index >= effect->index)
      effect->next_index++;

    }
  else if(effect->frame_counter >= 0)
    effect->frame_counter++;
  }

void lemuria_manage_effects(lemuria_engine_t * e)
  {
  manage_effect(e, &(e->background),
                background_effects,
                num_background_effects,
                MIN_BACKGROUND_FRAMES,
                0.02);
  
  manage_effect(e, &(e->foreground),
                foreground_effects,
                num_foreground_effects,
                MIN_FOREGROUND_FRAMES,
                0.02);
  
  manage_effect(e, &(e->texture),
                texture_effects,
                num_texture_effects,
                MIN_TEXTURE_FRAMES,
                0.4);
  }

int lemuria_num_effects(lemuria_engine_t * l, int type)
  {
  switch(type)
    {
    case LEMURIA_EFFECT_BACKGROUND:
      return num_background_effects;
      break;
    case LEMURIA_EFFECT_FOREGROUND:
      return num_foreground_effects;
      break;
    case LEMURIA_EFFECT_TEXTURE:
      return num_texture_effects;
      break;
    }
  return 0;
  }

const char * lemuria_effect_name(lemuria_engine_t * l, int type, int index)
  {
  switch(type)
    {
    case LEMURIA_EFFECT_BACKGROUND:
      return background_effects[index].name;
      break;
    case LEMURIA_EFFECT_FOREGROUND:
      return foreground_effects[index].name;
      break;
    case LEMURIA_EFFECT_TEXTURE:
      return texture_effects[index].name;
      break;
    }
  return (const char*)0;
  }

const char * lemuria_effect_label(lemuria_engine_t * l, int type, int index)
  {
  switch(type)
    {
    case LEMURIA_EFFECT_BACKGROUND:
      return background_effects[index].label;
      break;
    case LEMURIA_EFFECT_FOREGROUND:
      return foreground_effects[index].label;
      break;
    case LEMURIA_EFFECT_TEXTURE:
      return texture_effects[index].label;
      break;
    }
  return (const char*)0;
  }

void lemuria_change_effect(lemuria_engine_t * l, int type)
  {
  lemuria_effect_t * e;
  int m;
  switch(type)
    {
    case LEMURIA_EFFECT_BACKGROUND:
      e = &l->background;
      m = num_background_effects;
      break;
    case LEMURIA_EFFECT_FOREGROUND:
      e = &l->foreground;
      m = num_foreground_effects;
      break;
    case LEMURIA_EFFECT_TEXTURE:
      e = &l->texture;
      m = num_texture_effects;
      break;
    default:
      return;
    }

  if(e->mode == EFFECT_RUNNING)
    {
    e->mode = EFFECT_FINISH;
    e->next_index = lemuria_random_int(l, 0, m-2);
    if(e->next_index >= e->index)
      e->next_index++;
    }

  }

void lemuria_set_effect(lemuria_engine_t * l, int type, int index)
  {
  lemuria_effect_t * e;
  int m;
  switch(type)
    {
    case LEMURIA_EFFECT_BACKGROUND:
      e = &l->background;
      m = num_background_effects;
      break;
    case LEMURIA_EFFECT_FOREGROUND:
      e = &l->foreground;
      m = num_foreground_effects;
      break;
    case LEMURIA_EFFECT_TEXTURE:
      e = &l->texture;
      m = num_texture_effects;
      break;
    default:
      return;
    }
  if((index >= 0) && (index < m))
    {
    if(e->mode == EFFECT_RUNNING)
      {
      e->mode = EFFECT_FINISH;
      e->next_index = index;
      }
    }
  }

void lemuria_next_effect(lemuria_engine_t * l, int type)
  {
  lemuria_effect_t * e;
  int m;
  switch(type)
    {
    case LEMURIA_EFFECT_BACKGROUND:
      e = &l->background;
      m = num_background_effects;
      break;
    case LEMURIA_EFFECT_FOREGROUND:
      e = &l->foreground;
      m = num_foreground_effects;
      break;
    case LEMURIA_EFFECT_TEXTURE:
      e = &l->texture;
      m = num_texture_effects;
      break;
    default:
      return;
    }
  if(e->mode == EFFECT_RUNNING)
    {
    e->mode = EFFECT_FINISH;
    e->next_index++;
    if(e->next_index >= m)
      e->next_index = 0;
    }
  }
