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

//#define TEST_TRANSFORM

static float line_colors[][2][4] =
  {
    { { 1.0, 0.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 } },
    { { 0.2, 0.2, 1.0, 1.0 }, { 1.0, 0.5, 0.0, 1.0 } },
    { { 0.2, 0.5, 1.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 } },
    { { 1.0, 0.0, 0.0, 1.0 }, { 1.0 ,1.0, 1.0, 1.0 } },
    { { 0.0, 0.0, 1.0, 1.0 }, { 1.0 ,1.0, 1.0, 1.0 } },
    //    { { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.4, 0.2, 1.0 } }
  };

static int num_line_colors = sizeof(line_colors)/sizeof(float[4][2]);

typedef struct
  {
  float line_1_x[512];
  float line_1_y[512];

  float line_2_x[512];
  float line_2_y[512];

  int line_start_index;
  int line_end_index;

  int transform_start_index;
  int transform_end_index;
  
/*   float color_1[3]; */
/*   float color_2[3]; */

  lemuria_range_t line_color;
  lemuria_range_t transform;
    
  //  int color_counter;

  int frame_counter;
  
  } oscilloscope_data;

static void * init_oscilloscope(lemuria_engine_t * e)
  {
  oscilloscope_data * data;

#ifdef DEBUG
  fprintf(stderr, "init_oscilloscope...");
#endif

  data = calloc(1, sizeof(oscilloscope_data));

  
  data->line_start_index = lemuria_random_int(e, 0, num_line_colors-1);
  data->line_end_index = lemuria_random_int(e, 0, num_line_colors-1);
  
  lemuria_range_init(e, &data->line_color, 4, 25, 75);
#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  return data;
  }

// #define MIN_COLOR 0.1

static void draw_oscilloscope(lemuria_engine_t * e, void * user_data)
  {
  int i;
  float line_color[4];

  oscilloscope_data * data = (oscilloscope_data*)(user_data);

  if(e->beat_detected)
    {
    if(lemuria_range_done(&data->line_color))
      {
      /* Change line color */

      if(lemuria_decide(e, 0.1))
        {
        data->line_start_index = data->line_end_index;
        data->line_end_index = lemuria_random_int(e, 0, num_line_colors-1);
        lemuria_range_init(e, &data->line_color, 4, 25, 75);
        }
      }
    }

  lemuria_range_update(&data->line_color);
   
  //  fprintf(stderr, "draw_oscilloscope\n");
  
  glEnable(GL_LINE_SMOOTH);

  if(!(data->frame_counter % 3))
    {
    /* Create the vertex arrays */
    
    for(i = 0; i < 512; i++)
      {
      data->line_1_x[i] = (float)(i)/256.0 - 1.0;
      data->line_1_y[i] =
        (float)(e->time_buffer_read[0][i])/(32768.0*2.0)+0.5;
      
      data->line_2_x[i] = (float)(i)/256.0 - 1.0; 
      data->line_2_y[i] =
      (float)(e->time_buffer_read[1][i])/(32768.0*2.0)-0.5;
      }
    lemuria_range_get(&data->line_color,
                          line_colors[data->line_start_index][0],
                          line_colors[data->line_end_index][0],
                          line_color);
    
    glColor4f(line_color[0], line_color[1], line_color[2], line_color[3]);
    glLineWidth(2);
    
    glBegin(GL_LINE_STRIP);
    
    for(i = 0; i < 512; i++)
      {
      glVertex2f(data->line_1_x[i], data->line_1_y[i]);
      }  
    
    glEnd();
    
    lemuria_range_get(&data->line_color,
                          line_colors[data->line_start_index][1],
                          line_colors[data->line_end_index][1],
                          line_color);
    
    
    glColor4f(line_color[0], line_color[1], line_color[2], line_color[3]);
    
    glBegin(GL_LINE_STRIP);
    
    for(i = 0; i < 512; i++)
      { 
      glVertex2f(data->line_2_x[i], data->line_2_y[i]);
      }
    
    glEnd();

    }

  data->frame_counter++;
  }

static void delete_oscilloscope(void * e)
  {
  oscilloscope_data * data = (oscilloscope_data *)(e);
  free(data);
  }

effect_plugin_t oscilloscope_effect =
  {
    .init =    init_oscilloscope,
    .draw =    draw_oscilloscope,
    .cleanup = delete_oscilloscope
  };
