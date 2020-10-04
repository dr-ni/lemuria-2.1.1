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

#include <inttypes.h>

#include <stdlib.h>

#include <stdio.h>

#include <lemuria_private.h>
#include <effect.h>
#include <utils.h>

#include "meshes/texture_grids.incl"

#define AUTO_TEXTURE
#define TEXTURE_TRANSFORM 10

#define TRANSFORM_MODE_SHIFT  0
#define TRANSFORM_MODE_ROTATE 1
#define TRANSFORM_MODE_ZOOM   2

#define GRID_MODE_UNIFORM 0
#define GRID_MODE_SINE_X  1
#define GRID_MODE_SINE_Y  2

#define GRID_MODE_ZOOM_1  3
#define GRID_MODE_ZOOM_2  4

#define GRID_MODE_MAX     4

#define NUM_TEXTURES 2

static int color_matrices[][4] =
  {
    { 1, 2, 0, 3 },
    { 0, 1, 2, 3 }
  };

typedef struct
  {
  float * start_transform;
  float * end_transform;
  
  int start_mode;
  int end_mode;
  
  int background_index;
  lemuria_range_t transform;

  float darken[4];

  int color_matrix_index;
  
  } transform_data;

struct lemuria_texture_s
  {
  int refcounts[NUM_TEXTURES];
  unsigned int textures[NUM_TEXTURES];
  transform_data texture_transform;
  char * image_buffer;
  int color_matrix_index;

  int grid_mode;
  float (*grid)[GRID_SIZE][GRID_SIZE][2];
  };

static void set_grid(lemuria_texture_t * t)
  {
  switch(t->grid_mode)
    {
    case GRID_MODE_UNIFORM:
      //      fprintf(stderr, "GRID_MODE_UNIFORM\n");
      t->grid = NULL;
      break;
    case GRID_MODE_SINE_X:
      t->grid = &texture_grid_sine_x;
      //      fprintf(stderr, "GRID_MODE_SINE_X\n");
      break;
    case GRID_MODE_SINE_Y:
      t->grid = &texture_grid_sine_y;
      //      fprintf(stderr, "GRID_MODE_SINE_Y\n");
      break;
    case GRID_MODE_ZOOM_1:
      t->grid = &texture_grid_zoom_1;
      //      fprintf(stderr, "GRID_MODE_ZOOM_1\n");
      break;
    case GRID_MODE_ZOOM_2:
      t->grid = &texture_grid_zoom_2;
      //      fprintf(stderr, "GRID_MODE_ZOOM_2\n");
      break;
    }
  }

/*
 *  The first 4 entries must not be changed since they make
 *  patterns, which are suited for silent periods
 */

#define SIN_ROTATE 0.034899
#define COS_ROTATE 0.999381

static float shift_transforms[][7] =
  {
    /* Translation only */
    { 1.0, 0.0, 0.01,
      0.0, 1.0, 0.01, 0.0 },
    { 1.0, 0.0, -0.01,
      0.0, 1.0, 0.01, 0.0 },
    { 1.0, 0.0, 0.01,
      0.0, 1.0, -0.01, 0.0 },
    { 1.0, 0.0, -0.01,
      0.0, 1.0, -0.01, 0.0 },
  };

static float rotation_transforms[][7] =
  {
    /* Rotation and Zoom in */
    {  0.99 * COS_ROTATE, 0.99 * SIN_ROTATE, 0.0,
      -0.99 * SIN_ROTATE, 0.99 * COS_ROTATE, 0.0, 0.0 },
    {  0.99 * COS_ROTATE, -0.99 * SIN_ROTATE, 0.0,
       0.99 * SIN_ROTATE, 0.99 * COS_ROTATE, 0.0, 0.0 },
    /* Rotations */
    {  COS_ROTATE,  SIN_ROTATE, 0.0,
      -SIN_ROTATE,  COS_ROTATE, 0.0, 1.0 },
    {  COS_ROTATE, -SIN_ROTATE, 0.0,
       SIN_ROTATE,  COS_ROTATE, 0.0, 1.0 },
  };

static float zoom_transforms[][7] =
  {
    /* Rotation and Zoom Out */
    {  1.01 * COS_ROTATE, 1.01 * SIN_ROTATE, 0.0,
      -1.01 * SIN_ROTATE, 1.01 * COS_ROTATE, 0.0, 1.0 },
    /* Zoom Out */
    {  1.01, 0.0, 0.0,
       0.0, 1.01, 0.0, 1.0 },
  };

static float background_colors[][3] =
  {
    { 0.0, 0.0, 0.0 },
    { 1.0, 1.0, 1.0 },
    { 0.0, 0.0, 0.5 },
  };

// #define DRAW_FRAME_INDEX(id) ((id > 5))

// static int num_transforms = sizeof(textures_y)/sizeof(textures_y[0]);

#ifdef AUTO_TEXTURE
static int num_zoom_transforms =
sizeof(zoom_transforms)/sizeof(zoom_transforms[0]);

static int num_shift_transforms =
sizeof(shift_transforms)/sizeof(shift_transforms[0]);

static int num_rotation_transforms =
sizeof(rotation_transforms)/sizeof(rotation_transforms[0]);

#endif

void lemuria_texture_create(lemuria_engine_t * e)
  {
  int i;
  lemuria_texture_t * ret;
  
  ret = calloc(1, sizeof(*ret));
  
  // Create the texture
  
  glGenTextures(NUM_TEXTURES, ret->textures);
  
  ret->image_buffer = malloc(TEXTURE_SIZE * TEXTURE_SIZE * 4);

  for(i = 0; i < NUM_TEXTURES; i++)
    {
    glBindTexture(GL_TEXTURE_2D, ret->textures[i]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXTURE_SIZE, TEXTURE_SIZE,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, ret->image_buffer);
    }
  lemuria_range_init(e, &ret->texture_transform.transform,  6, 100, 200);

#ifdef AUTO_TEXTURE
  ret->texture_transform.start_mode = lemuria_random_int(e, 0, 2);
  ret->texture_transform.end_mode   = lemuria_random_int(e, 0, 1);
  if(ret->texture_transform.end_mode == ret->texture_transform.start_mode)
    ret->texture_transform.end_mode++;

  switch(ret->texture_transform.start_mode)
    {
    case TRANSFORM_MODE_SHIFT:
      ret->texture_transform.start_transform =
        shift_transforms[lemuria_random_int(e, 0, num_shift_transforms-1)];
      break;
    case TRANSFORM_MODE_ROTATE:
      ret->texture_transform.start_transform =
        rotation_transforms[lemuria_random_int(e, 0, num_rotation_transforms-1)];
      break;
    case TRANSFORM_MODE_ZOOM:
      ret->texture_transform.start_transform =
        zoom_transforms[lemuria_random_int(e, 0, num_zoom_transforms-1)];
      break;
    }
  
  switch(ret->texture_transform.end_mode)
    {
    case TRANSFORM_MODE_SHIFT:
      ret->texture_transform.end_transform =
        shift_transforms[lemuria_random_int(e, 0, num_shift_transforms-1)];
      break;
    case TRANSFORM_MODE_ROTATE:
      ret->texture_transform.end_transform =
        rotation_transforms[lemuria_random_int(e, 0, num_rotation_transforms-1)];
      break;
    case TRANSFORM_MODE_ZOOM:
      ret->texture_transform.end_transform =
        zoom_transforms[lemuria_random_int(e, 0, num_zoom_transforms-1)];
      break;
    }

  ret->grid_mode = lemuria_random_int(e, 0, GRID_MODE_MAX);
  
  set_grid(ret);
  
#else
  ret->texture_transform.start_transform = TEXTURE_TRANSFORM;
  ret->texture_transform.end_transform   = TEXTURE_TRANSFORM;
#endif
  ret->texture_transform.background_index =
    lemuria_random_int(e, 0, (sizeof(background_colors)/
                           sizeof(background_colors[0]))-1);
  

  ret->texture_transform.darken[0] = 1.0;
  ret->texture_transform.darken[1] = 1.0;
  ret->texture_transform.darken[2] = 1.0;
  ret->texture_transform.darken[3] = 1.0;
  
  e->lemuria_texture = ret;
  }

void lemuria_texture_destroy(lemuria_engine_t * e)
  {
  glDeleteTextures(NUM_TEXTURES, e->lemuria_texture->textures);
  free(e->lemuria_texture->image_buffer);
  free(e->lemuria_texture);
  e->lemuria_texture = (void*)0;
  }

void lemuria_texture_bind(lemuria_engine_t * e, int num)
  {
  glBindTexture(GL_TEXTURE_2D, e->lemuria_texture->textures[num]);
  //  if(glGetError())
  //    fprintf(stderr, "Oops %d\n", num);
  }

void lemuria_texture_ref(lemuria_engine_t * e, int num)
  {
  if(num)
    {
    e->lemuria_texture->refcounts[0]++;
    if((num == 1) && !e->lemuria_texture->refcounts[1])
      e->lemuria_texture->color_matrix_index =
        lemuria_random(e, 0, sizeof(color_matrices)/sizeof(color_matrices[0])-1);
    }
  e->lemuria_texture->refcounts[num]++;
  }

void lemuria_texture_unref(lemuria_engine_t * e, int num)
  {
  //  fprintf(stderr, "lemuria_texture_unref %d\n", num);
  if(num)
    e->lemuria_texture->refcounts[0]--;
  e->lemuria_texture->refcounts[num]--;
  }

static void convert_image(lemuria_texture_t * t)
  {
  int i;
  char pixel[4];
  char * pos = t->image_buffer;
  for(i = 0; i < TEXTURE_SIZE*TEXTURE_SIZE; i++)
    {
    pixel[0] = pos[color_matrices[t->color_matrix_index][0]];
    pixel[1] = pos[color_matrices[t->color_matrix_index][1]];
    pixel[2] = pos[color_matrices[t->color_matrix_index][2]];
    pixel[3] = pos[color_matrices[t->color_matrix_index][3]];
    pos[0] = 0xff - pixel[0];
    pos[1] = 0xff - pixel[1];
    pos[2] = 0xff - pixel[2];
    pos[3] = pixel[3];
    pos += 4;
    }
  }

static void draw_grid(lemuria_texture_t * t)
  {
  int i, j;
  float x, y1, y2;

  y1 = -1.0;
  
  for(i = 1; i < GRID_SIZE; i++)
    {
    y2 = -1.0 + 2.0 * i / (float)(GRID_SIZE - 1);
    glBegin(GL_QUAD_STRIP);
    for(j = 0; j < GRID_SIZE; j++)
      {
      x = -1.0 + 2.0 * j / (float)(GRID_SIZE - 1);
      glTexCoord2f((*t->grid)[i-1][j][0], (*t->grid)[i-1][j][1]);
      glVertex2f(x, y1);

      glTexCoord2f((*t->grid)[i][j][0], (*t->grid)[i][j][1]);
      glVertex2f(x, y2);
      }
    glEnd();
    y1 = y2;
    }
  }

static void draw_nogrid(lemuria_texture_t * t)
  {
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0); glVertex2f(-1.0, -1.0);
  glTexCoord2f(1.0, 0.0); glVertex2f( 1.0, -1.0);
  glTexCoord2f(1.0, 1.0); glVertex2f( 1.0,  1.0);
  glTexCoord2f(0.0, 1.0); glVertex2f(-1.0,  1.0);
  
  glEnd();
  }

void lemuria_texture_update(lemuria_engine_t * e)
  {
  int random_number;
  float tmp_matrix[6];
  float matrix_2D[6];
  
  float matrix_3D[16];

  transform_data * d = &(e->lemuria_texture->texture_transform);

  if(!e->lemuria_texture->refcounts[0])
    return;
  
  /* Update transformation */

#ifdef AUTO_TEXTURE      
  if(e->quiet)
    {
    d->end_mode = TRANSFORM_MODE_SHIFT;
    d->end_transform =
      shift_transforms[lemuria_random_int(e, 0, num_shift_transforms-1)];
    lemuria_range_init(e, &d->transform, 6, 100, 200);
    }
  else
#endif
    if(e->beat_detected)
    {
    if(lemuria_decide(e, 0.20))
      {
      /* Change transformation */
#ifdef AUTO_TEXTURE

      random_number = lemuria_random_int(e, 0, GRID_MODE_MAX-1);
      if(random_number >= e->lemuria_texture->grid_mode)
        random_number++;
      
      e->lemuria_texture->grid_mode = random_number;
      set_grid(e->lemuria_texture);
      
      d->start_transform = d->end_transform;
      d->start_mode = d->end_mode;

      d->end_mode = lemuria_random_int(e, 0, 1);
      if(d->end_mode >= d->start_mode)
        d->end_mode++;

      switch(d->end_mode)
        {
        case TRANSFORM_MODE_SHIFT:
          d->end_transform =
            shift_transforms[lemuria_random_int(e, 0, num_shift_transforms-1)];
          break;
        case TRANSFORM_MODE_ROTATE:
          d->end_transform =
            rotation_transforms[lemuria_random_int(e, 0, num_rotation_transforms-1)];
          break;
        case TRANSFORM_MODE_ZOOM:
          d->end_transform =
        zoom_transforms[lemuria_random_int(e, 0, num_zoom_transforms-1)];
          break;
        }
      
      lemuria_range_init(e, &d->transform, 6, 100, 200);
#endif
      
      //      d->darken = lemuria_random(0.95, 1.0);
      //      d->darken = 0.99;

      d->background_index =
          lemuria_random_int(e, 0, (sizeof(background_colors)/
                                 sizeof(background_colors[0]))-1);
      

      
      }
    if(lemuria_decide(e, 0.5))
      {
      random_number = lemuria_random_int(e, 0, 2);
      d->darken[0] = (random_number == 0) ? 0.95: 1.0;
      d->darken[1] = (random_number == 1) ? 0.95: 1.0;
      d->darken[2] = (random_number == 2) ? 0.95: 1.0;
      }
    else
      {
      d->darken[0] = 1.0;
      d->darken[1] = 1.0;
      d->darken[2] = 1.0;
      }
    }

  lemuria_range_update(&d->transform);
 
  lemuria_range_get_cos(&d->transform,
                        d->start_transform,
                        d->end_transform,
                        tmp_matrix);

  /* Shift Coordinate System */

  matrix_2D[0] = tmp_matrix[0];
  matrix_2D[1] = tmp_matrix[1];
  matrix_2D[2] = tmp_matrix[2] + 0.5 - 0.5 * (tmp_matrix[0] + tmp_matrix[1]);
  matrix_2D[3] = tmp_matrix[3];
  matrix_2D[4] = tmp_matrix[4];
  matrix_2D[5] = tmp_matrix[5] + 0.5 - 0.5 * (tmp_matrix[3] + tmp_matrix[4]);

  /* Set up the Opengl stuff */
  
  glViewport(0,
             0, TEXTURE_SIZE, TEXTURE_SIZE);
  glDisable(GL_DEPTH_TEST);                 // Disable Depth Buffer
  glShadeModel(GL_FLAT);

  lemuria_set_perspective(e, 0, 0.0);

  /* Initialize the matrix */

  matrix_3D[0] = matrix_2D[0];
  matrix_3D[1] = matrix_2D[3];
  matrix_3D[2] = 0.0;
  matrix_3D[3] = 0.0;

  matrix_3D[4] = matrix_2D[1];
  matrix_3D[5] = matrix_2D[4];
  matrix_3D[6] = 0.0;
  matrix_3D[7] = 0.0;

  matrix_3D[8] = 0.0;
  matrix_3D[9] = 0.0;
  matrix_3D[10] = 1.0;
  matrix_3D[11] = 0.0;

  matrix_3D[12] = matrix_2D[2];
  matrix_3D[13] = matrix_2D[5];
  matrix_3D[14] = 0.0;
  matrix_3D[15] = 1.0;

  /* Put the last texture into the pixel buffer and transform it */

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glColor4fv(d->darken);
    
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, e->lemuria_texture->textures[0]);

  glMatrixMode(GL_TEXTURE);
  glPushMatrix();

  glLoadMatrixf(matrix_3D);

  if(e->beat_detected && lemuria_decide(e, 0.01))
    {
    /* Nothing */
    //    fprintf(stderr, "Don't draw\n");
    }
  else if(e->lemuria_texture->grid)
    draw_grid(e->lemuria_texture);
  else
    draw_nogrid(e->lemuria_texture);
  
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  
  glDisable(GL_TEXTURE_2D);

  /* Draw Frame if necessary */

  if((!lemuria_range_done(&d->transform) && (d->start_transform[6] != 0.0)) ||
     (d->end_transform[6] != 0.0))
    {
    glLineWidth(5.0);
    glColor3fv(background_colors[d->background_index]);
    glBegin(GL_LINE_STRIP);

    glVertex2f(-1.0, -1.0);
    glVertex2f( 1.0, -1.0);
    glVertex2f( 1.0,  1.0);
    glVertex2f(-1.0,  1.0);
    glVertex2f(-1.0, -1.0);

    glEnd();
    }
  
  /* Let the effect plugin draw the new stuff */
  
  e->texture.effect->draw(e, e->texture.data);

  /* Copy the pixel buffer into the texture */
  
  
  glBindTexture(GL_TEXTURE_2D, e->lemuria_texture->textures[0]);
  glFlush();
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, TEXTURE_SIZE,
                      TEXTURE_SIZE);

  /* Get other image */

  if(e->lemuria_texture->refcounts[1])
    {
    glReadPixels(0, 0, TEXTURE_SIZE,
                 TEXTURE_SIZE, GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 e->lemuria_texture->image_buffer);
    
    convert_image(e->lemuria_texture);
    
    glBindTexture(GL_TEXTURE_2D, e->lemuria_texture->textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXTURE_SIZE, TEXTURE_SIZE,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 e->lemuria_texture->image_buffer);
    
    //  glMatrixMode(GL_COLOR);
    //  glPushMatrix();
    //  glLoadMatrixf(color_matrices[0]);
    
    
    //  glPopMatrix();
    }
  
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glEnable(GL_DEPTH_TEST);
  }
