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
#include <utils.h>
#include <math.h>
#include <stdio.h>

void lemuria_range_init(lemuria_engine_t * e,
                        lemuria_range_t * r,
                        int _num_values,
                        int min_steps, int max_steps)
  {
  r->count = 0;
  r->count_max = lemuria_random_int(e, min_steps, max_steps);
  r->num_values = _num_values;
  }

void lemuria_range_update(lemuria_range_t * r)
  {
  r->count++;
  }

void lemuria_range_get(lemuria_range_t * r,
                       float * start_values,
                       float * end_values,
                       float * ret)
  {
  int i;
  float start_weight;
  float end_weight;

  end_weight = (float)r->count / (float)r->count_max;

  if(end_weight > 1.0)
    end_weight = 1.0;
  
  start_weight = 1.0 - end_weight;
  
  for(i = 0; i < r->num_values; i++)
    ret[i] = start_weight * start_values[i] + end_weight * end_values[i];
  }

void lemuria_range_get_n(lemuria_range_t * r,
                         float * start_values,
                         float * end_values,
                         float * ret,
                         int n)
  {
  int i;
  float start_weight;
  float end_weight;

  end_weight = (float)r->count / (float)r->count_max;

  if(end_weight > 1.0)
    end_weight = 1.0;
  
  start_weight = 1.0 - end_weight;
  
  for(i = 0; i < n; i++)
    ret[i] = start_weight * start_values[i] + end_weight * end_values[i];
  }

void lemuria_range_get_cos(lemuria_range_t * r,
                           float * start_values, float * end_values,
                           float * ret)
  {
  int i;
  float start_weight;
  float end_weight;

  if(r->count > r->count_max)
    end_weight = 1.0;
  else
    end_weight = 0.5 *
      (1.0 - cos(M_PI * (float)r->count / (float)r->count_max));
  
  start_weight = 1.0 - end_weight;
  
  for(i = 0; i < r->num_values; i++)
    ret[i] = start_weight * start_values[i] + end_weight * end_values[i];
  }

void lemuria_range_get_cos_n(lemuria_range_t * r,
                             float * start_values, float * end_values,
                             float * ret,
                             int n)
  {
  int i;
  float start_weight;
  float end_weight;

  if(r->count > r->count_max)
    end_weight = 1.0;
  else
    end_weight = 0.5 *
      (1.0 - cos(M_PI * (float)r->count / (float)r->count_max));
  
  start_weight = 1.0 - end_weight;
  
  for(i = 0; i < n; i++)
    ret[i] = start_weight * start_values[i] + end_weight * end_values[i];
  }

int lemuria_range_done(lemuria_range_t * r)
  {
  return (r->count >= r->count_max) ? 1 : 0;     
  }
