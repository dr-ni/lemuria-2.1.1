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
#include <math.h>

#include <lemuria_private.h>
#include <utils.h>
#include <object.h>
#include <material.h>

#define TENTACLE_LENGTH 10.0
#define TENTACLE_HALF_WIDTH  0.5

void lemuria_seaplant_init(lemuria_engine_t * e,
                           lemuria_object_t * obj);

static lemuria_material_t seaplant_materials[] =
  {
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.219f, 0.416f, 0.000f, 1.000f },
      .ref_diffuse =  { 0.439f, 0.833f, 0.000f, 1.000f },
      .shininess = 128
    },
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.502f, 0.000f, 0.283f, 1.000f },
      .ref_diffuse =  { 1.000f, 0.000f, 0.564f, 1.000f },
      .shininess = 128
    },
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.502f, 0.366f, 0.000f, 1.000f },
      .ref_diffuse =  { 1.000f, 0.729f, 0.000f, 1.000f },
      .shininess = 128
    },
    
  };

static void draw_leaf(lemuria_tentacle_t * t)
  {
  int i;
  float y;
  float width;
  
  glBegin(GL_QUAD_STRIP);
  
  for(i = 0; i < t->num_points; i++)
    {
    y = (float)i / (float)(t->num_points-1);

    width = (1.0 - y * y) * TENTACLE_HALF_WIDTH;
    
    glNormal3f(0.0, t->points[i].normals[0], t->points[i].normals[1]);
    
    glVertex3f(-width, t->points[i].coords[0], t->points[i].coords[1]);
    glVertex3f(width, t->points[i].coords[0], t->points[i].coords[1]);
    
    }
  glEnd();
  }

static void draw_seaplant(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  int i;
  
  glEnable(GL_NORMALIZE);
  //  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  glShadeModel(GL_SMOOTH);
  lemuria_object_rotate(obj);
  lemuria_set_material(&seaplant_materials[obj->data.seaplant.material],
                       GL_FRONT_AND_BACK);

  glMatrixMode(GL_MODELVIEW);
  
  for(i = 0; i < SEAPLANT_NUM_LEAVES; i++)
    {
    glPushMatrix();
    glRotatef(360.0 * i / (float)(SEAPLANT_NUM_LEAVES), 0.0, 1.0, 0.0);
    glTranslatef(0.0, 0.0, 3.0);
    glScalef(obj->data.seaplant.scale_factors[i],
             obj->data.seaplant.scale_factors[i],
             obj->data.seaplant.scale_factors[i]);
             
    draw_leaf(&(obj->data.seaplant.tentacles[i % SEAPLANT_NUM_TENTACLES]));
    glPopMatrix();
    }
  
  
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
  glDisable(GL_NORMALIZE);
  }


static void update_seaplant(lemuria_engine_t * e,
                            lemuria_object_t * obj)
  {
  int i;
  for(i = 0; i < SEAPLANT_NUM_TENTACLES; i++)
    {
    lemuria_tentacle_update(&(obj->data.seaplant.tentacles[i]));
    }
  }

static void cleanup_seaplant(lemuria_object_t * obj)
  {
  int i;

  for(i = 0; i < SEAPLANT_NUM_TENTACLES; i++)
    lemuria_tentacle_cleanup(&(obj->data.seaplant.tentacles[i]));
  
  }

void lemuria_seaplant_init(lemuria_engine_t * e,
                           lemuria_object_t * obj)
  {
  int i;

  for(i = 0;  i < SEAPLANT_NUM_TENTACLES; i++)
    {
    lemuria_tentacle_init(&(obj->data.seaplant.tentacles[i]),
                          SEAPLANT_TENTACLE_SIZE,     /* num_points */
                          TENTACLE_LENGTH,            /* length */
                          lemuria_random(e, 0.08, 0.13), /* frequency */
                          lemuria_random(e, 2.5, 3.5),   /* Wavelength */
                          lemuria_random(e, 0.0, M_PI),  /* Phase */
                          0.25 * M_PI                 /* Amplitude */);
    }

  for(i = 0;  i < SEAPLANT_NUM_LEAVES; i++)
    {
    obj->data.seaplant.scale_factors[i] = lemuria_random(e, 0.5, 1.0);
    }
  
  obj->update = update_seaplant;
  obj->cleanup = cleanup_seaplant;
  obj->draw = draw_seaplant;
  
  obj->data.seaplant.material = lemuria_random_int(e, 0,
                                                   sizeof(seaplant_materials)/sizeof(seaplant_materials[0])-1);
  
  }
