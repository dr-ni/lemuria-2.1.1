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

#include <stdlib.h>
#include <stdio.h>

#include <lemuria_private.h>
#include <effect.h>
#include <utils.h>
#include <object.h>

#include "gradients/lemuria_deepsea_ceiling_1.incl"
#include "gradients/lemuria_deepsea_ceiling_2.incl"

#include "gradients/lemuria_deepsea_floor_1.incl"

#define MAX_OBJECTS     6
#define MAX_TEAPOTS     1 /* Teapots need a lot of CPU */

#define CEILING_HEIGHT   10.0
#define FLOOR_HEIGHT    -10.0

#define Z_MIN          -250.0
#define Z_MAX             0.0
#define X_MAX           200.0

#define CEILING_TEXTURE_SIZE_X 6
#define CEILING_TEXTURE_SIZE_Y 4

#define FLOOR_TEXTURE_SIZE_X 6
#define FLOOR_TEXTURE_SIZE_Y 4

// #define WORLD_ADVANCE 0.005

// #define WORLD_SPEED (WORLD_ADVANCE * (Z_MAX - Z_MIN) / FLOOR_TEXTURE_SIZE_Y)

#define WORLD_SPEED 0.625

#define FLOOR_ADVANCE   ((WORLD_SPEED * FLOOR_TEXTURE_SIZE_Y)/(Z_MAX - Z_MIN))
#define CEILING_ADVANCE ((WORLD_SPEED * CEILING_TEXTURE_SIZE_Y)/(Z_MAX - Z_MIN))

static float ceiling_coords[4][3] =
  {
    { -X_MAX, CEILING_HEIGHT, Z_MIN },
    {  X_MAX, CEILING_HEIGHT, Z_MIN },
    {  X_MAX, CEILING_HEIGHT, Z_MAX },
    { -X_MAX, CEILING_HEIGHT, Z_MAX },
  };

static float floor_coords[4][3] =
  {
    { -X_MAX, FLOOR_HEIGHT, Z_MIN },
    {  X_MAX, FLOOR_HEIGHT, Z_MIN },
    {  X_MAX, FLOOR_HEIGHT, Z_MAX },
    { -X_MAX, FLOOR_HEIGHT, Z_MAX },
  };

static struct
  {
  float fog_color[4];
  lemuria_background_data_t ceiling;
  lemuria_background_data_t floor;
  }
worlds[] =
  {
    {
      .fog_color = { 0.0, 0.0, 0.5, 1.0 },
      .ceiling =
      {
        .texture_mode =    LEMURIA_TEXTURE_CLOUDS_ROTATE,
        .clouds_gradient = gradient_lemuria_deepsea_ceiling_1,
        .clouds_size =     0,
      },
    
      .floor =
      {
        .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
        .clouds_gradient = gradient_lemuria_deepsea_floor_1,
        .clouds_size =     0,
      }
    },
    {
      .fog_color = { 0.6, 0.0, 0.6, 1.0 },
      .ceiling =
      {
        .texture_mode =    LEMURIA_TEXTURE_CLOUDS_ROTATE,
        .clouds_gradient = gradient_lemuria_deepsea_ceiling_2,
        .clouds_size =     0,
      },
    
      .floor =
      {
        .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
        .clouds_gradient = gradient_lemuria_deepsea_floor_1,
        .clouds_size =     0,
      }
    },
  };

#define NUM_WORLDS (sizeof(worlds)/sizeof(worlds[0]))

typedef struct
  {
  lemuria_background_t ceiling;
  float ceiling_texture_offset_y;

  lemuria_background_t floor;
  float floor_texture_offset_y;

  lemuria_object_t objects[MAX_OBJECTS];

  int world_index;

  int num_teapots;
  
  } deepsea_data;

/* Initialize object */

static void init_object(lemuria_engine_t * e, deepsea_data * d)
  {
  float coords[3];
  float delta_coords[3];
  int index, random_number;
  lemuria_object_type_t type = 0;

  delta_coords[0] = 0.0;
  delta_coords[1] = 0.0;
  
  for(index = 0; index < MAX_OBJECTS; index++)
    {
    if(!d->objects[index].draw)
      break;
    }
  if(index == MAX_OBJECTS)
    return;

  if(d->num_teapots < MAX_TEAPOTS)
    random_number = lemuria_random_int(e, 0, 4);
  else
    random_number = lemuria_random_int(e, 1, 4);

  //  random_number = 4;
  
  switch(random_number)
    {
    case 0:
      type = LEMURIA_OBJECT_TEAPOT_STATIC;
      coords[0] = lemuria_random(e, -0.1 * X_MAX, 0.1 * X_MAX);
      coords[1] = FLOOR_HEIGHT;
      coords[2] = Z_MIN;
      delta_coords[2] = WORLD_SPEED;
      d->num_teapots++;
      break;
    case 1:
      type = LEMURIA_OBJECT_BUBBLES;
      coords[0] = lemuria_random(e, -0.1 * X_MAX, 0.1 * X_MAX);
      coords[1] = FLOOR_HEIGHT;
      coords[2] = Z_MIN;
      delta_coords[2] = WORLD_SPEED;
      break;
      
    case 2:
      type = LEMURIA_OBJECT_MANTA;
      
      coords[0] = lemuria_random(e, -0.3 * X_MAX, 0.3 * X_MAX);
      coords[1] = lemuria_random(e, 0.5 * FLOOR_HEIGHT, 0.5 * CEILING_HEIGHT);
      
      if(lemuria_decide(e, 0.5))
        {
        coords[2] = Z_MIN;
        delta_coords[2] = 0.6;
        }
      else
        {
        coords[2] = Z_MAX;
        delta_coords[2] = -0.3;
        }
      
      break;
    case 3:
      type = LEMURIA_OBJECT_FISH;

      coords[0] = lemuria_random(e, -0.3 * X_MAX, 0.3 * X_MAX);
      coords[1] = lemuria_random(e, 0.5 * FLOOR_HEIGHT, 0.5 * CEILING_HEIGHT);
      
      if(lemuria_decide(e, 0.5))
        {
        coords[2] = Z_MIN;
        delta_coords[2] = 0.6;
        }
      else
        {
        coords[2] = Z_MAX;
        delta_coords[2] = -0.3;
        }
      break;
    case 4:
      type = LEMURIA_OBJECT_SEAPLANT;
      coords[0] = lemuria_random(e, -0.1 * X_MAX, 0.1 * X_MAX);
      coords[1] = FLOOR_HEIGHT;
      coords[2] = Z_MIN;
      delta_coords[2] = WORLD_SPEED;
      break;

    }
  
  lemuria_object_init(e, &(d->objects[index]), type, coords, delta_coords);
  }

/* Update all objects */

static void draw_objects(lemuria_engine_t * e, deepsea_data * d)
  {
  int i;
  for(i = 0; i < MAX_OBJECTS; i++)
    {
    if(d->objects[i].draw)
      {
      lemuria_object_update(e, &(d->objects[i]));
        if((d->objects[i].coords[2] < Z_MIN) ||
           (d->objects[i].coords[2] > Z_MAX))
          {
          if(d->objects[i].type == LEMURIA_OBJECT_TEAPOT_STATIC)
            d->num_teapots--;
          lemuria_object_cleanup(&(d->objects[i]));
          
          }
      }
    
    if(d->objects[i].draw)
      lemuria_object_draw(e, &d->objects[i]);
    }
  }

static void * init_deepsea(lemuria_engine_t * e)
  {
  deepsea_data * d;
      
#ifdef DEBUG
  fprintf(stderr, "init_deepsea...");
#endif

  d = calloc(1, sizeof(deepsea_data));

  d->world_index = lemuria_random_int(e, 0, NUM_WORLDS-1);
  //  d->world_index = 1;
  
  lemuria_background_init(e, &(d->ceiling),
                          &(worlds[d->world_index].ceiling));

  lemuria_background_init(e, &(d->floor),
                          &(worlds[d->world_index].floor));
    
#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  
  return d;
  }

static void draw_deepsea(lemuria_engine_t * e, void * user_data)
  {
  int flip_y = 0;
  deepsea_data * d = (deepsea_data*)(user_data);

  if(e->beat_detected && lemuria_decide(e, 0.5))
    init_object(e, d);
  
  lemuria_background_update(&(d->ceiling));
  
  glClearColor(worlds[d->world_index].fog_color[0],
               worlds[d->world_index].fog_color[1],
               worlds[d->world_index].fog_color[2],
               worlds[d->world_index].fog_color[3]);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  lemuria_set_perspective(e, 1, 1000.0);

  /* Fog */
#if 0
  glFogf(GL_FOG_START, -0.1 * Z_MIN);
  glFogf(GL_FOG_END,   -Z_MIN);
  glFogi(GL_FOG_MODE, GL_LINEAR );
#else
  glFogf(GL_FOG_DENSITY, 1.0/50.0);
  glFogi(GL_FOG_MODE, GL_EXP);
#endif
  
  glFogfv(GL_FOG_COLOR, worlds[d->world_index].fog_color);
  glEnable(GL_FOG);
  
  
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_2D);
  
  lemuria_background_set(&(d->ceiling));
  lemuria_background_draw(&(d->ceiling),
                          ceiling_coords,
                          CEILING_TEXTURE_SIZE_X,
                          CEILING_TEXTURE_SIZE_Y,
                          &(d->ceiling_texture_offset_y),
                          -CEILING_ADVANCE, &flip_y);
  flip_y = 0;
  
  lemuria_background_set(&(d->floor));
  lemuria_background_draw(&(d->floor),
                          floor_coords,
                          FLOOR_TEXTURE_SIZE_X,
                          FLOOR_TEXTURE_SIZE_Y,
                          &(d->floor_texture_offset_y),
                          -FLOOR_ADVANCE, &flip_y);
  
  glDisable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  draw_objects(e, d);
  
  glDisable(GL_FOG);

  
  }

static void delete_deepsea(void * data)
  {
  int i;
  deepsea_data * d = (deepsea_data*)(data);
  
  for(i = 0; i < MAX_OBJECTS; i++)
    {
    if(d->objects[i].draw)
      lemuria_object_cleanup(&(d->objects[i]));
    }
  
  free(d);
  }



effect_plugin_t deepsea_effect =
  {
    .init =    init_deepsea,
    .draw =    draw_deepsea,
    .cleanup = delete_deepsea,
  };
