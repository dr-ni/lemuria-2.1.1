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
#include <material.h>

void lemuria_satellite_init(lemuria_engine_t * e,
                            lemuria_object_t * obj);


static lemuria_material_t satellite_material =
  {
    .ref_specular = { 0.0f, 0.0f, 0.0f, 1.0f },
    .ref_ambient =  { 0.2f, 0.2f, 0.2f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 128
  };

static lemuria_material_t satellite_antenna_material =
  {
    .ref_specular = { 0.0f, 0.0f, 0.0f, 1.0f },
    .ref_ambient =  { 0.4f, 0.4f, 0.4f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 128
  };

static lemuria_material_t satellite_solar_material =
  {
    .ref_specular = { 0.0f, 0.0f, 0.0f, 1.0f },
    .ref_ambient =  { 0.0f, 0.0f, 0.5f, 1.0f },
    .ref_diffuse =  { 0.0f, 0.0f, 1.0f, 1.0f },
    .shininess = 128
  };

#define SATELLITE_RADIUS 1.0
#define SATELLITE_HEIGHT 1.0
#define SATELLITE_ANTENNA_ANGLE 40.0
#define SATELLITE_ANTENNA_RADIUS 2.5
#define SATELLITE_ANTENNA_THETA_STEPS 3

#define SATELLITE_SOLAR_OFFSET 0.3
#define SATELLITE_SOLAR_WIDTH  1.0
#define SATELLITE_SOLAR_LENGTH 5.0
#define SATELLITE_SOLAR_ANGLE 30.0

#define SATELLITE_ANTENNA_TOP_RADIUS 0.2
#define SATELLITE_ANTENNA_TOP_HEIGHT 0.3

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

static void draw_disc(float r, float z, float z_n)
  {
  float x_1 = r;
  float y_1 = 0.0;
  float x_2;
  float y_2;
  
  float phi;
  int i;

  glBegin(GL_TRIANGLES);

  glNormal3f(0.0, 0.0, z_n);
  
  for(i = 0; i <= OBJECT_PHI_STEPS; i++)
    {
    phi = 2.0 * M_PI * (i+1)/ (float)OBJECT_PHI_STEPS;
    x_2 = r * cos(phi);
    y_2 = r * sin(phi);

    if(z_n > 0.0)
      {
      glVertex3f(x_1, y_1, z);
      glVertex3f(x_2, y_2, z);
      }
    else
      {
      glVertex3f(x_2, y_2, z);
      glVertex3f(x_1, y_1, z);
      }
    glVertex3f(0.0, 0.0, z);

    x_1 = x_2;
    y_1 = y_2; 
    }
  
  glEnd();
    
  }

static void draw_satellite(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  int i;
  float theta_1;
  float theta_2;
  float z_n_1;
  float z_n_2;

  float r_n_1;
  float r_n_2;

  float z_1, r_1, z_2, r_2;

  float x_1, x_2, y, z;

  glLineWidth(2.0);
  glShadeModel(GL_SMOOTH);

  glEnable(GL_POLYGON_SMOOTH);
  glEnable(GL_LINE_SMOOTH);

  //  glEnable(GL_CULL_FACE);
  
  lemuria_set_material(&satellite_material, GL_FRONT_AND_BACK);
    

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  //  lemuria_set_light(&satellite_light, GL_LIGHT0);
    
  // lemuria_rotator_update(&obj->rotator);

  glRotatef(obj->data.satellite.rotation[0], 1.0, 0.0, 0.0);
  glRotatef(obj->data.satellite.rotation[1], 0.0, 1.0, 0.0);
  glRotatef(obj->data.satellite.rotation[2], 0.0, 0.0, 1.0);

  
  /* Draw main body */
  
  draw_rotation_strip(SATELLITE_RADIUS, SATELLITE_RADIUS,
                      -0.5 * SATELLITE_HEIGHT, 0.5 * SATELLITE_HEIGHT,
                      0.0, 0.0,
                      1.0, 1.0);

  draw_disc(SATELLITE_RADIUS, 0.5 * SATELLITE_HEIGHT, 1.0);
  draw_disc(SATELLITE_RADIUS, -0.5 * SATELLITE_HEIGHT, -1.0);
  
  /* Draw antenna */

  theta_1 = 0.0;
  z_n_1 = 1.0;
  r_n_1 = 0.0;
  z_1 = 0.5 * SATELLITE_HEIGHT;
  r_1 = 0.0;

  lemuria_set_material(&satellite_antenna_material, GL_FRONT_AND_BACK);
  
  for(i = 0; i < SATELLITE_ANTENNA_THETA_STEPS; i++)
    {
    theta_2 = M_PI * SATELLITE_ANTENNA_ANGLE / 180.0 *
      (float)i / (float)(SATELLITE_ANTENNA_THETA_STEPS-1);
    
    z_n_2 = cos(theta_2);    
    r_n_2 = -sin(theta_2);
    r_2 = sin(theta_2) * SATELLITE_ANTENNA_RADIUS;
    z_2 = 0.5 * SATELLITE_HEIGHT +
      SATELLITE_ANTENNA_RADIUS * (1.0 - cos(theta_2));

    draw_rotation_strip(r_2, r_1, z_2, z_1,
                        z_n_2, z_n_1, r_n_2, r_n_1);
      
    z_n_1 = z_n_2;
    r_n_1 = r_n_2;
    z_1 = z_2;
    r_1 = r_2;
    }

    /* Draw solar cells */

  lemuria_set_material(&satellite_solar_material, GL_FRONT_AND_BACK);

  x_1 = SATELLITE_RADIUS + SATELLITE_SOLAR_OFFSET;
  x_2 = SATELLITE_RADIUS + SATELLITE_SOLAR_OFFSET + SATELLITE_SOLAR_LENGTH;

  y = SATELLITE_SOLAR_WIDTH * cos(SATELLITE_SOLAR_ANGLE * M_PI / 180.0);
  z = SATELLITE_SOLAR_WIDTH * sin(SATELLITE_SOLAR_ANGLE * M_PI / 180.0);
    
  glBegin(GL_QUADS);
  glVertex3f(x_1, -y, -z);
  glVertex3f(x_2, -y, -z);

  glVertex3f(x_2, y, z);
  glVertex3f(x_1, y, z);

  glVertex3f(-x_2, -y, -z);
  glVertex3f(-x_1, -y, -z);

  glVertex3f(-x_1, y, z);
  glVertex3f(-x_2, y, z);
  
  glEnd();

  /* Draw the top of the antenna */
  
  r_1 = SATELLITE_ANTENNA_TOP_RADIUS;
  r_2 = SATELLITE_ANTENNA_TOP_RADIUS;

  r_n_1 = 1.0;
  r_n_2 = 1.0;
  z_n_1 = 0.0;
  z_n_2 = 0.0;

  z_1 = 0.5 * SATELLITE_HEIGHT + SATELLITE_ANTENNA_RADIUS 
    - 0.5 * SATELLITE_ANTENNA_TOP_HEIGHT;

  z_2 = 0.5 * SATELLITE_HEIGHT + SATELLITE_ANTENNA_RADIUS 
    + 0.5 * SATELLITE_ANTENNA_TOP_HEIGHT;

  lemuria_set_material(&satellite_antenna_material, GL_FRONT);

  draw_rotation_strip(r_1, r_2, z_1, z_2,
                      z_n_1, z_n_2, r_n_1, r_n_2);

  draw_disc(SATELLITE_ANTENNA_TOP_RADIUS, z_1, -1.0);
  draw_disc(SATELLITE_ANTENNA_TOP_RADIUS, z_2,  1.0);
  
  
  /* Draw line stuff */
  
  glColor4f(0.7, 0.7, 0.7, 1.0);

  lemuria_set_material(&satellite_antenna_material, GL_FRONT_AND_BACK);
  
  glBegin(GL_LINES);

  r_1 = SATELLITE_ANTENNA_RADIUS *
    sin(M_PI * SATELLITE_ANTENNA_ANGLE / 180.0);
  z_1 = 0.5 * SATELLITE_HEIGHT + SATELLITE_ANTENNA_RADIUS *
    (1.0 - cos(M_PI * SATELLITE_ANTENNA_ANGLE / 180.0));

  z_2 = 0.5 * SATELLITE_HEIGHT + SATELLITE_ANTENNA_RADIUS;

  glNormal3f(0.707 * (z_2 - z_1)/SATELLITE_ANTENNA_RADIUS,
             0.707 * (z_2 - z_1)/SATELLITE_ANTENNA_RADIUS,
             r_1 / SATELLITE_ANTENNA_RADIUS);
    
  glVertex3f(0.707 * r_1, 0.707 * r_1, z_1);
  glVertex3f(0.0, 0.0, z_2);

  glNormal3f(-0.707 * (z_2 - z_1)/SATELLITE_ANTENNA_RADIUS,
             0.707 * (z_2 - z_1)/SATELLITE_ANTENNA_RADIUS,
             r_1 / SATELLITE_ANTENNA_RADIUS);

  glVertex3f(-0.707 * r_1, 0.707 * r_1, z_1);
  glVertex3f(0.0, 0.0, z_2);

  glNormal3f(-0.707 * (z_2 - z_1)/SATELLITE_ANTENNA_RADIUS,
             -0.707 * (z_2 - z_1)/SATELLITE_ANTENNA_RADIUS,
             r_1 / SATELLITE_ANTENNA_RADIUS);
  
  glVertex3f(-0.707 * r_1, -0.707 * r_1, z_1);
  glVertex3f(0.0, 0.0, z_2);

  glNormal3f(0.707 * (z_2 - z_1)/SATELLITE_ANTENNA_RADIUS,
             -0.707 * (z_2 - z_1)/SATELLITE_ANTENNA_RADIUS,
             r_1 / SATELLITE_ANTENNA_RADIUS);

  
  glVertex3f(0.707 * r_1, -0.707 * r_1, z_1);
  glVertex3f(0.0, 0.0, z_2);

  /* Draw supports for the solar cells */

  glVertex3f(SATELLITE_RADIUS, 0.0, 0.0);
  glVertex3f(SATELLITE_RADIUS + SATELLITE_SOLAR_OFFSET, 0.0, 0.0);

  glVertex3f(-SATELLITE_RADIUS, 0.0, 0.0);
  glVertex3f(-SATELLITE_RADIUS - SATELLITE_SOLAR_OFFSET, 0.0, 0.0);
  
  glEnd();

  glDisable(GL_POLYGON_SMOOTH);
  glDisable(GL_LINE_SMOOTH);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);

  //  glDisable(GL_CULL_FACE);

  }

void lemuria_satellite_init(lemuria_engine_t * e,
                            lemuria_object_t * obj)
  {
  obj->draw = draw_satellite;
  obj->data.satellite.rotation[0] = lemuria_random(e, 0.0, 360.0);
  obj->data.satellite.rotation[1] = lemuria_random(e, 0.0, 360.0);
  obj->data.satellite.rotation[2] = lemuria_random(e, 0.0, 360.0);
  }
