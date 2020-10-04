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

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <lemuria_private.h>
#include <utils.h>
#include "images/clouds_small.incl"
#include "images/clouds_large.incl"

static uint8_t * create_clouds(uint8_t * gradient_data, int size)
  {
  int i;

  uint8_t * src, *dst;
    
  uint8_t * image = malloc(3*clouds_small_width*clouds_small_height);

  if(size == 0)
    src = (uint8_t*)clouds_small_data;
  else
    src = (uint8_t*)clouds_large_data;

  dst = image;
  
  i = clouds_small_width*clouds_small_height;
  
  while(i--)
    {
    dst[0] = gradient_data[(*src)*4];
    dst[1] = gradient_data[(*src)*4+1];
    dst[2] = gradient_data[(*src)*4+2];
    src++;
    dst += 3;
    }
  //  fprintf(stderr, "Create clouds, image pointer: %p\n", image);
  return image;
  }

static void update_clouds_rotate(lemuria_background_t * b)
  {
  int index;
  uint8_t * src, *dst;
  int i;
  
  /* Check if we want to reverse the direction */
  
  if(b->engine->beat_detected && lemuria_decide(b->engine, 0.1))
    {
    b->delta_palette_index = -b->delta_palette_index;
    }
  /* Update palette_index */
  
  b->palette_index += b->delta_palette_index;
  if(b->palette_index > 255)
    b->palette_index -= 256;
  if(b->palette_index < 0)
    b->palette_index += 256;

  /* Create image */

  if(b->config->clouds_size == 0)
    src = (uint8_t*)clouds_small_data;
  else
    src = (uint8_t*)clouds_large_data;

  dst = b->image;
  
  i = clouds_small_width*clouds_small_height;
  
  while(i--)
    {
    index = *src + b->palette_index;
    if(index > 255)
      index -= 256;
    
    dst[0] = b->config->clouds_gradient[index*4];
    dst[1] = b->config->clouds_gradient[index*4+1];
    dst[2] = b->config->clouds_gradient[index*4+2];
    src++;
    dst += 3;
    }
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, b->texture);
  glTexImage2D(GL_TEXTURE_2D, 0, 3,
               TEXTURE_SIZE,
               TEXTURE_SIZE,
               0, GL_RGB, GL_UNSIGNED_BYTE,
               b->image);
  glDisable(GL_TEXTURE_2D);
  }

void lemuria_background_update(lemuria_background_t * b)
  {
  switch(b->config->texture_mode)
    {
    case LEMURIA_TEXTURE_CLOUDS_ROTATE:
      update_clouds_rotate(b);
      break;
    default:
      break;
    }
  }


void lemuria_background_init(lemuria_engine_t * e,
                             lemuria_background_t * b,
                             lemuria_background_data_t * d)
  {
  uint8_t * texture_data;
  
  b->config = d;
  b->engine = e;
  switch(b->config->texture_mode)
    {
    case LEMURIA_TEXTURE_LEMURIA:
      lemuria_texture_ref(b->engine, 1);
      break;
    case LEMURIA_TEXTURE_CLOUDS:
    case LEMURIA_TEXTURE_CLOUDS_ROTATE:
      texture_data = create_clouds(b->config->clouds_gradient,
                                   b->config->clouds_size);
      glGenTextures(1, &(b->texture));
      glBindTexture(GL_TEXTURE_2D, b->texture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      glTexImage2D(GL_TEXTURE_2D, 0, 3,
                   TEXTURE_SIZE,
                   TEXTURE_SIZE,
                   0, GL_RGB, GL_UNSIGNED_BYTE,
                   texture_data);
      if(b->config->texture_mode == LEMURIA_TEXTURE_CLOUDS)
        free(texture_data);
      else
        {
        b->image = texture_data;
        b->delta_palette_index = 2;
        }
      break;
    case LEMURIA_TEXTURE_GOOM:
      lemuria_goom_ref(e);
      break;
    case LEMURIA_TEXTURE_XAOS:
      lemuria_xaos_ref(e);
      break;
    }
  b->texture_flip_y = 0;
  }

void lemuria_background_set(lemuria_background_t * b)
  {
  //  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 0.0);
  switch(b->config->texture_mode)
    {
    case LEMURIA_TEXTURE_LEMURIA:
      lemuria_texture_bind(b->engine, 1);
      break;
    case LEMURIA_TEXTURE_CLOUDS:
    case LEMURIA_TEXTURE_CLOUDS_ROTATE:
      glBindTexture(GL_TEXTURE_2D, b->texture);
      //      fprintf(stderr, "Background clouds\n");
      break;
    case LEMURIA_TEXTURE_GOOM:
      lemuria_goom_bind(b->engine);
      break;
    case LEMURIA_TEXTURE_XAOS:
      lemuria_xaos_bind(b->engine);
      break;
    }
  //  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0);
  }

void lemuria_background_delete(lemuria_background_t * b)
  {
  switch(b->config->texture_mode)
    {
    case LEMURIA_TEXTURE_LEMURIA:
      lemuria_texture_unref(b->engine, 1);
      break;
    case LEMURIA_TEXTURE_CLOUDS:
      glDeleteTextures(1, &(b->texture));
      break;
    case LEMURIA_TEXTURE_CLOUDS_ROTATE:
      glDeleteTextures(1, &(b->texture));
      free(b->image);
      break;
    case LEMURIA_TEXTURE_GOOM:
      lemuria_goom_unref(b->engine);
      break;
    case LEMURIA_TEXTURE_XAOS:
      lemuria_xaos_unref(b->engine);
      break;
    }
  
  }

#define TEXTURE_BORDER (1.0/512.0)

static void draw_band(float coords[4][3], float texture_y_1,
                      float texture_y_2, int x_steps)
  {
  int i;
  float fac, anti_fac;
  float texture_x = TEXTURE_BORDER;
  
  glBegin(GL_QUAD_STRIP);
  
  for(i = 0; i <= x_steps; i++)
    {
    fac = (float)i/(float)x_steps;
    anti_fac = 1.0 - fac;
    
    glTexCoord2f(texture_x, texture_y_1);

    glVertex3f(coords[0][0] * anti_fac + coords[1][0] * fac,
               coords[0][1] * anti_fac + coords[1][1] * fac,
               coords[0][2] * anti_fac + coords[1][2] * fac);

    glTexCoord2f(texture_x, texture_y_2);

    glVertex3f(coords[3][0] * anti_fac + coords[2][0] * fac,
               coords[3][1] * anti_fac + coords[2][1] * fac,
               coords[3][2] * anti_fac + coords[2][2] * fac);
    
    texture_x = 1.0 - texture_x;
    }
  glEnd();

#if 0
  glColor3f(0.0, 1.0, 0.0);
  glDisable(GL_TEXTURE_2D);
  
  glBegin(GL_LINE_STRIP);

  glVertex3f(coords[0][0], coords[0][1], coords[0][2]+0.01);
  glVertex3f(coords[1][0], coords[1][1], coords[1][2]+0.01);
  glVertex3f(coords[2][0], coords[2][1], coords[2][2]+0.01);
  glVertex3f(coords[3][0], coords[3][1], coords[3][2]+0.01);
  glVertex3f(coords[0][0], coords[0][1], coords[0][2]+0.01);
  
  
  glEnd();
  glEnable(GL_TEXTURE_2D);
#endif
  }

static float color_matrix[] =
  {
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0
  };

void lemuria_background_draw(lemuria_background_t * b,
                             float coords[4][3],
                             int texture_size_x,
                             int texture_size_y,
                             float * texture_offset_y,
                             float texture_advance_y,
                             int * flip_y)
  {
  int i;
  
  float texture_y_1;
  float texture_y_2;

  float band_coords[4][3];
  
  float y_fac;

  float anti_y_fac;
  
  *texture_offset_y += texture_advance_y;
  
  if(*texture_offset_y > 1.0)
    {
    *texture_offset_y -= 1.0;
    (*flip_y) = !(*flip_y);
    }
  else if(*texture_offset_y < 0.0)
    {
    *texture_offset_y += 1.0;
    (*flip_y) = !(*flip_y);
    }
  
  //  fprintf(stderr, "%f\n", *texture_offset_y);

  /* interpol_y_2 - interpol_y_1 = delta_interpol */
       
  switch(b->config->texture_mode)
    {
    case LEMURIA_TEXTURE_GOOM:
    case LEMURIA_TEXTURE_XAOS:

      /* Draw first partial band */

      if(!*flip_y)
        {
        texture_y_1 = 1.0 - *texture_offset_y - TEXTURE_BORDER;
        texture_y_2 = 1.0 - TEXTURE_BORDER;
        }
      else
        {
        texture_y_1 = *texture_offset_y + TEXTURE_BORDER;
        texture_y_2 = TEXTURE_BORDER;
        }
      
      band_coords[0][0] = coords[0][0];
      band_coords[0][1] = coords[0][1];
      band_coords[0][2] = coords[0][2];

      band_coords[1][0] = coords[1][0];
      band_coords[1][1] = coords[1][1];
      band_coords[1][2] = coords[1][2];

      y_fac = *texture_offset_y/(float)texture_size_y;
      anti_y_fac = 1.0 - y_fac;
      
      band_coords[2][0] = coords[1][0] * anti_y_fac + coords[2][0] * y_fac;
      band_coords[2][1] = coords[1][1] * anti_y_fac + coords[2][1] * y_fac;
      band_coords[2][2] = coords[1][2] * anti_y_fac + coords[2][2] * y_fac;

      band_coords[3][0] = coords[0][0] * anti_y_fac + coords[3][0] * y_fac;
      band_coords[3][1] = coords[0][1] * anti_y_fac + coords[3][1] * y_fac;
      band_coords[3][2] = coords[0][2] * anti_y_fac + coords[3][2] * y_fac;

      //      fprintf(stderr, "flip_y: %d\n", *flip_y);

      draw_band(band_coords, texture_y_1, texture_y_2, texture_size_x);

      /* Draw main part */

      for(i = 0; i < texture_size_y - 1; i++)
        {
        texture_y_1 = texture_y_2;
        texture_y_2 = 1.0 - texture_y_2;
        
        /* Old top coords -> new bottom coords */
        band_coords[0][0] = band_coords[3][0];
        band_coords[0][1] = band_coords[3][1];
        band_coords[0][2] = band_coords[3][2];
        
        band_coords[1][0] = band_coords[2][0];
        band_coords[1][1] = band_coords[2][1];
        band_coords[1][2] = band_coords[2][2];

        /* New top coords */
                
        y_fac = (*texture_offset_y + (float)(i+1))/(float)texture_size_y;
        anti_y_fac = 1.0 - y_fac;
        
        band_coords[3][0] = coords[0][0] * anti_y_fac + coords[3][0] * y_fac;
        band_coords[3][1] = coords[0][1] * anti_y_fac + coords[3][1] * y_fac;
        band_coords[3][2] = coords[0][2] * anti_y_fac + coords[3][2] * y_fac;

        band_coords[2][0] = coords[1][0] * anti_y_fac + coords[2][0] * y_fac;
        band_coords[2][1] = coords[1][1] * anti_y_fac + coords[2][1] * y_fac;
        band_coords[2][2] = coords[1][2] * anti_y_fac + coords[2][2] * y_fac;

        draw_band(band_coords, texture_y_1, texture_y_2, texture_size_x);
        }

      /* Draw final band */

      /* Old top coords -> new bottom coords */
      band_coords[0][0] = band_coords[3][0];
      band_coords[0][1] = band_coords[3][1];
      band_coords[0][2] = band_coords[3][2];
      
      band_coords[1][0] = band_coords[2][0];
      band_coords[1][1] = band_coords[2][1];
      band_coords[1][2] = band_coords[2][2];

      band_coords[3][0] = coords[3][0];
      band_coords[3][1] = coords[3][1];
      band_coords[3][2] = coords[3][2];
      
      band_coords[2][0] = coords[2][0];
      band_coords[2][1] = coords[2][1];
      band_coords[2][2] = coords[2][2];

      texture_y_1 = texture_y_2;

      if(texture_y_1 > 0.5)
        {
        texture_y_2 = 1.0 - (1.0 - *texture_offset_y);
        }
      else
        {
        texture_y_2 = (1.0 - *texture_offset_y);
        }
      draw_band(band_coords, texture_y_1, texture_y_2, texture_size_x);
      break;
    case LEMURIA_TEXTURE_CLOUDS:
    case LEMURIA_TEXTURE_CLOUDS_ROTATE:
      glBegin(GL_QUADS);
      
      glTexCoord2f(0.0, *texture_offset_y);
      glVertex3f(coords[0][0], coords[0][1], coords[0][2]);

      glTexCoord2f((float)texture_size_x, *texture_offset_y);
      glVertex3f(coords[1][0], coords[1][1], coords[1][2]);

      glTexCoord2f((float)texture_size_x,
                   (float)texture_size_y + *texture_offset_y);
      glVertex3f(coords[2][0], coords[2][1], coords[2][2]);

      glTexCoord2f(0.0,
                   (float)texture_size_y + *texture_offset_y);
      glVertex3f(coords[3][0], coords[3][1], coords[3][2]);

      glEnd();
      break;
    case LEMURIA_TEXTURE_LEMURIA:
      glMatrixMode(GL_COLOR);

      glPushMatrix();

      glMultMatrixf(color_matrix);
      if(glGetError())
        fprintf(stderr, "Oops\n");

      glBegin(GL_QUADS);
      
      glTexCoord2f(0.0, *texture_offset_y);
      glVertex3f(coords[0][0], coords[0][1], coords[0][2]);

      glTexCoord2f((float)texture_size_x, *texture_offset_y);
      glVertex3f(coords[1][0], coords[1][1], coords[1][2]);

      glTexCoord2f((float)texture_size_x,
                   (float)texture_size_y + *texture_offset_y);
      glVertex3f(coords[2][0], coords[2][1], coords[2][2]);

      glTexCoord2f(0.0,
                   (float)texture_size_y + *texture_offset_y);
      glVertex3f(coords[3][0], coords[3][1], coords[3][2]);

      glEnd();
      glPopMatrix();
      break;

    }
  }


