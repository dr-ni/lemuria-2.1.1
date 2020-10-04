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
#include <math.h>

#include <lemuria_private.h>
#include <utils.h>
#include <object.h>
#include <light.h>
#include <material.h>

void lemuria_sputnik_init(lemuria_engine_t * e,
                          lemuria_object_t * obj);


#define UFO_COCKPIT_THETA_STEPS 10

static lemuria_material_t sputnik_material =
  {
    .ref_specular = { 0.0f, 0.0f, 0.0f, 0.0f },
    .ref_ambient =  { 0.3f, 0.3f, 0.3f, 1.0f },
    .ref_diffuse =  { 0.7f, 0.7f, 0.7f, 1.0f },
    .shininess = 128
  };

static lemuria_light_t sputnik_light =
  {
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 0.0f, 0.0f, 0.0f, 1.0f },
    .position = { 10.0f, 10.0f, 0.0f, 1.0f },
  };

#define SPUTNIK_RADIUS         1.0
#define SPUTNIK_ANTENNA_ANGLE  30.0*M_PI/180.0
#define SPUTNIK_THETA_STEP     20
#define SPUTNIK_ANTENNA_LENGTH 10.0

#define OBJECT_PHI_STEPS 20

static void draw_rotation_strip(float r_1, float r_2,
                                float z_1, float z_2,
                                float z_n_1, float z_n_2,
                                float r_n_1, float r_n_2)
  {
  float phi;
  int i;

  float sin_phi = 0.0;
  float cos_phi = 0.0;
    
  glBegin(GL_QUAD_STRIP);

  //  phi = 0.0;
  for(i = 0; i <= OBJECT_PHI_STEPS; i++)
    {
    phi = 2.0 * M_PI * (i+1)/ (float)OBJECT_PHI_STEPS;
    sin_phi = sin(phi);
    cos_phi = cos(phi);

    glNormal3f(r_n_1 * cos_phi, r_n_1 * sin_phi, z_n_1);
    glVertex3f(r_1 * sin_phi, r_1 * cos_phi, z_1);

    glNormal3f(r_n_2 * cos_phi, r_n_2 * sin_phi, z_n_2);
    glVertex3f(r_2 * sin_phi, r_2 * cos_phi, z_2);
    }
  glEnd();
  }


static void draw_sputnik(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  
  float theta_2;
  float sin_theta_1 = 0.0;
  float cos_theta_1 = -1.0;

  float sin_theta_2, cos_theta_2;

  int i;

  glShadeModel(GL_SMOOTH);
  glLineWidth(2.0);
  lemuria_set_material(&sputnik_material, GL_FRONT_AND_BACK);
  
  //  glRotatef(obj->data.sputnik.rotation[0], 1.0, 1.0, 0.0);
  //  glRotatef(obj->data.sputnik.rotation[1], 0.0, 1.0, 0.0);
  //  glRotatef(obj->data.sputnik.rotation[2], 0.0, 0.0, 1.0);

  lemuria_object_rotate(obj);
  
  glRotatef(90.0, 0.0, 1.0, 0.0);
  
  lemuria_set_light(&sputnik_light, GL_LIGHT0);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
    
  //  theta_1 = 0.5 * M_PI;
  
  for(i = 1; i < SPUTNIK_THETA_STEP; i++)
    {
    theta_2 = M_PI - (i * M_PI)/(UFO_COCKPIT_THETA_STEPS-1);
    sin_theta_2 = sin(theta_2);
    cos_theta_2 = cos(theta_2);
    
    draw_rotation_strip(SPUTNIK_RADIUS * sin_theta_1,
                        SPUTNIK_RADIUS * sin_theta_2,
                        SPUTNIK_RADIUS * cos_theta_1,
                        SPUTNIK_RADIUS * cos_theta_2,
                        cos_theta_1, cos_theta_2,
                        sin_theta_1, sin_theta_2);
    
    sin_theta_1 = sin_theta_2;
    cos_theta_1 = cos_theta_2;
    
    }

  /* Draw the antennas, so the kapitalistic world will hear us */

  sin_theta_1 = sin(SPUTNIK_ANTENNA_ANGLE);
  cos_theta_1 = sin(SPUTNIK_ANTENNA_ANGLE);
  
  glBegin(GL_LINES);

  glNormal3f(- cos_theta_1 * 0.707, -cos_theta_1 * 0.707, sin_theta_1);
  
  glVertex3f(- SPUTNIK_RADIUS * cos_theta_1 * 0.707,
             - SPUTNIK_RADIUS * cos_theta_1 * 0.707,
             SPUTNIK_RADIUS * sin_theta_1);
  
  glVertex3f(- (SPUTNIK_RADIUS * cos_theta_1 * 0.707 +
                SPUTNIK_ANTENNA_LENGTH * sin_theta_1 * 0.707),
             - (SPUTNIK_RADIUS * cos_theta_1 * 0.707 +
                SPUTNIK_ANTENNA_LENGTH * sin_theta_1 * 0.707),
             (SPUTNIK_RADIUS * sin_theta_1 - SPUTNIK_ANTENNA_LENGTH * cos_theta_1));

  /* */
  
  glNormal3f(  cos_theta_1 * 0.707, -cos_theta_1 * 0.707, sin_theta_1);
  
  glVertex3f(  SPUTNIK_RADIUS * cos_theta_1 * 0.707,
             - SPUTNIK_RADIUS * cos_theta_1 * 0.707,
               SPUTNIK_RADIUS * sin_theta_1);

  glVertex3f(  (SPUTNIK_RADIUS * cos_theta_1 * 0.707 +
                SPUTNIK_ANTENNA_LENGTH * sin_theta_1 * 0.707),
             - (SPUTNIK_RADIUS * cos_theta_1 * 0.707 +
                SPUTNIK_ANTENNA_LENGTH * sin_theta_1 * 0.707),
             (SPUTNIK_RADIUS * sin_theta_1 - SPUTNIK_ANTENNA_LENGTH * cos_theta_1));

  /* */

  glNormal3f(  cos_theta_1 * 0.707, cos_theta_1 * 0.707, sin_theta_1);
  
  glVertex3f(  SPUTNIK_RADIUS * cos_theta_1 * 0.707,
               SPUTNIK_RADIUS * cos_theta_1 * 0.707,
               SPUTNIK_RADIUS * sin_theta_1);

  glVertex3f(  (SPUTNIK_RADIUS * cos_theta_1 * 0.707 +
                SPUTNIK_ANTENNA_LENGTH * sin_theta_1 * 0.707),
               (SPUTNIK_RADIUS * cos_theta_1 * 0.707 +
                SPUTNIK_ANTENNA_LENGTH * sin_theta_1 * 0.707),
             (SPUTNIK_RADIUS * sin_theta_1 - SPUTNIK_ANTENNA_LENGTH * cos_theta_1));

  /* */

  glNormal3f(  - cos_theta_1 * 0.707, cos_theta_1 * 0.707, sin_theta_1);
  
  glVertex3f(  - SPUTNIK_RADIUS * cos_theta_1 * 0.707,
               SPUTNIK_RADIUS * cos_theta_1 * 0.707,
               SPUTNIK_RADIUS * sin_theta_1);

  glVertex3f(  - (SPUTNIK_RADIUS * cos_theta_1 * 0.707 +
                SPUTNIK_ANTENNA_LENGTH * sin_theta_1 * 0.707),
               (SPUTNIK_RADIUS * cos_theta_1 * 0.707 +
                SPUTNIK_ANTENNA_LENGTH * sin_theta_1 * 0.707),
             (SPUTNIK_RADIUS * sin_theta_1 - SPUTNIK_ANTENNA_LENGTH * cos_theta_1));

  
  
  glEnd();
  
    
  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  
  }

void lemuria_sputnik_init(lemuria_engine_t * e,
                          lemuria_object_t * obj)
  {
  obj->draw = draw_sputnik;
  }
