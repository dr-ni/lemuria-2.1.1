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

#define MAX_X_STEPS 3
#define MAX_Y_STEPS 3

static lemuria_light_t light =
  {
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 0.0f, 0.0f, 0.0f, 0.0f },
    .position = { 3.0f, 3.0f, 2.0f, 1.0f },
  };

static lemuria_material_t material =
  {
    .ref_specular = { 0.0f, 0.0f, 0.0f, 1.0f },
    .ref_ambient =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 128
  };

#define ROTATE1_EXPLODE_STEPS 20

typedef struct _rotate1_data
  {
  float angle;
  int explode;
  float offsets[ROTATE1_EXPLODE_STEPS][ROTATE1_EXPLODE_STEPS][3];
  float speed;
  int old_x_steps;
  int old_y_steps;
  } rotate1_data;

typedef struct _wave1_data
  {
  float t;
  float omega;     /* Angular frequency */
  float beta;      /* Phase velocity    */
  float amplitude;

  float k_angle;
  float k_x;
  float k_y;

  int old_x_steps;
  int old_y_steps;

  lemuria_range_t amplitude_range;
  float amplitude_start;
  float amplitude_end;

  lemuria_range_t k_angle_range;
  float k_angle_start;
  float k_angle_end;

  int exiting;

  int long_wave; // This is too long to be recognized as a wave but looks
                 // cool also
  
  } wave1_data;


typedef union _aux_data
  {
  rotate1_data rotate1;
  wave1_data wave1;
  } aux_data;

typedef struct squares_data_s
  {
  lemuria_rotator_t rotator;

  aux_data aux;
  int aux_initialized;
  
  void (*get_coords)(lemuria_engine_t * e,
                     struct squares_data_s * d,
                     int u,
                     int v,
                     float coords[4][3],
                     float normals[4][3]); 

  lemuria_range_t texture_range;

  //  float min_values[MAX_NUM_VALUES];
  //  float max_values[MAX_NUM_VALUES];
  //  float values[MAX_NUM_VALUES];
  
  int x_steps;
  int y_steps;

  float texture_coords_x[2][2];
  float texture_coords_y[2][2];
  
  float * texture_x_start;
  float * texture_x_end;
  
  float * texture_y_start;
  float * texture_y_end;

  int last;  // 1 if drawing function is called the last time for this frame
  int first; // 1 if drawing function is called the first time for this frame

  int beat;

  int do_exit;
  lemuria_engine_t * engine;
  } squares_data_t;

static void init_texture_coords(squares_data_t * d, int x_steps, int y_steps)
  {
  float * tmp;

  tmp = d->texture_x_start;
  d->texture_x_start = d->texture_x_end;
  d->texture_x_end = tmp;

  tmp = d->texture_y_start;
  d->texture_y_start = d->texture_y_end;
  d->texture_y_end = tmp;

  d->texture_x_end[0] = - (float)(x_steps / 2);
  d->texture_x_end[1] = (float)(x_steps / 2 + x_steps % 2);

  d->texture_y_end[0] = - (float)(y_steps / 2);
  d->texture_y_end[1] = (float)(y_steps / 2 + y_steps % 2);
  
  }

static void get_coords_square(lemuria_engine_t * e,
                              struct squares_data_s * d,
                              int u,
                              int v,
                              float coords[4][3],
                              float normals[4][3])
  {
  float x_1, x_2, y_1, y_2;

  normals[0][0] = 0.0;
  normals[0][1] = 0.0;
  normals[0][2] = 1.0;

  normals[1][0] = 0.0;
  normals[1][1] = 0.0;
  normals[1][2] = 1.0;

  normals[2][0] = 0.0;
  normals[2][1] = 0.0;
  normals[2][2] = 1.0;

  normals[3][0] = 0.0;
  normals[3][1] = 0.0;
  normals[3][2] = 1.0;

  coords[0][2] =  0.0;
  coords[1][2] =  0.0;
  coords[2][2] =  0.0;
  coords[3][2] =  0.0;

  x_1 = -1.0 + 2.0 * (float)(u) /(float)(d->x_steps);
  y_1 = -1.0 + 2.0 * (float)(v) /(float)(d->y_steps);

  x_2 = x_1 + 2.0 /(float)(d->x_steps);
  y_2 = y_1 + 2.0 /(float)(d->y_steps);
  
  coords[0][0] =  x_1;
  coords[0][1] =  y_1;

  coords[1][0] =  x_2;
  coords[1][1] =  y_1;

  coords[2][0] =  x_2;
  coords[2][1] =  y_2;

  coords[3][0] =  x_1;
  coords[3][1] =  y_2;
  
  }

static void get_coords_rotate1(lemuria_engine_t * e,
                               struct squares_data_s * d,
                               int u,
                               int v,
                               float coords[4][3],
                               float normals[4][3])
  {
  float sin_phi;
  float cos_phi;

  float delta_x, delta_y;
  float factor;
  float size_x = 1.0 / (float)(d->x_steps);
  float size_y = 1.0 / (float)(d->y_steps);

  int i, j;
  
  if(!d->aux_initialized)
    {
    //    fprintf(stderr, "Initializing...");
    d->aux.rotate1.angle = 0.0;

    d->aux.rotate1.explode = lemuria_decide(e, 0.5);
    //    d->aux.rotate1.explode = 1;
    if(d->aux.rotate1.explode)
      {
      //      fprintf(stderr, "explode\n");

      d->aux.rotate1.old_x_steps = d->x_steps;
      d->aux.rotate1.old_y_steps = d->y_steps;
            
      d->x_steps = ROTATE1_EXPLODE_STEPS;
      d->y_steps = ROTATE1_EXPLODE_STEPS;
      for(i = 0; i < ROTATE1_EXPLODE_STEPS; i++)
        for(j = 0; j < ROTATE1_EXPLODE_STEPS; j++)
          {
          d->aux.rotate1.offsets[i][j][0] = lemuria_random(e, -1.0, 1.0);
          d->aux.rotate1.offsets[i][j][1] = lemuria_random(e, -1.0, 1.0);
          d->aux.rotate1.offsets[i][j][2] = lemuria_random(e, -1.0, 1.0);
          }
      d->aux.rotate1.speed = 0.02;
      }
    else
      {
      d->aux.rotate1.speed = 0.02;
      //      fprintf(stderr, "no explode\n");
      }
    d->aux_initialized = 1;
    }

  sin_phi = sin(d->aux.rotate1.angle);
  cos_phi = cos(d->aux.rotate1.angle);

  /* The rotation part */
  
  if(u % 2 == v % 2) // Change x and z
    {
    coords[0][0] = - size_x * cos_phi;
    coords[0][1] = - size_y;
    coords[0][2] =   size_x * sin_phi;

    coords[1][0] =   size_x * cos_phi;
    coords[1][1] = - size_y;
    coords[1][2] = - size_x * sin_phi;

    coords[2][0] =   size_x * cos_phi;
    coords[2][1] =   size_y;
    coords[2][2] = - size_x * sin_phi;

    coords[3][0] = - size_x * cos_phi;
    coords[3][1] =   size_y;
    coords[3][2] =   size_x * sin_phi;
    
    normals[0][0] = normals[1][0] =
      normals[2][0] = normals[3][0] = sin_phi;
      
    normals[0][1] = normals[1][1] =
      normals[2][1] = normals[3][1] = 0.0;
      
    normals[0][2] = normals[1][2] =
      normals[2][2] = normals[3][2] = cos_phi;
    }
  else  // Change y and z
    {
    coords[0][0] = - size_x;
    coords[0][1] = - size_y * cos_phi;
    coords[0][2] = - size_y * sin_phi;

    coords[1][0] =   size_x;
    coords[1][1] = - size_y * cos_phi;
    coords[1][2] = - size_y * sin_phi;

    coords[2][0] =   size_x;
    coords[2][1] =   size_y * cos_phi;
    coords[2][2] =   size_y * sin_phi;

    coords[3][0] = - size_x;
    coords[3][1] =   size_y * cos_phi;
    coords[3][2] =   size_y * sin_phi;
    
    normals[0][0] = normals[1][0] =
      normals[2][0] = normals[3][0] = 0.0;
      
    normals[0][1] = normals[1][1] =
      normals[2][1] = normals[3][1] = - sin_phi;
      
    normals[0][2] = normals[1][2] =
      normals[2][2] = normals[3][2] = cos_phi;

    }

  /* Shift the squares */

  delta_x = ( -(d->x_steps - 1) + u * 2) * size_x;
  delta_y = ( -(d->y_steps - 1) + v * 2) * size_y;
  
  coords[0][0] += delta_x;
  coords[0][1] += delta_y;

  coords[1][0] += delta_x;
  coords[1][1] += delta_y;

  coords[2][0] += delta_x;
  coords[2][1] += delta_y;

  coords[3][0] += delta_x;
  coords[3][1] += delta_y; 

  // Do the explosion part
  
  if(d->aux.rotate1.explode)
    {
    factor = 1.0 - cos_phi;
    coords[0][0] += factor*d->aux.rotate1.offsets[u][v][1];
    coords[0][1] += factor*d->aux.rotate1.offsets[u][v][2];
    coords[0][2] += factor*d->aux.rotate1.offsets[u][v][3];
    coords[1][0] += factor*d->aux.rotate1.offsets[u][v][1];
    coords[1][1] += factor*d->aux.rotate1.offsets[u][v][2];
    coords[1][2] += factor*d->aux.rotate1.offsets[u][v][3];
    coords[2][0] += factor*d->aux.rotate1.offsets[u][v][1];
    coords[2][1] += factor*d->aux.rotate1.offsets[u][v][2];
    coords[2][2] += factor*d->aux.rotate1.offsets[u][v][3];
    coords[3][0] += factor*d->aux.rotate1.offsets[u][v][1];
    coords[3][1] += factor*d->aux.rotate1.offsets[u][v][2];
    coords[3][2] += factor*d->aux.rotate1.offsets[u][v][3];
    }
    
  /* Increment angle */

  if(d->last)
    {
    d->aux.rotate1.angle += d->aux.rotate1.speed;
    
    if(d->aux.rotate1.angle > 2.0 * M_PI)
      {
      d->aux.rotate1.angle -= 2.0 * M_PI;
      if(d->aux.rotate1.explode || lemuria_decide(e, 0.8) ||
         d->do_exit)
        {
        d->get_coords = get_coords_square;
        d->aux_initialized = 0;
        if(d->aux.rotate1.explode)
          {
          d->x_steps = d->aux.rotate1.old_x_steps;
          d->y_steps = d->aux.rotate1.old_y_steps;
          //        fprintf(stderr, "%d %d\n", d->x_steps, d->y_steps);
          d->y_steps = d->aux.rotate1.old_y_steps;
          }
        }
      }
    }
  }

static void get_wave1_point(struct squares_data_s * d,
                            float x,
                            float y,
                            float coords[3],
                            float normals[3])
  {
  float alpha;
  float sin_alpha;
  float sin_angle;
  float cos_angle;
  float angle;
  
  angle = (d->aux.wave1.omega * d->aux.wave1.t -
           (x * d->aux.wave1.k_x + y * d->aux.wave1.k_y));
  
  sin_angle = sin(angle);
  cos_angle = cos(angle);
  
  
  coords[0] = x;
  coords[1] = y;
  coords[2] = d->aux.wave1.amplitude * sin_angle;

  alpha = atan(d->aux.wave1.amplitude * cos_angle);

  sin_alpha = sin(alpha);

  normals[0] = - d->aux.wave1.k_x * sin_alpha;
  normals[1] = - d->aux.wave1.k_y * sin_alpha;
  normals[2] = cos(alpha);

  }

static void get_coords_wave1(lemuria_engine_t * e,
                             struct squares_data_s * d,
                             int u,
                             int v,
                             float coords[4][3],
                             float normals[4][3])
  {
  float x;
  float y;
  
  if(!d->aux_initialized)
    {
    //    fprintf(stderr, "Initializing waves\n");
    d->aux.wave1.t = 0.0;


    d->aux.wave1.k_angle = 0.25 * M_PI;

    d->aux.wave1.old_x_steps = d->x_steps;
    d->aux.wave1.old_y_steps = d->y_steps;

    d->x_steps = 40;
    d->y_steps = 40;

    d->aux.wave1.exiting = 0;
    
    d->aux_initialized = 1;

    d->aux.wave1.amplitude_start = 0.0;
    
    d->aux.wave1.k_angle_start = lemuria_random(e, 0.0, 0.5 * M_PI);
    d->aux.wave1.k_angle_end = lemuria_random(e, 0.0, 0.5 * M_PI);

    d->aux.wave1.long_wave = lemuria_random_int(e, 0, 1);
    //    d->aux.wave1.long_wave = 1;
    
    if(d->aux.wave1.long_wave)
      {
      d->aux.wave1.amplitude_end = lemuria_random(e, 1.0, 2.0);

      /* omega = 2.0 * Pi / period_time_in_frame */
    
      d->aux.wave1.omega = 2.0 * M_PI / (200.0);

      /* Beta (Propagation constant): 2.0 * Pi / wavelength */
    
      d->aux.wave1.beta = 2.0 * M_PI / 5.0;
      }
    else
      {
      d->aux.wave1.amplitude_end = lemuria_random(e, 0.1, 0.2);

      /* omega = 2.0 * Pi / period_time_in_frame */
    
      d->aux.wave1.omega = 2.0 * M_PI / (30.0);

      /* Beta (Propagation constant): 2.0 * Pi / wavelength */
    
      d->aux.wave1.beta = 2.0 * M_PI / 0.7;
      }
    
    lemuria_range_init(e, &d->aux.wave1.k_angle_range, 1, 100, 300);
    lemuria_range_init(e, &d->aux.wave1.amplitude_range, 1, 100, 300);
    }

  if(d->first)
    {
    if(d->beat)
      {
      if((lemuria_decide(e, 0.2) || d->do_exit) &&
         lemuria_range_done(&d->aux.wave1.amplitude_range))
        {
        d->aux.wave1.exiting = 1;
        d->aux.wave1.amplitude_end = 0;
        d->aux.wave1.amplitude_start = d->aux.wave1.amplitude;
        lemuria_range_init(e, &d->aux.wave1.amplitude_range, 1, 100, 300);
        }
      if(lemuria_range_done(&d->aux.wave1.k_angle_range) &&
         lemuria_decide(e, 0.3))
        {
        d->aux.wave1.k_angle_start = d->aux.wave1.k_angle_end;
        d->aux.wave1.k_angle_end = lemuria_random(e, 0.0, 0.5 * M_PI);
        lemuria_range_init(e, &d->aux.wave1.k_angle_range, 1, 100, 300);
        }
      
      if(lemuria_range_done(&d->aux.wave1.amplitude_range) &&
         !(d->aux.wave1.long_wave) && lemuria_decide(e, 0.3))
        {
        d->aux.wave1.amplitude_start = d->aux.wave1.amplitude_end;
        d->aux.wave1.amplitude_end = lemuria_random(e, 0.05, 0.2);
        lemuria_range_init(e, &d->aux.wave1.amplitude_range, 1, 100, 300);
        }
      }
    
    lemuria_range_update(&d->aux.wave1.k_angle_range);
    lemuria_range_update(&d->aux.wave1.amplitude_range);
    
    lemuria_range_get(&d->aux.wave1.k_angle_range, 
                      &d->aux.wave1.k_angle_start,
                      &d->aux.wave1.k_angle_end,
                      &d->aux.wave1.k_angle);
    
    lemuria_range_get(&d->aux.wave1.amplitude_range, 
                      &d->aux.wave1.amplitude_start,
                      &d->aux.wave1.amplitude_end,
                      &d->aux.wave1.amplitude);
            
    d->aux.wave1.k_x = d->aux.wave1.beta * cos(d->aux.wave1.k_angle);
    d->aux.wave1.k_y = d->aux.wave1.beta * sin(d->aux.wave1.k_angle);
    }
  
  x = (float)(2*u)/(float)(d->x_steps) - 1.0;
  y = (float)(2*v)/(float)(d->y_steps) - 1.0;
    
  get_wave1_point(d, x, y, coords[0], normals[0]);

  x = (float)(2*u+2)/(float)(d->x_steps) - 1.0;

  get_wave1_point(d, x, y, coords[1], normals[1]);

  y = (float)(2*v+2)/(float)(d->y_steps) - 1.0;
  
  get_wave1_point(d, x, y, coords[2], normals[2]);

  x = (float)(2*u)/(float)(d->x_steps) - 1.0;

  get_wave1_point(d, x, y, coords[3], normals[3]);
  
  if(d->last)
    {
    d->aux.wave1.t += 1.0;
    if(d->aux.wave1.omega * d->aux.wave1.t > 2.0 * M_PI)
      d->aux.wave1.t -= 2.0 * M_PI / d->aux.wave1.omega;

    if(d->aux.wave1.exiting &&
       lemuria_range_done(&(d->aux.wave1.amplitude_range)))
      {
      d->x_steps = d->aux.wave1.old_x_steps;
      d->y_steps = d->aux.wave1.old_y_steps;
      d->get_coords = get_coords_square;
      d->aux_initialized = 0;
      //      fprintf(stderr, "Exiting waves\n");
      }

    }
    
  }


static void draw_squares(lemuria_engine_t * e, void * user_data)
  {
  int i;
  int j;
  
  float texture_coords_max_x[2];
  float texture_coords_max_y[2];

  float texture_coords_x[2];
  float texture_coords_y[2];
  
  float coords[4][3];
  float normals[4][3];

  int random_number;
  
  squares_data_t * data = (squares_data_t*)(user_data);

  data->beat = e->beat_detected;
  
  lemuria_rotator_update(&(data->rotator));

  lemuria_range_update(&data->texture_range);

  if(e->foreground.mode == EFFECT_FINISH)
    {
    //    fprintf(stderr, "Squares finish\n");
    e->foreground.mode = EFFECT_FINISHING;
    data->do_exit = 1;
    lemuria_rotator_turnto(&(data->rotator),
                           0.0, 0.0, 0.0, 100);
    }

  //  if(data->do_exit && 
  
  if(e->beat_detected && !data->do_exit)
    {
    if(lemuria_decide(e, 0.1))
      lemuria_rotator_change(e, &(data->rotator));
    
    if((data->get_coords == get_coords_square) &&
       lemuria_range_done(&(data->texture_range)) &&
       lemuria_decide(e, 0.2))
      {
      // What to do now?

      if((data->x_steps == 1) || (data->y_steps == 1))
        {
        lemuria_range_init(e, &(data->texture_range), 2, 80, 200);
        data->x_steps = lemuria_random_int(e, 2, MAX_X_STEPS);
        data->y_steps = data->x_steps;
        init_texture_coords(data, data->x_steps, data->y_steps);
        }
      else
        {
        random_number = lemuria_random_int(e, 0, 2);
        //        random_number = 2;
        switch(random_number)
          {
          case 0:
            lemuria_range_init(e, &(data->texture_range), 2, 80, 200);
            data->x_steps = lemuria_random_int(e, 1, MAX_X_STEPS);
            data->y_steps = data->x_steps;
            init_texture_coords(data, data->x_steps, data->y_steps);
            break;
          case 1:
            data->get_coords = get_coords_rotate1;
            break;
          case 2:
            data->get_coords = get_coords_wave1;
            break;
          }
        data->aux_initialized = 0;
        }
      }
    }
  
/*   lemuria_range_get(&data->deform_range, */
/*                     data->min_values, data->max_values, data->values); */
  
  glShadeModel(GL_SMOOTH);

  glEnable(GL_TEXTURE_2D);
  
  // Enable lighting
  
  lemuria_set_perspective(e, 1, 20.0);
  
  // Set up and enable light 0

  glEnable(GL_LIGHT0);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  
  //  glEnable(GL_POLYGON_SMOOTH);
  
  // All materials hereafter have full specular reflectivity
  // with a high shine

  lemuria_set_material(&material, GL_FRONT_AND_BACK);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  
  //  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light_world);

  lemuria_set_light(&light, GL_LIGHT0);
  glEnable(GL_LIGHTING);
 
  glMatrixMode(GL_MODELVIEW); 
  glPushMatrix();
  glLoadIdentity();
  glRotatef(90.0, 0.0, 1.0, 0.0);
  lemuria_rotate(&(data->rotator));

  glScalef(0.6, 0.6, 0.6);
  
  lemuria_texture_bind(e, 0);

  /* Set up the texture coordinates */

  lemuria_range_get(&(data->texture_range),
                    data->texture_x_start, data->texture_x_end,
                    texture_coords_max_x);

  lemuria_range_get(&(data->texture_range),
                    data->texture_y_start, data->texture_y_end,
                    texture_coords_max_y);
  
  /* Now, draw the squares */
  
  glBegin(GL_QUADS);

  texture_coords_y[0] = texture_coords_max_y[0];

  //  fprintf(stderr, "Steps: %d %d\n", data->x_steps, data->y_steps);

  data->first = 1;
  
  for(i = 0; i < data->y_steps; i++)
    {
    texture_coords_y[1] =
      texture_coords_max_y[0] +
      (texture_coords_max_y[1] - texture_coords_max_y[0]) *
      (i+1) / data->y_steps;

    texture_coords_x[0] = texture_coords_max_x[0];
    
    for(j = 0; j < data->x_steps; j++)
      {

      if((i == data->y_steps - 1) && (j == data->x_steps - 1))
        data->last = 1;
      else
        data->last = 0;
      
      texture_coords_x[1] =
        texture_coords_max_x[0] +
        (texture_coords_max_x[1] - texture_coords_max_x[0]) *
        (j+1) / data->x_steps;
      
      data->get_coords(e, data, j, i, coords, normals);
      
      glTexCoord2f(texture_coords_x[0], texture_coords_y[0]);
      glNormal3f(normals[0][0], normals[0][1], normals[0][2]);
      glVertex3f( coords[0][0],  coords[0][1],  coords[0][2]);

      glTexCoord2f(texture_coords_x[1], texture_coords_y[0]);
      glNormal3f(normals[1][0], normals[1][1], normals[1][2]);
      glVertex3f( coords[1][0],  coords[1][1],  coords[1][2]);

      glTexCoord2f(texture_coords_x[1], texture_coords_y[1]);
      glNormal3f(normals[2][0], normals[2][1], normals[2][2]);
      glVertex3f( coords[2][0],  coords[2][1],  coords[2][2]);

      glTexCoord2f(texture_coords_x[0], texture_coords_y[1]);
      glNormal3f(normals[3][0], normals[3][1], normals[3][2]);
      glVertex3f( coords[3][0],  coords[3][1],  coords[3][2]);
            
      texture_coords_x[0] = texture_coords_x[1];
      data->first = 0;
      }

    texture_coords_y[0] = texture_coords_y[1];

    }

  glEnd();
  
  glPopMatrix();
  
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);

  if(data->do_exit && (data->get_coords == get_coords_square) &&
     lemuria_rotator_done(&(data->rotator)))
    {
    //    fprintf(stderr, "Squares done\n");
    e->foreground.mode = EFFECT_DONE;
    
    }
  
     
  }

static void * init_squares(lemuria_engine_t * e)
  {
  int index;
  squares_data_t * data;

#ifdef DEBUG
  fprintf(stderr, "init_squares...");
#endif
  
  data = calloc(1, sizeof(squares_data_t));

  data->engine = e;
  lemuria_texture_ref(data->engine, 0);

  //  fprintf(stderr, "init_squares\n");
  
  e->foreground.mode = EFFECT_RUNNING;

  lemuria_rotator_reset(&(data->rotator));
  lemuria_rotator_change(e, &(data->rotator));
  
  data->get_coords = get_coords_square;
  data->x_steps = lemuria_random_int(e, 1, MAX_X_STEPS);
  //      data->y_steps = lemuria_random_int(1, MAX_Y_STEPS);
  data->y_steps = data->x_steps;
  
  data->aux_initialized = 0;

  lemuria_range_init(e, &(data->texture_range), 2, 50, 100);
    
  data->texture_x_start = data->texture_coords_x[0];
  data->texture_x_end   = data->texture_coords_x[1];
  
  data->texture_y_start = data->texture_coords_y[0];
  data->texture_y_end   = data->texture_coords_y[1];

  index = 1;
  
  init_texture_coords(data, data->x_steps, data->y_steps);
  init_texture_coords(data, data->x_steps, data->y_steps);

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif

  return data;
  }

static void delete_squares(void * e)
  {
  squares_data_t * data = (squares_data_t*)(e);
  lemuria_texture_unref(data->engine, 0);
  free(data);
  }

effect_plugin_t squares_effect =
  {
    .init =    init_squares,
    .draw =    draw_squares,
    .cleanup = delete_squares
  };
