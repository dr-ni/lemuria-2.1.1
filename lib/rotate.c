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

/* Initialize with reasonable values */

void lemuria_rotator_init(lemuria_engine_t * e, lemuria_rotator_t * r)
  {
  r->angle[0] =      lemuria_random(e, 0.0, 360.0);
  r->angle[1] =      lemuria_random(e, 0.0, 360.0);
  r->angle[2] =      lemuria_random(e, 0.0, 360.0);
  
  lemuria_rotator_change(e, r);
  r->turn_frame = -1;
  r->turn_end = 0;
  }

void lemuria_rotator_reset(lemuria_rotator_t * r)
  {
  r->angle[0] = 0.0;
  r->angle[1] = 0.0;
  r->angle[2] = 0.0;
  
  r->turn_frame = -1;
  r->turn_end = 0;
  
  }

/* Change program */

void lemuria_rotator_change(lemuria_engine_t * e,
                            lemuria_rotator_t * r)
  {
  r->turn_frame = -1;
  r->turn_end = 0;
  
  //  fprintf(stderr, "lemuria_change\n");

  r->delta_angle[0] = lemuria_random(e, 1.0, 2.0);
  r->delta_angle[1] = lemuria_random(e, 1.0, 2.0);
  r->delta_angle[2] = lemuria_random(e, 1.0, 2.0);

  if(lemuria_decide(e, 0.5))
    r->delta_angle[0] *= -1.0;
  if(lemuria_decide(e, 0.5))
    r->delta_angle[1] *= -1.0;
  if(lemuria_decide(e, 0.5))
    r->delta_angle[2] *= -1.0;
  }

/* Update the current values */

void lemuria_rotator_update(lemuria_rotator_t * r)
  {
  if(r->turn_frame == r->turn_end)
    return;
  
  if(r->turn_frame < 0)
    {
    r->angle[0] += r->delta_angle[0];
    r->angle[1] += r->delta_angle[1];
    r->angle[2] += r->delta_angle[2];
    
    /*   fprintf(stderr, "%f\n", r->axis_theta_step); */
    
    /* Correct wrong values */
    
    if(r->angle[0] < 0.0)
      r->angle[0] += 360.0;
    if(r->angle[0] > 360.0)
      r->angle[0] -= 360.0;

    if(r->angle[1] < 0.0)
      r->angle[1] += 360.0;
    if(r->angle[1] > 360.0)
      r->angle[1] -= 360.0;

    if(r->angle[2] < 0.0)
      r->angle[2] += 360.0;
    if(r->angle[2] > 360.0)
      r->angle[2] -= 360.0;
    }
  else
    {
    r->angle[0] = r->angle_start[0] +
      (r->angle_end[0] - r->angle_start[0])*
      (float)(r->turn_frame)/(float)(r->turn_end-1);

    r->angle[1] = r->angle_start[1] +
      (r->angle_end[1] - r->angle_start[1])*
      (float)(r->turn_frame)/(float)(r->turn_end-1);
    
    r->angle[2] = r->angle_start[2] +
      (r->angle_end[2] - r->angle_start[2])*
      (float)(r->turn_frame)/(float)(r->turn_end-1);

    // Done with turning
    
    r->turn_frame++;
    
    }
  //  rotator_set_coords(r);
  
  }

/* Rotate */

void lemuria_rotate(lemuria_rotator_t * r)
  {
  glRotatef(r->angle[0], 1.0, 0.0, 0.0);
  glRotatef(r->angle[1], 0.0, 1.0, 0.0);
  glRotatef(r->angle[2], 0.0, 0.0, 1.0);
  }

static void matrixmul(float m1[3][3], float m2[3][3], float ret[3][3])
  {
  ret[0][0] = m1[0][0]*m2[0][0] +  m1[0][1]*m2[1][0] + m1[0][2]*m2[2][0];
  ret[0][1] = m1[0][0]*m2[0][1] +  m1[0][1]*m2[1][1] + m1[0][2]*m2[2][1];
  ret[0][2] = m1[0][0]*m2[0][2] +  m1[0][1]*m2[1][2] + m1[0][2]*m2[2][2];

  ret[1][0] = m1[1][0]*m2[0][0] +  m1[1][1]*m2[1][0] + m1[1][2]*m2[2][0];
  ret[1][1] = m1[1][0]*m2[0][1] +  m1[1][1]*m2[1][1] + m1[1][2]*m2[2][1];
  ret[1][2] = m1[1][0]*m2[0][2] +  m1[1][1]*m2[1][2] + m1[1][2]*m2[2][2];

  ret[2][0] = m1[2][0]*m2[0][0] +  m1[2][1]*m2[1][0] + m1[2][2]*m2[2][0];
  ret[2][1] = m1[2][0]*m2[0][1] +  m1[2][1]*m2[1][1] + m1[2][2]*m2[2][1];
  ret[2][2] = m1[2][0]*m2[0][2] +  m1[2][1]*m2[1][2] + m1[2][2]*m2[2][2];
  
  }
                      

// ( xx(1-c)+c   xy(1-c)-zs  xz(1-c)+ys   0  )
// |                                         |
// | yx(1-c)+zs  yy(1-c)+c   yz(1-c)-xs   0  |
// | xz(1-c)-ys  yz(1-c)+xs  zz(1-c)+c    0  |
// |                                         |
// (      0           0           0       1  )

void lemuria_rotate_get_matrix(lemuria_rotator_t * r, float matrix[3][3])
  {
  float matrix_x[3][3];
  float matrix_y[3][3];
  float matrix_z[3][3];
  float matrix_tmp[3][3];
  
  float c;
  float s;

  c = cos(r->angle[0] * M_PI / 180.0);
  s = sin(r->angle[0] * M_PI / 180.0);
  
  matrix_x[0][0] = 1.0;
  matrix_x[0][1] = 0.0;
  matrix_x[0][2] = 0.0;
  matrix_x[1][0] = 0.0;
  matrix_x[1][1] =  c;
  matrix_x[1][2] = -s;
  matrix_x[2][0] = 0.0;
  matrix_x[2][1] = s;
  matrix_x[2][2] = c;

  c = cos(r->angle[1] * M_PI / 180.0);
  s = sin(r->angle[1] * M_PI / 180.0);

  matrix_y[0][0] = c;
  matrix_y[0][1] = 0.0;
  matrix_y[0][2] = s;
  matrix_y[1][0] = 0.0;
  matrix_y[1][1] = 1.0;
  matrix_y[1][2] = 0.0;
  matrix_y[2][0] = -s;
  matrix_y[2][1] = 0.0;
  matrix_y[2][2] = c;

  c = cos(r->angle[2] * M_PI / 180.0);
  s = sin(r->angle[2] * M_PI / 180.0);
  
  matrix_z[0][0] = c;
  matrix_z[0][1] = -s;
  matrix_z[0][2] = 0.0;
  matrix_z[1][0] = s;
  matrix_z[1][1] = c;
  matrix_z[1][2] = 0.0;
  matrix_z[2][0] = 0.0;
  matrix_z[2][1] = 0.0;
  matrix_z[2][2] = 1.0;

  matrixmul(matrix_y, matrix_z, matrix_tmp);
  matrixmul(matrix_x, matrix_tmp, matrix);
  
  }

void lemuria_rotator_turnto(lemuria_rotator_t * r,
                            float angle_x,
                            float angle_y,
                            float angle_z,
                            int num_frames)
  {
  //  fprintf(stderr, "Turnto %f %f %f\n", angle_end, axis_theta_end, axis_phi_end);
  r->turn_frame = 0;
  r->turn_end = num_frames;

  r->angle_end[0] = angle_x;
  r->angle_end[1] = angle_y;
  r->angle_end[2] = angle_z;
  
  r->angle_start[0]   = r->angle[0];
  r->angle_start[1]   = r->angle[1];
  r->angle_start[2]   = r->angle[2];
  }

int lemuria_rotator_done(lemuria_rotator_t * r)
  {
  return (r->turn_frame == r->turn_end) ? 1 : 0;
  }

void lemuria_get_rotation_matrix(float angle, float axis[3],
                                 float matrix[3][3])
  {
  float cos_angle;
  float sin_angle;

  cos_angle = cos(angle);
  sin_angle = sin(angle);

  /*
   *   ( xx(1-c)+c      xy(1-c)-zs  xz(1-c)+ys   0  )
   *   |                                            |
   *   | yx(1-c)+zs	yy(1-c)+c   yz(1-c)-xs	 0  |
   *   | xz(1-c)-ys	yz(1-c)+xs  zz(1-c)+c	 0  |
   *   |                                             |
   *   ( 0               0           0            1  )
   */
  matrix[0][0] = axis[0]*axis[0]*(1.0-cos_angle) + cos_angle;
  matrix[0][1] = axis[0]*axis[1]*(1.0-cos_angle) - axis[2]*sin_angle;
  matrix[0][2] = axis[0]*axis[2]*(1.0-cos_angle) + axis[1]*sin_angle;

  matrix[1][0] = axis[1]*axis[0]*(1.0-cos_angle) + axis[2]*sin_angle;
  matrix[1][1] = axis[1]*axis[1]*(1.0-cos_angle) + cos_angle;
  matrix[1][2] = axis[1]*axis[2]*(1.0-cos_angle) - axis[0]*sin_angle;

  matrix[2][0] = axis[2]*axis[0]*(1.0-cos_angle) - axis[1]*sin_angle;
  matrix[2][1] = axis[2]*axis[1]*(1.0-cos_angle) + axis[0]*sin_angle;
  matrix[2][2] = axis[2]*axis[2]*(1.0-cos_angle) + cos_angle;
  
  }

