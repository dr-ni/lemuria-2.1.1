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
#include <stdlib.h>
#include <math.h>

#include <lemuria_private.h>
#include <effect.h>
#include <utils.h>

#define NUM_SAMPLES 256
#define NUM_FRAMES   2

#define DRAW_LINE   0
#define DRAW_POINTS 1

static float line_colors[][3] =
  {
    { 1.0, 1.0, 0.0 },
    { 1.0, 0.4, 0.4 },
    { 0.4, 0.4, 1.0 }
  };

static int num_line_colors = sizeof(line_colors)/sizeof(line_colors[0]);

typedef struct
  {
  float line_1_x[512];
  float line_1_y[512];

  int color_start;
  int color_end;
  
  lemuria_range_t color_range;
    
  //  int color_counter;

  int frame_counter;
  int draw_mode;
  } vectorscope_data;


static void * init_vectorscope(lemuria_engine_t * e)
  {
  vectorscope_data * d;

#ifdef DEBUG
  fprintf(stderr, "init_vectorscope...");
#endif
  
  d = calloc(1, sizeof(vectorscope_data));
    
  d->color_start = lemuria_random_int(e, 0, num_line_colors-1);
  d->color_end =   lemuria_random_int(e, 0, num_line_colors-1);
  d->draw_mode = lemuria_random_int(e, 0, 1);
  lemuria_range_init(e, &d->color_range, 3, 25, 75);

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif

  return d;
  }

#define SQRT_2 0.707106871

static void draw_vectorscope(lemuria_engine_t * e, void * user_data)
  {
  int i;
  float color[3];

  float coords_orig[2];
  float coords_transformed[2];
  float factor;
  
  vectorscope_data * d = (vectorscope_data *)(user_data);

  lemuria_range_update(&d->color_range);
  if(e->beat_detected && lemuria_range_done(&d->color_range) &&
     lemuria_decide(e, 0.5))
    {
    d->color_start = d->color_end;
    d->color_end = lemuria_random_int(e, 0, num_line_colors-2);
    if(d->color_end >= d->color_start)
      d->color_end++;
    lemuria_range_init(e, &d->color_range, 3, 25, 75);
    }

  // Check whether to draw at all

  d->frame_counter++;
  if(d->frame_counter < NUM_FRAMES)
    return;
  d->frame_counter = 0;

  // Draw the stuff

  lemuria_range_get(&d->color_range,
                    line_colors[d->color_start],
                    line_colors[d->color_end],
                    color);
  glColor3fv(color);

  if(d->draw_mode == DRAW_POINTS)
    {
    if(e->beat_detected)
      glPointSize(5.0);
    else
      glPointSize(3.0);
    
    glBegin(GL_POINTS);
    }
  else
    {
    if(e->beat_detected)
      glLineWidth(5.0);
    else
      glLineWidth(2.0);
    glBegin(GL_LINE_STRIP);
    }
  factor = 1.0 + 0.3 * (32768.0 / (float)(e->loudness+1));

  for(i = 0; i < NUM_SAMPLES; i++)
    {
    coords_orig[0] = factor * (float)(e->time_buffer_read[0][i])/32768.0;
    coords_orig[1] = factor * (float)(e->time_buffer_read[1][i])/32768.0;

    coords_transformed[0] = SQRT_2 * coords_orig[0] - SQRT_2 * coords_orig[1];
    coords_transformed[1] = SQRT_2 * coords_orig[0] + SQRT_2 * coords_orig[1];
    
    glVertex2f(coords_transformed[0], coords_transformed[1]);
    }
  glEnd();
  }

static void delete_vectorscope(void * d)
  {
  vectorscope_data * data = (vectorscope_data *)(d);
  free(data);
  }

effect_plugin_t vectorscope_effect =
  {
    .init =    init_vectorscope,
    .draw =    draw_vectorscope,
    .cleanup = delete_vectorscope
  };
