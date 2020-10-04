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
// #include "object.h"
#include <effect.h>
#include <tentacle.h>

static void calc_normals(lemuria_tentacle_t * t)
  {
  int i;
  float angle;

  angle = atan2(t->points[1].coords[1] - t->points[0].coords[1],
                t->points[1].coords[0] - t->points[0].coords[0]) + 0.5 * M_PI;
  t->points[0].normals[0] = cos(angle);
  t->points[0].normals[1] = sin(angle);

  for(i = 1; i < t->num_points - 1; i++)
    {
    angle = atan2(t->points[i+1].coords[1] - t->points[i-1].coords[1],
                  t->points[i+1].coords[0] - t->points[i-1].coords[0]) + 0.5 * M_PI;
    t->points[i].normals[0] = cos(angle);
    t->points[i].normals[1] = sin(angle);
    }

  angle = atan2(t->points[t->num_points-1].coords[1] - t->points[t->num_points-2].coords[1],
                t->points[t->num_points-1].coords[0] - t->points[t->num_points-2].coords[0]) + 0.5 * M_PI;
  t->points[t->num_points-1].normals[0] = cos(angle);
  t->points[t->num_points-1].normals[1] = sin(angle);
  
  }

static void calc_coords(lemuria_tentacle_t * t)
  {
  float angle;
  float tan_angle;
  int i;

  for(i = 1; i < t->num_points; i++)
    {
    tan_angle = t->amplitude * sin(t->phase - 2.0 * M_PI * (float)i * t->segment_length / t->wavelength);
    angle = atan(tan_angle);
    t->points[i].coords[0] = t->points[i-1].coords[0] + t->segment_length * cos(angle);
    t->points[i].coords[1] = t->points[i-1].coords[1] + t->segment_length * sin(angle);
    }
  }

void lemuria_tentacle_init(lemuria_tentacle_t * t,
                           int num_points, float length,
                           float frequency, float wavelength,
                           float phase, float amplitude)
  {
  t->segment_length = length / (float)(num_points - 1);
  t->num_points = num_points;
  t->frequency = frequency;
  t->wavelength = wavelength;
  t->phase = phase;
  t->points = malloc(num_points * sizeof(*(t->points)));
  t->amplitude = amplitude,
  t->points[0].coords[0] = 0.0;
  t->points[0].coords[1] = 0.0;

  calc_coords(t);
#if 0
  for(i = 1; i < t->num_points; i++)
    {
    t->points[i].coords[0] = t->points[i-1].coords[0] + t->segment_length;
    t->points[i].coords[1] = 0.0;
    }
#endif
  calc_normals(t);
  }

void lemuria_tentacle_cleanup(lemuria_tentacle_t * t)
  {
  free(t->points);
  //  free(t);
  }

void lemuria_tentacle_update(lemuria_tentacle_t * t)
  {
  t->phase += t->frequency;
  if(t->phase > 2.0 * M_PI)
    t->phase -= 2.0 * M_PI;
  calc_coords(t);
  calc_normals(t);
  }

/* Plugin for testing purposes */

typedef struct
  {
  lemuria_tentacle_t t;
  } tentacle_data;

static void * init_tentacle(lemuria_engine_t * e)
  {
  tentacle_data * ret = calloc(1, sizeof(*ret));
  lemuria_tentacle_init(&(ret->t),
                        40, /* num_points */
                        2.5,  /* length */
                        0.1, /* frequency */
                        1.0,  /* Wavelength */
                        0.0,  /* Phase */
                        0.25 * M_PI /* Amplitude */);
  
  return ret;
  }

static void draw_tentacle(lemuria_engine_t * e, void * data)
  {
  int i;
  tentacle_data * d = (tentacle_data*)data;

  lemuria_tentacle_update(&(d->t));

  glLineWidth(1.0);
  glBegin(GL_LINES);
  for(i = 0; i < d->t.num_points; i++)
    {
    /* Draw segment */
    if(i < d->t.num_points-1)
      {
      glColor3f(0.0, 1.0, 0.0);
      glVertex2f(d->t.points[i].coords[0]   - 1.0, d->t.points[i].coords[1]);
      glVertex2f(d->t.points[i+1].coords[0] - 1.0, d->t.points[i+1].coords[1]);
      }
    /* Draw normal */
    glColor3f(1.0, 0.0, 0.0);
    glVertex2f(d->t.points[i].coords[0] - 1.0, d->t.points[i].coords[1]);
    glVertex2f(d->t.points[i].coords[0] - 1.0 + 0.1 * d->t.points[i].normals[0],
               d->t.points[i].coords[1] +       0.1 * d->t.points[i].normals[1]);
    }
  glEnd();
  }

static void delete_tentacle(void * d)
  {
  
  }

effect_plugin_t tentacle_effect =
  {
    .init =    init_tentacle,
    .draw =    draw_tentacle,
    .cleanup = delete_tentacle,
  };
