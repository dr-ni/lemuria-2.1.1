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

#include <stdlib.h>
#include <lemuria_private.h>
#include <utils.h>
#include <object.h>
#include <effect.h>
#include <material.h>

#define HALF_WIDTH 4.0
#define LENGTH     2.0

#define STEPS      20

void lemuria_manta_init(lemuria_engine_t * e,
                        lemuria_object_t * obj);

static lemuria_material_t manta_materials[] =
  {
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.52f, 0.42f, 0.13f, 1.0f },
      .ref_diffuse =  { 1.0f, 0.8f, 0.24f, 1.0f },
      .shininess = 128
    },
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.06f, 0.57f, 0.41f, 1.0f },
      .ref_diffuse =  { 0.11f, 1.00f, 0.72f, 1.0f },
      .shininess = 128
    },
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.60f, 0.09f, 0.43f, 1.0f },
      .ref_diffuse =  { 1.00f, 0.15f, 0.72f, 1.0f },
      .shininess = 128
    },
    
  };


static void update_manta(lemuria_engine_t * e,
                         lemuria_object_t * obj)
  {
  lemuria_tentacle_update(&(obj->data.manta.t));
  }

static void cleanup_manta(lemuria_object_t * obj)
  {
  lemuria_tentacle_cleanup(&(obj->data.manta.t));
  }

static void draw_wing(lemuria_object_t * obj, float mirror)
  {
  int i;
  int j;
  float delta_x;
  float delta_x2;
  float x;
    
  delta_x = LENGTH / (float)STEPS;
  delta_x2 = delta_x * 2.0;
  
  for(i = 0; i < STEPS; i++)
    {
    x = -0.5 * LENGTH + i * delta_x;

    glBegin(GL_TRIANGLE_STRIP);

    glNormal3f(0.0,
               obj->data.manta.t.points[i].normals[1],
               mirror * obj->data.manta.t.points[i].normals[0]);
    
    glVertex3f(x,
               obj->data.manta.t.points[i].coords[1],
               mirror * obj->data.manta.t.points[i].coords[0]);
    
    for(j = 0; j < STEPS - i; j++)
      {
      glNormal3f(0.0,
                 obj->data.manta.t.points[i+1].normals[1],
                 mirror * obj->data.manta.t.points[i+1].normals[0]);
      
      glVertex3f(x + delta_x,
                 obj->data.manta.t.points[i+1].coords[1],
                 mirror * obj->data.manta.t.points[i+1].coords[0]);

      glNormal3f(0.0,
                 obj->data.manta.t.points[i].normals[1],
                 mirror * obj->data.manta.t.points[i].normals[0]);
      
      glVertex3f(x + delta_x2,
                 obj->data.manta.t.points[i].coords[1],
                 mirror * obj->data.manta.t.points[i].coords[0]);
      x += delta_x2;
      }
    glEnd();
    }

  }

static void draw_manta(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  glEnable(GL_NORMALIZE);
  //  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  glShadeModel(GL_SMOOTH);
  lemuria_object_rotate(obj);
  lemuria_set_material(&manta_materials[obj->data.manta.material],
                       GL_FRONT_AND_BACK);

  draw_wing(obj, 1.0);
  draw_wing(obj, -1.0);
  
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
  glDisable(GL_NORMALIZE);
  }

void lemuria_manta_init(lemuria_engine_t * e,
                        lemuria_object_t * obj)
  {
  lemuria_tentacle_init(&(obj->data.manta.t),
                        STEPS+1, /* num_points */
                        HALF_WIDTH,  /* length */
                        0.05, /* frequency */
                        10.0,  /* Wavelength */
                        0.0,  /* Phase */
                        0.25 * M_PI /* Amplitude */);

  obj->update = update_manta;
  obj->cleanup = cleanup_manta;
  obj->draw = draw_manta;
  
  obj->data.manta.material =
    lemuria_random_int(e, 0,
                       sizeof(manta_materials)/sizeof(manta_materials[0])-1);
  //  fprintf(stderr, "Material: %d\n", obj->data.manta.material);
  }

