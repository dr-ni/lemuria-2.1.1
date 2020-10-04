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

#define TRANSITION_MIN 100
#define TRANSITION_MAX 200

typedef struct cube_data_s
  {
  lemuria_rotator_t rotator;
  lemuria_scale_t scale;
  lemuria_offset_t offset;
  
  //  unsigned int cube_list;
  void (*draw_cube_func)(lemuria_engine_t * e, struct cube_data_s*);

  lemuria_range_t range;
  lemuria_engine_t * engine;
  } cube_data;

static lemuria_light_t light =
  {
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 0.0f, 0.0f, 0.0f, 1.0f },
    .position = { 0.0f, 0.0f, 10.0f, 1.0f },
  };

static lemuria_material_t material =
  {
    .ref_specular = { 0.0f, 0.0f, 0.0f, 0.0f },
    .ref_ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 128
  };

static void draw_cube_normal(lemuria_engine_t * e, cube_data * data)
  {
  glColor3f(1.0, 1.0, 1.0);
  glEnable(GL_CULL_FACE);
  glBegin(GL_QUADS);
  // Front Face
  glNormal3f(0.0, 0.0, 1.0);
  glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0,  1.0);
  glTexCoord2f(1.0, 0.0); glVertex3f( 1.0, -1.0,  1.0);
  glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0,  1.0);
  glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0,  1.0);
  // Back Face
  glNormal3f(0.0, 0.0,-1.0); 
  glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0, -1.0);
  glTexCoord2f(1.0, 1.0); glVertex3f(-1.0,  1.0, -1.0);
  glTexCoord2f(0.0, 1.0); glVertex3f( 1.0,  1.0, -1.0);
  glTexCoord2f(0.0, 0.0); glVertex3f( 1.0, -1.0, -1.0); 
  // Top Face
  glNormal3f(0.0, 1.0, 0.0);
  glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0, -1.0);
  glTexCoord2f(0.0, 0.0); glVertex3f(-1.0,  1.0,  1.0);
  glTexCoord2f(1.0, 0.0); glVertex3f( 1.0,  1.0,  1.0); 
  glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0, -1.0);
  // Bottom Face
  glNormal3f(0.0, -1.0, 0.0); 
  glTexCoord2f(1.0, 1.0); glVertex3f(-1.0, -1.0, -1.0);
  glTexCoord2f(0.0, 1.0); glVertex3f( 1.0, -1.0, -1.0);
  glTexCoord2f(0.0, 0.0); glVertex3f( 1.0, -1.0,  1.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0,  1.0);
  // Right face
  glNormal3f(1.0,0.0,0.0); 
  glTexCoord2f(1.0, 0.0); glVertex3f( 1.0, -1.0, -1.0);
  glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0, -1.0);
  glTexCoord2f(0.0, 1.0); glVertex3f( 1.0,  1.0,  1.0);
  glTexCoord2f(0.0, 0.0); glVertex3f( 1.0, -1.0,  1.0);
  // Left Face
  glNormal3f(-1.0,0.0,0.0); 
  glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, -1.0);
  glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0,  1.0);
  glTexCoord2f(1.0, 1.0); glVertex3f(-1.0,  1.0,  1.0);
  glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0, -1.0);
  glEnd();
  glDisable(GL_CULL_FACE);
  }

static void draw_cube_explode(lemuria_engine_t * e, cube_data * data)
  {
  float offset = 0.0;
  float explode_start;
  float explode_end;
  float explode;

  float scale_factor = lemuria_scale_get(&(data->scale));

  float angle;
  float cos_angle = 1.0;
  float sin_angle = 0.0;
  
  lemuria_range_update(&(data->range));
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

  switch(e->foreground.mode)
    {
    case EFFECT_FINISHING:
      explode_start = 0.0;
      explode_end   = 0.5 * M_PI;
      lemuria_range_get(&(data->range), &explode_start,
                        &explode_end, &explode);
      explode = 1.0 - cos(explode);
      offset = (10.0 * explode) / scale_factor;

      angle = 4.0 * M_PI * explode;
      cos_angle = cos(angle);
      sin_angle = sin(angle);

      if(lemuria_range_done(&(data->range)))
        e->foreground.mode = EFFECT_DONE;
      break;
    case EFFECT_STARTING:
      explode_start = 0.5 * M_PI;
      explode_end   = 0.0;
      lemuria_range_get(&(data->range), &explode_start,
                        &explode_end, &explode);

      explode = 1.0 - cos(explode);
      offset = (10.0 * explode) / scale_factor;

      angle = 4.0 * M_PI * explode;
      cos_angle = cos(angle);
      sin_angle = sin(angle);

      if(lemuria_range_done(&(data->range)))
        {
        data->draw_cube_func = draw_cube_normal;
        e->foreground.mode = EFFECT_RUNNING;
        }
      break;
    case EFFECT_RUNNING:
      explode_start = 0.0;
      explode_end   = 2.0 * M_PI;
      lemuria_range_get(&(data->range), &explode_start,
                        &explode_end, &explode);
      offset = 2.0 * (1.0 - cos(explode));
      break;
    }
  
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_QUADS);
  // Front Face
  glNormal3f(0.0, 0.0, 1.0);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-cos_angle-sin_angle, -cos_angle+sin_angle,  1.0 + offset);
  glTexCoord2f(1.0, 0.0);
  glVertex3f( cos_angle-sin_angle, -cos_angle-sin_angle,  1.0 + offset);
  glTexCoord2f(1.0, 1.0);
  glVertex3f( cos_angle+sin_angle,  cos_angle-sin_angle,  1.0 + offset);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-cos_angle+sin_angle,  cos_angle+sin_angle,  1.0 + offset);
  // Back Face
  glNormal3f(0.0, 0.0,-1.0); 
  glTexCoord2f(1.0, 0.0);
  glVertex3f(-cos_angle+sin_angle, -cos_angle-sin_angle, -1.0 - offset);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(-cos_angle-sin_angle,  cos_angle-sin_angle, -1.0 - offset);
  glTexCoord2f(0.0, 1.0);
  glVertex3f( cos_angle-sin_angle,  cos_angle+sin_angle, -1.0 - offset);
  glTexCoord2f(0.0, 0.0);
  glVertex3f( cos_angle+sin_angle, -cos_angle+sin_angle, -1.0 - offset); 
  // Top Face
  glNormal3f(0.0, 1.0, 0.0);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-cos_angle-sin_angle,  1.0 + offset, -cos_angle+sin_angle);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-cos_angle+sin_angle,  1.0 + offset,  cos_angle+sin_angle);
  glTexCoord2f(1.0, 0.0);
  glVertex3f( cos_angle+sin_angle,  1.0 + offset,  cos_angle-sin_angle); 
  glTexCoord2f(1.0, 1.0);
  glVertex3f( cos_angle-sin_angle,  1.0 + offset, -cos_angle-sin_angle);
  // Bottom Face
  glNormal3f(0.0, -1.0, 0.0); 
  glTexCoord2f(1.0, 1.0);
  glVertex3f(-cos_angle+sin_angle, -1.0 - offset, -cos_angle-sin_angle);
  glTexCoord2f(0.0, 1.0);
  glVertex3f( cos_angle+sin_angle, -1.0 - offset, -cos_angle+sin_angle);
  glTexCoord2f(0.0, 0.0);
  glVertex3f( cos_angle-sin_angle, -1.0 - offset,  cos_angle+sin_angle);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(-cos_angle-sin_angle, -1.0 - offset,  cos_angle-sin_angle);
  // Right face
  glNormal3f(1.0,0.0,0.0); 
  glTexCoord2f(1.0, 0.0);
  glVertex3f( 1.0 + offset, -cos_angle+sin_angle, -cos_angle-sin_angle);
  glTexCoord2f(1.0, 1.0);
  glVertex3f( 1.0 + offset,  cos_angle+sin_angle, -cos_angle+sin_angle);
  glTexCoord2f(0.0, 1.0);
  glVertex3f( 1.0 + offset,  cos_angle-sin_angle,  cos_angle+sin_angle);
  glTexCoord2f(0.0, 0.0);
  glVertex3f( 1.0 + offset, -cos_angle-sin_angle,  cos_angle-sin_angle);
  // Left Face
  glNormal3f(-1.0,0.0,0.0); 
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-1.0 - offset, -cos_angle-sin_angle, -cos_angle+sin_angle);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(-1.0 - offset, -cos_angle+sin_angle,  cos_angle+sin_angle);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(-1.0 - offset,  cos_angle+sin_angle,  cos_angle-sin_angle);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-1.0 - offset,  cos_angle-sin_angle, -cos_angle-sin_angle);
  glEnd();
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
  }

static void draw_cube(lemuria_engine_t * e, void * user_data)
  {
  float scale_factor;
  cube_data * data = (cube_data*)(user_data);

  /* Check whether to change something */

  if(e->foreground.mode == EFFECT_FINISH)
    {
    e->foreground.mode = EFFECT_FINISHING;
    lemuria_range_init(e, &(data->range), 1, TRANSITION_MIN, TRANSITION_MAX);
    data->draw_cube_func = draw_cube_explode;
    }
  
  if(e->foreground.mode == EFFECT_RUNNING)
    {
    if(e->beat_detected)
      {
      //    fprintf(stderr, "beat detected\n");
      if(lemuria_decide(e, 0.1))
        lemuria_rotator_change(e, &(data->rotator));
      
      if(lemuria_decide(e, 0.1))
        lemuria_scale_change(e, &(data->scale));
      
      if(lemuria_decide(e, 0.1))
        lemuria_offset_change(e, &(data->offset));
      if(lemuria_decide(e, 0.1))
        lemuria_offset_kick(&(data->offset));
      }
    
    lemuria_rotator_update(&(data->rotator));
    lemuria_scale_update(&(data->scale));
    lemuria_offset_update(&(data->offset));
    }
  
/*   fprintf(stderr, "Scale factor: %f\n", scale_factor); */
  
  glShadeModel(GL_SMOOTH);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_NORMALIZE);
  
  // Enable lighting
  glEnable(GL_LIGHTING);

  lemuria_set_perspective(e, 1, 10.0);
  
  // Set up and enable light 0

  lemuria_set_light(&light, GL_LIGHT0);
  glEnable(GL_LIGHT0);
  
  // All materials hereafter have full specular reflectivity
  // with a high shine

  lemuria_set_material(&material, GL_FRONT_AND_BACK);
  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  
  scale_factor = lemuria_scale_get(&(data->scale));

  //  fprintf(stderr, "Scale factor: %f\n", scale_factor);

  lemuria_offset_translate(&(data->offset));
    
  glScalef(scale_factor, scale_factor, scale_factor);
  lemuria_rotate(&(data->rotator));
  //  glRotatef(90.0, 0.0, 1.0, 0.0);
  
  lemuria_texture_bind(e, 0);

  data->draw_cube_func(e, data);

  glPopMatrix();
  
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  //  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_NORMALIZE);
  }

static void * init_cube(lemuria_engine_t * e)
  {
  cube_data * data;

#ifdef DEBUG
  fprintf(stderr, "init_cube...");
#endif
  
  data = calloc(1, sizeof(cube_data));

  data->engine = e;
  lemuria_texture_ref(data->engine, 0);
    
  e->foreground.mode = EFFECT_STARTING;

  lemuria_rotator_init(e, &(data->rotator));
  lemuria_scale_init(e, &(data->scale), 0.1, 0.6);
  lemuria_offset_init(e, &(data->offset));

  data->draw_cube_func = draw_cube_explode;
  lemuria_range_init(e, &(data->range), 1, TRANSITION_MIN, TRANSITION_MAX);
#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  return data;
  }

static void delete_cube(void * e)
  {
  cube_data * data = (cube_data*)(e);
  lemuria_texture_unref(data->engine, 0);
  free(data);
  }

effect_plugin_t cube_effect =
  {
    .init =    init_cube,
    .draw =    draw_cube,
    .cleanup = delete_cube,
  };
