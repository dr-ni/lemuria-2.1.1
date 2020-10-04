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

#include <lemuria_private.h>
#include <effect.h>
#include <utils.h>

#include <light.h>
#include <material.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <particle.h>

#include "images/fountain1.incl"
#include "images/fountain2.incl"
#include "images/fountain3.incl"

#define PHASE_INDEX         0
#define OFFSET_X_INDEX      1
#define OFFSET_Y_INDEX      2
#define OFFSET_Z_INDEX      3
#define OFFSET_FACTOR_INDEX 4
#define DELTA_ANGLE_INDEX   5

#define TICS_PER_BEAT 20
#define NUM_PARTICLES 100

#define MIN_SCALE 0.1
#define MAX_SCALE 0.6

#define MIN_SIZE 0.05
#define MAX_SIZE 0.2

#define TRANSITION_MIN 100
#define TRANSITION_MAX 100

typedef struct
  {
  float phase;
  float offset_x;
  float offset_y;
  float offset_z;
  float offset_factor;
  float delta_angle;
  } particle_data;

// Boundary conditions:
// x(0) = 0.0, dx/dphi(0) = 1
// y(0) = 1.0, dy/dphi(0) = 0
// z(0) = 1.0, dz/dphi(0) = 0

static void get_coords_loop1(float angle,
                             lemuria_particle_t * p,
                             particle_data * d)
  {
  p->x = sin(angle + d->phase);
  p->y = cos(1.0 * (angle + d->phase));
  p->z = cos(2.0 * (angle + d->phase));
  }

static void get_coords_loop2(float angle,
                             lemuria_particle_t * p,
                             particle_data * d)
  {
  p->x = sin(angle + d->phase);
  p->y = cos(2.0 * (angle + d->phase));
  p->z = cos(3.0 * (angle + d->phase));
  }

static void get_coords_loop3(float angle,
                             lemuria_particle_t * p,
                             particle_data * d)
  {
  p->x = sin(angle + d->phase);
  p->y = cos(3.0 * (angle + d->phase));
  p->z = cos(4.0 * (angle + d->phase));
  }

static void get_coords_loop4(float angle,
                             lemuria_particle_t * p,
                             particle_data * d)
  {
  p->x = sin(angle + d->phase);
  p->y = cos(1.0 * (angle + d->phase));
  p->z = cos(3.0 * (angle + d->phase));
  }

static void get_coords_loop5(float angle,
                             lemuria_particle_t * p,
                             particle_data * d)
  {
  p->x = sin(angle + d->phase);
  p->y = cos(1.0 * (angle + d->phase));
  p->z = cos(4.0 * (angle + d->phase));
  }

typedef void(*GET_COORDS_FUNC)(float, lemuria_particle_t*,
                               particle_data * d);

static GET_COORDS_FUNC get_coords_funcs[] =
  {
    get_coords_loop1,
    get_coords_loop2,
    get_coords_loop3,
    get_coords_loop4,
    get_coords_loop5,
  };

static int num_coords_funcs =
sizeof(get_coords_funcs)/sizeof(get_coords_funcs[0]);

static GET_COORDS_FUNC new_coords_func(lemuria_engine_t * e,
                                GET_COORDS_FUNC old)
  {
  int i;
  int old_index;
  int new_index;
  if(old)
    {
    old_index = 0;
    for(i = 0; i < num_coords_funcs; i++)
      {
      if(old == get_coords_funcs[i])
        {
        old_index = i;
        break;
        }
      }
    new_index = lemuria_random_int(e, 0, num_coords_funcs-2);
    if(new_index >= old_index)
      new_index++;
    }
  else
    new_index = lemuria_random(e, 0, num_coords_funcs-1);

  return get_coords_funcs[new_index];
  }

#define SQUARE_OFF      0
#define SQUARE_FADE_IN  1
#define SQUARE_FADE_OUT 2
#define SQUARE_ON       3

typedef struct
  {
  lemuria_particle_system_t * particles;
  particle_data data[NUM_PARTICLES];
  float angle;
  void (*get_coords_func_1)(float angle,
                            lemuria_particle_t * p,
                            particle_data*);

  void (*get_coords_func_2)(float angle,
                            lemuria_particle_t * p,
                            particle_data*);
  
  float scale_start[3];
  float scale_end[3];

  lemuria_range_t scale_range;
  lemuria_rotator_t rotator;

  lemuria_range_t transition_range;
  
  } fountain_data;


static void * init_fountain(lemuria_engine_t * e)
  {
  int i;
  float phi;
  float theta;
  int texture_size = 0;
  char * texture_data = 0;
  int random_number;
  
  fountain_data * ret;
  
#ifdef DEBUG
  fprintf(stderr, "init_fountain\n");
#endif

  ret = calloc(1, sizeof(fountain_data));

  lemuria_range_init(e, &(ret->transition_range), 1,
                     TRANSITION_MIN, TRANSITION_MAX);
  e->foreground.mode = EFFECT_STARTING;


#if 1
  random_number = lemuria_random_int(e, 0, 2);
  //  random_number = 2;
  switch(random_number)
    {
    case 0:
      texture_size = fountain1_width;
      texture_data = fountain1_data;
      break;
    case 1:
      texture_size = fountain2_width;
      texture_data = fountain2_data;
      break;
    case 2:
      texture_size = fountain3_width;
      texture_data = fountain3_data;
      break;
      
    }
#else
  texture_size = fountain1_width;
  texture_data = fountain1_data;
    
#endif
  ret->particles = lemuria_create_particles(e, NUM_PARTICLES,
                                            texture_size,
                                            (unsigned char*)texture_data,
                                            MIN_SIZE,
                                            MAX_SIZE);
  lemuria_rotator_init(e, &(ret->rotator));
  
  for(i = 0; i < NUM_PARTICLES; i++)
    {
    phi = lemuria_random(e, 0.0, 2.0 * M_PI);
    theta = lemuria_random(e, 0.0, M_PI);

    ret->data[i].offset_x = cos(phi) * sin(theta);
    ret->data[i].offset_y = sin(phi) * sin(theta);
    ret->data[i].offset_z =            cos(theta);
    ret->data[i].phase =
      (float)i * 2.0 * M_PI /(float)(NUM_PARTICLES);

    ret->data[i].offset_factor = 0.0;
    ret->data[i].delta_angle = lemuria_random(e, 0.01, 0.03);
    
    if(lemuria_decide(e, 0.5))
      ret->data[i].delta_angle *= -1.0;
    }
  ret->angle = 0.0;
  ret->get_coords_func_1 = new_coords_func(e, NULL);
  ret->get_coords_func_2 = NULL;
  
  ret->scale_start[0] = lemuria_random(e, MIN_SCALE, MAX_SCALE);
  ret->scale_start[1] = lemuria_random(e, MIN_SCALE, MAX_SCALE);
  ret->scale_start[2] = lemuria_random(e, MIN_SCALE, MAX_SCALE);

  ret->scale_end[0] = lemuria_random(e, MIN_SCALE, MAX_SCALE);
  ret->scale_end[1] = lemuria_random(e, MIN_SCALE, MAX_SCALE);
  ret->scale_end[2] = lemuria_random(e, MIN_SCALE, MAX_SCALE);

  lemuria_range_init(e, &(ret->scale_range), 3, 50, 100);


#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif

  return ret;
  }

static void apply_beat(lemuria_engine_t * e,
                       int loudness, fountain_data * d)
  {
  int index = lemuria_random_int(e, 0, NUM_PARTICLES-1);

  d->data[index].offset_factor =
    0.05 + (float)loudness / 32768.0 * lemuria_random(e, 0.1, 0.2);
    
  }

#define SQUARE_Z 0.0

static void draw_fountain(lemuria_engine_t * e, void * user_data)
  {
  int i;

  //  int m, n;
  
  float scale[3];

  float transition_start;
  float transition_end;
  float transition;
  int do_exit;
  float matrix[3][3];
  float transition_z;
  float tmp_x, tmp_y, tmp_z;
      
  fountain_data * d = (fountain_data *)user_data;
  
  // Draw square

  //  if(!lemuria_range_done(&(d->square_range)) || (d->square_alpha > 0.5))

  if(e->foreground.mode == EFFECT_FINISH)
    {
    lemuria_range_init(e, &(d->transition_range), 1,
                       TRANSITION_MIN, TRANSITION_MAX);
    e->foreground.mode = EFFECT_FINISHING;
    }
  
  // Do particles
  
  d->angle += 0.015;
  if(d->angle > 2.0* M_PI)
    {
    d->angle -= 2.0* M_PI;
    if(d->get_coords_func_2)
      {
      d->get_coords_func_1 = d->get_coords_func_2;
      d->get_coords_func_2 = NULL;
      }
    else if(lemuria_decide(e, 0.8))
      {
      //      fprintf(stderr, "Changing program");
      d->get_coords_func_2 = new_coords_func(e, d->get_coords_func_1);
      }
    }
  
  if(e->beat_detected)
    {
    for(i = 0; i < TICS_PER_BEAT; i++)
      apply_beat(e, e->loudness, d);

    if(lemuria_range_done(&(d->scale_range)) && lemuria_decide(e, 0.1))
      {
      memcpy(d->scale_start, d->scale_end, 3 * sizeof(float));
      d->scale_end[0] = lemuria_random(e, MIN_SCALE, MAX_SCALE);
      d->scale_end[1] = lemuria_random(e, MIN_SCALE, MAX_SCALE);
      d->scale_end[2] = lemuria_random(e, MIN_SCALE, MAX_SCALE);
      lemuria_range_init(e, &(d->scale_range), 3, 50, 100);
      }
    if(lemuria_decide(e, 0.1))
      lemuria_rotator_change(e, &(d->rotator));

    }

  lemuria_rotator_update(&(d->rotator));

  lemuria_rotate_get_matrix(&(d->rotator), matrix);
  
  lemuria_range_update(&(d->scale_range));
  lemuria_range_get(&(d->scale_range),
                    d->scale_start,
                    d->scale_end, scale);
  
  for(i = 0; i < NUM_PARTICLES; i++)
    {
    // Check, if we have a get_coords_func_2

    if(d->get_coords_func_2 &&
       (d->data[i].phase > 2.0 * M_PI - d->angle))
      d->get_coords_func_2(d->angle,
                           &(d->particles->particles[i]),
                           &(d->data[i]));
    else
      d->get_coords_func_1(d->angle,
                           &(d->particles->particles[i]),
                           &(d->data[i]));

    d->particles->particles[i].angle += d->data[i].delta_angle;
    if(d->particles->particles[i].angle > 2.0 * M_PI)
      d->particles->particles[i].angle -= 2.0 * M_PI;
    else if(d->particles->particles[i].angle < 0.0)
      d->particles->particles[i].angle += 2.0 * M_PI;
    
    d->particles->particles[i].x *= scale[0];
    d->particles->particles[i].y *= scale[1];
    d->particles->particles[i].z *= scale[2];
    
    d->particles->particles[i].x +=
      d->data[i].offset_factor * d->data[i].offset_x;
    d->particles->particles[i].y +=
      d->data[i].offset_factor * d->data[i].offset_y;

    d->particles->particles[i].z +=
      d->data[i].offset_factor * d->data[i].offset_z;
    d->data[i].offset_factor *= 0.98;

    // Rotate the particles

    tmp_x = d->particles->particles[i].x;
    tmp_y = d->particles->particles[i].y;
    tmp_z = d->particles->particles[i].z;
    
    d->particles->particles[i].x =
      matrix[0][0] * tmp_x + matrix[0][1] * tmp_y + matrix[0][2] * tmp_z;

    d->particles->particles[i].y =
      matrix[1][0] * tmp_x + matrix[1][1] * tmp_y + matrix[1][2] * tmp_z;

    d->particles->particles[i].z =
      matrix[2][0] * tmp_x + matrix[2][1] * tmp_y + matrix[2][2] * tmp_z;
    
    }

  do_exit = 0;
  
  if(e->foreground.mode == EFFECT_FINISHING)
    {
    if(lemuria_range_done(&(d->transition_range)))
      do_exit = 1;
    
    transition_start = 0.0;
    transition_end = 0.5 * M_PI;
    
    lemuria_range_update(&(d->transition_range));
    lemuria_range_get(&(d->transition_range),
                      &transition_start,
                      &transition_end,
                      &transition);

    transition_z = 5.0 * (1.0 - cos(transition));
        
    for(i = 0; i < NUM_PARTICLES; i++)
      d->particles->particles[i].z += transition_z;
    
    }
  else if(e->foreground.mode == EFFECT_STARTING)
    {
    transition_end = 0.0;
    transition_start = 0.5 * M_PI;
    lemuria_range_update(&(d->transition_range));
    lemuria_range_get(&(d->transition_range),
                      &transition_start,
                      &transition_end,
                      &transition);
    transition_z = 5.0 * (1.0 - cos(transition));
    for(i = 0; i < NUM_PARTICLES; i++)
      {
      d->particles->particles[i].z += transition_z;
      }
    if(lemuria_range_done(&(d->transition_range)))
      e->foreground.mode = EFFECT_RUNNING;
    }
  lemuria_draw_particles(d->particles);
  if(do_exit)
    e->foreground.mode = EFFECT_DONE;
  }

static void delete_fountain(void * user_data)
  {
  fountain_data * d = (fountain_data *)user_data;
  lemuria_destroy_particles(d->particles);
  free(d);
  }

effect_plugin_t fountain_effect =
  {
    .init =    init_fountain,
    .draw =    draw_fountain,
    .cleanup = delete_fountain,
  };
