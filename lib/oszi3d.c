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

//#include "light.h"
//#include "material.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Transformation matrices: These are calculated by OpenGL
// and saves here once

#define DRAW_SPARK     0
#define DRAW_MULTIPLE  1
#define DRAW_AFTERGLOW 2

#define DRAW_MAX       2

//#define PRINT_MATRICES

#ifdef PRINT_MATRICES

static int matrix_initialized = 0;

static float matrix_zoom_in[16];
static float matrix_zoom_out[16];

static float matrix_zoom_in_rotate[16];
static float matrix_zoom_out_rotate[16];

static void print_matrix(float * matrix, const char * name)
  {
  int i;
  fprintf(stderr, "static float matrix_%s[16] = \n  { ", name);
  for(i = 0; i < 4; i++)
    {
    fprintf(stderr, "%f, %f, %f, %f,\n",
            matrix[4*i],matrix[4*i+1],
            matrix[4*i+2],matrix[4*i+3]);
    }
  fprintf(stderr, "};\n");
  }

static void init_matrices()
  {
  if(matrix_initialized)
    return;

  matrix_initialized = 1;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -0.05);
  glScalef(0.95, 0.95, 0.95);
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix_zoom_out);
  print_matrix(matrix_zoom_out, "zoom_out");
               
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -0.05);
  glScalef(0.95, 0.95, 0.95);
  glRotatef(2.0, 0.0, 0.0, 1.0);
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix_zoom_out_rotate);
  print_matrix(matrix_zoom_out_rotate, "zoom_out_rotate");

  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0.0, 0.0, 0.05);
  glScalef(1.05, 1.05, 1.05);
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix_zoom_in);
  print_matrix(matrix_zoom_in, "zoom_in");

  glLoadIdentity();
  glTranslatef(0.0, 0.0, 0.05);
  glScalef(1.05, 1.05, 1.05);
  glRotatef(2.0, 0.0, 0.0, 1.0);
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix_zoom_in_rotate);
  print_matrix(matrix_zoom_in_rotate, "zoom_in_rotate");

  //  glLoadIdentity();
  
  glPopMatrix();
  }

#else
static float matrix_zoom_out[16] = 
  { 0.950000, 0.000000, 0.000000, 0.000000,
    0.000000, 0.950000, 0.000000, 0.000000,
    0.000000, 0.000000, 0.950000, 0.000000,
    0.000000, 0.000000, -0.050000, 1.000000,
  };

static float matrix_zoom_out_rotate[16] = 
  { 0.949421, 0.033155, 0.000000, 0.000000,
    -0.033155, 0.949421, 0.000000, 0.000000,
    0.000000, 0.000000, 0.950000, 0.000000,
    0.000000, 0.000000, -0.050000, 1.000000,
  };

static float matrix_zoom_in[16] = 
  { 1.050000, 0.000000, 0.000000, 0.000000,
    0.000000, 1.050000, 0.000000, 0.000000,
    0.000000, 0.000000, 1.050000, 0.000000,
    0.000000, 0.000000, 0.050000, 1.000000,
  };

static float matrix_zoom_in_rotate[16] = 
  { 1.049360, 0.036644, 0.000000, 0.000000,
    -0.036644, 1.049360, 0.000000, 0.000000,
    0.000000, 0.000000, 1.050000, 0.000000,
    0.000000, 0.000000, 0.050000, 1.000000,
  };
#endif

// Colors

static float line_colors[][3] =
  {
    { 0.98, 1.00, 0.64 },
    { 0.72, 0.98, 1.00 },
    { 1.00, 0.83, 0.94 },
    { 0.88, 1.00, 0.88 }
  };

static int num_line_colors = sizeof(line_colors)/sizeof(line_colors[0]);

#define NSAMPLES      512
#define NLINES        30
#define MAX_AFTERGLOW 6

// #define MAX_ALPHA_THICKNESS 10

typedef struct line_data_s
  {
  float color[3];
  float coords[NSAMPLES][3];
  } line_data;

typedef void (*get_coords_func)(float coords_l[NSAMPLES][3],
                                float coords_r[NSAMPLES][3],
                                float direction_l[NSAMPLES][3],
                                float direction_r[NSAMPLES][3],
                                int transition);

typedef struct
  {
  get_coords_func get_coords;

  float * matrix_l;
  float * matrix_r;

  //  float reverse_l;
  //  float reverse_r;
  
  } coords_data;

#define FINISH_NONE      0
#define FINISH_QUEUED    1
#define FINISH_RUNNING   2

typedef struct
  {
  line_data lines_l[NLINES];
  line_data lines_r[NLINES];

  int active_lines; // Number of lines to draw right now
  int delta_active_lines;

  int draw_mode;
    
  coords_data * coords_start;
  coords_data * coords_end;
  int coords_index;

  lemuria_range_t get_coords_range;
  lemuria_range_t color_range;
  
  float * start_color_l;
  float * end_color_l;

  float * start_color_r;
  float * end_color_r;

  int finish_status;
  
  } oszi3d_data;

// Horizontal lines

static void get_coords_horizontal(float coords_l[NSAMPLES][3],
                                  float coords_r[NSAMPLES][3],
                                  float direction_l[NSAMPLES][3],
                                  float direction_r[NSAMPLES][3],
                                  int transition)
     
  {
  int i;
  float t;
  
  float y = transition ? -2.0 : 0.5;
  //  fprintf(stderr, "get_coords_horizontal\n");
  for(i = 0; i < NSAMPLES; i++)
    {
    t = (float)i/(float)(NSAMPLES-1);
    
    coords_l[i][0] = -1.0 + t * 2.0;
    coords_r[i][0] = -1.0 + t * 2.0;

    coords_l[i][1] = -y;
    coords_r[i][1] =  y;

    coords_l[i][2] =  0.0;
    coords_r[i][2] =  0.0;
    
    direction_l[i][0] = 0.0;
    direction_r[i][0] = 0.0;
    
    direction_l[i][1] = 0.5;
    direction_r[i][1] = 0.5;
    
    direction_l[i][2] = 0.0;
    direction_r[i][2] = 0.0;
    }
  }

static void get_coords_horizontal_1(float coords_l[NSAMPLES][3],
                                    float coords_r[NSAMPLES][3],
                                    float direction_l[NSAMPLES][3],
                                    float direction_r[NSAMPLES][3],
                                    int transition)
     
  {
  int i;
  float t;
  
  float y = transition ? -2.0 : 0.5;
  //  fprintf(stderr, "get_coords_horizontal\n");
  for(i = 0; i < NSAMPLES; i++)
    {
    t = (float)i/(float)(NSAMPLES-1);
    
    coords_l[i][0] = -1.0 + t * 2.0;
    coords_r[i][0] = -1.0 + t * 2.0;

    coords_l[i][1] = -y;
    coords_r[i][1] =  y;

    coords_l[i][2] =  0.0;
    coords_r[i][2] =  0.0;
    
    direction_l[i][0] = 0.0;
    direction_r[i][0] = 0.0;
    
    direction_l[i][1] = 0.5;
    direction_r[i][1] = 0.5;
    
    direction_l[i][2] = 0.0;
    direction_r[i][2] = 0.0;
    }
  }


// Vertical lines

static void get_coords_vertical(float coords_l[NSAMPLES][3],
                                float coords_r[NSAMPLES][3],
                                float direction_l[NSAMPLES][3],
                                float direction_r[NSAMPLES][3],
                                int transition)
     
  {
  int i;
  float t;
  float x = transition ? -2.5 : 1.0;
  //  fprintf(stderr, "get_coords_vertical\n");
  
  for(i = 0; i < NSAMPLES; i++)
    {
    t = (float)i/(float)(NSAMPLES-1);
    
    coords_l[i][0] = -x;
    coords_r[i][0] =  x;

    coords_l[i][1] = -1.0 + t * 2.0;
    coords_r[i][1] = -1.0 + t * 2.0;

    coords_l[i][2] =  0.0;
    coords_r[i][2] =  0.0;
    
    direction_l[i][0] = 0.5;
    direction_r[i][0] = 0.5;
    
    direction_l[i][1] = 0.0;
    direction_r[i][1] = 0.0;
    
    direction_l[i][2] = 0.0;
    direction_r[i][2] = 0.0;
    }
  }

static void get_coords_vertical_1(float coords_l[NSAMPLES][3],
                                  float coords_r[NSAMPLES][3],
                                  float direction_l[NSAMPLES][3],
                                  float direction_r[NSAMPLES][3],
                                  int transition)
     
  {
  int i;
  float t;
  float x = transition ? -2.5 : 0.5;
  //  fprintf(stderr, "get_coords_vertical\n");
  
  for(i = 0; i < NSAMPLES; i++)
    {
    t = (float)i/(float)(NSAMPLES-1);
    
    coords_l[i][0] = -x;
    coords_r[i][0] =  x;

    coords_l[i][1] = -1.0 + t * 2.0;
    coords_r[i][1] = -1.0 + t * 2.0;

    coords_l[i][2] =  0.0;
    coords_r[i][2] =  0.0;
    
    direction_l[i][0] = 0.5;
    direction_r[i][0] = 0.5;
    
    direction_l[i][1] = 0.0;
    direction_r[i][1] = 0.0;
    
    direction_l[i][2] = 0.0;
    direction_r[i][2] = 0.0;
    }
  }

static void get_coords_circular(float coords_l[NSAMPLES][3],
                                float coords_r[NSAMPLES][3],
                                float direction_l[NSAMPLES][3],
                                float direction_r[NSAMPLES][3],
                                int transition)
  {
  int i;
  float t;

  float radius_factor = transition ? 5.0 : 1.0;
  float cos_angle, sin_angle;
  //  fprintf(stderr, "get_coords_circular\n");
  
  for(i = 0; i < NSAMPLES; i++)
    {
    t = (float)i/(float)(NSAMPLES-1);

    cos_angle = cos(2.0*M_PI*t);
    sin_angle = sin(2.0*M_PI*t);
    
    coords_l[i][0] = radius_factor * 0.7 * cos_angle;
    coords_r[i][0] = radius_factor * 0.4 * cos_angle;
    
    coords_l[i][1] = radius_factor * 0.7 * sin_angle;
    coords_r[i][1] = radius_factor * 0.4 * sin_angle;
    
    coords_l[i][2] = 0.0;
    coords_r[i][2] = 0.0;
    
    direction_l[i][0] = 0.3 * cos_angle;
    direction_r[i][0] = 0.2 * cos_angle;
    
    direction_l[i][1] = 0.3 * sin_angle;
    direction_r[i][1] = 0.2 * sin_angle;
    
    direction_l[i][2] = 0.0;
    direction_r[i][2] = 0.0;
    }
  }

static void get_coords_circular_1(float coords_l[NSAMPLES][3],
                                  float coords_r[NSAMPLES][3],
                                  float direction_l[NSAMPLES][3],
                                  float direction_r[NSAMPLES][3],
                                  int transition)
  {
  int i;
  float t;

  float radius_factor = transition ? 5.0 : 1.0;
  float cos_angle, sin_angle;
  //  fprintf(stderr, "get_coords_circular\n");
  
  for(i = 0; i < NSAMPLES; i++)
    {
    t = (float)i/(float)(NSAMPLES-1);

    cos_angle = cos(M_PI*t - 0.5 * M_PI);
    sin_angle = sin(M_PI*t - 0.5 * M_PI);
    
    coords_l[i][0] = -radius_factor * 0.7 * cos_angle;
    coords_r[i][0] =  radius_factor * 0.7 * cos_angle;
    
    coords_l[i][1] =  radius_factor * 0.7 * sin_angle;
    coords_r[i][1] =  radius_factor * 0.7 * sin_angle;
    
    coords_l[i][2] = 0.0;
    coords_r[i][2] = 0.0;
    
    direction_l[i][0] =    0.3 * cos_angle;
    direction_r[i][0] =    0.3 * cos_angle;
    
    direction_l[i][1] = -0.3 * sin_angle;
    direction_r[i][1] =  0.3 * sin_angle;
    
    direction_l[i][2] = 0.0;
    direction_r[i][2] = 0.0;
    }
  }


coords_data coords_stuff[] =
  {
#if 1
    {
      .get_coords = get_coords_horizontal,

      .matrix_l = matrix_zoom_out,
      .matrix_r = matrix_zoom_out,

    },
    {
      .get_coords = get_coords_horizontal_1,

      .matrix_l = matrix_zoom_in,
      .matrix_r = matrix_zoom_in,

    },
    {
      .get_coords = get_coords_horizontal,

      .matrix_l = matrix_zoom_out_rotate,
      .matrix_r = matrix_zoom_out_rotate,

    },
#endif
    {
      .get_coords = get_coords_vertical,

      .matrix_l = matrix_zoom_out,
      .matrix_r = matrix_zoom_out,

    },
    {
      .get_coords = get_coords_vertical_1,

      .matrix_l = matrix_zoom_in,
      .matrix_r = matrix_zoom_in,

    },
#if 1
    {
      .get_coords = get_coords_circular,

      .matrix_l = matrix_zoom_in_rotate,
      .matrix_r = matrix_zoom_out_rotate,

    },
    {
      .get_coords = get_coords_circular_1,

      .matrix_l = matrix_zoom_out_rotate,
      .matrix_r = matrix_zoom_out_rotate,
    }
#endif

  };

static int num_coords = sizeof(coords_stuff)/sizeof(coords_stuff[0]);

// #define NUM_COORDS_FUNCS 3

static void update_samples(lemuria_engine_t * e,
                           float * samples_l,
                           float * samples_r)
  {
  int i;
  
  for(i = 0; i < NSAMPLES; i++)
    {
    samples_l[i] =
      (float)(e->time_buffer_read[0][i])/(32768.0);
    samples_r[i] =
      (float)(e->time_buffer_read[1][i])/(32768.0);
    }
  }

static void transform_coords(float * in_coords,
                             float * out_coords,
                             float * matrix)
  {
  out_coords[0] =
    in_coords[0] * matrix[0] + in_coords[1] * matrix[1] +
    in_coords[2] * matrix[2] +                matrix[3];

  out_coords[1] =
    in_coords[0] * matrix[4] + in_coords[1] * matrix[5] +
    in_coords[2] * matrix[6] +                matrix[7];

  out_coords[2] =
    in_coords[0] * matrix[8]  + in_coords[1] * matrix[9] +
    in_coords[2] * matrix[10] +                matrix[11];
  }

static void draw_oszi3d(lemuria_engine_t * e, void * user_data)
  {
  int i, j;
  oszi3d_data * d;

  float start_coords_l[NSAMPLES][3];
  float start_coords_r[NSAMPLES][3];
  float start_direction_l[NSAMPLES][3];
  float start_direction_r[NSAMPLES][3];

  float end_coords_l[NSAMPLES][3];
  float end_coords_r[NSAMPLES][3];
  float end_direction_l[NSAMPLES][3];
  float end_direction_r[NSAMPLES][3];

  float coords_l[NSAMPLES][3];
  float coords_r[NSAMPLES][3];
  float direction_l[NSAMPLES][3];
  float direction_r[NSAMPLES][3];

  float samples_l[NSAMPLES];
  float samples_r[NSAMPLES];
  float matrix_l[16];
  float matrix_r[16];

  float alpha;
  
  int random_number;

  int transition_start = 0;
  int transition_end = 0;
  int random_number_1;
  d = (oszi3d_data*)(user_data);

  //  fprintf(stderr, "draw oszi...");
  
  // Update number of active lines
  
  d->active_lines += d->delta_active_lines;
  
  if(d->active_lines < 1)
    {
    d->active_lines = 1;
    d->delta_active_lines = 0;
    d->draw_mode = DRAW_SPARK;
    }
  else if((d->draw_mode == DRAW_MULTIPLE) && (d->active_lines > NLINES))
    {
    d->active_lines = NLINES;
    d->delta_active_lines = 0;
    }
  else if((d->draw_mode == DRAW_AFTERGLOW) &&
          (d->active_lines > MAX_AFTERGLOW))
    {
    d->active_lines = MAX_AFTERGLOW;
    d->delta_active_lines = 0;
    }
  
  //  fprintf(stderr, "Active lines: %d\n", d->active_lines);

  // Check, if we should finish
  
  if(e->foreground.mode == EFFECT_FINISH)
    {
    e->foreground.mode = EFFECT_FINISHING;
    if((d->delta_active_lines > 0) || (d->active_lines > 1))
      d->delta_active_lines = -1;
    
    if(lemuria_range_done(&(d->get_coords_range)))
      {
      d->coords_start = d->coords_end;
      lemuria_range_init(e, &(d->get_coords_range),
                         3, 100, 200);
      
      random_number = lemuria_random_int(e, 0, num_coords - 1);
      d->coords_end = &(coords_stuff[random_number]);
      d->finish_status = FINISH_RUNNING;
      }
    else
      {
      d->finish_status = FINISH_QUEUED;
      //      fprintf(stderr, "Queueing exit command\n");
      }
    }
  
  if((d->finish_status == FINISH_QUEUED) &&
     (lemuria_range_done(&(d->get_coords_range))))
    {
    d->coords_start = d->coords_end;
    lemuria_range_init(e, &(d->get_coords_range),
                       3, 100, 200);
    
    random_number = lemuria_random_int(e, 0, num_coords - 1);
    d->coords_end = &(coords_stuff[random_number]);
    //    d->coords_index = random_number;
    d->finish_status = FINISH_RUNNING;
    //    fprintf(stderr, "Qeueud finish now running\n");
    }
  
  // Change programs
  
  if(e->beat_detected && (e->foreground.mode == EFFECT_RUNNING))
    {
    // Change number of active lines

    if(d->delta_active_lines == 0)
      {
      if((d->active_lines > 1) && lemuria_decide(e, 0.05))
        d->delta_active_lines = -1;
      else if((d->draw_mode == DRAW_SPARK) && lemuria_decide(e, 0.5))
        {
        d->active_lines = 1;
        d->delta_active_lines = 1;
        if(lemuria_decide(e, 0.5))
          d->draw_mode = DRAW_MULTIPLE;
        else
          d->draw_mode = DRAW_AFTERGLOW;
        }
      }
    
    if(lemuria_range_done(&(d->get_coords_range)) && lemuria_decide(e, 0.1))
      {
      d->coords_start = d->coords_end;
      lemuria_range_init(e, &(d->get_coords_range),
                         3, 100, 200);

      random_number = lemuria_random_int(e, 0, num_coords - 2);
      if(random_number >= d->coords_index)
        random_number++;
      d->coords_end = &(coords_stuff[random_number]);
      d->coords_index = random_number;
      }
    if(lemuria_range_done(&(d->color_range)) && lemuria_decide(e, 0.1))
      {
      d->start_color_l = d->end_color_l;
      d->start_color_r = d->end_color_r;
      
      random_number = lemuria_random_int(e, 0, num_line_colors-1);
      d->end_color_l = line_colors[random_number];
      
      random_number_1 = lemuria_random_int(e, 0, num_line_colors-2);
      if(random_number_1 >= random_number)
        random_number_1++;
      d->end_color_r = line_colors[random_number_1];
      lemuria_range_init(e, &(d->color_range),
                         3, NLINES/2, 2*NLINES);
      }
    }
  else if(e->quiet && d->delta_active_lines < NLINES)
    d->delta_active_lines = 1;
  
  // Update ranges
  
  lemuria_range_update(&(d->get_coords_range));
  lemuria_range_update(&(d->color_range));
    
  // Update older lines
    
  if(d->draw_mode == DRAW_MULTIPLE)
    {
    lemuria_range_get_n(&(d->get_coords_range),
                        d->coords_start->matrix_l,
                        d->coords_end->matrix_l,
                        matrix_l, 16);
    lemuria_range_get_n(&(d->get_coords_range),
                        d->coords_start->matrix_r,
                        d->coords_end->matrix_r,
                        matrix_r, 16);
    
    for(i = d->active_lines-1; i > 0; i--)
      {
      for(j = 0; j < NSAMPLES; j++)
        {
        transform_coords(d->lines_l[i-1].coords[j],
                         d->lines_l[i].coords[j],
                         matrix_l);
        transform_coords(d->lines_r[i-1].coords[j],
                         d->lines_r[i].coords[j],
                         matrix_r);
        }
      memcpy(d->lines_l[i].color, d->lines_l[i-1].color, sizeof(float) * 3);
      memcpy(d->lines_r[i].color, d->lines_r[i-1].color, sizeof(float) * 3);
      }
    }
  else if(d->draw_mode == DRAW_AFTERGLOW)
    {
    // This could be avoided but would make the code harder to debug
    for(i = d->active_lines-1; i > 0; i--)
      {
      memcpy(&(d->lines_l[i]), &(d->lines_l[i-1]), sizeof(line_data));
      memcpy(&(d->lines_r[i]), &(d->lines_r[i-1]), sizeof(line_data));
      }
    }
  
  // Make newest line
  
  update_samples(e, samples_l, samples_r);

  //  fprintf(stderr, "1");
  
  if(d->finish_status == FINISH_RUNNING)
    transition_end = 1;
  else if(e->foreground.mode == EFFECT_STARTING)
    transition_start = 1;
  
  d->coords_start->get_coords(start_coords_l,
                              start_coords_r,
                              start_direction_l,
                              start_direction_r,
                              transition_start);
  
  d->coords_end->get_coords(end_coords_l,
                            end_coords_r,
                            end_direction_l,
                            end_direction_r,
                            transition_end);
  //  fprintf(stderr, "2");

  
  for(i = 0; i < NSAMPLES; i++)
    {
    lemuria_range_get(&(d->get_coords_range),
                      start_coords_l[i],
                      end_coords_l[i],
                      coords_l[i]);

    lemuria_range_get(&(d->get_coords_range),
                      start_coords_r[i],
                      end_coords_r[i],
                      coords_r[i]);
    
    lemuria_range_get(&(d->get_coords_range),
                      start_direction_l[i],
                      end_direction_l[i],
                      direction_l[i]);
    lemuria_range_get(&(d->get_coords_range),
                      start_direction_r[i],
                      end_direction_r[i],
                      direction_r[i]);
    
    }

  
  // Update newest line

  // Colors
  
  lemuria_range_get(&(d->color_range),
                    d->start_color_l,
                    d->end_color_l,
                    d->lines_l[0].color);
  lemuria_range_get(&(d->color_range),
                    d->start_color_r,
                    d->end_color_r,
                    d->lines_r[0].color);
  
  for(j = 0; j < NSAMPLES; j++)
    {
    d->lines_l[0].coords[j][0] = coords_l[j][0]+
      direction_l[j][0]*samples_l[j];
    d->lines_r[0].coords[j][0] = coords_r[j][0]+
      direction_r[j][0]*samples_r[j];

    d->lines_l[0].coords[j][1] = coords_l[j][1]+
      direction_l[j][1]*samples_l[j];
    d->lines_r[0].coords[j][1] = coords_r[j][1]+
      direction_r[j][1]*samples_r[j];

    d->lines_l[0].coords[j][2] = coords_l[j][2]+
      direction_l[j][2]*samples_l[j];
    d->lines_r[0].coords[j][2] = coords_r[j][2]+
      direction_r[j][2]*samples_r[j];


    }
  
  // Draw the stuff
 
  lemuria_set_perspective(e, 1, 50.0);

  glDisable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
  
  if(d->draw_mode == DRAW_SPARK)
    {
    glLineWidth(20.0);
    
    glColor4f(d->lines_l[0].color[0],
              d->lines_l[0].color[1],
              d->lines_l[0].color[2], 0.3);;
    glBegin(GL_LINE_STRIP);
    for(j = 0; j < NSAMPLES; j++)
      {
      glVertex3fv(d->lines_l[0].coords[j]);
      }
    glEnd();
    
    glColor4f(d->lines_r[0].color[0],
              d->lines_r[0].color[1],
              d->lines_r[0].color[2], 0.3);
    glBegin(GL_LINE_STRIP);
    for(j = 0; j < NSAMPLES; j++)
      {
      glVertex3fv(d->lines_r[0].coords[j]);
      }
    glEnd();
    //      }
    }
  else if(d->draw_mode == DRAW_MULTIPLE)
    {
    glLineWidth(2.0);
    for(i = d->active_lines-1; i > 0; i--)
      {
      alpha = (float)(NLINES - 1 - i)/(float)(NLINES-1);
      
      glColor4f(d->lines_l[i].color[0],
                d->lines_l[i].color[1],
                d->lines_l[i].color[2], alpha);
      
      glBegin(GL_LINE_STRIP);
      for(j = 0; j < NSAMPLES; j++)
        {
        glVertex3fv(d->lines_l[i].coords[j]);
        }
      glEnd();
      
      glColor4f(d->lines_r[i].color[0],
                d->lines_r[i].color[1],
                d->lines_r[i].color[2], alpha);
      glBegin(GL_LINE_STRIP);
      for(j = 0; j < NSAMPLES; j++)
        {
        glVertex3fv(d->lines_r[i].coords[j]);
        }
      glEnd();
      }
    }

  else if(d->draw_mode == DRAW_AFTERGLOW)
    {
    glLineWidth(2.0);
    for(i = d->active_lines-1; i > 0; i--)
      {
      alpha = (float)(MAX_AFTERGLOW - 1 - i)/(float)(MAX_AFTERGLOW-1);
      
      glColor4f(d->lines_l[i].color[0],
                d->lines_l[i].color[1],
                d->lines_l[i].color[2], alpha);
      
      glBegin(GL_LINE_STRIP);
      for(j = 0; j < NSAMPLES; j++)
        {
        glVertex3fv(d->lines_l[i].coords[j]);
        }
      glEnd();
      
      glColor4f(d->lines_r[i].color[0],
                d->lines_r[i].color[1],
                d->lines_r[i].color[2], alpha);
      glBegin(GL_LINE_STRIP);
      for(j = 0; j < NSAMPLES; j++)
        {
        glVertex3fv(d->lines_r[i].coords[j]);
        }
      glEnd();
      }
    }
  
  // Draw latest line
  glLineWidth(2.0);
  glColor4f(d->lines_l[0].color[0],
            d->lines_l[0].color[1],
            d->lines_l[0].color[2], 1.0);
  glBegin(GL_LINE_STRIP);
  for(j = 0; j < NSAMPLES; j++)
    {
    glVertex3fv(d->lines_l[0].coords[j]);
    }
  glEnd();
  
  glColor4f(d->lines_r[0].color[0],
            d->lines_r[0].color[1],
            d->lines_r[0].color[2], 1.0);
  glBegin(GL_LINE_STRIP);
  for(j = 0; j < NSAMPLES; j++)
    {
    glVertex3fv(d->lines_r[0].coords[j]);
    }
  glEnd();
  

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);


  
  if(lemuria_range_done(&(d->get_coords_range)))
    {
    if(d->finish_status == FINISH_RUNNING)
      e->foreground.mode = EFFECT_DONE;
    else if(e->foreground.mode == EFFECT_STARTING)
      e->foreground.mode = EFFECT_RUNNING;
    }
  //  fprintf(stderr, "done\n");

  }

static void * init_oszi3d(lemuria_engine_t * e)
  {
  int random_number, random_number_1;
  oszi3d_data * data;
#ifdef DEBUG
  fprintf(stderr, "init_oszi3d...");
#endif

#ifdef PRINT_MATRICES
  init_matrices();
#endif
  e->foreground.mode = EFFECT_STARTING;
  
  data = calloc(1, sizeof(oszi3d_data));

  data->active_lines = 1;
  data->delta_active_lines = 0;

  data->draw_mode = DRAW_SPARK;
    
  random_number = lemuria_random_int(e, 0, num_coords-1);
  data->coords_start = &(coords_stuff[random_number]);

  random_number = lemuria_random_int(e, 0, num_coords-1);
  data->coords_end = &(coords_stuff[random_number]);
  data->coords_index = random_number;
    
  lemuria_range_init(e, &(data->get_coords_range),
                     3, 100, 200);
  
  random_number = lemuria_random_int(e, 0, num_line_colors-1);
  data->start_color_l = line_colors[random_number];

  random_number_1 = lemuria_random_int(e, 0, num_line_colors-2);
  if(random_number_1 >= random_number)
    random_number_1++;
  data->start_color_r = line_colors[random_number_1];

  random_number = lemuria_random_int(e, 0, num_line_colors-1);
  data->end_color_l = line_colors[random_number];

  random_number_1 = lemuria_random_int(e, 0, num_line_colors-2);
  if(random_number_1 >= random_number)
    random_number_1++;
  data->end_color_r = line_colors[random_number_1];
  
  lemuria_range_init(e, &(data->color_range),
                     3, 100, 200);
#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  return data;
  }

static void delete_oszi3d(void * e)
  {
  oszi3d_data * data = (oszi3d_data*)(e);
  free(data);
  }

effect_plugin_t oszi3d_effect =
  {
    .init =    init_oszi3d,
    .draw =    draw_oszi3d,
    .cleanup = delete_oszi3d,
  };
