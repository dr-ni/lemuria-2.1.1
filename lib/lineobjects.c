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
#include <object.h>
#include <effect.h>

#include "meshes/star.incl"
#include "meshes/dodekaeder.incl"

#include "gradients/lemuria_lineobjects_1.incl"

static float colors[][2][4] =
  {
    /*  Background              Foreground */
    { { 1.0, 1.0, 0.75, 1.0 }, { 1.0, 1.0, 0.0, 1.0 } },
    { { 1.0, 0.75, 0.75, 1.0 }, { 1.0, 0.0, 0.0, 1.0 } },
    { { 0.75, 1.0, 0.75, 1.0 }, { 0.0, 1.0, 0.0, 1.0 } },
    { { 0.75, 0.75, 1.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 } },
    
    { { 0.75, 1.0, 1.0, 1.0 }, { 0.0, 1.0, 1.0, 1.0 } },
    { { 1.0, 0.75, 1.0, 1.0 }, { 1.0, 0.0, 1.0, 1.0 } },
  };

static lemuria_background_data_t background =
  {
    .texture_mode = LEMURIA_TEXTURE_CLOUDS_ROTATE,
    gradient_lemuria_lineobjects_1,
    clouds_size : 1,
  };

#define NUM_COLORS (sizeof(colors)/sizeof(colors[0]))

#define NUM_OBJECTS_Z  40

#define MAX_OBJECTS_XY 10
#define MIN_OBJECTS_XY 5

#define ZYLINDER_RADIUS 10.0
#define ZYLINDER_STEPS  20

#define OBJECT_DISTANCE_Z  7.0
#define OBJECT_RADIUS_XY   5.0

#define LINE_WIDTH_MIN 0.5
#define LINE_WIDTH_MAX 3.0

#define LINE_WIDTH_BLEND 0.99

#define COLOR_BLEND_NORMAL     0.90
#define COLOR_BLEND_TRANSITION 0.50


#define SPEED       0.2

#define OBJECT_TYPES 4

#define Z_START (-OBJECT_DISTANCE_Z*(NUM_OBJECTS_Z-1))

#define TEXTURE_ADVANCE_MIN -0.01
#define TEXTURE_ADVANCE_MAX 0.01

typedef struct _lineobjects_data
  {
  union
    {
    struct
      {
      float angle;
      float delta_angle;
      } star;
    struct
      {
      int axis_index;
      float axis[3];
      float angle;
      float delta_angle;
      } cube;
    } draw_data;

  struct
    {
    float color[3];
    float line_width;
    } object_data[NUM_OBJECTS_Z];

  float xy_coords[MAX_OBJECTS_XY][2];
    
  void (*draw)(struct _lineobjects_data*, float * coords);
  void (*update)(lemuria_engine_t * e, struct _lineobjects_data*);
  
  lemuria_range_t color_range;

  int color_start;
  int color_end;

  int num_objects_xy;

  float z;

  int object_index;

  lemuria_background_t background;

  float texture_advance_start[2];
  float texture_advance_end[2];
  
  float texture_coords[2];
  lemuria_range_t texture_coords_range;

  float z_transition;
  } lineobjects_data;


#define STAR_DIAMETER 5.0

static void draw_star(lineobjects_data * d, float * coords)
  {
  int i;

  float cos_phi = cos(d->draw_data.star.angle);
  float sin_phi = sin(d->draw_data.star.angle);
  

  glBegin(GL_LINE_STRIP);
  for(i = 0; i < 11; i++)
    glVertex3f(coords[0] + cos_phi * star_coords[i][0] + sin_phi * star_coords[i][1],
               coords[1] + -sin_phi * star_coords[i][0] + cos_phi * star_coords[i][1],
               coords[2] + star_coords[i][2]);
  glEnd();
  }

static void update_star(lemuria_engine_t * e, struct _lineobjects_data * d)
  {
  if(e->beat_detected && lemuria_decide(e, 0.2))
    {
    d->draw_data.star.delta_angle = -d->draw_data.star.delta_angle;
    }
  d->draw_data.star.angle += d->draw_data.star.delta_angle;
  
  while(d->draw_data.star.angle < 0.0)
    d->draw_data.star.angle += 2.0 * M_PI;
  
  while(d->draw_data.star.angle > 2.0 * M_PI)
    d->draw_data.star.angle -= 2.0 * M_PI;
  }

static void update_cube(lemuria_engine_t * e, struct _lineobjects_data * d)
  {
  int random_number, i;
  int change_axis = 0;
  d->draw_data.cube.angle += d->draw_data.cube.delta_angle;
  
  while(d->draw_data.cube.angle < 0.0)
    {
    d->draw_data.cube.angle += 90.0;
    change_axis = 1;
    }
  
  while(d->draw_data.cube.angle > 90.0)
    {
    d->draw_data.cube.angle -= 90.0;
    change_axis = 1;
    }

  if(lemuria_decide(e, 0.2) && change_axis)
    {
    random_number = lemuria_random_int(e, 0, 1);
    if(random_number >= d->draw_data.cube.axis_index)
      random_number++;

    d->draw_data.cube.axis_index = random_number;
    
    for(i = 0; i < 3; i++)
      d->draw_data.cube.axis[i] = (i == d->draw_data.cube.axis_index) ? 1.0 : 0.0;

    if(lemuria_decide(e, 0.5))
      d->draw_data.cube.delta_angle = -d->draw_data.cube.delta_angle;
    }
  
  }

#define CUBE_HALF_SIZE 0.7

static void draw_cube(lineobjects_data * d, float * coords)
  {
  glPushMatrix();

  glTranslatef(coords[0], coords[1], coords[2]);
  glRotatef(d->draw_data.cube.angle, d->draw_data.cube.axis[0],
            d->draw_data.cube.axis[1], d->draw_data.cube.axis[2]);

  glBegin(GL_LINE_STRIP);
  glVertex3f(-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE);
  glVertex3f( CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE);
  glVertex3f( CUBE_HALF_SIZE,  CUBE_HALF_SIZE, CUBE_HALF_SIZE);
  glVertex3f(-CUBE_HALF_SIZE,  CUBE_HALF_SIZE, CUBE_HALF_SIZE);
  glVertex3f(-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE);
  glEnd();

  glBegin(GL_LINE_STRIP);
  glVertex3f(-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE);
  glVertex3f( CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE);
  glVertex3f( CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE);
  glVertex3f(-CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE);
  glVertex3f(-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE);
  glEnd();

  glBegin(GL_LINES);
  glVertex3f(-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE);
  glVertex3f(-CUBE_HALF_SIZE, -CUBE_HALF_SIZE,  CUBE_HALF_SIZE);

  glVertex3f( CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE);
  glVertex3f( CUBE_HALF_SIZE, -CUBE_HALF_SIZE,  CUBE_HALF_SIZE);

  glVertex3f( CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE);
  glVertex3f( CUBE_HALF_SIZE,  CUBE_HALF_SIZE,  CUBE_HALF_SIZE);

  glVertex3f(-CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE);
  glVertex3f(-CUBE_HALF_SIZE,  CUBE_HALF_SIZE,  CUBE_HALF_SIZE);
  glEnd();

  
  glPopMatrix();
  }

#if 0

#define CROSS_HALF_SIZE   0.3
#define CROSS_FACTOR      3.0

static void draw_cross_bar()
  {
  glBegin(GL_LINE_STRIP);
  glVertex3f(-CROSS_HALF_SIZE, -CROSS_FACTOR*CROSS_HALF_SIZE, CROSS_HALF_SIZE);
  glVertex3f( CROSS_HALF_SIZE, -CROSS_FACTOR*CROSS_HALF_SIZE, CROSS_HALF_SIZE);
  glVertex3f( CROSS_HALF_SIZE,  CROSS_FACTOR*CROSS_HALF_SIZE, CROSS_HALF_SIZE);
  glVertex3f(-CROSS_HALF_SIZE,  CROSS_FACTOR*CROSS_HALF_SIZE, CROSS_HALF_SIZE);
  glVertex3f(-CROSS_HALF_SIZE, -CROSS_FACTOR*CROSS_HALF_SIZE, CROSS_HALF_SIZE);
  glEnd();

  glBegin(GL_LINE_STRIP);
  glVertex3f(-CROSS_HALF_SIZE, -CROSS_FACTOR*CROSS_HALF_SIZE, -CROSS_HALF_SIZE);
  glVertex3f( CROSS_HALF_SIZE, -CROSS_FACTOR*CROSS_HALF_SIZE, -CROSS_HALF_SIZE);
  glVertex3f( CROSS_HALF_SIZE,  CROSS_FACTOR*CROSS_HALF_SIZE, -CROSS_HALF_SIZE);
  glVertex3f(-CROSS_HALF_SIZE,  CROSS_FACTOR*CROSS_HALF_SIZE, -CROSS_HALF_SIZE);
  glVertex3f(-CROSS_HALF_SIZE, -CROSS_FACTOR*CROSS_HALF_SIZE, -CROSS_HALF_SIZE);
  glEnd();

  glBegin(GL_LINES);
  glVertex3f(-CROSS_HALF_SIZE, -CROSS_FACTOR*CROSS_HALF_SIZE, CROSS_HALF_SIZE);
  glVertex3f(-CROSS_HALF_SIZE, -CROSS_FACTOR*CROSS_HALF_SIZE, -CROSS_HALF_SIZE);

  glVertex3f( CROSS_HALF_SIZE, -CROSS_FACTOR*CROSS_HALF_SIZE, CROSS_HALF_SIZE);
  glVertex3f( CROSS_HALF_SIZE, -CROSS_FACTOR*CROSS_HALF_SIZE, -CROSS_HALF_SIZE);

  glVertex3f( CROSS_HALF_SIZE,  CROSS_FACTOR*CROSS_HALF_SIZE, CROSS_HALF_SIZE);
  glVertex3f( CROSS_HALF_SIZE,  CROSS_FACTOR*CROSS_HALF_SIZE, -CROSS_HALF_SIZE);

  glVertex3f(-CROSS_HALF_SIZE,  CROSS_FACTOR*CROSS_HALF_SIZE, CROSS_HALF_SIZE);
  glVertex3f(-CROSS_HALF_SIZE,  CROSS_FACTOR*CROSS_HALF_SIZE, -CROSS_HALF_SIZE);
  glEnd();

  }


static void draw_cross(lineobjects_data * d, float * coords)
  {
  glPushMatrix();

  glTranslatef(coords[0], coords[1], coords[2]);
  glRotatef(d->draw_data.cube.angle, d->draw_data.cube.axis[0],
            d->draw_data.cube.axis[1], d->draw_data.cube.axis[2]);

  draw_cross_bar();

  glRotatef(90.0, 1.0, 0.0, 0.0);

  draw_cross_bar();

  glRotatef(90.0, 0.0, 0.0, 1.0);

  draw_cross_bar();

  
  glPopMatrix();
  }
#endif

#define OCTAEDER_HALF_WIDTH  0.8
#define OCTAEDER_HALF_HEIGHT 0.8

static void update_octaeder(lemuria_engine_t * e, struct _lineobjects_data * d)
  {
  int random_number, i;
  int change_axis = 0;
  d->draw_data.cube.angle += d->draw_data.cube.delta_angle;
  
  while(d->draw_data.cube.angle < 0.0)
    {
    d->draw_data.cube.angle += 180.0;
    change_axis = 1;
    }
  
  while(d->draw_data.cube.angle > 180.0)
    {
    d->draw_data.cube.angle -= 180.0;
    change_axis = 1;
    }

  if(lemuria_decide(e, 0.2) && change_axis)
    {
    random_number = lemuria_random_int(e, 0, 1);
    if(random_number >= d->draw_data.cube.axis_index)
      random_number++;

    d->draw_data.cube.axis_index = random_number;
    
    for(i = 0; i < 3; i++)
      d->draw_data.cube.axis[i] = (i == d->draw_data.cube.axis_index) ? 1.0 : 0.0;

    if(lemuria_decide(e, 0.5))
      d->draw_data.cube.delta_angle = -d->draw_data.cube.delta_angle;
    }
  
  }

static void draw_octaeder(lineobjects_data * d, float * coords)
  {
  glPushMatrix();

  glTranslatef(coords[0], coords[1], coords[2]);
  glRotatef(d->draw_data.cube.angle, d->draw_data.cube.axis[0],
            d->draw_data.cube.axis[1], d->draw_data.cube.axis[2]);

  glBegin(GL_LINE_STRIP);
  glVertex3f(-OCTAEDER_HALF_WIDTH, 0.0, -OCTAEDER_HALF_WIDTH);
  glVertex3f( OCTAEDER_HALF_WIDTH, 0.0, -OCTAEDER_HALF_WIDTH);
  glVertex3f( OCTAEDER_HALF_WIDTH, 0.0,  OCTAEDER_HALF_WIDTH);
  glVertex3f(-OCTAEDER_HALF_WIDTH, 0.0,  OCTAEDER_HALF_WIDTH);
  glVertex3f(-OCTAEDER_HALF_WIDTH, 0.0, -OCTAEDER_HALF_WIDTH);
  glEnd();
  
  glBegin(GL_LINES);

  glVertex3f(-OCTAEDER_HALF_WIDTH, 0.0, -OCTAEDER_HALF_WIDTH);
  glVertex3f(0.0, OCTAEDER_HALF_HEIGHT, 0.0);
  
  glVertex3f( OCTAEDER_HALF_WIDTH, 0.0, -OCTAEDER_HALF_WIDTH);
  glVertex3f(0.0, OCTAEDER_HALF_HEIGHT, 0.0);
  
  glVertex3f(-OCTAEDER_HALF_WIDTH, 0.0,  OCTAEDER_HALF_WIDTH);
  glVertex3f(0.0, OCTAEDER_HALF_HEIGHT, 0.0);
  
  glVertex3f( OCTAEDER_HALF_WIDTH, 0.0,  OCTAEDER_HALF_WIDTH);
  glVertex3f(0.0, OCTAEDER_HALF_HEIGHT, 0.0);

  glVertex3f(-OCTAEDER_HALF_WIDTH, 0.0, -OCTAEDER_HALF_WIDTH);
  glVertex3f(0.0, -OCTAEDER_HALF_HEIGHT, 0.0);
  
  glVertex3f( OCTAEDER_HALF_WIDTH, 0.0, -OCTAEDER_HALF_WIDTH);
  glVertex3f(0.0, -OCTAEDER_HALF_HEIGHT, 0.0);
  
  glVertex3f(-OCTAEDER_HALF_WIDTH, 0.0,  OCTAEDER_HALF_WIDTH);
  glVertex3f(0.0, -OCTAEDER_HALF_HEIGHT, 0.0);
  
  glVertex3f( OCTAEDER_HALF_WIDTH, 0.0,  OCTAEDER_HALF_WIDTH);
  glVertex3f(0.0, -OCTAEDER_HALF_HEIGHT, 0.0);
  
  glEnd();

  
  glPopMatrix();
  }

static void update_dodekaeder(lemuria_engine_t * e, struct _lineobjects_data * d)
  {
  int random_number, i;
  int change_axis = 0;
  d->draw_data.cube.angle += d->draw_data.cube.delta_angle;
  
  while(d->draw_data.cube.angle < 0.0)
    {
    d->draw_data.cube.angle += 360.0;
    change_axis = 1;
    }
  
  while(d->draw_data.cube.angle > 360.0)
    {
    d->draw_data.cube.angle -= 360.0;
    change_axis = 1;
    }

  if(lemuria_decide(e, 0.2) && change_axis)
    {
    random_number = lemuria_random_int(e, 0, 1);
    if(random_number >= d->draw_data.cube.axis_index)
      random_number++;

    d->draw_data.cube.axis_index = random_number;
    
    for(i = 0; i < 3; i++)
      d->draw_data.cube.axis[i] = (i == d->draw_data.cube.axis_index) ? 1.0 : 0.0;

    if(lemuria_decide(e, 0.5))
      d->draw_data.cube.delta_angle = -d->draw_data.cube.delta_angle;
    }
  
  }

static void draw_dodekaeder(lineobjects_data * d, float * coords)
  {
  int i;
  glPushMatrix();

  glTranslatef(coords[0], coords[1], coords[2]);
  glRotatef(d->draw_data.cube.angle, d->draw_data.cube.axis[0],
            d->draw_data.cube.axis[1], d->draw_data.cube.axis[2]);

  glBegin(GL_LINES);

  for(i = 0; i < 30; i++)
    {
    glVertex3fv(dodekaeder_vertices[dodekaeder_line_indices[i][0]]);
    glVertex3fv(dodekaeder_vertices[dodekaeder_line_indices[i][1]]);
    }
  
  glEnd();
  
  glPopMatrix();
  }

static void set_object(lemuria_engine_t * e, lineobjects_data * d)
  {
  int i;
  //  fprintf(stderr, "Set object %d\n", d->object_index);
  switch(d->object_index)
    {
    case 0:
      d->draw = draw_star;
      d->update = update_star;
      d->draw_data.star.delta_angle = 0.03;
      d->draw_data.star.angle = 0.0;
      break;
    case 1:
      d->draw = draw_cube;
      d->update = update_cube;
      d->draw_data.cube.delta_angle = 1.5;
      d->draw_data.cube.angle = 0.0;

      d->draw_data.cube.axis_index = lemuria_random_int(e, 0, 2);

      for(i = 0; i < 3; i++)
        d->draw_data.cube.axis[i] = (i == d->draw_data.cube.axis_index) ? 1.0 : 0.0;
      break;
    case 2:
      d->draw = draw_octaeder;
      d->update = update_octaeder;
      d->draw_data.cube.delta_angle = 1.5;
      d->draw_data.cube.angle = 0.0;

      d->draw_data.cube.axis_index = lemuria_random_int(e, 0, 2);

      for(i = 0; i < 3; i++)
        d->draw_data.cube.axis[i] = (i == d->draw_data.cube.axis_index) ? 1.0 : 0.0;
      break;
    case 3:
      d->draw = draw_dodekaeder;
      d->update = update_dodekaeder;
      d->draw_data.cube.delta_angle = 1.5;
      d->draw_data.cube.angle = 0.0;

      d->draw_data.cube.axis_index = lemuria_random_int(e, 0, 2);

      for(i = 0; i < 3; i++)
        d->draw_data.cube.axis[i] = (i == d->draw_data.cube.axis_index) ? 1.0 : 0.0;
      
#if 0
    case 4:
      d->draw = draw_cross;
      d->update = update_cube;
      d->draw_data.cube.delta_angle = 1.5;
      d->draw_data.cube.angle = 0.0;

      d->draw_data.cube.axis_index = lemuria_random_int(0, 2);

      for(i = 0; i < 3; i++)
        d->draw_data.cube.axis[i] = (i == d->draw_data.cube.axis_index) ? 1.0 : 0.0;
      break;
#endif

    }
  }

static void draw_background(lineobjects_data * d, float * texture_coords)
  {
  int i;
  float phi;
  float cos_phi, sin_phi;
  
  /* Draw Background */
  
  lemuria_background_set(&(d->background));

  // #define ZYLINDER_RADIUS 10.0
  // #define ZYLINDER_STEPS  20
  
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUAD_STRIP);
  for(i = 0; i < ZYLINDER_STEPS + 1; i++)
    {
    phi = (float)i / (float)(ZYLINDER_STEPS) * 2.0 * M_PI;
    sin_phi = sin(phi);
    cos_phi = cos(phi);

    glTexCoord2f(texture_coords[0],
                 (float)i / (float)(ZYLINDER_STEPS) + texture_coords[1]);
    
    glVertex3f(ZYLINDER_RADIUS * cos_phi,
               ZYLINDER_RADIUS * sin_phi,
               0.0);

    glTexCoord2f(texture_coords[0] + 1.0,
                 (float)i / (float)(ZYLINDER_STEPS) + texture_coords[1]);

    glVertex3f(ZYLINDER_RADIUS * cos_phi,
               ZYLINDER_RADIUS * sin_phi,
               Z_START);
    

    }
  glEnd();
  glDisable(GL_TEXTURE_2D);


  }

static void draw_lineobjects(lemuria_engine_t * e, void * data)
  {
  float color_blend;
  float texture_advance[2];
  int random_number;
  float coords[3];
  int i, j, z_index;
  float fg_color[4];
  float bg_color[4];
  lineobjects_data * d;
  
  d = (lineobjects_data *)data;

  
  if(e->background.mode == EFFECT_FINISH)
    e->background.mode = EFFECT_FINISHING;
  
  lemuria_background_update(&(d->background));
    
  glMatrixMode(GL_MODELVIEW);

  if(e->beat_detected)
    {
    if(lemuria_range_done(&(d->color_range)) && lemuria_decide(e, 0.2))
      {
      d->color_start = d->color_end;
      d->color_end   = lemuria_random_int(e, 0, NUM_COLORS - 1);
      lemuria_range_init(e, &(d->color_range),
                         4, 50, 100);
      }

    if(lemuria_range_done(&(d->texture_coords_range)) &&
       lemuria_decide(e, 0.2))
      {
      d->texture_advance_start[0] = d->texture_advance_end[0];
      d->texture_advance_start[1] = d->texture_advance_end[1];
      
      d->texture_advance_end[0] = lemuria_random(e,
                                                 TEXTURE_ADVANCE_MIN,
                                                 TEXTURE_ADVANCE_MAX);
      d->texture_advance_end[1] = lemuria_random(e,
                                                 TEXTURE_ADVANCE_MIN,
                                                 TEXTURE_ADVANCE_MAX);
      
      lemuria_range_init(e, &(d->texture_coords_range),
                         2, 100, 200);
      }
    
    if(lemuria_decide(e, 0.02))
      {
      random_number = lemuria_random_int(e, 0, OBJECT_TYPES-2);
      if(random_number >= d->object_index)
        random_number++;
      d->object_index = random_number;
      set_object(e, d);
      }
    }
  
  
  lemuria_range_update(&(d->color_range));
  lemuria_range_update(&(d->texture_coords_range));
  
  lemuria_range_get(&(d->color_range),
                    colors[d->color_start][0],
                    colors[d->color_end][0],
                    bg_color);

  lemuria_range_get(&(d->color_range),
                    colors[d->color_start][1],
                    colors[d->color_end][1],
                    fg_color);

  lemuria_range_get_cos(&(d->texture_coords_range),
                        d->texture_advance_start,
                        d->texture_advance_start,
                        texture_advance);
  
  d->texture_coords[0] += texture_advance[0];
  d->texture_coords[1] += texture_advance[1];
  while(d->texture_coords[0] > 1.0)
    d->texture_coords[0] -= 1.0;
  while(d->texture_coords[0] < 0.0)
    d->texture_coords[0] += 1.0;
  
  while(d->texture_coords[1] > 1.0)
    d->texture_coords[1] -= 1.0;
  while(d->texture_coords[1] < 0.0)
    d->texture_coords[1] += 1.0;
  
  /* Draw Objects */

  color_blend = (e->background.mode == EFFECT_RUNNING) ?
    COLOR_BLEND_NORMAL : COLOR_BLEND_TRANSITION;
  
  for(i = 1; i < NUM_OBJECTS_Z; i++)
    {
    /* Update other objects */
    
    d->object_data[i].line_width =
      LINE_WIDTH_BLEND * d->object_data[i-1].line_width +
      (1.0 - LINE_WIDTH_BLEND) *
      LINE_WIDTH_MIN;

    d->object_data[i].color[0] =
      color_blend * d->object_data[i-1].color[0] +
      (1.0 - color_blend) * bg_color[0];

    d->object_data[i].color[1] =
      color_blend * d->object_data[i-1].color[1] +
      (1.0 - color_blend) * bg_color[1];

    d->object_data[i].color[2] =
      color_blend * d->object_data[i-1].color[2] +
      (1.0 - color_blend) * bg_color[2];

    d->object_data[i].color[3] =
      color_blend * d->object_data[i-1].color[3] +
      (1.0 - color_blend) * bg_color[3];
    }
#if 0
  fprintf(stderr, "Line width 1: %d %f\n",
          e->beat_detected,
          d->object_data[0].line_width);
#endif
  
  if(e->beat_detected)
    {
    d->object_data[0].line_width =
      LINE_WIDTH_MIN + (0.5 + 0.5 * (float)e->loudness / 32768.0) *
      (LINE_WIDTH_MAX - LINE_WIDTH_MIN);
#if 0
    d->object_data[0].color[0] = 1.0;
    d->object_data[0].color[1] = 1.0;
    d->object_data[0].color[2] = 1.0;
    d->object_data[0].color[3] = 1.0;
#endif
    }
  else
    {
    d->object_data[0].line_width =
      LINE_WIDTH_BLEND * d->object_data[0].line_width +
      (1.0 - LINE_WIDTH_BLEND) * LINE_WIDTH_MIN;
    
    }

  d->object_data[0].color[0] =
    color_blend * d->object_data[0].color[0] +
    (1.0 - color_blend) * fg_color[0];
  
  d->object_data[0].color[1] =
    color_blend * d->object_data[0].color[1] +
    (1.0 - color_blend) * fg_color[1];

  d->object_data[0].color[2] =
    color_blend * d->object_data[0].color[2] +
    (1.0 - color_blend) * fg_color[2];
  
  d->object_data[0].color[3] =
    color_blend * d->object_data[0].color[3] +
    (1.0 - color_blend) * fg_color[3];

  d->z += SPEED;

  if(e->background.mode == EFFECT_FINISHING)
    {
    d->z_transition += 5.0 * SPEED;
    }
  else
    {
    while(d->z > OBJECT_DISTANCE_Z)
      {
      d->z -= OBJECT_DISTANCE_Z;
      }
    }
  
  //  fprintf(stderr, "Line width 2: %f\n", d->object_data[0].line_width);
  
  glClearColor(bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  lemuria_set_perspective(e, 1, 1000.0);

  glFogf(GL_FOG_START, - 0.6 * Z_START - d->z_transition);
  glFogf(GL_FOG_END,   - Z_START - d->z_transition);
  glFogfv(GL_FOG_COLOR, bg_color);
  
  glFogi(GL_FOG_MODE,   GL_LINEAR );
  glEnable(GL_FOG);
#if 0
  texture_color[0] = 1.0 - fg_color[1];
  texture_color[1] = 1.0 - fg_color[2];
  texture_color[2] = 1.0 - fg_color[0];
  texture_color[3] = 1.0;
#endif
  glColor4fv(fg_color);  
  draw_background(d, d->texture_coords);
  glDisable(GL_FOG);
  
  coords[2] = Z_START - 5.0 + d->z;
  
  z_index = NUM_OBJECTS_Z - 1;

  d->update(e, d);
  
  if(e->antialias)
    {
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    //    glDisable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    if(e->antialias > 1)
      glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    else
      glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    }

  
  for(i = 0; i < NUM_OBJECTS_Z; i++)
    {
    glColor4fv(d->object_data[z_index].color);
    glLineWidth(d->object_data[z_index].line_width);
    
    for(j = 0; j < d->num_objects_xy; j++)
      {
      coords[0] = d->xy_coords[j][0];
      coords[1] = d->xy_coords[j][1];
      d->draw(d, coords);
      }
    coords[2] += OBJECT_DISTANCE_Z;
    z_index--;
    }

  if(e->antialias)
    {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    //    glEnable(GL_DEPTH_TEST);
    }
  
  if((e->background.mode == EFFECT_FINISHING) &&
     (d->z_transition > -Z_START))
    {
    e->background.mode = EFFECT_DONE;
    }    
  
  }

static void * init_lineobjects(lemuria_engine_t * e)
  {
  float phi;
  int i;
  lineobjects_data * d;
  //  int random_number;

  d = calloc(1, sizeof(*d));

  lemuria_background_init(e,
                          &(d->background),
                          &background);
  
  d->color_start = lemuria_random_int(e, 0, NUM_COLORS - 1);
  d->color_end   = lemuria_random_int(e, 0, NUM_COLORS - 2);
  if(d->color_end >= d->color_start)
    d->color_end++;
  
  lemuria_range_init(e, &(d->color_range),
                     4, 50, 100);

  d->texture_advance_end[0] = lemuria_random(e,
                                             TEXTURE_ADVANCE_MIN,
                                             TEXTURE_ADVANCE_MAX);
  d->texture_advance_end[1] = lemuria_random(e,
                                             TEXTURE_ADVANCE_MIN,
                                             TEXTURE_ADVANCE_MAX);

  lemuria_range_init(e, &(d->texture_coords_range),
                     2, 100, 200);
    
  
  d->num_objects_xy = lemuria_random_int(e,
                                         MIN_OBJECTS_XY, MAX_OBJECTS_XY);
  //  d->num_objects_xy = MAX_OBJECTS_XY;
  
  phi = 0.0;
  
  for(i = 0; i < d->num_objects_xy; i++)
    {
    d->xy_coords[i][0] = OBJECT_RADIUS_XY * sin(phi);
    d->xy_coords[i][1] = OBJECT_RADIUS_XY * cos(phi);
    phi += 2.0 * M_PI / (float)(d->num_objects_xy);
    }

  d->object_index = lemuria_random_int(e, 0, OBJECT_TYPES-1);
  //  d->object_index = 3;
  set_object(e, d);

  d->object_data[0].line_width = LINE_WIDTH_MIN;
  
  return d;
  }

static void delete_lineobjects(void * data)
  {
  lineobjects_data * d = (lineobjects_data*)(data);
  lemuria_background_delete(&(d->background));
  free(d);
  }

effect_plugin_t lineobjects_effect =
  {
    .init =    init_lineobjects,
    .draw =    draw_lineobjects,
    .cleanup = delete_lineobjects,
  };
