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

#include <lemuria_private.h>
#include <effect.h>
#include <utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct
  {
  int dummy;
  } blur_data;

static float point_colors[][4] =
  {
    { 0.1, 0.1, 1.0, 1.0 },
    { 1.0, 1.0, 0.0, 1.0 },
    { 1.0, 1.0, 1.0, 1.0 },
    //    { 0.4, 0.4, 0.5, 1.0 },
    //    { 0.0, 1.0, 0.0, 1.0 },
  };

static int num_point_colors = sizeof(point_colors)/sizeof(float[4]);

static void * init_blur()
  {
  blur_data * ret;
#ifdef DEBUG
  fprintf(stderr, "init_blur...");
#endif
  
  ret = calloc(1, sizeof(blur_data));

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif

  return ret;
  }

#define DOTS_PER_BEAT 4

static void draw_blur(lemuria_engine_t * e, void * data)
  {
  float x, y;
  
  int random_number;
  int i;
  
  float * point_color;
  //  blur_data * d = (blur_data*)(data);

  glEnable(GL_POINT_SMOOTH);

  if(e->beat_detected)
    {
    glPointSize(10.0);

    glBegin(GL_POINTS);

    for(i = 0; i < DOTS_PER_BEAT; i++)
      {
      random_number = lemuria_random_int(e, 0, num_point_colors-1);
      point_color = point_colors[random_number];
      glColor4fv(point_color);
      
      x = lemuria_random(e, -1.0, 1.0);
      y = lemuria_random(e, -1.0, 1.0);
      glVertex2f(x, y);
      }
    glEnd();
    }
  
  glEnable(GL_DEPTH_TEST);                 // Enable Depth Buffer
  glDisable(GL_POINT_SMOOTH);
  }

static void cleanup_blur(void * data)
  {
  blur_data * d = (blur_data*)(data);
  free(d);
  }

effect_plugin_t blur_effect =
  {
    .init =    init_blur,
    .draw =    draw_blur,
    .cleanup = cleanup_blur,
  };
