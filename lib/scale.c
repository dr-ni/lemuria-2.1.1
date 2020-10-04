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

void lemuria_scale_init(lemuria_engine_t * e,
                        lemuria_scale_t * s,
                        float _min_scale, float _max_scale)
  {
  s->scale_min = _min_scale;
  s->scale_max = _max_scale;

  s->scale_start = lemuria_random(e, s->scale_min, s->scale_max);
  s->scale_end   = lemuria_random(e, s->scale_min, s->scale_max);

  lemuria_range_init(e, &(s->scale_range), 1, 50, 150);
  
  }

void lemuria_scale_update(lemuria_scale_t * s)
  {
  lemuria_range_update(&(s->scale_range));
  }

/* Change program */

void lemuria_scale_change(lemuria_engine_t * e,
                          lemuria_scale_t * s)
  {
  s->scale_start = lemuria_scale_get(s);
  s->scale_end   = lemuria_random(e, s->scale_min, s->scale_max);
  
  lemuria_range_init(e, &(s->scale_range), 1, 50, 150);
  }

/* Get the current scale factor */

float lemuria_scale_get(lemuria_scale_t * s)
  {
  float ret;
  lemuria_range_get_cos(&(s->scale_range), &(s->scale_start),
                        &(s->scale_end),
                        &ret);
  return ret;
  }
