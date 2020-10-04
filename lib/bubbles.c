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

#include <light.h>
#include <material.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Bubble setup */

#define NUM_BUBBLES 15

#define X_MIN -4.0
#define X_MAX  4.0

#define Z_MIN -5.0
#define Z_MAX  0.0

#define Y_MIN -5.0
#define Y_MAX  5.0

#define BUBBLE_SIZE_MIN 0.2
#define BUBBLE_SIZE_MAX 0.7

#define FREQ_MIN 0.1
#define FREQ_MAX 0.3

#define SPEED_MIN 0.02
#define SPEED_MAX 0.06

// For drawing

#define PHI_STEPS    20
#define THETA_STEPS  20

typedef struct
  {
  float size;

  float scale_y;
  float scale_xz;

  float x, y, z;

  float scale_angle;
  float scale_freq;

  float speed;
  
  } bubble;

static void create_bubble(lemuria_engine_t * e, bubble * b)
  {
  /* TODO: Adjust according to the current volume */

  b->size = lemuria_random(e, BUBBLE_SIZE_MIN, BUBBLE_SIZE_MAX);

  b->x = lemuria_random(e, X_MIN, X_MAX);
  b->z = lemuria_random(e, Z_MIN, Z_MAX);
  b->y = Y_MIN;

  b->scale_freq = FREQ_MIN +
    (FREQ_MAX - FREQ_MIN) * (1.0 - (b->size - BUBBLE_SIZE_MIN)/(BUBBLE_SIZE_MAX - BUBBLE_SIZE_MIN));

  b->speed = SPEED_MIN +
    (SPEED_MAX - SPEED_MIN) * (1.0 - (b->size - BUBBLE_SIZE_MIN)/(BUBBLE_SIZE_MAX - BUBBLE_SIZE_MIN));
  
  b->scale_angle = lemuria_random(e, 0.0, 360.0);

  b->scale_y  = 1.0 + 0.5 * sin(b->scale_angle);
  b->scale_xz = 1.0 / b->scale_y;

  //  fprintf(stderr, "Size: %f, .x = %f, z: %f\n", b->size, b->x, b->z);

  };

static int update_bubble(lemuria_engine_t * e, bubble * b, int finishing)
  {
  int ret = 0;
  if((b->y > Y_MAX) && !finishing)
    {
    create_bubble(e, b);
    ret = 1;
    }
  
  b->y += b->speed;

  b->scale_angle += b->scale_freq;

  if(b->scale_angle > 360.0)
    b->scale_angle -= 360.0;

  b->scale_y  = 1.0 + 0.1 * sin(b->scale_angle);
  b->scale_xz = 1.0 / b->scale_y;
  return ret;
  }




typedef struct
  {
  bubble bubbles[NUM_BUBBLES];
  unsigned int bubble_list;

  //  void (*update_bubble_func)(bubble * b);
  lemuria_engine_t * engine;
  } bubbles_data;

static void create_bubble_mesh(bubbles_data * data)
  {
  int i, j;

/*   float phi_1, theta_1; */
  float phi_2, theta_2;

  float phi_norm_1 = 0.0;
  float phi_norm_2;

  float theta_norm_1 = 0.0;
  float theta_norm_2;

  float cos_theta_1 = 1.0;
  float cos_theta_2;

  float sin_theta_1 = 0.0;
  float sin_theta_2;

  float cos_phi_1 = 1.0;
  float cos_phi_2;

  float sin_phi_1 = 0.0;
  float sin_phi_2;

  glNewList(data->bubble_list, GL_COMPILE);
  
  glBegin(GL_QUADS);
  
  for(i = 0; i < THETA_STEPS; i++)
    {
    theta_norm_2 = (float)(i+1)/(float)THETA_STEPS;
    theta_2 = theta_norm_2 * M_PI;

    cos_theta_2 = cos(theta_2);
    sin_theta_2 = sin(theta_2);

    phi_norm_1 = 0.0;

    cos_phi_1 = 1.0;
    sin_phi_1 = 0.0;
        
    for(j = 0; j < PHI_STEPS; j++)
      {
      phi_norm_2 = (float)(j+1)/(float)PHI_STEPS;
      phi_2 = phi_norm_2 * 2.0 * M_PI;
      
      cos_phi_2 = cos(phi_2);
      sin_phi_2 = sin(phi_2);
      
      glTexCoord2f(2.0 * phi_norm_1, theta_norm_1);
      glNormal3f(cos_phi_1 * sin_theta_1, cos_theta_1,
                 sin_phi_1 * sin_theta_1);
      glVertex3f(cos_phi_1 * sin_theta_1, cos_theta_1,
                 sin_phi_1 * sin_theta_1);

      glTexCoord2f(2.0 * phi_norm_2, theta_norm_1);
      glNormal3f(cos_phi_2 * sin_theta_1, cos_theta_1,
                 sin_phi_2 * sin_theta_1);
      glVertex3f(cos_phi_2 * sin_theta_1, cos_theta_1,
                 sin_phi_2 * sin_theta_1);

      glTexCoord2f(2.0 * phi_norm_2, theta_norm_2);
      glNormal3f(cos_phi_2 * sin_theta_2, cos_theta_2,
                 sin_phi_2 * sin_theta_2);
      glVertex3f(cos_phi_2 * sin_theta_2, cos_theta_2,
                 sin_phi_2 * sin_theta_2);

      glTexCoord2f(2.0 * phi_norm_1, theta_norm_2);
      glNormal3f(cos_phi_1 * sin_theta_2, cos_theta_2,
                 sin_phi_1 * sin_theta_2);
      glVertex3f(cos_phi_1 * sin_theta_2, cos_theta_2,
                 sin_phi_1 * sin_theta_2);
      
      phi_norm_1 = phi_norm_2;
      cos_phi_1 = cos_phi_2;
      sin_phi_1 = sin_phi_2;
      
      }
    cos_theta_1 = cos_theta_2;
    sin_theta_1 = sin_theta_2;
    theta_norm_1 = theta_norm_2;
    }
  glEnd();

  glEndList();
  }

static lemuria_light_t light =
  {
    .ambient =  { 0.8f, 0.8f, 0.8f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .position = { 5.0f, 5.0f, 2.0f, 1.0f }
  };

static lemuria_material_t material =
  {
    .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .ref_diffuse =  { 0.8f, 0.8f, 0.8f, 1.0f },
    .shininess = 128
  };


static void draw_bubbles(lemuria_engine_t * e, void * user_data)
  {
  int i;
  int updated = 0;
  int do_exit;
  int finishing;
  bubbles_data * data = (bubbles_data*)user_data;

  if(e->foreground.mode == EFFECT_FINISH)
    {
    e->foreground.mode = EFFECT_FINISHING;
    }
  
  if(e->foreground.mode == EFFECT_FINISHING)
    {
    finishing = 1;
    do_exit = 1;
    }
  else
    {
    finishing = 0;
    do_exit = 0;
    }

  glShadeModel(GL_SMOOTH);
  glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //  glEnable(GL_BLEND);
  
  lemuria_set_perspective(e, 1, 50.0);
  
  // Set up and enable light 0

  lemuria_set_light(&light, GL_LIGHT0);
  glEnable(GL_LIGHT0);

  // Enable lighting
  glEnable(GL_LIGHTING);

  glEnable(GL_CULL_FACE);
  
  // All materials hereafter have full specular reflectivity
  // with a high shine

  lemuria_set_material(&material, GL_FRONT_AND_BACK);
  glEnable(GL_NORMALIZE);
  lemuria_texture_bind(e, 0);
  
  glMatrixMode(GL_MODELVIEW);

  glPushMatrix();
  glLoadIdentity();
    
  for(i = 0; i < NUM_BUBBLES; i++)
    {
    if(update_bubble(e, &data->bubbles[i], finishing))
      updated = 1;

    if((data->bubbles[i].y < Y_MAX) && do_exit)
      do_exit = 0;
    
    
    glPushMatrix();
    glTranslatef(data->bubbles[i].x, data->bubbles[i].y, data->bubbles[i].z);
    
    glScalef(data->bubbles[i].scale_xz * data->bubbles[i].size,
             data->bubbles[i].scale_y  * data->bubbles[i].size,
             data->bubbles[i].scale_xz * data->bubbles[i].size);
    
    glCallList(data->bubble_list);
    glPopMatrix();
    }

  if(updated && (e->foreground.mode == EFFECT_STARTING))
    {
    e->foreground.mode = EFFECT_RUNNING;
    }
  
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);

  glDisable(GL_CULL_FACE);
  //  glDisable(GL_BLEND);

  //  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glDisable(GL_NORMALIZE);

  if(do_exit)
    {
    e->foreground.mode = EFFECT_DONE;
    }
  
  //  glDisable(GL_COLOR_MATERIAL);
  }

static void * init_bubbles(lemuria_engine_t * e)
  {
  int i, random_number;
  bubbles_data * d;
#ifdef DEBUG
  fprintf(stderr, "init_bubbles...");
#endif
  
  d = calloc(1, sizeof(bubbles_data));
  d->engine = e;
  e->foreground.mode = EFFECT_RUNNING;
  lemuria_texture_ref(d->engine, 0);
  
  random_number = lemuria_random_int(e, 0, 1);

  d->bubble_list = glGenLists(1);
  create_bubble_mesh(d);

  
  create_bubble_mesh(d);
 
  for(i = 0; i < NUM_BUBBLES; i++)
    create_bubble(e, &d->bubbles[i]);
#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif


  return d;
  }

static void delete_bubbles(void * data)
  {
  bubbles_data * d = (bubbles_data*)(data);

  glDeleteLists(d->bubble_list, 1);
  lemuria_texture_unref(d->engine, 0);
  free(d);
  }

effect_plugin_t bubbles_effect =
  {
    .init =    init_bubbles,
    .draw =    draw_bubbles,
    .cleanup = delete_bubbles,
  };
