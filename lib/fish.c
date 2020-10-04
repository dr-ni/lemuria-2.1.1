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

#define FISH_ANGLE_START (20.0*M_PI/180.0)
#define FIN_WIDTH 1.0

void lemuria_fish_init(lemuria_engine_t * e,
                       lemuria_object_t * obj);


static lemuria_material_t fish_materials[] =
  {
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.0f, 0.2f, 0.0f, 1.0f },
      .ref_diffuse =  { 0.0f, 0.7f, 0.0f, 1.0f },
      .shininess = 128
    },
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.2f, 0.0f, 0.0f, 1.0f },
      .ref_diffuse =  { 0.7f, 0.0f, 0.0f, 1.0f },
      .shininess = 128
    },
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.2f, 0.2f, 0.0f, 1.0f },
      .ref_diffuse =  { 0.7f, 0.7f, 0.0f, 1.0f },
      .shininess = 128
    },
  };

#if 0
static void matrixmul(float ret[3], float mat[3][3], float vec[3])
  {
  ret[0] = mat[0][0] * vec[0] + mat[0][1] * vec[1] + mat[0][2] * vec[2];
  ret[1] = mat[1][0] * vec[0] + mat[1][1] * vec[1] + mat[1][2] * vec[2];
  ret[2] = mat[2][0] * vec[0] + mat[2][1] * vec[1] + mat[2][2] * vec[2];
  }
#endif
static void cross_product(float * ret, float * v_1, float * v_2)
  {
#if 0
  float len;
#endif
  ret[0] = v_1[1] * v_2[2] - v_1[2] * v_2[1];
  ret[1] = v_1[2] * v_2[0] - v_1[0] * v_2[2];
  ret[2] = v_1[0] * v_2[1] - v_1[1] * v_2[0];
#if 0
  len = sqrt(ret[0] * ret[0] + ret[1] * ret[1] + ret[2] * ret[2]);
  ret[0] /= len;
  ret[1] /= len;
  ret[2] /= len;
#endif
  }

static void update_fish(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  int i;
  float phase;
  float cos_angle;
  float sin_angle;
  float angle;

  float vec_1[3];
  float vec_2[3];
  
  obj->data.fish.phase_t += obj->data.fish.omega;
  if(obj->data.fish.phase_t > 2.0 * M_PI)
    obj->data.fish.phase_t -= 2.0 * M_PI;
    
  for(i = 0; i < FISH_FIN_STEPS; i++)
    {
    phase = obj->data.fish.phase_t - obj->data.fish.beta * (float)i;
    angle = obj->data.fish.amplitude * cos(phase);
    cos_angle = cos(angle);
    sin_angle = sin(angle);
    
    obj->data.fish.fin_coords[i][1][0] = obj->data.fish.fin_coords[i][0][0] +
      FIN_WIDTH * obj->data.fish.fin_angles_cos[i] * cos_angle;

    obj->data.fish.fin_coords[i][1][1] = FIN_WIDTH * sin_angle;
    
    obj->data.fish.fin_coords[i][1][2] = obj->data.fish.fin_coords[i][0][2] -
      FIN_WIDTH * obj->data.fish.fin_angles_sin[i] * cos_angle;
        
    }

  /* Calculate normals */

  vec_1[0] = obj->data.fish.fin_coords[0][1][0] - obj->data.fish.fin_coords[0][0][0];
  vec_1[1] = obj->data.fish.fin_coords[0][1][1] - obj->data.fish.fin_coords[0][0][1];
  vec_1[2] = obj->data.fish.fin_coords[0][1][2] - obj->data.fish.fin_coords[0][0][2];

  vec_2[0] = obj->data.fish.fin_coords[1][1][0] - obj->data.fish.fin_coords[0][1][0];
  vec_2[1] = obj->data.fish.fin_coords[1][1][1] - obj->data.fish.fin_coords[0][1][1];
  vec_2[2] = obj->data.fish.fin_coords[1][1][2] - obj->data.fish.fin_coords[0][1][2];

  cross_product(obj->data.fish.fin_normals[0], vec_1, vec_2);
  
  for(i = 1; i < FISH_FIN_STEPS-1; i++)
    {
    vec_1[0] = obj->data.fish.fin_coords[0][1][0] - obj->data.fish.fin_coords[0][0][0];
    vec_1[1] = obj->data.fish.fin_coords[0][1][1] - obj->data.fish.fin_coords[0][0][1];
    vec_1[2] = obj->data.fish.fin_coords[0][1][2] - obj->data.fish.fin_coords[0][0][2];

    vec_2[0] = obj->data.fish.fin_coords[i+1][1][0] - obj->data.fish.fin_coords[i-1][1][0];
    vec_2[1] = obj->data.fish.fin_coords[i+1][1][1] - obj->data.fish.fin_coords[i-1][1][1];
    vec_2[2] = obj->data.fish.fin_coords[i+1][1][2] - obj->data.fish.fin_coords[i-1][1][2];

    cross_product(obj->data.fish.fin_normals[i], vec_1, vec_2);
    }

  vec_1[0] = obj->data.fish.fin_coords[FISH_FIN_STEPS-1][1][0] -
    obj->data.fish.fin_coords[FISH_FIN_STEPS-1][0][0];
  vec_1[1] = obj->data.fish.fin_coords[FISH_FIN_STEPS-1][1][1] -
    obj->data.fish.fin_coords[FISH_FIN_STEPS-1][0][1];
  vec_1[2] = obj->data.fish.fin_coords[FISH_FIN_STEPS-1][1][2] -
    obj->data.fish.fin_coords[FISH_FIN_STEPS-1][0][2];
  
  vec_2[0] = obj->data.fish.fin_coords[FISH_FIN_STEPS-1][1][0] -
    obj->data.fish.fin_coords[FISH_FIN_STEPS-2][1][0];
  vec_2[1] = obj->data.fish.fin_coords[FISH_FIN_STEPS-1][1][1] -
    obj->data.fish.fin_coords[FISH_FIN_STEPS-2][1][1];
  vec_2[2] = obj->data.fish.fin_coords[FISH_FIN_STEPS-1][1][2] -
    obj->data.fish.fin_coords[FISH_FIN_STEPS-2][1][2];
  
  cross_product(obj->data.fish.fin_normals[FISH_FIN_STEPS-1], vec_1, vec_2);
  }

#define THETA_STEPS 20
#define PHI_STEPS   20

static void draw_fish(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  int i, j;
  float sin_theta_1;
  float sin_theta_2;
  float cos_theta_1;
  float cos_theta_2;
  float theta_2;
  float phi;
  float cos_phi;
  float sin_phi;
  float coords[3];
  glShadeModel(GL_SMOOTH);
  lemuria_object_rotate(obj);
  lemuria_set_material(&fish_materials[obj->data.fish.material],
                       GL_FRONT_AND_BACK);
  //  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  //  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  /* Draw body */
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_NORMALIZE);

  glPushMatrix();
  glScalef(obj->data.fish.axis_x,
           obj->data.fish.axis_y,
           obj->data.fish.axis_z);

  cos_theta_1 = 1.0;
  sin_theta_1 = 0.0;
  
  for(i = 1; i < THETA_STEPS; i++)
    {
    theta_2 = (float)i * M_PI / (float)(THETA_STEPS-1);
    cos_theta_2 = cos(theta_2);
    sin_theta_2 = sin(theta_2);

    glBegin(GL_QUAD_STRIP);

    for(j = 0; j < PHI_STEPS; j++)
      {
      phi = 2.0 * (float)j * M_PI / (float)(PHI_STEPS-1); 
      cos_phi = cos(phi);
      sin_phi = sin(phi);
      
      coords[0] = cos_theta_2;
      coords[1] = cos_phi * sin_theta_2;
      coords[2] = sin_phi * sin_theta_2;

      glNormal3fv(coords);
      glVertex3fv(coords);
            
      coords[0] = cos_theta_1;
      coords[1] = cos_phi * sin_theta_1;
      coords[2] = sin_phi * sin_theta_1;

      glNormal3fv(coords);
      glVertex3fv(coords);
      }
    glEnd();

    cos_theta_1 = cos_theta_2;
    sin_theta_1 = sin_theta_2;
    
    }

  glPopMatrix();
  
  /* Draw fins */
#if 0
  glDisable(GL_LIGHTING);

  glBegin(GL_LINES);
  for(i = 0; i < FISH_FIN_STEPS; i+=2)
    {
    glColor3f(0.0, 1.0, 0.0);

    glVertex3fv(obj->data.fish.fin_coords[i][0]);
    glVertex3fv(obj->data.fish.fin_coords[i][1]);

    glVertex3f(obj->data.fish.fin_coords[i][0][0],
               obj->data.fish.fin_coords[i][0][1],
               -obj->data.fish.fin_coords[i][0][2]);

    glVertex3f(obj->data.fish.fin_coords[i][1][0],
               obj->data.fish.fin_coords[i][1][1],
               -obj->data.fish.fin_coords[i][1][2]);

    /* Normals */
    
    glColor3f(1.0, 0.0, 0.0);

    glVertex3f(obj->data.fish.fin_coords[i][1][0],
               obj->data.fish.fin_coords[i][1][1],
               obj->data.fish.fin_coords[i][1][2]);

    glVertex3f(obj->data.fish.fin_coords[i][1][0] + 2.0 * obj->data.fish.fin_normals[i][0],
               obj->data.fish.fin_coords[i][1][1] + 2.0 * obj->data.fish.fin_normals[i][1],
               obj->data.fish.fin_coords[i][1][2] + 2.0 * obj->data.fish.fin_normals[i][2]);

    glVertex3f(obj->data.fish.fin_coords[i][1][0],
               obj->data.fish.fin_coords[i][1][1],
               -obj->data.fish.fin_coords[i][1][2]);

    glVertex3f(obj->data.fish.fin_coords[i][1][0] + 2.0 * obj->data.fish.fin_normals[i][0],
               obj->data.fish.fin_coords[i][1][1] + 2.0 * obj->data.fish.fin_normals[i][1],
               -obj->data.fish.fin_coords[i][1][2] + 2.0 * obj->data.fish.fin_normals[i][2]);
    
    }
  glEnd();
#else
  glBegin(GL_QUAD_STRIP);
  for(i = 0; i < FISH_FIN_STEPS; i++)
    {
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3fv(obj->data.fish.fin_coords[i][0]);

    glNormal3fv(obj->data.fish.fin_normals[i]);
    glVertex3fv(obj->data.fish.fin_coords[i][1]);
    }
  glEnd();
  
  glBegin(GL_QUAD_STRIP);
  for(i = 0; i < FISH_FIN_STEPS; i++)
    {
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(obj->data.fish.fin_coords[i][0][0],
               obj->data.fish.fin_coords[i][0][1],
               -obj->data.fish.fin_coords[i][0][2]);
    
    glNormal3f(obj->data.fish.fin_normals[i][0],
               obj->data.fish.fin_normals[i][1],
               -obj->data.fish.fin_normals[i][2]);

    glVertex3f(obj->data.fish.fin_coords[i][1][0],
               obj->data.fish.fin_coords[i][1][1],
               -obj->data.fish.fin_coords[i][1][2]);
    }
  glEnd();
#endif
  
  /* Reset everything */

  glDisable(GL_NORMALIZE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
  }

void lemuria_fish_init(lemuria_engine_t * e,
                      lemuria_object_t * obj)
  {
  int i;
  float angle;
  float cos_angle, sin_angle;
  float fin_angle;
  
  obj->draw = draw_fish;
  obj->update = update_fish;

  obj->data.fish.material = lemuria_random_int(e, 0,
                                               sizeof(fish_materials) /
                                               sizeof(fish_materials[0])-1);

  obj->data.fish.axis_x = 2.0;
  obj->data.fish.axis_y = 0.5;
  obj->data.fish.axis_z = 1.0;

  obj->data.fish.omega = 0.1;
  obj->data.fish.beta = 0.1;
  obj->data.fish.amplitude = 1.0;
  
  /* Init fin coordinates and angles */

  for(i = 0; i < FISH_FIN_STEPS; i++)
    {
    angle = FISH_ANGLE_START +
      (float)i * (M_PI - 2.0 * FISH_ANGLE_START)/(float)(FISH_FIN_STEPS);
    cos_angle = cos(angle);
    sin_angle = sin(angle);
    
    obj->data.fish.fin_coords[i][0][0] = obj->data.fish.axis_x * cos_angle;
    obj->data.fish.fin_coords[i][0][1] = 0.0;
    obj->data.fish.fin_coords[i][0][2] = -obj->data.fish.axis_z * sin_angle;

    fin_angle = atan2(obj->data.fish.axis_x * sin_angle,
                                         obj->data.fish.axis_z * cos_angle);

    obj->data.fish.fin_angles_cos[i] = cos(fin_angle);
    obj->data.fish.fin_angles_sin[i] = sin(fin_angle);
    }
  
  }
