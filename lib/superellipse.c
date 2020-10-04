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

// Formulas here are taken from
// http://astronomy.swin.edu.au/~pbourke/opengl/superellipsoid/
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

#define CASSINI_PHI_STEPS 40
#define CASSINI_THETA_STEPS 40

#define ELLIPSE_PHI_STEPS  20
#define ELLIPSE_THETA_STEPS 20
#define ELLIPSE_EPSILON 1e-6


static lemuria_light_t light =
  {
    .ambient =  { 0.3f, 0.3f, 0.3f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .position = { 10.0f, 10.0f, 30.0f, 1.0f },
  };

static lemuria_material_t material =
  {
    .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_ambient =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 50
  };

static float cassini_special_transition[2] = { 7.0, 1.0 };

static float cassini_specials[][2] =
  {
    { 0.01, 1.0 },   // Sphere
    { 1.2, 1.0   }, // Separated
  };

static int num_cassini_specials =
sizeof(cassini_specials)/sizeof(cassini_specials[0]);

static float ellipse_specials[][2] =
  {
    { 1.0, 1.0 },   // Sphere
    { 1.0, 0.03 },
    { 1.0, 2.0 },
    { 1.0, 4.0 },
    { 0.03, 1.0 },
    { 0.03, 0.03 },
    { 0.03, 2.0 },
    { 0.03, 4.0 },
    { 2.0, 1.0 },   // Sphere
    { 2.0, 0.03 },
    { 2.0, 2.0 },
    { 2.0, 4.0 },
    { 4.0, 1.0 },   // Sphere
    { 4.0, 0.03 },
    { 4.0, 2.0 },
    { 4.0, 4.0 },
  };

static int num_ellipse_specials =
sizeof(ellipse_specials)/sizeof(ellipse_specials[0]);

struct cassini_data
  {
  float coords[CASSINI_THETA_STEPS+1 ][2];
  float normals[CASSINI_THETA_STEPS+1][2];
  float lengths[CASSINI_THETA_STEPS];
  };

struct ellipse_data
  {
  float xy_phi[ELLIPSE_PHI_STEPS+1][2];
  float rz_theta[ELLIPSE_THETA_STEPS+1][2];

  float n_xy_phi[ELLIPSE_PHI_STEPS+1][2];
  float n_rz_theta[ELLIPSE_THETA_STEPS+1][2];

  float lengths_theta[ELLIPSE_THETA_STEPS+1];
  float lengths_phi[ELLIPSE_THETA_STEPS+1];
  float length_theta;
  float length_phi;
  };

typedef union
  {
  struct cassini_data cassini;
  struct ellipse_data ellipse;

  } aux_data;

typedef struct superellipse_data_s
  {
  aux_data aux;
  lemuria_rotator_t rotator;
  lemuria_scale_t scale_xy;
  lemuria_scale_t scale_z;
  lemuria_offset_t offset;
  float special[2];
  
  //  unsigned int superellipse_list;
  void (*draw_func)(lemuria_engine_t * e, struct superellipse_data_s*);

  lemuria_range_t deform_range;

  float * special_start;
  float * special_end;
  int must_stop;
  lemuria_engine_t * engine;
  } superellipse_data;

#if 0

static void superellipse(superellipse_data * d,
                         float theta, float phi, float * p, float * n)
  {
  float cos_theta;
  float sin_theta;
  float cos_phi;
  float sin_phi;

  float pow_cos_theta;
  
  cos_theta = cos(theta);
  sin_theta = sin(theta);
  cos_phi   = cos(phi);
  sin_phi   = sin(phi);

  pow_cos_theta = pow(cos_theta, d->special[0]);
  
  p[0] = pow_cos_theta * pow(cos_phi, d->special[1]);
  p[1] = pow_cos_theta * pow(sin_phi, d->special[1]);
  p[2] = pow(sin_theta, d->special[0]);

  pow_cos_theta = pow(cos_theta, 2.0 - d->special[0]);

  n[0] = pow_cos_theta * pow(cos_phi, 2.0 - d->special[1]);
  n[1] = pow_cos_theta * pow(sin_phi, 2.0 - d->special[1]);
  n[2] = pow(sin_theta, 2.0 - d->special[0]);
  }

#endif

static void draw_ellipse_part(struct ellipse_data * e,
                              int mirror_x, int mirror_y, int mirror_z,
                              float texture_x_start, float texture_x_end,
                              float texture_y_start, float texture_y_end)
  {
  int i, j;
  float sign_x;
  float sign_y;
  float sign_z;
  
  int num_mirror = 0;

  float texture_y_1;
  float texture_y_2;
  float texture_x;
  
  if(mirror_x)
    {
    sign_x = -1.0;
    num_mirror++;
    }
  else
    sign_x = 1.0;

  if(mirror_y)
    {
    sign_y = -1.0;
    num_mirror++;
    }
  else
    sign_y = 1.0;

  if(mirror_z)
    {
    sign_z = -1.0;
    num_mirror++;
    }
  else
    sign_z = 1.0;

  texture_y_1 = texture_y_start;
  texture_y_2 = texture_y_start;
  
  if(!(num_mirror % 2))
    {
    for(i = 0; i < ELLIPSE_THETA_STEPS; i++)
      {
      glBegin(GL_QUAD_STRIP);

      texture_y_2 += e->lengths_theta[i]/e->length_theta * (texture_y_end - texture_y_start);
      texture_x = texture_x_start;
      for(j = 0; j <= ELLIPSE_PHI_STEPS; j++)
        {
#if 0
        if((i== ELLIPSE_THETA_STEPS-1) && !num_mirror)
          fprintf(stderr, "length[%d]: %f, %f\n", j, e->lengths_phi[j], e->length_phi);
#endif
        glTexCoord2f(texture_x, texture_y_1);
        glNormal3f(sign_x * e->n_rz_theta[i][0]*e->n_xy_phi[j][0],
                   sign_y * e->n_rz_theta[i][0]*e->n_xy_phi[j][1],
                   sign_z * e->n_rz_theta[i][1]);
        glVertex3f(sign_x * e->rz_theta[i][0]*e->xy_phi[j][0],
                   sign_y * e->rz_theta[i][0]*e->xy_phi[j][1],
                   sign_z * e->rz_theta[i][1]);
        
        glTexCoord2f(texture_x, texture_y_2);
        glNormal3f(sign_x * e->n_rz_theta[i+1][0]*e->n_xy_phi[j][0],
                   sign_y * e->n_rz_theta[i+1][0]*e->n_xy_phi[j][1],
                   sign_z * e->n_rz_theta[i+1][1]);
        glVertex3f(sign_x * e->rz_theta[i+1][0]*e->xy_phi[j][0],
                   sign_y * e->rz_theta[i+1][0]*e->xy_phi[j][1],
                   sign_z * e->rz_theta[i+1][1]);
        texture_x += e->lengths_phi[j]/e->length_phi * (texture_x_end - texture_x_start);
        }
      texture_y_1 = texture_y_2;
      glEnd();
      }
    }
  else
    {
    for(i = 0; i < ELLIPSE_THETA_STEPS; i++)
      {
      glBegin(GL_QUAD_STRIP);
      
      texture_y_2 += e->lengths_theta[i]/e->length_theta * (texture_y_end - texture_y_start);
      texture_x = texture_x_start;
      for(j = 0; j <= ELLIPSE_PHI_STEPS; j++)
        {
        glTexCoord2f(texture_x, texture_y_2);
        glNormal3f(sign_x * e->n_rz_theta[i+1][0]*e->n_xy_phi[j][0],
                   sign_y * e->n_rz_theta[i+1][0]*e->n_xy_phi[j][1],
                   sign_z * e->n_rz_theta[i+1][1]);
        glVertex3f(sign_x * e->rz_theta[i+1][0]*e->xy_phi[j][0],
                   sign_y * e->rz_theta[i+1][0]*e->xy_phi[j][1],
                   sign_z * e->rz_theta[i+1][1]);

        glTexCoord2f(texture_x, texture_y_1);
        glNormal3f(sign_x * e->n_rz_theta[i][0]*e->n_xy_phi[j][0],
                   sign_y * e->n_rz_theta[i][0]*e->n_xy_phi[j][1],
                   sign_z * e->n_rz_theta[i][1]);
        glVertex3f(sign_x * e->rz_theta[i][0]*e->xy_phi[j][0],
                   sign_y * e->rz_theta[i][0]*e->xy_phi[j][1],
                   sign_z * e->rz_theta[i][1]);
        texture_x += e->lengths_phi[j]/e->length_phi * (texture_x_end - texture_x_start);
        }
      texture_y_1 = texture_y_2;
      glEnd();
      }
    
    }
  }

static void draw_ellipse(lemuria_engine_t * e, superellipse_data * d)
  {
  int i;
  float theta, phi;
  float sinus, cosinus;
  float tmp_1, tmp_2;
  
  /*
    float xy_phi[ELLIPSE_PHI_STEPS+1][2];
    float rz_theta[ELLIPSE_THETA_STEPS+1][2];
    
    float n_xy_phi[ELLIPSE_PHI_STEPS+1][2];
    float n_rz_theta[ELLIPSE_THETA_STEPS+1][2];
  */

  d->aux.ellipse.length_theta = 0.0;
  d->aux.ellipse.length_phi = 0.0;
  
  for(i = 0; i <= ELLIPSE_THETA_STEPS; i++)
    {
    if(!i)
      {
      if(d->special[0] < 1.0)
        {
        d->aux.ellipse.rz_theta[i][0] = 1.0;
        d->aux.ellipse.rz_theta[i][1] = 0.0;
        d->aux.ellipse.n_rz_theta[i][0] = 1.0;
        d->aux.ellipse.n_rz_theta[i][1] = 0.0;
        continue;
        }
      else
        theta = ELLIPSE_EPSILON;
      }
    else if(i == ELLIPSE_THETA_STEPS)
      {
      if(d->special[0] < 1.0)
        {
        d->aux.ellipse.rz_theta[i][0] = 0.0;
        d->aux.ellipse.rz_theta[i][1] = 1.0;
        d->aux.ellipse.n_rz_theta[i][0] = 0.0;
        d->aux.ellipse.n_rz_theta[i][1] = 1.0;

        tmp_1 = d->aux.ellipse.rz_theta[i-1][0]-d->aux.ellipse.rz_theta[i][0];
        tmp_2 = d->aux.ellipse.rz_theta[i-1][1]-d->aux.ellipse.rz_theta[i][1];
        
        d->aux.ellipse.lengths_theta[i-1] = sqrt(tmp_1*tmp_1 + tmp_2*tmp_2);
        d->aux.ellipse.length_theta += d->aux.ellipse.lengths_theta[i-1];
        continue;
        }
      else
      theta = 0.5 * M_PI - ELLIPSE_EPSILON;
      }
    else
      theta = (float)i/(float)ELLIPSE_THETA_STEPS*0.5*M_PI;

    cosinus = cos(theta);
    sinus = sin(theta);
    
    d->aux.ellipse.rz_theta[i][0] = pow(cosinus, d->special[0]);
    d->aux.ellipse.rz_theta[i][1] = pow(sinus, d->special[0]);
    d->aux.ellipse.n_rz_theta[i][0] = pow(cosinus, 2.0 - d->special[0]);
    d->aux.ellipse.n_rz_theta[i][1] = pow(sinus, 2.0 - d->special[0]);

    if(i)
      {
      tmp_1 = d->aux.ellipse.rz_theta[i-1][0]-d->aux.ellipse.rz_theta[i][0];
      tmp_2 = d->aux.ellipse.rz_theta[i-1][1]-d->aux.ellipse.rz_theta[i][1];
      
      d->aux.ellipse.lengths_theta[i-1] = sqrt(tmp_1*tmp_1 + tmp_2*tmp_2);
      d->aux.ellipse.length_theta += d->aux.ellipse.lengths_theta[i-1];
      }
    }

  for(i = 0; i <= ELLIPSE_PHI_STEPS; i++)
    {
    if(!i)
      {
      if(d->special[1] < 1.0)
        {
        d->aux.ellipse.xy_phi[i][0] = 1.0;
        d->aux.ellipse.xy_phi[i][1] = 0.0;
        d->aux.ellipse.n_xy_phi[i][0] = 1.0;
        d->aux.ellipse.n_xy_phi[i][1] = 0.0;
        continue;
        }
      else
        phi = ELLIPSE_EPSILON;
      }
    else if(i == ELLIPSE_PHI_STEPS)
      {
      if(d->special[1] < 1.0)
        {
        d->aux.ellipse.xy_phi[i][0] = 0.0;
        d->aux.ellipse.xy_phi[i][1] = 1.0;
        d->aux.ellipse.n_xy_phi[i][0] = 0.0;
        d->aux.ellipse.n_xy_phi[i][1] = 1.0;

        tmp_1 = d->aux.ellipse.xy_phi[i-1][0]-d->aux.ellipse.xy_phi[i][0];
        tmp_2 = d->aux.ellipse.xy_phi[i-1][1]-d->aux.ellipse.xy_phi[i][1];
        
        d->aux.ellipse.lengths_phi[i-1] = sqrt(tmp_1*tmp_1 + tmp_2*tmp_2);
        d->aux.ellipse.length_phi += d->aux.ellipse.lengths_phi[i-1];
        
        continue;
        }
      else
        phi = 0.5 * M_PI - ELLIPSE_EPSILON;
      }
    else
      phi = (float)i/(float)ELLIPSE_PHI_STEPS*0.5*M_PI;
    
    cosinus = cos(phi);
    sinus = sin(phi);

    d->aux.ellipse.xy_phi[i][0] = pow(cosinus, d->special[1]);
    d->aux.ellipse.xy_phi[i][1] = pow(sinus, d->special[1]);
    d->aux.ellipse.n_xy_phi[i][0] = pow(cosinus, 2.0 - d->special[1]);
    d->aux.ellipse.n_xy_phi[i][1] = pow(sinus, 2.0 - d->special[1]);

    if(i)
      {
      tmp_1 = d->aux.ellipse.xy_phi[i-1][0]-d->aux.ellipse.xy_phi[i][0];
      tmp_2 = d->aux.ellipse.xy_phi[i-1][1]-d->aux.ellipse.xy_phi[i][1];
      
      d->aux.ellipse.lengths_phi[i-1] = sqrt(tmp_1*tmp_1 + tmp_2*tmp_2);
      d->aux.ellipse.length_phi += d->aux.ellipse.lengths_phi[i-1];
      }
    }

  // Now, draw the stuff

  // Theta: 0..90, PHI: 0..90
  
  draw_ellipse_part(&(d->aux.ellipse), 0, 0, 0,
                    0.0, 0.25, 0.5, 1.0);

  // Theta: -90..0, PHI: 0..90

  draw_ellipse_part(&(d->aux.ellipse), 0, 0, 1,
                    0.0, 0.25, 0.5, 0.0);

  // Theta: 0..90, PHI: 360..270
  
  draw_ellipse_part(&(d->aux.ellipse), 0, 1, 0,
                    1.0, 0.75, 0.5, 1.0);

  // Theta: -90..0, PHI: 360..270

  draw_ellipse_part(&(d->aux.ellipse), 0, 1, 1,
                    1.0, 0.75, 0.5, 0.0);

  // Theta: 0..90, PHI: 180..90
  
  draw_ellipse_part(&(d->aux.ellipse), 1, 0, 0,
                    0.5, 0.25, 0.5, 1.0);

  // Theta: -90..0, PHI: 180..90

  draw_ellipse_part(&(d->aux.ellipse), 1, 0, 1,
                    0.5, 0.25, 0.5, 0.0);

  // Theta: 0..90, PHI: 180..270
  
  draw_ellipse_part(&(d->aux.ellipse), 1, 1, 0,
                    0.5, 0.75, 0.5, 1.0);

  // Theta: -90..0, PHI: 180..270

  draw_ellipse_part(&(d->aux.ellipse), 1, 1, 1,
                    0.5, 0.75, 0.5, 0.0);
  };

static void cassini(superellipse_data * d,
                    float theta, float * x, float sign)
  {
  float sin_2_theta; 
  float c;
  float tmp;
    
  sin_2_theta = sin(2.0 * theta);

  tmp = d->special[1]*d->special[1]*d->special[1]*d->special[1]/
    (d->special[0]*d->special[0]*d->special[0]*d->special[0]) -
    sin_2_theta*sin_2_theta;

  if(tmp < 0.0)
    tmp = 0.0;
  
  c = d->special[0]*d->special[0] * (cos(2.0 * theta) + sign * sqrt(tmp));
  //  fprintf(stderr, "theta: %f, c: %f tmp: %f\n", theta, c, tmp);

  if(c < 0.0)
    c = 0.0;
 
  c = sqrt(c);

  x[0] = cos(theta) * c;
  x[1] = sin(theta) * c;
  }

//#define DEBUG_PRINTF

#ifdef DEBUG_PRINTF
static int printed = 0;
static int ring_number = 0;
static float normal_factor = 0.1;
#endif

static void
draw_cassini_ring(float * p_1,
                  float * p_2,
                  float * n_1,
                  float * n_2,
                  float texture_y_1,
                  float texture_y_2)
  {

  int i;
  float phi;
  
  float sin_phi = 0.0;
  float cos_phi = 1.0;
  
  //  glPolygonMode(GL_FRONT, GL_LINE);
  glBegin(GL_QUAD_STRIP);

#ifdef DEBUG_PRINTF
  if(!printed)
    {
    //    if(!ring_number)
    //      {
      printf("%f %f %f %f\n", p_2[0], p_2[1],
             normal_factor *
             n_2[0]/sqrt(n_2[0]*n_2[0]+
                         n_2[1]*n_2[1]),
             normal_factor *
             n_2[1]/sqrt(n_2[0]*n_2[0]+
                         n_2[1]*n_2[1]));
    
      //      }
      
    printf("%f %f %f %f\n", p_1[0], p_1[1],
           normal_factor *
           n_1[0]/sqrt(n_1[0]*n_1[0]+
                       n_1[1]*n_1[1]),
           normal_factor *
           n_1[1]/sqrt(n_1[0]*n_1[0]+
                       n_1[1]*n_1[1]));
    ring_number++;
    if(ring_number == CASSINI_THETA_STEPS)
      {
      printed = 1;
      fflush(stdout);
      }
    
    }
  
#endif
  
  for(i = 0; i <= CASSINI_PHI_STEPS; i++)
    {
    phi = 2.0 * M_PI * (float)(i) / (float)CASSINI_PHI_STEPS;
    
    cos_phi = cos(phi);
    sin_phi = sin(phi);
    
    glTexCoord2f(phi/(2.0*M_PI), 0.5 + texture_y_1);
    glNormal3f(n_2[1] * cos_phi, n_2[1] * sin_phi, n_2[0]);
    glVertex3f(p_2[1] * cos_phi, p_2[1] * sin_phi, p_2[0]);

    glTexCoord2f(phi/(2.0*M_PI), 0.5 + texture_y_2);
    glNormal3f(n_1[1] * cos_phi, n_1[1] * sin_phi, n_1[0]);
    glVertex3f(p_1[1] * cos_phi, p_1[1] * sin_phi, p_1[0]);
    }
  glEnd();

  glBegin(GL_QUAD_STRIP);
  
  for(i = 0; i <= CASSINI_PHI_STEPS; i++)
    {
    phi = 2.0 * M_PI * (float)(i) / (float)CASSINI_PHI_STEPS;
    
    cos_phi = cos(phi);
    sin_phi = sin(phi);

    glTexCoord2f(phi/(2.0*M_PI), 0.5 - texture_y_2);
    glNormal3f(n_1[1] * cos_phi, n_1[1] * sin_phi, -n_1[0]);
    glVertex3f(p_1[1] * cos_phi, p_1[1] * sin_phi, -p_1[0]);

    glTexCoord2f(phi/(2.0*M_PI), 0.5 - texture_y_1);
    glNormal3f(n_2[1] * cos_phi, n_2[1] * sin_phi, -n_2[0]);
    glVertex3f(p_2[1] * cos_phi, p_2[1] * sin_phi, -p_2[0]);
    }
  glEnd();

  //  glPolygonMode(GL_FRONT, GL_FILL);
  }

static void draw_cassini(lemuria_engine_t * e, superellipse_data * d)
  {

  float texture_y_1;
  float texture_y_2;
  float  theta_max, theta;

  float length;
  
  int i;
  
  float sign;
  float tmp_1, tmp_2;
  
  // Calculate coordinates
  
  if(d->special[0] < d->special[1])
    {
    theta_max = 0.5 * M_PI;
    
    for(i = 0; i <= CASSINI_THETA_STEPS; i++)
      {
      theta = (float)i/(float)(CASSINI_THETA_STEPS) * theta_max;
      cassini(d, theta, d->aux.cassini.coords[i], 1.0);
      }

    // We know these in advance
    
    d->aux.cassini.normals[0][0] = 1.0;
    d->aux.cassini.normals[0][1] = 0.0;
        
    d->aux.cassini.normals[CASSINI_THETA_STEPS][0] = 0.0;
    d->aux.cassini.normals[CASSINI_THETA_STEPS][1] = 1.0;
    }
  else
    {
    theta_max = 0.5 *
      asin(d->special[1]*d->special[1]/(d->special[0]*d->special[0]));

    for(i = 0; i <= CASSINI_THETA_STEPS; i++)
      {
      if(i <= CASSINI_THETA_STEPS / 2)
        {
        theta = (float)i/(float)(CASSINI_THETA_STEPS/2) * theta_max;
        sign = 1.0;
        }
      else
        {
        theta = theta_max - (float)(i-CASSINI_THETA_STEPS/2)/
          (float)(CASSINI_THETA_STEPS/2) * theta_max;
        sign = -1.0;
        }
      cassini(d, theta, d->aux.cassini.coords[i], sign);
      }
    d->aux.cassini.normals[0][0] = 1.0;
    d->aux.cassini.normals[0][1] = 0.0;
        
    d->aux.cassini.normals[CASSINI_THETA_STEPS][0] = -1.0;
    d->aux.cassini.normals[CASSINI_THETA_STEPS][1] =  0.0;

    }

  // Calculate normals

  for(i = 1; i < CASSINI_THETA_STEPS; i++)
    {
    d->aux.cassini.normals[i][0] =
      (d->aux.cassini.coords[i+1][1] - d->aux.cassini.coords[i-1][1]);
    d->aux.cassini.normals[i][1] =
      -(d->aux.cassini.coords[i+1][0] - d->aux.cassini.coords[i-1][0]);
    }

  // Calculate lengths and total length

  length = 0.0;
  
  for(i = 0; i < CASSINI_THETA_STEPS; i++)
    {
    tmp_1 = d->aux.cassini.coords[i+1][0] - d->aux.cassini.coords[i][0];
    tmp_2 = d->aux.cassini.coords[i+1][1] - d->aux.cassini.coords[i][1];
    d->aux.cassini.lengths[i] = sqrt(tmp_1 * tmp_1 + tmp_2 * tmp_2);
    length += d->aux.cassini.lengths[i];
    }

  // Draw the stuff

  texture_y_1 = 0.5;
    
  for(i = 0; i < CASSINI_THETA_STEPS; i++)
    {
    texture_y_2 = texture_y_1 - 0.5 * d->aux.cassini.lengths[i] / length;

    draw_cassini_ring(d->aux.cassini.coords[i],
                      d->aux.cassini.coords[i+1],
                      d->aux.cassini.normals[i],
                      d->aux.cassini.normals[i+1],
                      texture_y_2,
                      texture_y_1);

    texture_y_1 = texture_y_2;
    }
  
  
  };


static void change_program(lemuria_engine_t * e,
                           superellipse_data * data)
  {
  if(!lemuria_decide(e, 0.3))
    return;
    
  if(data->draw_func == draw_cassini)
    {
    // Cassini Oval became a sphere
    if(data->special_end == cassini_specials[0])
      {

      // Change Cassini
      if(lemuria_decide(e, 0.3))
        {
        data->special_start = data->special_end;
        data->special_end =
          cassini_specials[lemuria_random_int(e, 1, num_cassini_specials-1)];
        //        fprintf(stderr, "Change Cassini\n");
        }
      // Make ellipse
      else
        {
        data->special_start = ellipse_specials[0];
        data->special_end =
          ellipse_specials[lemuria_random_int(e, 1, num_ellipse_specials-1)];
        data->draw_func = draw_ellipse;
        //        fprintf(stderr, "Cassini -> Ellipse\n");
        }
      lemuria_range_init(e, &(data->deform_range), 2, 100, 200);
      }
    // Cassini Oval became something different
    else
      {
      data->special_start = data->special_end;
      data->special_end = cassini_specials[0];
      lemuria_range_init(e, &(data->deform_range), 2, 100, 200);
      //      fprintf(stderr, "Change Cassini\n");
      }
    //        fprintf(stderr, "Draw Ellipse\n");
    }
  else // Ellipse
    {
    if(data->special_end == ellipse_specials[0]) // Ellipse became sphere
      {
      if(lemuria_decide(e, 0.2))
        {
        if(lemuria_decide(e, 0.7)) // Make cassini
          {
          data->special_start = cassini_specials[0];
          data->special_end = cassini_specials[1];
          data->draw_func = draw_cassini;
          lemuria_offset_reset(e, &(data->offset));
          //          fprintf(stderr, "Ellipse -> Cassini\n");
          }
        else
          {
          data->special_start = data->special_end;
          data->special_end =
            ellipse_specials[lemuria_random_int(e, 1, num_ellipse_specials-1)];
          //          fprintf(stderr, "Change Ellipse\n");
          }
        lemuria_range_init(e, &(data->deform_range), 2, 100, 200);
        }
                    
      }
    else
      {
      data->special_start = data->special_end;
      if(lemuria_decide(e, 0.2))
        data->special_end = ellipse_specials[0];
      else
        data->special_end =
          ellipse_specials[lemuria_random_int(e, 1, num_ellipse_specials-1)];
      lemuria_range_init(e, &(data->deform_range), 2, 100, 200);
      //      fprintf(stderr, "Change Ellipse\n");
      }
    //        fprintf(stderr, "Draw Cassini\n");
    }
  }
    
  

static void draw_superellipse(lemuria_engine_t * e, void * user_data)
  {
  //  int random_number;
  float scale_xy;
  float scale_z;
  superellipse_data * data = (superellipse_data*)(user_data);

  /* Check whether to change something */

  if(e->foreground.mode == EFFECT_FINISH)
    {
    e->foreground.mode = EFFECT_FINISHING;
    //    data->draw_func = draw_cassini;
    lemuria_offset_reset(e, &(data->offset));

    data->scale_z.scale_min = 0.3;
    data->scale_z.scale_max = 0.8;
    lemuria_scale_change(e, &(data->scale_z));
    data->must_stop = 1;
    }

  if(data->must_stop)
    {
    if(data->draw_func == draw_cassini)
      {
      if(lemuria_range_done(&(data->deform_range)) &&
         (data->special_end != cassini_special_transition))
        {
        data->special_start = data->special_end;
        data->special_end = cassini_special_transition;
        lemuria_range_init(e, &(data->deform_range), 2, 100, 200);
        }
      }
    // Shpere again, now blow it up
    if((data->draw_func == draw_ellipse) && lemuria_range_done(&(data->deform_range)))
      {
      // Shpere again, now blow it up
      if(data->special_end == ellipse_specials[0])
        {
        data->special_start = cassini_specials[0];
        data->special_end = cassini_special_transition;
        data->draw_func = draw_cassini;
        lemuria_range_init(e, &(data->deform_range), 2, 100, 200);
        }
      // Something different, make it a sphere
      else
        {
        data->special_start = data->special_end;
        data->special_end = ellipse_specials[0];
        lemuria_range_init(e, &(data->deform_range), 2, 100, 200);
        }
      }
    }
  else if(e->beat_detected)
    {
    if((e->foreground.mode == EFFECT_RUNNING) && lemuria_decide(e, 0.1))
      lemuria_rotator_change(e, &(data->rotator));

    if(lemuria_decide(e, 0.1))
      {
      if(lemuria_decide(e, 0.2))
        {
        data->scale_z.scale_min = 0.03;
        data->scale_z.scale_max = 0.05;
        }
      else
        {
        data->scale_z.scale_min = 0.3;
        data->scale_z.scale_max = 0.8;
        }
      lemuria_scale_change(e, &(data->scale_xy));
      lemuria_scale_change(e, &(data->scale_z));
      }
    
    if((e->foreground.mode == EFFECT_RUNNING) &&
       lemuria_range_done(&(data->deform_range)))
      change_program(e, data);

    if(lemuria_decide(e, 0.1) && data->draw_func != draw_cassini)
      lemuria_offset_change(e, &(data->offset));

    if(lemuria_decide(e, 0.1) && data->draw_func != draw_cassini)
      lemuria_offset_kick(&(data->offset));
    }
  
  if((e->foreground.mode == EFFECT_RUNNING) ||
     ((e->foreground.mode == EFFECT_FINISHING) &&
      data->draw_func != draw_cassini))
    {
    lemuria_rotator_update(&(data->rotator));
    }

  lemuria_offset_update(&(data->offset));
  lemuria_scale_update(&(data->scale_xy));
  lemuria_scale_update(&(data->scale_z));
    
  lemuria_range_update(&(data->deform_range));
  if((e->foreground.mode == EFFECT_STARTING) &&
     (lemuria_range_done(&(data->deform_range))))
    {
    e->foreground.mode = EFFECT_RUNNING;
    data->scale_xy.scale_min = 0.3;
    data->scale_xy.scale_max = 0.8;
    data->scale_z.scale_min = 0.3;
    data->scale_z.scale_max = 0.8;
    }
  
  lemuria_range_get_cos(&(data->deform_range), data->special_start,
                        data->special_end, data->special);
  
  //  fprintf(stderr, "Special: %f %f\n", data->special[0], data->special[1]);
  
  glShadeModel(GL_SMOOTH);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_NORMALIZE);

  //  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
  //  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

  lemuria_set_perspective(e, 1, 10.0);
  
  // Set up and enable light 0

  lemuria_set_light(&light, GL_LIGHT0);
  glEnable(GL_LIGHT0);
  
  // All materials hereafter have full specular reflectivity
  // with a high shine

  lemuria_set_material(&material, GL_FRONT);

  // Enable lighting
  glEnable(GL_LIGHTING);

  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  
  scale_xy = lemuria_scale_get(&(data->scale_xy));
  scale_z = lemuria_scale_get(&(data->scale_z));
  //  fprintf(stderr, "Scale factor: %f %f\n", scale_xy, scale_z);
  
  lemuria_offset_translate(&(data->offset));
  
  
  lemuria_rotate(&(data->rotator));
  //  glRotatef(90.0, 1.0, 0.0, 0.0);

  //  glScalef(0.7, 0.7, 0.7);

  glScalef(scale_xy, scale_xy, scale_z);

  lemuria_texture_bind(e, 0);

  //  glEnable(GL_CULL_FACE);
  
  data->draw_func(e, data);

  //  glDisable(GL_CULL_FACE);
  
  glPopMatrix();
  
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  //  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_NORMALIZE);

  // Check for stop
  if(data->must_stop && (data->draw_func == draw_cassini) &&
     (data->special_end == cassini_special_transition) &&
     lemuria_range_done(&(data->deform_range)))
    e->foreground.mode = EFFECT_DONE;
  }

static void * init_superellipse(lemuria_engine_t * e)
  {
  int random_number;
  superellipse_data * data;

#ifdef DEBUG
  fprintf(stderr, "init_superellipse...");
#endif
  
  data = calloc(1, sizeof(superellipse_data));
  data->engine = e;
  lemuria_texture_ref(data->engine, 0);

  e->foreground.mode = EFFECT_STARTING;

  lemuria_rotator_init(e, &(data->rotator));
  lemuria_scale_init(e, &(data->scale_xy), 0.4, 0.6);
  lemuria_scale_init(e, &(data->scale_z), 0.4, 0.6);
  lemuria_offset_init(e, &(data->offset));

  data->draw_func = draw_cassini;

  data->special_start = cassini_special_transition;
  //  data->special_start = ellipse_specials[0];
  
  random_number = lemuria_random_int(e, 0, num_cassini_specials-1);
  
  data->special_end = cassini_specials[random_number];
  
  lemuria_range_init(e, &(data->deform_range), 2, 200, 400);
  
  //  data->special[0] = 2.0;
  //  data->special[1] = 1.0;

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  
  return data;
  }

static void delete_superellipse(void * e)
  {
  superellipse_data * data = (superellipse_data*)(e);
  lemuria_texture_unref(data->engine, 0);
  free(data);
  }

effect_plugin_t superellipse_effect =
  {
    .init =    init_superellipse,
    .draw =    draw_superellipse,
    .cleanup = delete_superellipse,
  };
