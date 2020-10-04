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
#include <string.h>

static lemuria_light_t light =
  {
    .position = { 5.0f, 5.0f, 10.0f, 1.0f },
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 1.0f, 1.0f, 1.0f, 1.0f }
  };
                                                                                
static lemuria_material_t material =
  {
    .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 128
  };

// #define TESTONLY

#define EPS 1.0e-4

#define DRAW_GRID

/*
 *  First part of the sourcefile is the pure mathematics
 *  of this thing. Lemuria- and OpenGL related stuff comes
 *  after
 */


/* Number of grid points in each direction */

#define SIZE_PHI   160
#define SIZE_THETA 80

#define DEFORM_INT   0
#define DEFORM_FLOAT 1

#define TRANSITION_NONE      0
#define TRANSITION_DEFORM_1  1
#define TRANSITION_DEFORM_2  2
#define TRANSITION_EXPLODE   3

typedef struct
  {
  float f;
  float f_deriv;
  float g;
  float g_deriv;
  } coords_data;

typedef struct bubble_data_s
  {
  float r_phi_1;
  float r_phi_2;
  float r_theta_1;
  float r_theta_2;

  float exp_phi;
  float exp_theta;

  int m_phi;
  int m_theta;
  float (*r_func)(coords_data * data_u, coords_data * data_v,
                  float * d_r_d_u, float * d_r_d_v);
  
  float (*g_phi_func)(float t, float e, float * deriv);
  float (*g_theta_func)(float t, float e, float * deriv);
  
  } bubble_data_t;

static void bubble_data_copy(bubble_data_t * dst,
                      bubble_data_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }

static void bubble_data_copy_real(bubble_data_t * dst,
                           bubble_data_t * src)
  {
  dst->r_phi_1   = src->r_phi_1;
  dst->r_phi_2   = src->r_phi_2;
  dst->r_theta_1 = src->r_theta_1;
  dst->r_theta_2 = src->r_theta_2;

  dst->exp_phi   = src->exp_phi;
  dst->exp_theta = src->exp_theta;
  }

static void bubble_data_interpolate_real(lemuria_range_t * range,
                                  bubble_data_t * start,
                                  bubble_data_t * end,
                                  bubble_data_t * ret)
  {
  lemuria_range_get(range, &(start->r_phi_1), &(end->r_phi_1), &(ret->r_phi_1));
  lemuria_range_get(range, &(start->r_phi_2), &(end->r_phi_2), &(ret->r_phi_2));

  lemuria_range_get(range, &(start->r_theta_1), &(end->r_theta_1), &(ret->r_theta_1));
  lemuria_range_get(range, &(start->r_theta_2), &(end->r_theta_2), &(ret->r_theta_2));

  lemuria_range_get(range, &(start->exp_phi), &(end->exp_phi), &(ret->exp_phi));
  lemuria_range_get(range, &(start->exp_theta), &(end->exp_theta), &(ret->exp_theta));
  
  }
                                  
typedef struct
  {
  float coords[SIZE_PHI * SIZE_THETA * 3];
  float normals[SIZE_PHI * SIZE_THETA * 3];

  float * coord_matrix[SIZE_THETA][SIZE_PHI];
  float * normal_matrix[SIZE_THETA][SIZE_PHI];

  float r_max;
    
  } bubble_coords_t;

static void init_bubble_coords(bubble_coords_t * c)
  {
  int i, j;
  for(i = 0; i < SIZE_THETA; i++)
    {
    for(j = 0; j < SIZE_PHI; j++)
      {
      c->coord_matrix[i][j]  = &(c->coords[i*SIZE_PHI*3 + j * 3]);
      c->normal_matrix[i][j] = &(c->normals[i*SIZE_PHI*3 + j * 3]);
      }
    }
  }

/* Utility functions */

static float g_func_1(float t, float e, float * deriv)
  {
  float ret;
  if(t < EPS)
    {
    t = EPS;
    ret = 0.0;
    }
  else
    ret = pow(t, e);
  *deriv = e * pow(t, e - 1.0);
  return ret;
  }

static float g_func_2(float t, float e, float * deriv)
  {
  float ret;
  if(t < EPS)
    {
    ret = 0.0;
    t = EPS;
    *deriv = e * pow(1.0 - 2.0 * t, e - 1.0);
    }
  else if(t < 0.5)
    {
    ret = 0.5 * (1.0 - pow(1.0 - 2.0 * t, e));
    *deriv = e * pow(1.0 - 2.0 * t, e - 1.0);
    }
  else
    {
    ret = 0.5 * (1.0 + pow(2.0 * t - 1.0, e));
    *deriv = e * pow(2.0 * t  - 1.0, e - 1.0);
    }
  return ret;
  }

static float f_phi(bubble_data_t * d, float u, float * deriv)
  {
  *deriv = 0.5 * (d->r_phi_2 - d->r_phi_1) *
    sin(2.0 * M_PI * d->m_phi * u) * 2.0 * M_PI * d->m_phi;
  return 0.5 * (d->r_phi_1 + d->r_phi_2 +
                (d->r_phi_1 - d->r_phi_2) * cos(2.0 * M_PI * d->m_phi * u));
  }

static float f_theta(bubble_data_t * d, float v, float * deriv)
  {
  *deriv = 0.5 * (d->r_theta_2 - d->r_theta_1) *
    sin(M_PI * d->m_theta * v) * M_PI * d->m_theta;
  return 0.5 * (d->r_theta_1 + d->r_theta_2 +
                (d->r_theta_1 - d->r_theta_2) *
                cos(M_PI * d->m_theta * v));
  }

/* Functions for getting the radius and derivatives with respect to u and v */

/* Addition */

static float get_r_1(coords_data * data_u, coords_data * data_v,
                     float * d_r_d_u, float * d_r_d_v)
  {
  float ret;

  ret = data_u->f + data_v->f;

  *d_r_d_u = data_u->f_deriv;
  *d_r_d_v = data_v->f_deriv;

  return ret;
  }

/* Multiplication */

static float get_r_2(coords_data * data_u, coords_data * data_v,
                     float * d_r_d_u, float * d_r_d_v)
  {
  float ret;
  ret = data_u->f * data_v->f;

  *d_r_d_u = data_u->f_deriv * data_v->f;
  *d_r_d_v = data_v->f_deriv * data_u->f;
  return ret;
  }

/* f_phi / f_theta */

static float get_r_3(coords_data * data_u, coords_data * data_v,
                     float * d_r_d_u, float * d_r_d_v)
  {
  float ret;
  ret = data_u->f / data_v->f;

  *d_r_d_u = data_u->f_deriv / data_v->f;
  *d_r_d_v = - data_u->f * data_v->f_deriv / (data_v->f * data_v->f);
  return ret;
  }

static float get_r_4(coords_data * data_u, coords_data * data_v,
                     float * d_r_d_u, float * d_r_d_v)
  {
  float ret;
  ret = data_v->f / data_u->f;

  *d_r_d_u = - (data_v->f * data_u->f_deriv) / (data_u->f * data_u->f);
  *d_r_d_v = data_v->f_deriv / data_u->f;
  return ret;
  }


/* Calculate the grid */
static void init_bubble_real(lemuria_engine_t * e, bubble_data_t * d)
  {
  d->r_phi_1 = lemuria_random(e, 0.5, 2.0);
  d->r_phi_2 = lemuria_random(e, 0.5, 2.0);
  d->r_theta_1 = lemuria_random(e, 0.5, 2.0);
  d->r_theta_2 = lemuria_random(e, 0.5, 2.0);
  d->exp_phi = lemuria_random(e, 1.0/3.0, 3.0);
  d->exp_theta = lemuria_random(e, 1.0/3.0, 3.0);
  }

static void init_bubble(lemuria_engine_t * e, bubble_data_t * d)
  {
  int random_number;
  init_bubble_real(e, d);
  d->m_phi   = lemuria_random_int(e, 0, 5);
  d->m_theta = lemuria_random_int(e, 0, 10);
  
  random_number = lemuria_random_int(e, 0, 3);

  switch(random_number)
    {
    case 0:
      d->r_func = get_r_1;
      break;
    case 1:
      d->r_func = get_r_2;
      break;
    case 2:
      d->r_func = get_r_3;
      break;
    case 3:
      d->r_func = get_r_4;
      break;
    }

  random_number = lemuria_random_int(e, 0, 1);
  switch(random_number)
    {
    case 0:
      d->g_phi_func = g_func_1;
      break;
    case 1:
      d->g_phi_func = g_func_2;
      break;
    }

  random_number = lemuria_random_int(e, 0, 1);
  switch(random_number)
    {
    case 0:
      d->g_theta_func = g_func_1;
      break;
    case 1:
      d->g_theta_func = g_func_2;
      break;
    }
  
  }

static void init_bubble_sphere(bubble_data_t * d)
  {
  d->r_phi_1   = 1.0;
  d->r_phi_2   = 1.0;
  d->r_theta_1 = 1.0;
  d->r_theta_2 = 1.0;
  d->exp_phi   = 1.0;
  d->exp_theta = 1.0;

  d->m_phi   = 0;
  d->m_theta = 0;

  d->r_func       = get_r_1;
  d->g_phi_func   = g_func_1;
  d->g_theta_func = g_func_1;
  }

static void get_bubble_coords(bubble_data_t * d,
                              bubble_coords_t * c)
  {
  int i, j;
  float u, v;

  float r;
  float d_r_d_u;
  float d_r_d_v;
  
  float deriv_u[3];
  float deriv_v[3];

  float cos_phi, sin_phi;
  float cos_theta, sin_theta;
  float normal_len;
  
  coords_data data_u[SIZE_PHI];
  coords_data data_v;

  c->r_max = 0.0;
    
  for(j = 0; j < SIZE_PHI; j++)
    {
    u = (float)j/(float)(SIZE_PHI-1);
    data_u[j].f = f_phi(d, u, &(data_u[j].f_deriv));
    data_u[j].g = d->g_phi_func(u, d->exp_phi, &(data_u[j].g_deriv));
    }
    
  for(i = 0; i < SIZE_THETA; i++)
    {
    v = (float)i/(float)(SIZE_THETA-1);

    data_v.f = f_theta(d, v, &(data_v.f_deriv));
    data_v.g = d->g_theta_func(v, d->exp_theta, &(data_v.g_deriv));

    cos_theta = cos(data_v.g * M_PI);
    sin_theta = sin(data_v.g * M_PI);
    
    
    for(j = 0; j < SIZE_PHI; j++)
      {
      u = (float)j/(float)(SIZE_PHI-1);

      cos_phi = cos(data_u[j].g * 2.0 * M_PI);
      sin_phi = sin(data_u[j].g * 2.0 * M_PI);
      
      r = d->r_func(&(data_u[j]), &data_v, &d_r_d_u, &d_r_d_v);

      /* Calculate derivative with respect to u */

      deriv_u[0] = sin_theta * (r * (- sin_phi) * 2.0 * M_PI * data_u[j].g_deriv +
                                d_r_d_u * cos_phi);

      deriv_u[1] = sin_theta * (r * (  cos_phi) * 2.0 * M_PI * data_u[j].g_deriv +
                                d_r_d_u * sin_phi);

      deriv_u[2] = d_r_d_u * cos_theta;

      /* Calculate derivative with respect to v */
      
      deriv_v[0] = cos_phi * (r * cos_theta * 0.5 * M_PI * data_v.g_deriv +
                              d_r_d_v * sin_theta);
      deriv_v[1] = sin_phi * (r * cos_theta * 0.5 * M_PI * data_v.g_deriv +
                              d_r_d_v * sin_theta);

      deriv_v[2] = d_r_d_v * cos_theta - r * sin_theta * 0.5 * M_PI * data_v.g_deriv;

      /* Calculate the normal vector */

      c->normal_matrix[i][j][0] = (deriv_u[1] * deriv_v[2] - deriv_u[2] * deriv_v[1]);
      c->normal_matrix[i][j][1] = (deriv_u[2] * deriv_v[0] - deriv_u[0] * deriv_v[2]);
      c->normal_matrix[i][j][2] = (deriv_u[0] * deriv_v[1] - deriv_u[1] * deriv_v[0]);
#if 1
      normal_len = sqrt(c->normal_matrix[i][j][0] * c->normal_matrix[i][j][0] +
                        c->normal_matrix[i][j][1] * c->normal_matrix[i][j][1] +
                        c->normal_matrix[i][j][2] * c->normal_matrix[i][j][2]);
      c->normal_matrix[i][j][0] /= normal_len;
      c->normal_matrix[i][j][1] /= normal_len;
      c->normal_matrix[i][j][2] /= normal_len;
#endif 
        
      
      c->coord_matrix[i][j][0] = r * sin_theta * cos_phi;
      c->coord_matrix[i][j][1] = r * sin_theta * sin_phi;
      c->coord_matrix[i][j][2] = r * cos_theta;

      if(r > c->r_max)
        c->r_max = r;
      }
    }
  
  //  fprintf(stderr, "r_max: %f\n", r_max);
  }

#ifdef TESTONLY
int main(int argc, char ** argv)
  {
  int i;
  float t;
  float deriv;
  float func;
  bubble_data_t d;
  init_bubble(&d);
  for(i = 0; i < 100; i++)
    {
    t = (float)i / 99.0;
    
    //    func = g_func_2(t, 0.5, &deriv);

    func = f_theta(&d, t, &deriv);
    
    printf("%f %f %f\n", t, func, deriv);
    }
  
  return 0;
  }
#endif

/* Here comes the lemuria and OpenGL related part */

typedef struct
  {
  bubble_data_t data_1;
  bubble_data_t data_2;
  bubble_data_t draw_data;

  bubble_coords_t coords_1;
  bubble_coords_t coords_2;
  bubble_coords_t draw_coords;
  lemuria_rotator_t rotator;

  int deform_mode;
  lemuria_range_t deform_range;
  lemuria_offset_t offset;

  lemuria_scale_t scale_x;
  lemuria_scale_t scale_y;
  lemuria_scale_t scale_z;

  int transition_mode;
  lemuria_engine_t * engine;
  } hyperbubble_data;

#if 0

static void draw_hyperbubble_grid(bubble_coords_t * coords)
  {
  int i, j;
  glLineWidth(1.0);
  glBegin(GL_LINES);
  
  for(i = 0; i < SIZE_THETA-1;  i++)
    {
    for(j = 0; j < SIZE_PHI-1;  j++)
      {
      /* Draw grid */
      glColor3f(0.0, 1.0, 0.0);

      glVertex3fv(coords->coord_matrix[i][j]);
      glVertex3fv(coords->coord_matrix[i+1][j]);
            
      glVertex3fv(coords->coord_matrix[i][j]);
      glVertex3fv(coords->coord_matrix[i][j+1]);
      
      /* Draw Normal */
#if 0
      glColor3f(1.0, 0.0, 0.0);
      glVertex3fv(coords->coord_matrix[i][j]);

      glVertex3f(coords->coord_matrix[i][j][0]+coords->normal_matrix[i][j][0],
                 coords->coord_matrix[i][j][1]+coords->normal_matrix[i][j][1],
                 coords->coord_matrix[i][j][2]+coords->normal_matrix[i][j][2]);
#endif
      
      }
    }
  
  glEnd();
  
  }

#endif

static void draw_hyperbubble_texture(lemuria_engine_t * e,
                                     bubble_coords_t * coords)
  {
  int i, j;
    
  /* Draw the stuff */
  
  for(i = 0; i < SIZE_THETA-1; i++)
    {
    glBegin(GL_QUAD_STRIP);
    
    for(j = 0; j < SIZE_PHI; j++)
      {
      glTexCoord2f((float)j/(float)(SIZE_PHI-1),
                   (float)(i+1)/(float)(SIZE_THETA-1));
      
      glNormal3fv(coords->normal_matrix[i+1][j]);
      glVertex3fv(coords->coord_matrix[i+1][j]);

      glTexCoord2f((float)j/(float)(SIZE_PHI-1),
                   (float)(i)/(float)(SIZE_THETA-1));

      glNormal3fv(coords->normal_matrix[i][j]);
      glVertex3fv(coords->coord_matrix[i][j]);
      }
    
    glEnd();
    }
  }

/* Progress = 0: Beginning of the explosion */
/* Progress = 1: End       of the explosion */

#define EXPLODE_PHI_STEPS   80
#define EXPLODE_THETA_STEPS 40

static void draw_hyperbubble_explode(lemuria_engine_t * e,
                                     float progress)
  {
  int i, j;
  float vector[3];
  float theta, phi;

  float sin_theta_1;
  float sin_theta_2;
  float cos_theta_1;
  float cos_theta_2;

  float sin_phi_1;
  float sin_phi_2;
  float cos_phi_1;
  float cos_phi_2;

  float cos_theta_mid;
  float sin_theta_mid;

  float cos_phi_mid;
  float sin_phi_mid;
    
  sin_theta_1 = 0.0;
  cos_theta_1 = 1.0;

  sin_phi_1 = 0.0;
  cos_phi_1 = 1.0;
  
  glBegin(GL_QUADS);
    
  for(i = 0; i < EXPLODE_THETA_STEPS-1; i++)
    {
    theta = M_PI * (float)(i+1) / (float)(EXPLODE_THETA_STEPS-1);

    sin_theta_2 = sin(theta);
    cos_theta_2 = cos(theta);

    theta += 0.5 * M_PI / (float)(EXPLODE_THETA_STEPS-1);

    cos_theta_mid = cos(theta);
    sin_theta_mid = sin(theta);
        
    for(j = 0; j < EXPLODE_PHI_STEPS; j++)
      {
      phi = 2.0 * M_PI * (float)(j+1) / (float)(EXPLODE_PHI_STEPS-1);
      
      sin_phi_2 = sin(phi);
      cos_phi_2 = cos(phi);

      phi += M_PI / (float)(EXPLODE_PHI_STEPS-1);
      
      cos_phi_mid = cos(phi);
      sin_phi_mid = sin(phi);

      vector[0] = sin_theta_mid * cos_phi_mid;
      vector[1] = sin_theta_mid * sin_phi_mid;
      vector[2] = cos_theta_mid;
      
      /* Draw the quad */

      glNormal3f(sin_theta_2 * cos_phi_1,
                 sin_theta_2 * sin_phi_1,
                 cos_theta_2);
      
      glTexCoord2f((float)j/(float)(EXPLODE_PHI_STEPS-1),
                   (float)(i+1)/(float)(EXPLODE_THETA_STEPS-1));
      
      glVertex3f(sin_theta_2 * cos_phi_1 + progress * 10.0 * vector[0],
                 sin_theta_2 * sin_phi_1 + progress * 10.0 * vector[1],
                 cos_theta_2             + progress * 10.0 * vector[2]);

      glNormal3f(sin_theta_2 * cos_phi_2,
                 sin_theta_2 * sin_phi_2,
                 cos_theta_2);
      
      glTexCoord2f((float)(j+1)/(float)(EXPLODE_PHI_STEPS-1),
                   (float)(i+1)/(float)(EXPLODE_THETA_STEPS-1));
      
      glVertex3f(sin_theta_2 * cos_phi_2 + progress * 10.0 * vector[0],
                 sin_theta_2 * sin_phi_2 + progress * 10.0 * vector[1],
                 cos_theta_2             + progress * 10.0 * vector[2]);

      glNormal3f(sin_theta_1 * cos_phi_2,
                 sin_theta_1 * sin_phi_2,
                 cos_theta_1);
      
      glTexCoord2f((float)(j+1)/(float)(EXPLODE_PHI_STEPS-1),
                   (float)i/(float)(EXPLODE_THETA_STEPS-1));
      
      glVertex3f(sin_theta_1 * cos_phi_2 + progress * 10.0 * vector[0],
                 sin_theta_1 * sin_phi_2 + progress * 10.0 * vector[1],
                 cos_theta_1             + progress * 10.0 * vector[2]);

      glNormal3f(sin_theta_1 * cos_phi_1,
                 sin_theta_1 * sin_phi_1,
                 cos_theta_1);
      
      glTexCoord2f((float)j/(float)(EXPLODE_PHI_STEPS-1),
                   (float)i/(float)(EXPLODE_THETA_STEPS-1));
      
      glVertex3f(sin_theta_1 * cos_phi_1 + progress * 10.0 * vector[0],
                 sin_theta_1 * sin_phi_1 + progress * 10.0 * vector[1],
                 cos_theta_1             + progress * 10.0 * vector[2]);
      
      cos_phi_1 = cos_phi_2;
      sin_phi_1 = sin_phi_2;
      }
    cos_theta_1 = cos_theta_2;
    sin_theta_1 = sin_theta_2;
    
    }
  
  glEnd();
  }

static void transform_to_sphere(lemuria_engine_t * e,
                                hyperbubble_data * d)
  {
  d->deform_mode = DEFORM_INT;
  
  bubble_data_copy(&(d->data_1), &(d->data_2));

  lemuria_range_init(e, &(d->deform_range),
                     3*SIZE_PHI*SIZE_THETA, 100, 200);
  
  init_bubble_sphere(&(d->data_2));
  get_bubble_coords(&(d->data_1), &(d->coords_1));
  get_bubble_coords(&(d->data_2), &(d->coords_2));
  d->transition_mode = TRANSITION_DEFORM_2;
  }

static void draw_hyperbubble(lemuria_engine_t * e,
                             void * user_data)
  {
  int do_deform = 0;
  int do_deform_before;
  float scale_factor;

  float explode_start;
  float explode_end;
  float explode;
  hyperbubble_data * d = (hyperbubble_data*)(user_data);
    
  if(e->foreground.mode == EFFECT_FINISH)
    {
    e->foreground.mode = EFFECT_FINISHING;
        
    if(lemuria_range_done(&(d->deform_range)))
      {
      /* Transform to sphere */
      transform_to_sphere(e, d);
      }
    else
      d->transition_mode = TRANSITION_DEFORM_1;
    }

  if(e->foreground.mode == EFFECT_STARTING)
    {
    do_deform = 1;
    lemuria_range_update(&(d->deform_range));
    if(lemuria_range_done(&(d->deform_range)))
      {
      e->foreground.mode = EFFECT_RUNNING;
      d->transition_mode = TRANSITION_NONE;
      }
    }
  else if(e->foreground.mode == EFFECT_FINISHING)
    {
    lemuria_range_update(&(d->deform_range));

    do_deform = 1;
    if(lemuria_range_done(&(d->deform_range)))
      {
      if(d->transition_mode == TRANSITION_DEFORM_1)
        {
        transform_to_sphere(e, d);
        }
      else if(d->transition_mode == TRANSITION_DEFORM_2)
        {
        lemuria_range_init(e, &(d->deform_range),
                           1, 100, 200);
        d->transition_mode = TRANSITION_EXPLODE;
        }
      }

    if(d->transition_mode != TRANSITION_EXPLODE)
      {
      lemuria_rotator_update(&(d->rotator));
      lemuria_offset_update(&(d->offset));
      }
    }

  else if(e->foreground.mode == EFFECT_RUNNING)
    {
    /* Check wether to change something */
    
    do_deform_before = !lemuria_range_done(&(d->deform_range));
    
    lemuria_range_update(&(d->deform_range));
    
    do_deform = !lemuria_range_done(&(d->deform_range));
    
    if(do_deform_before && !do_deform)
      {
      /* Copy the seocond data set to the first */
      
      if(d->deform_mode == DEFORM_FLOAT)
        bubble_data_copy_real(&(d->data_1), &(d->data_2));
      else
        bubble_data_copy(&(d->data_1), &(d->data_2));
      //    fprintf(stderr, "Transformation done\n");
      }
    
    if(e->beat_detected)
      {
      if(lemuria_decide(e, 0.3))
        {
        lemuria_scale_change(e, &(d->scale_x));
        lemuria_scale_change(e, &(d->scale_y));
        lemuria_scale_change(e, &(d->scale_z));
        }
      
      if(lemuria_decide(e, 0.3))
        lemuria_offset_change(e, &(d->offset));
      if(lemuria_decide(e, 0.3))
        lemuria_offset_kick(&(d->offset));
      
      
      if(!do_deform && lemuria_decide(e, 0.7))
        {
        /* Change the deformation */
        
        if(lemuria_decide(e, 0.7)) /* Do integer deformation */
          {
          //          fprintf(stderr, "Starting integer transformation\n");
          d->deform_mode = DEFORM_INT;
          lemuria_range_init(e, &(d->deform_range),
                             3*SIZE_PHI*SIZE_THETA, 100, 200);

          init_bubble(e, &(d->data_2));
          get_bubble_coords(&(d->data_1), &(d->coords_1));
          get_bubble_coords(&(d->data_2), &(d->coords_2));

          }
        else
          {
          //          fprintf(stderr, "Starting float transformation\n");
          d->deform_mode = DEFORM_FLOAT;
          lemuria_range_init(e, &(d->deform_range),
                             1, 100, 200);

          bubble_data_copy(&(d->draw_data), &(d->data_1));
          bubble_data_copy(&(d->data_2), &(d->data_1));

          init_bubble_real(e, &(d->data_2));
          
          }
        }
      
      //    fprintf(stderr, "beat detected\n");
      if(lemuria_decide(e, 0.1))
        lemuria_rotator_change(e, &(d->rotator));
       
      //      if(lemuria_decide(0.1))
      //        lemuria_scale_change(&(d->scale));
       
      //      if(lemuria_decide(0.1))
      //        lemuria_offset_change(&(d->offset));
      }
     
    lemuria_rotator_update(&(d->rotator));
    //    lemuria_scale_update(&(d->scale));
    //    lemuria_offset_update(&(d->offset));
    lemuria_offset_update(&(d->offset));
    lemuria_scale_update(&(d->scale_x));
    lemuria_scale_update(&(d->scale_y));
    lemuria_scale_update(&(d->scale_z));
    }

  glShadeModel(GL_SMOOTH);
  glEnable(GL_NORMALIZE);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  //  glColor4f(1.0, 1.0, 1.0, 1.0);

  glEnable(GL_TEXTURE_2D);
  lemuria_set_perspective(e, 1, 10.0);

  lemuria_set_light(&light, GL_LIGHT0);
  glEnable(GL_LIGHT0);

  lemuria_set_material(&material, GL_FRONT_AND_BACK);


  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  // glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  glEnable(GL_LIGHTING);

  lemuria_texture_bind(e, 0);

  /* Draw stuff */
  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  lemuria_offset_translate(&(d->offset));
  lemuria_rotate(&(d->rotator));
    
  if(d->transition_mode == TRANSITION_EXPLODE)
    {
    if(e->foreground.mode == EFFECT_STARTING)
      {
      explode_start = 1.0;
      explode_end   = 0.0;
      }
    else
      {
      explode_start = 0.0;
      explode_end   = 1.0;
      }
    
    lemuria_range_get(&(d->deform_range),
                      &explode_start,
                      &explode_end,
                      &explode);

    glScalef(lemuria_scale_get(&(d->scale_x)),
             lemuria_scale_get(&(d->scale_y)),
             lemuria_scale_get(&(d->scale_z)));

    draw_hyperbubble_explode(e, explode);
    }
  else
    {
    if(!do_deform)
      {
      get_bubble_coords(&(d->data_1), &(d->draw_coords));
      scale_factor = 1.0 / d->draw_coords.r_max;
      }
    
    else if(d->deform_mode == DEFORM_INT)
      {
      lemuria_range_get(&(d->deform_range),
                        d->coords_1.normals, d->coords_2.normals,
                        d->draw_coords.normals);
      
      lemuria_range_get(&(d->deform_range),
                        d->coords_1.coords, d->coords_2.coords,
                        d->draw_coords.coords);
      lemuria_range_get_n(&(d->deform_range),
                          &(d->coords_1.r_max), &(d->coords_2.r_max),
                          &scale_factor, 1);
      scale_factor = 1.0 / scale_factor;
      }
    else /* Deform real */
      {
      bubble_data_interpolate_real(&(d->deform_range),
                                   &(d->data_1),
                                   &(d->data_2),
                                   &(d->draw_data));
      get_bubble_coords(&(d->draw_data), &(d->draw_coords));
      scale_factor = 1.0 / d->draw_coords.r_max;
      }

    glScalef(lemuria_scale_get(&(d->scale_x)) * scale_factor,
             lemuria_scale_get(&(d->scale_y)) * scale_factor,
             lemuria_scale_get(&(d->scale_z)) * scale_factor);


    draw_hyperbubble_texture(e, &(d->draw_coords));
    }
  
  //  draw_hyperbubble_grid(&(d->draw_coords));
  //  

  
  glPopMatrix();

  /* Disable stuff */

  glDisable(GL_NORMALIZE);

  //  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  //  glColor4f(1.0, 1.0, 1.0, 1.0);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHT0);
  
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
  //  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
  glDisable(GL_LIGHTING);
 
  if((d->transition_mode == TRANSITION_EXPLODE) &&
     (e->foreground.mode == EFFECT_FINISHING) && 
     (lemuria_range_done(&(d->deform_range))))
    e->foreground.mode = EFFECT_DONE;
  }

static void * init_hyperbubble(lemuria_engine_t * e)
  {
  hyperbubble_data * d;
                                                                               
#ifdef DEBUG
  fprintf(stderr, "init_hyperbubble...");
#endif
  
  d = calloc(1, sizeof(hyperbubble_data));

  d->engine = e;
  lemuria_texture_ref(d->engine, 0);
    
  init_bubble_sphere(&(d->data_1));
  init_bubble(e, &(d->data_2));

  init_bubble_coords(&(d->coords_1));
  init_bubble_coords(&(d->coords_2));
  init_bubble_coords(&(d->draw_coords));
  lemuria_rotator_init(e, &(d->rotator));
  lemuria_offset_init(e, &(d->offset));

  lemuria_scale_init(e, &(d->scale_x), 0.2, 0.8);
  lemuria_scale_init(e, &(d->scale_y), 0.2, 0.8);
  lemuria_scale_init(e, &(d->scale_z), 0.2, 0.8);
    
  e->foreground.mode = EFFECT_STARTING;
  d->transition_mode = TRANSITION_EXPLODE;
  lemuria_range_init(e, &(d->deform_range), 1, 100, 200);
    
  //  lemuria_rotator_init(&(d->rotator));
  //  lemuria_scale_init(&(d->scale), 0.1, 0.6);
  //  lemuria_offset_init(&(d->offset));
                                                                                
  //  d->draw_hyperbubble_func = draw_hyperbubble_explode;
  //  lemuria_range_init(&(d->range), 1, TRANSITION_MIN, TRANSITION_MAX);
#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  return d;
  }

static void delete_hyperbubble(void * e)
  {
  hyperbubble_data * data = (hyperbubble_data*)(e);
  lemuria_texture_unref(data->engine, 0);
  free(data);
  }

effect_plugin_t hyperbubble_effect =
  {
    .init =    init_hyperbubble,
    .draw =    draw_hyperbubble,
    .cleanup = delete_hyperbubble,
  };
