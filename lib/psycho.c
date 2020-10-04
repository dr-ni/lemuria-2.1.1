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
#include <stdio.h>

#include <stdlib.h>
#include <lemuria_private.h>
#include <utils.h>
#include <object.h>
#include <effect.h>

#include "gradients/lemuria_psycho_1.incl"
#include "gradients/lemuria_psycho_2.incl"
#include "gradients/lemuria_psycho_3.incl"
#include "gradients/lemuria_psycho_4.incl"
#include "gradients/lemuria_psycho_5.incl"

#define MAX_Y 1.25
#define MAX_X ((MAX_Y*4.0)/3.0)

#define BG_SPEED_MIN 0.005
#define BG_SPEED_MAX 0.01

static float bg_coords[4][3] =
  {
    { -MAX_X, -MAX_Y, 0.0 },
    {  MAX_X, -MAX_Y, 0.0 },
    {  MAX_X,  MAX_Y, 0.0 },
    { -MAX_X,  MAX_Y, 0.0 }
  };

#define MAX_OBJECTS 2

/* Object directions */

#define DIR_LEFT_RIGHT 0
#define DIR_RIGHT_LEFT 1
#define DIR_BOTTOM_TOP 2
#define DIR_TOP_BOTTOM 3
#define DIR_NEAR_FAR   4
#define DIR_FAR_NEAR   5

#define Z_MIN -30.0
#define Z_MAX -5.0

#define X_MAX(z) (CAMERA_Z - z)/CAMERA_Z*(MAX_X+1.0)
#define Y_MAX(z) (CAMERA_Z - z)/CAMERA_Z*(MAX_Y+1.0)

static lemuria_background_data_t backgrounds[] =
  {
    {
      .texture_mode = LEMURIA_TEXTURE_CLOUDS_ROTATE,
      gradient_lemuria_psycho_1,
      clouds_size : 0,
    },
    {
      .texture_mode = LEMURIA_TEXTURE_CLOUDS_ROTATE,
      gradient_lemuria_psycho_2,
      clouds_size : 0,
    },
    {
      .texture_mode = LEMURIA_TEXTURE_CLOUDS_ROTATE,
      gradient_lemuria_psycho_3,
      clouds_size : 0,
    },
    {
      .texture_mode = LEMURIA_TEXTURE_CLOUDS_ROTATE,
      gradient_lemuria_psycho_4,
      clouds_size : 0,
    },
    {
      .texture_mode = LEMURIA_TEXTURE_CLOUDS_ROTATE,
      gradient_lemuria_psycho_5,
      clouds_size : 0,
    },
  };

typedef struct
  {
  lemuria_background_t bg;

  float bg_texture_start;
  int bg_texture_flip;
  int num_objects;
  float bg_speed_start;
  float bg_speed_end;
  lemuria_range_t bg_speed_range;

  struct
    {
    lemuria_object_t obj;
    int direction;
    } objects[MAX_OBJECTS];
  
  } psycho_data;

static void * init_psycho(lemuria_engine_t * e)
  {
  psycho_data * d;
  int random_number;
    
  d = calloc(1, sizeof(*d));
  d->bg_speed_start = lemuria_random(e, BG_SPEED_MIN, BG_SPEED_MAX);
  if(lemuria_decide(e, 0.5))
    d->bg_speed_start *= -1.0;
  d->bg_speed_end = d->bg_speed_start;

  random_number = lemuria_random_int(e, 0, sizeof(backgrounds)/sizeof(backgrounds[0])-1);
  //  random_number = 4;
      
  lemuria_background_init(e, &(d->bg), &backgrounds[random_number]);
  lemuria_range_init(e, &(d->bg_speed_range), 1, 10, 20);
  return d;
  }

static void object_init(lemuria_engine_t * e,
                        psycho_data * d, int index)
  {
  int random_number;
  int type = 0;
  float speed = 0.0;
  float speed_factor;
  float coords[3];
  float delta_coords[3];
  float start_coords_factor = 1.0;
    
  random_number = lemuria_random_int(e, 0, 3);
  //  random_number = 2;
  switch(random_number)
    {
    case 0:
      speed = lemuria_random(e, 0.1, 0.2);
      type = LEMURIA_OBJECT_TEAPOT;
      break;
    case 1:
      speed = lemuria_random(e, 0.1, 0.3);
      type = LEMURIA_OBJECT_FISH;
      break;
    case 2:
      speed = lemuria_random(e, 0.1, 0.2);
      type = LEMURIA_OBJECT_MANTA;
      break;
    case 3:
      speed = lemuria_random(e, 0.1, 0.2);
      type = LEMURIA_OBJECT_PLATON;
      start_coords_factor = 2.0;
      break;
    }

  d->objects[index].direction = lemuria_random_int(e, 0, 3);
  //  d->objects[index].direction = DIR_TOP_BOTTOM;
  
  switch(d->objects[index].direction)
    {
    case DIR_LEFT_RIGHT:
      coords[2] = lemuria_random(e, Z_MIN, Z_MAX);  
      coords[0] = -X_MAX(coords[2]) * start_coords_factor;
      coords[1] = lemuria_random(e, -Y_MAX(coords[2]), Y_MAX(coords[2]));
            
      speed_factor = (CAMERA_Z - coords[2])/CAMERA_Z * 0.1;
      
      //      coords[1] = 0;
      delta_coords[0] = speed * speed_factor;
      delta_coords[1] = 0.0;
      delta_coords[2] = 0.0;
      
      break;
    case DIR_RIGHT_LEFT:
      coords[2] = lemuria_random(e, Z_MIN, Z_MAX);  
      coords[0] = X_MAX(coords[2]) * start_coords_factor;
      coords[1] = lemuria_random(e, -Y_MAX(coords[2]), Y_MAX(coords[2]));

      speed_factor = (CAMERA_Z - coords[2])/CAMERA_Z * 0.2;

      delta_coords[0] = -speed * speed_factor;
      delta_coords[1] = 0.0;
      delta_coords[2] = 0.0;
      
      break;
    case DIR_BOTTOM_TOP:
      coords[2] = lemuria_random(e, Z_MIN, Z_MAX);  
      coords[1] = -Y_MAX(coords[2]) * start_coords_factor;
      coords[0] = lemuria_random(e, -X_MAX(coords[2]), X_MAX(coords[2]));
      
      speed_factor = (CAMERA_Z - coords[2])/CAMERA_Z * 0.2;

      delta_coords[1] = speed * speed_factor;
      delta_coords[0] = 0.0;
      delta_coords[2] = 0.0;
      break;
    case DIR_TOP_BOTTOM:
      coords[2] = lemuria_random(e, Z_MIN, Z_MAX);  
      coords[1] = Y_MAX(coords[2]) * start_coords_factor;
      coords[0] = lemuria_random(e, -X_MAX(coords[2]), X_MAX(coords[2]));

      speed_factor = (CAMERA_Z - coords[2])/CAMERA_Z * 0.2;
      
      delta_coords[1] = -speed * speed_factor;
      delta_coords[0] = 0.0;
      delta_coords[2] = 0.0;
      break;
    case DIR_NEAR_FAR:
      break;
    case DIR_FAR_NEAR:
      break;
    }
  
  lemuria_object_init(e, &(d->objects[index].obj),
                      type, coords, delta_coords);
  //  fprintf(stderr, "Object_init\n");
  //  fprintf(stderr, "Coords: %f %f %f\n", coords[0], coords[1], coords[2]);
  //  fprintf(stderr, "delta Coords: %f %f %f\n",
  //          delta_coords[0], delta_coords[1], delta_coords[2]);
  }

static int object_done(psycho_data * d, int index)
  {
#if 0
  fprintf(stderr, "Coords 1: %f %f %f\n",
          d->objects[index].obj.coords[0],
          d->objects[index].obj.coords[1],
          d->objects[index].obj.coords[2]);
#endif
  switch(d->objects[index].direction)
    {
    case DIR_LEFT_RIGHT:
      if(d->objects[index].obj.coords[0] >
         X_MAX(d->objects[index].obj.coords[2]))
        return 1;
      break;
    case DIR_RIGHT_LEFT:
      if(d->objects[index].obj.coords[0] <
         -X_MAX(d->objects[index].obj.coords[2]))
        return 1;
      break;
    case DIR_BOTTOM_TOP:
      if(d->objects[index].obj.coords[1] >
         Y_MAX(d->objects[index].obj.coords[2]))
        return 1;
      break;
    case DIR_TOP_BOTTOM:
      if(d->objects[index].obj.coords[1] <
         -Y_MAX(d->objects[index].obj.coords[2]))
        return 1;
      break;
    case DIR_NEAR_FAR:
      break;
    case DIR_FAR_NEAR:
      break;
    }
  return 0;
  }



static void draw_psycho(lemuria_engine_t * e, void * data)
  {
  psycho_data * d;
  int i, j, new_object = 0;
  float bg_speed;
  float coords[4][3];
  d = (psycho_data *)data;

  /* Calculate ccorinates */

  for(i = 0; i < 4; i++)
    for(j = 0; j < 3; j++)
      coords[i][j] = 
        bg_coords[i][j] * 
        ((float)(e->width)/(float)(e->height))/(4.0/3.0);

  lemuria_background_update(&(d->bg));
  
  glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  lemuria_set_perspective(e, 1, 1000.0);

  if(e->beat_detected &&
     lemuria_range_done(&(d->bg_speed_range)) &&
     lemuria_decide(e, 0.2))
    {
    d->bg_speed_start = d->bg_speed_end;
    d->bg_speed_end = lemuria_random(e, BG_SPEED_MIN, BG_SPEED_MAX);
    if(lemuria_decide(e, 0.5))
      d->bg_speed_end *= -1.0;
    lemuria_range_init(e, &(d->bg_speed_range), 1, 100, 200);
                                                                                
    }
  lemuria_range_update(&(d->bg_speed_range));
  lemuria_range_get_cos(&(d->bg_speed_range),
                    &(d->bg_speed_start),
                    &(d->bg_speed_end),
                    &(bg_speed));


  lemuria_background_set(&(d->bg));
  lemuria_background_draw(&(d->bg),
                          coords,
                          1,
                          1,
                          &(d->bg_texture_start),
                          bg_speed,
                          &(d->bg_texture_flip));
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glDisable(GL_TEXTURE_2D);
  glClear(GL_DEPTH_BUFFER_BIT);

  /* Check wether to enable an object */

  if((d->num_objects < MAX_OBJECTS) && e->beat_detected)
    {
    new_object = 1;
    }

  if(new_object && !d->num_objects)
    {
    object_init(e, d, 0);
    //    fprintf(stderr, "New Object\n");
    d->num_objects++;
    new_object = 0;
    }
  if(d->num_objects)
    {
    for(i = 0; i < MAX_OBJECTS; i++)
      {
      if(d->objects[i].obj.draw)
        {
        lemuria_object_update(e, &(d->objects[i].obj));

        if(object_done(d, i))
          {
          lemuria_object_cleanup(&(d->objects[i].obj));
          d->num_objects--;
          //          fprintf(stderr, "Object done \n");
          }
        }
      else if(new_object)
        {
        object_init(e, d, i);
        d->num_objects++;
        new_object = 0;
        }

      if(d->objects[i].obj.draw)
        {
        lemuria_object_draw(e, &(d->objects[i].obj));
        }
      }
    }
  }

static void delete_psycho(void * data)
  {
  int i;
  psycho_data * d = (psycho_data*)(data);
  lemuria_background_delete(&(d->bg));

  for(i = 0; i < MAX_OBJECTS; i++)
    {
    if(d->objects[i].obj.draw)
      lemuria_object_cleanup(&(d->objects[i].obj));
    }
  
  free(d);
  }

effect_plugin_t psycho_effect =
  {
    .init =    init_psycho,
    .draw =    draw_psycho,
    .cleanup = delete_psycho,
  };
