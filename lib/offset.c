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
#include <utils.h>

#include <string.h>

#include <stdio.h>
// #include <stdlib.h>
#include <math.h>

void lemuria_offset_init(lemuria_engine_t * e, lemuria_offset_t* off)
  {
  off->axis_start[0] = 0.0;
  off->axis_start[1] = 0.0;

  off->axis_end[0] = 0.0;
  off->axis_end[1] = 0.0;
  lemuria_range_init(e, &(off->axis_range), 2, 50, 100);
  lemuria_rotator_init(e, &(off->rotator));

  off->phi = 0.0;
  off->delta_phi = 0.01;
  off->delta_phi_fac = 0.0;
  }

void lemuria_offset_kick(lemuria_offset_t* off)
  {
  if(off->delta_phi_fac < 1.0)
    {
    //    fprintf(stderr, "Offset kick\n");
    off->delta_phi_fac += 2.0;
    }
  }

void lemuria_offset_change(lemuria_engine_t * e,
                           lemuria_offset_t* off)
  {
  if(lemuria_decide(e, 0.5) && lemuria_range_done(&(off->axis_range)))
    {
    off->axis_start[0] = off->axis_end[0];
    off->axis_start[1] = off->axis_end[1];

    off->axis_end[0] = lemuria_random(e, 0.3, 0.5);
    off->axis_end[1] = lemuria_random(e, 0.3, 0.5);
    lemuria_range_init(e, &(off->axis_range), 2, 50, 100);
    }
  else
    {
    lemuria_rotator_change(e, &(off->rotator));
    }
  }

void lemuria_offset_reset(lemuria_engine_t * e, lemuria_offset_t* off)
  {
  float axis[2];
  lemuria_range_get(&(off->axis_range), off->axis_start,
                    off->axis_end, axis);
  off->axis_start[0] = axis[0];
  off->axis_start[1] = axis[1];

  off->axis_end[0] = 0.0;
  off->axis_end[1] = 0.0;

  lemuria_range_init(e, &(off->axis_range), 2, 50, 100);
  
  }

void lemuria_offset_update(lemuria_offset_t* off)
  {
  float tmp_offset[3];
  float axis[2];
  
  float matrix[3][3];

  lemuria_range_update(&(off->axis_range));
  lemuria_rotator_update(&(off->rotator));
  
  off->phi += off->delta_phi * (1.0 + off->delta_phi_fac);
  if(off->phi > 2.0 * M_PI)
    off->phi -= 2.0 * M_PI;
  
  off->delta_phi_fac *= 0.99;
  //  fprintf(stderr, "Delta phi fac: %f\n", off->delta_phi_fac);
  lemuria_range_get(&(off->axis_range), off->axis_start,
                    off->axis_end, axis);
  
  tmp_offset[0] = axis[0] * sin(off->phi);
  tmp_offset[1] = axis[1] * cos(off->phi);
  tmp_offset[2] = 0.0;
  
  lemuria_rotate_get_matrix(&(off->rotator), matrix);

  off->offset[0] = matrix[0][0]*tmp_offset[0]+matrix[0][1]*tmp_offset[1]+matrix[0][2]*tmp_offset[2];
  off->offset[1] = matrix[1][0]*tmp_offset[0]+matrix[1][1]*tmp_offset[1]+matrix[1][2]*tmp_offset[2];
  off->offset[2] = matrix[2][0]*tmp_offset[0]+matrix[2][1]*tmp_offset[1]+matrix[2][2]*tmp_offset[2];
  
  }

void lemuria_offset_translate(lemuria_offset_t* off)
  {
  glTranslatef(off->offset[0], off->offset[1], off->offset[2]);
  }


float * lemuria_offset_get(lemuria_offset_t* off)
  {
  return off->offset;
  }


