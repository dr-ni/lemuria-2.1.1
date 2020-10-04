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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <lemuria_private.h>
#include <utils.h>
#include <particle.h>

#define EPSILON 1e-6

lemuria_particle_system_t *
lemuria_create_particles(lemuria_engine_t * e,
                         int num_particles,
                         int texture_size,
                         uint8_t * texture,
                         float min_size,
                         float max_size)
  {
  int i;
  
  /* Create object */
  
  lemuria_particle_system_t * ret =
    calloc(1, sizeof(lemuria_particle_system_t));

  /* Count the textures */
  
  /* Create textures */

  glGenTextures(1, &(ret->texture));

  glBindTexture(GL_TEXTURE_2D, ret->texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, 4, texture_size, texture_size,
               0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
  
  /* Create particles */

  ret->num_particles = num_particles;

  ret->particles = calloc(num_particles, sizeof(lemuria_particle_t));
  
  for(i = 0; i < num_particles; i++)
    {
    ret->particles[i].x = 0.0;
    ret->particles[i].y = 0.0;
    ret->particles[i].z = 0.0;

    ret->particles[i].angle = 0.0; // Rotation angle
    
    ret->particles[i].size = lemuria_random(e, min_size, max_size);
    
    }
  return ret;
  }


void lemuria_draw_particles(lemuria_particle_system_t * p)
  {
  int i;
  float delta;
  lemuria_particle_t * particle;
  float cos_angle, sin_angle;

  float particle_vector[3];  // Vector from the camera to the particle center
  float particle_vector_abs; // Length

  float coords_1[4][3];
  float coords_2[4][3];
    
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glColor4f(1.0, 1.0, 1.0, 1.0);

  //  glEnable(GL_ALPHA_TEST);
  //  glAlphaFunc(
  
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  glBindTexture(GL_TEXTURE_2D, p->texture);
  //  fprintf(stderr, "Texture bound, %d\n", glGetError());
  for(i = 0; i < p->num_particles; i++)
    {
    
    particle = &(p->particles[i]);
    delta = particle->size * 0.5;
    cos_angle = cos(particle->angle);
    sin_angle = sin(particle->angle);

    particle_vector[0] = particle->x - CAMERA_X;
    particle_vector[1] = particle->y - CAMERA_Y;
    particle_vector[2] = particle->z - CAMERA_Z;

    particle_vector_abs = sqrt(particle_vector[0]*particle_vector[0]+
                               particle_vector[1]*particle_vector[1]+
                               particle_vector[2]*particle_vector[2]);

    
    
    // Untransformed coordinates

    coords_1[0][0] = delta * (- cos_angle - sin_angle);
    coords_1[0][1] = delta * (  sin_angle - cos_angle);
    coords_1[0][2] = 0.0;

    coords_1[1][0] = delta * (  cos_angle - sin_angle);
    coords_1[1][1] = delta * (- sin_angle - cos_angle);
    coords_1[1][2] = 0.0;

    coords_1[2][0] = delta * (  cos_angle + sin_angle);
    coords_1[2][1] = delta * (- sin_angle + cos_angle);
    coords_1[2][2] = 0.0;

    coords_1[3][0] = delta * (- cos_angle + sin_angle);
    coords_1[3][1] = delta * (  sin_angle + cos_angle);
    coords_1[3][2] = 0.0;

    // Transform them
    
    coords_2[0][0] = particle->x + coords_1[0][0];
    coords_2[0][1] = particle->y + coords_1[0][1];
    coords_2[0][2] = particle->z + coords_1[0][2];

    coords_2[1][0] = particle->x + coords_1[1][0];
    coords_2[1][1] = particle->y + coords_1[1][1];
    coords_2[1][2] = particle->z + coords_1[1][2];

    coords_2[2][0] = particle->x + coords_1[2][0];
    coords_2[2][1] = particle->y + coords_1[2][1];
    coords_2[2][2] = particle->z + coords_1[2][2];

    coords_2[3][0] = particle->x + coords_1[3][0];
    coords_2[3][1] = particle->y + coords_1[3][1];
    coords_2[3][2] = particle->z + coords_1[3][2];
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(0.0, 1.0);
    glVertex3f(coords_2[0][0], coords_2[0][1], coords_2[0][2]);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(coords_2[1][0], coords_2[1][1], coords_2[1][2]);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(coords_2[2][0], coords_2[2][1], coords_2[2][2]);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(coords_2[3][0], coords_2[3][1], coords_2[3][2]);
    glEnd();
    }

  glEnable(GL_DEPTH_TEST);
  
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  
  }

void lemuria_destroy_particles(lemuria_particle_system_t *p)
  {
  glDeleteTextures(1, &(p->texture));

  free(p->particles);
  free(p);
  }
