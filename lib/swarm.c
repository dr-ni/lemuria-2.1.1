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
#include <math.h>

/* Square root of the number of fishes */

#define SQR_NUM 30
#define NUM     (SQR_NUM * SQR_NUM)

#define FISH_HALF_LENGTH 0.03
#define FISH_HALF_WIDTH  0.015

#define AXIS_MIN 0.3
#define AXIS_MAX 0.7

// #define DRAW_ATTRACTOR

typedef struct
  {
  float coords[3];
  float rot[3];

  float d_coords[3];
  float d_rot[3];
  
  float texture_coords[4][2];
  } fish_t;

typedef struct
  {
  fish_t fishes[SQR_NUM*SQR_NUM];

  lemuria_rotator_t rotator;
  lemuria_range_t   axis_range;

  float axis_start;
  float axis_end;

  float attractor_phi;
  int frames_since_boing;

  lemuria_engine_t * engine;
  } swarm_data;

static lemuria_light_t light =
  {
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .position = { 30.0f, 30.0f, 10.0f, 1.0f },
  };

static lemuria_material_t material =
  {
    .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_ambient =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 50
  };


static void draw_fish(fish_t * fish)
  {
  glPushMatrix();

  glTranslatef(fish->coords[0], fish->coords[1], fish->coords[2]);
  glRotatef(fish->rot[0] * 180.0 / M_PI, 1.0, 0.0, 0.0);
  glRotatef(fish->rot[1] * 180.0 / M_PI, 0.0, 1.0, 0.0);
  glRotatef(fish->rot[2] * 180.0 / M_PI, 0.0, 0.0, 1.0);

  /* The fish lies in the x-y plane and points into positive x direction */
  
  glBegin(GL_QUADS);

  glNormal3f(1.0, 0.0, 0.0);
  glTexCoord2fv(fish->texture_coords[0]);
  glVertex3f(FISH_HALF_LENGTH, 0.0, 0.0);

  glTexCoord2fv(fish->texture_coords[1]);
  glVertex3f(0.0, FISH_HALF_WIDTH, 0.0);

  glTexCoord2fv(fish->texture_coords[2]);
  glVertex3f(-FISH_HALF_LENGTH, 0.0, 0.0);

  glTexCoord2fv(fish->texture_coords[3]);
  glVertex3f(0.0, -FISH_HALF_WIDTH, 0.0);
  
  glEnd();
  glPopMatrix();
  }

static void update_fish(lemuria_engine_t * e,
                        fish_t * f, float * attractor,
                        float gravity, int boing)
  {
  float distance;
  float delta[3];
  float rot[3];

  float factor;
  delta[0] = attractor[0] - f->coords[0];
  delta[1] = attractor[1] - f->coords[1];
  delta[2] = attractor[2] - f->coords[2];

  distance = sqrt(delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2]);

  
  factor = 0.003 * gravity;
    
  if(distance < 0.01)
    factor *= 100;
  else
    factor /= distance;

  if(boing)
    {
    f->d_coords[0] += lemuria_random(e, -0.1, 0.1);
    f->d_coords[1] += lemuria_random(e, -0.1, 0.1);
    f->d_coords[2] += lemuria_random(e, -0.1, 0.1);
    }
  else if(distance > 1.0)
    {
    f->d_coords[0] *= 0.95;
    f->d_coords[1] *= 0.95;
    f->d_coords[2] *= 0.95;
    }
  
  f->d_coords[0] += delta[0] * factor;
  f->d_coords[1] += delta[1] * factor;
  f->d_coords[2] += delta[2] * factor;
  
  f->coords[0] += f->d_coords[0];
  f->coords[1] += f->d_coords[1];
  f->coords[2] += f->d_coords[2];

  /* Rotate */

  rot[0] = 0.0;
  
  /* Rotation around y axis (x-z) */
  
  rot[1] = atan2(delta[2], delta[0]);

  /* Rotation around z axis (x-y) */

  rot[2] = atan2(delta[1], delta[0]);

  f->rot[0] = 0.9 * f->rot[0] + 0.1 * rot[0];
  f->rot[1] = 0.9 * f->rot[1] + 0.1 * rot[1];
  f->rot[2] = 0.9 * f->rot[2] + 0.1 * rot[2];
  }

static void draw_swarm(lemuria_engine_t * e, void * user_data)
  {
  int i;
  float attractor[3];
  float attractor_t[3];
  float matrix[3][3];
  float gravity;
  int boing = 0;
  //  float max_distance = 0.0;
  
  swarm_data * d = (swarm_data*)user_data;

  gravity = 1.0;

  /* Check for finishing */

  if(e->foreground.mode == EFFECT_FINISH)
    {
    e->foreground.mode = EFFECT_FINISHING;

    lemuria_rotator_turnto(&(d->rotator),
                           0.0,
                           0.0,
                           lemuria_random(e, 0.0, 360.0),
                           lemuria_random_int(e, 200, 400));

    lemuria_range_get(&(d->axis_range), &(d->axis_start), &(d->axis_end),
                      attractor);
    d->axis_start = attractor[0];
    d->axis_end   = 10.0;
    lemuria_range_init(e, &(d->axis_range),
                       1, 200, 400);
    }
  
  
  /* Change stuff */

  if(e->foreground.mode == EFFECT_RUNNING)
    {
    d->frames_since_boing++;
    if(e->beat_detected)
      {
      if(lemuria_decide(e, 0.8))
        lemuria_rotator_change(e, &(d->rotator));

      if(lemuria_range_done(&(d->axis_range)) && 
         lemuria_decide(e, 0.7))
        {
        d->axis_start = d->axis_end;
        d->axis_end = lemuria_random(e, AXIS_MIN, AXIS_MAX);
        lemuria_range_init(e, &(d->axis_range),
                           1, 200, 400);
        }

      if(lemuria_decide(e, 0.2) && (d->frames_since_boing > 1000))
        {
        boing = 1;
        //        fprintf(stderr, "Boing\n");
        //      gravity = -50.0;
        d->frames_since_boing = 0;
        }
      }
    }
  /* Update */

  if(e->foreground.mode != EFFECT_STARTING)
    lemuria_rotator_update(&(d->rotator));
  
  lemuria_range_update(&(d->axis_range));

  if((e->foreground.mode == EFFECT_STARTING) && lemuria_range_done(&(d->axis_range)))
    {
    e->foreground.mode = EFFECT_RUNNING;
    }
  
  lemuria_range_get(&(d->axis_range), &(d->axis_start), &(d->axis_end),
                    attractor);

  attractor[1] = 0.0;
  attractor[2] = 0.0;

  d->attractor_phi += 0.003;
  if(d->attractor_phi > 2.0 * M_PI)
    d->attractor_phi = 2.0 * M_PI;
  
  //  attractor[0] = axis[0] * cos(d->attractor_phi);
  //  attractor[1] = axis[1] * sin(d->attractor_phi);
  //  attractor[2] = 0.0;
  
  lemuria_rotate_get_matrix(&(d->rotator), matrix);

  attractor_t[0] = matrix[0][0]*attractor[0] + matrix[0][1]*attractor[1] + matrix[0][2]*attractor[2];
  attractor_t[1] = matrix[1][0]*attractor[0] + matrix[1][1]*attractor[1] + matrix[1][2]*attractor[2];
  attractor_t[2] = matrix[2][0]*attractor[0] + matrix[2][1]*attractor[1] + matrix[2][2]*attractor[2];
  
  for(i = 0; i < NUM; i++)
    {
    update_fish(e, &(d->fishes[i]), attractor_t, gravity, boing);
    }

  /* Draw */
  
  glShadeModel(GL_SMOOTH);
  //  glEnable(GL_POLYGON_SMOOTH);

  glEnable(GL_NORMALIZE);
    
/*   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_LIGHTING_MODE_HP, */
/*             GL_TEXTURE_POST_SPECULAR_HP); */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  
  glEnable(GL_TEXTURE_2D);

  lemuria_texture_bind(e, 0);
  
  // Enable lighting
  
  lemuria_set_perspective(e, 1, 20.0);
  
  // Set up and enable light 0

  lemuria_set_light(&light, GL_LIGHT0);
  glEnable(GL_LIGHT0);
  
  lemuria_set_material(&material, GL_FRONT_AND_BACK);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  //  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
  
  glEnable(GL_LIGHTING);
  
  glMatrixMode(GL_MODELVIEW); 
  
  for(i = 0; i < NUM; i++)
    draw_fish(&(d->fishes[i]));
  
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
  //  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);

  glDisable(GL_COLOR_MATERIAL);


  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glDisable(GL_NORMALIZE);

#ifdef DRAW_ATTRACTOR
  glLineWidth(2.0);
  glColor3f(1.0, 0.5, 0.5);

  glDisable(GL_DEPTH_TEST);

  glBegin(GL_LINES);
  glVertex3f(attractor_t[0] - 0.05, attractor_t[1] - 0.05, attractor_t[2]);
  glVertex3f(attractor_t[0] + 0.05, attractor_t[1] + 0.05, attractor_t[2]);

  glVertex3f(attractor_t[0] + 0.05, attractor_t[1] - 0.05, attractor_t[2]);
  glVertex3f(attractor_t[0] - 0.05, attractor_t[1] + 0.05, attractor_t[2]);
  glEnd();
  glEnable(GL_DEPTH_TEST);
#endif
  
  /* Check if we are done */

/*   if(e->foreground.mode == EFFECT_FINISHING) */
/*     fprintf(stderr, "Max distance: %f\n", max_distance); */

  if((e->foreground.mode == EFFECT_FINISHING) &&
     (lemuria_range_done(&(d->axis_range))) &&
     (lemuria_rotator_done(&(d->rotator))))
    e->foreground.mode = EFFECT_DONE;
  }

static void init_coords(lemuria_engine_t * e,
                        swarm_data * d, float angle)
  {
  int i, j, index;
  float texture_x_1;
  float texture_y_1;
  float texture_x_2;
  float texture_y_2;
  float start_x;
  float start_y;
  
  start_x = cos(angle) * 10.0;
  start_y = sin(angle) * 10.0;
  
  index = 0;

  texture_x_1 = 0.0;
  texture_y_1 = 0.0;
  
  for(i = 0; i < SQR_NUM; i++)
    {
    texture_x_2 = (float)(i+1)/(float)(SQR_NUM);
    for(j = 0; j < SQR_NUM; j++)
      {
      texture_y_2 = (float)(j+1)/(float)(SQR_NUM);

      d->fishes[index].texture_coords[0][0] = texture_x_1;
      d->fishes[index].texture_coords[0][1] = texture_y_1;

      d->fishes[index].texture_coords[1][0] = texture_x_2;
      d->fishes[index].texture_coords[1][1] = texture_y_1;

      d->fishes[index].texture_coords[2][0] = texture_x_2;
      d->fishes[index].texture_coords[2][1] = texture_y_2;

      d->fishes[index].texture_coords[3][0] = texture_x_1;
      d->fishes[index].texture_coords[3][1] = texture_y_2;
      index++;
      texture_y_1 = texture_y_2;
      }
    texture_x_1 = texture_x_2;
    }

  /* Initialize positions */

  for(i = 0; i < NUM; i++)
    {
    d->fishes[i].coords[0] = start_x + lemuria_random(e, -1.0, 1.0);
    d->fishes[i].coords[1] = start_y + lemuria_random(e, -1.0, 1.0);
    d->fishes[i].coords[2] = lemuria_random(e, -1.0, 1.0);

    d->fishes[i].rot[0] = lemuria_random(e, 0.0, 2.0*M_PI);
    d->fishes[i].rot[1] = lemuria_random(e, 0.0, 2.0*M_PI);
    d->fishes[i].rot[2] = lemuria_random(e, 0.0, 2.0*M_PI);
    }
  }

static void * init_swarm(lemuria_engine_t * e)
  {
  swarm_data * data;
  data = calloc(1, sizeof(*data));

  data->engine = e;
  lemuria_texture_ref(data->engine, 0);

  e->foreground.mode = EFFECT_STARTING;
  
  /* Initialize texture coordinates */

  lemuria_rotator_init(e, &(data->rotator));

  /* Allow only rotations around the x axis */
    
  data->rotator.angle[0] = 0.0;
  data->rotator.angle[1] = 0.0;
  init_coords(e, data, data->rotator.angle[2] * M_PI / 180.0);
  
  lemuria_range_init(e, &(data->axis_range), 2, 200, 300);

  data->axis_start = 10.0;
  
  //  data->axis_start = lemuria_random(AXIS_MIN, AXIS_MAX);
  data->axis_end = lemuria_random(e, AXIS_MIN, AXIS_MAX);
  
  return data;
  }

static void delete_swarm(void * e)
  {
  swarm_data * data = (swarm_data*)(e);
  lemuria_texture_unref(data->engine, 0);
  free(data);
  }

effect_plugin_t swarm_effect =
  {
    .init =    init_swarm,
    .draw =    draw_swarm,
    .cleanup = delete_swarm,
  };
