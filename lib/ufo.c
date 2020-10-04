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
#include <light.h>
#include <material.h>

void lemuria_ufo_init(lemuria_engine_t * e,
                      lemuria_object_t * obj);

static lemuria_material_t ufo_material =
  {
    .ref_specular = { 0.0f, 0.0f, 0.0f, 0.0f },
    .ref_ambient =  { 0.2f, 0.2f, 0.2f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 128
  };

static lemuria_material_t ufo_material_cockpit =
  {
    .ref_specular = { 0.6f, 0.6f, 1.0f, 1.0f },
    .ref_ambient =  { 0.2f, 0.2f, 1.0f, 1.0f },
    .ref_diffuse =  { 0.3f, 0.3f, 0.3f, 1.0f },
    .shininess = 128
  };

static float ufo_radius[5] =
  {
    0.0, 1.5, 2.0, 1.5, 0.6
  };

static float ufo_z[5] =
  {
    -0.25, -0.15, 0.0, 0.15, 0.3
  };

#define UFO_COCKPIT_THETA_STEPS 10
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


static void draw_ufo(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  int i;
  float z_n_1, z_n_2;
  float r_n_1, r_n_2;

  float angle_1, angle_2;
  
  float theta_1;
  float theta_2;

  //  fprintf(stderr, "Draw ufo\n");
  
  glShadeModel(GL_SMOOTH);
  
  lemuria_set_material(&ufo_material, GL_FRONT);
  
  glRotatef(270.0, 1.0, 0.0, 0.0);
  //  glScalef(0.5, 0.5, 0.5);
    
  // Make bottom
  
  z_n_1 = -1.0;
  r_n_1 = 0.0;
  
  angle_1 = atan2(ufo_z[1] - ufo_z[0], ufo_radius[1] - ufo_radius[0])
    - 0.5 * M_PI;
  angle_2 = atan2(ufo_z[2] - ufo_z[1], ufo_radius[2] - ufo_radius[1])
    - 0.5 * M_PI;

  z_n_2 = sin(0.5 * (angle_1 + angle_2));

  r_n_2 = cos(0.5 * (angle_1 + angle_2));
  
  draw_rotation_strip(ufo_radius[0], ufo_radius[1],
                      ufo_z[0], ufo_z[1],
                      -1.0, z_n_2,
                      0.0,  r_n_2);

  z_n_1 = z_n_2;
  r_n_1 = r_n_2;

  z_n_2 = sin(angle_2);
  r_n_2 = cos(angle_2);
  
  draw_rotation_strip(ufo_radius[1], ufo_radius[2],
                      ufo_z[1], ufo_z[2],
                      z_n_1, z_n_2,
                      r_n_1, r_n_2);

  // Make Top
  
  angle_1 = atan2(ufo_z[3] - ufo_z[2], ufo_radius[3] - ufo_radius[2])
    - 0.5 * M_PI;
  angle_2 = atan2(ufo_z[4] - ufo_z[3], ufo_radius[4] - ufo_radius[3])
    - 0.5 * M_PI;
  
  z_n_1 = sin(angle_1);
  r_n_1 = cos(angle_1);
    
  z_n_2 = sin(0.5 * (angle_1 + angle_2));
  r_n_2 = cos(0.5 * (angle_1 + angle_2));
  
  draw_rotation_strip(ufo_radius[2], ufo_radius[3],
                      ufo_z[2], ufo_z[3],
                      z_n_1, z_n_2,
                      r_n_1, r_n_2);

  z_n_1 = z_n_2;
  r_n_1 = r_n_2;

  z_n_2 = sin(angle_2);
  r_n_2 = cos(angle_2);
    
  draw_rotation_strip(ufo_radius[3], ufo_radius[4],
                      ufo_z[3], ufo_z[4],
                      z_n_1, z_n_2,
                      r_n_1, r_n_2);

  // Make Cockpit
  
  lemuria_set_material(&ufo_material_cockpit, GL_FRONT);

  theta_1 = 0.5 * M_PI;
  
  for(i = 1; i < UFO_COCKPIT_THETA_STEPS; i++)
    {
    theta_2 = 0.5 * M_PI - (i * 0.5 * M_PI)/(UFO_COCKPIT_THETA_STEPS-1);

    draw_rotation_strip(ufo_radius[4] * sin(theta_1),
                        ufo_radius[4] * sin(theta_2),
                        ufo_z[4] + ufo_radius[4] * cos(theta_1),
                        ufo_z[4] + ufo_radius[4] * cos(theta_2),
                        cos(theta_1), cos(theta_2),
                        sin(theta_1), sin(theta_2));
    theta_1 = theta_2;
    }
  

  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHT1);
  
  }

void lemuria_ufo_init(lemuria_engine_t * e,
                      lemuria_object_t * obj)
  {
  obj->draw = draw_ufo;
  }
