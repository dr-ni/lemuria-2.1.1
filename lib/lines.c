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
#include <effect.h>
#include <utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct 
  {
  float z_start;
  float speed;
  float delta_z;
  
  lemuria_range_t line_color;

  int line_color_start_index;
  int line_color_end_index;

  unsigned int lines_list;

  } lines_data ;

#define LINE_MIN_RED     0.2
#define LINE_MIN_GREEN   0.2
#define LINE_MIN_BLUE    0.2

#define LINE_MAX_RED     1.0
#define LINE_MAX_GREEN   1.0
#define LINE_MAX_BLUE    1.0

static float line_colors[][4] =
  {
    { 0.3, 0.3, 1.0, 1.0 },
    { LINE_MIN_RED, LINE_MAX_GREEN, LINE_MIN_BLUE, 1.0 },
    { LINE_MIN_RED, LINE_MAX_GREEN, LINE_MAX_BLUE, 1.0 },
    { LINE_MAX_RED, LINE_MIN_GREEN, LINE_MIN_BLUE, 1.0 },
    { LINE_MAX_RED, LINE_MAX_GREEN, LINE_MIN_BLUE, 1.0 },
  };

static int num_line_colors = sizeof(line_colors)/sizeof(line_colors[0]);

static void delete_lines(void * data)
  {
  lines_data * d = (lines_data*)data;
  glDeleteLists(d->lines_list, 1);
  free(d);
  }

#define DELTA_Z_PLANES 10.0
#define DELTA_X_PLANES 10.0
#define DELTA_Y_PLANES 10.0

#define WORLD_HALF_WIDTH_PLANES 50

#define Z_MIN_PLANES -600.0
#define Z_MAX_PLANES 0.0

#define Z_STEPS_PLANES ((int)((Z_MAX_PLANES - Z_MIN_PLANES)/DELTA_Z_PLANES+0.5))

#define SPEED_PLANES 0.8

static void draw_lines_planes()
  {
  int i;
  float z, x;

  float x_min = - WORLD_HALF_WIDTH_PLANES  * DELTA_X_PLANES +
    0.5 * DELTA_X_PLANES;

/*   glFogf(GL_FOG_START, 60.0); */
/*   glFogf(GL_FOG_END, -Z_MIN_PLANES + DELTA_Z_PLANES); */

  glFogf(GL_FOG_DENSITY, 1.0/100.0);

  glFogi(GL_FOG_MODE, GL_EXP );

  glEnable(GL_FOG);
  
  glBegin(GL_LINES);

  /* X-Direction */

  z = Z_MIN_PLANES;
  
  for(i = 0; i < Z_STEPS_PLANES; i++)
    {
    glVertex3f(x_min, -DELTA_Y_PLANES, z);
    glVertex3f(-x_min, -DELTA_Y_PLANES, z);

    glVertex3f(x_min, DELTA_Y_PLANES, z);
    glVertex3f(-x_min, DELTA_Y_PLANES, z);
    z += DELTA_Z_PLANES;
    }

  //  z = Z_MIN_PLANES;

  /* z-Direction */

  x = x_min;
  
  for(i = -WORLD_HALF_WIDTH_PLANES; i <= WORLD_HALF_WIDTH_PLANES; i++)
    {
    glVertex3f(x, -DELTA_Y_PLANES, Z_MIN_PLANES);
    glVertex3f(x, -DELTA_Y_PLANES, Z_MAX_PLANES);

    glVertex3f(x, DELTA_Y_PLANES, Z_MIN_PLANES);
    glVertex3f(x, DELTA_Y_PLANES, Z_MAX_PLANES);

    x += DELTA_X_PLANES;
    }
  glEnd();
  glDisable(GL_FOG);
  }

static void draw_lines_planes_v()
  {
  int i;
  float z, x;

  float x_min = - WORLD_HALF_WIDTH_PLANES  * DELTA_X_PLANES +
    0.5 * DELTA_X_PLANES;

/*   glFogf(GL_FOG_START, 60.0); */
/*   glFogf(GL_FOG_END, -Z_MIN_PLANES + DELTA_Z_PLANES); */

  glFogf(GL_FOG_DENSITY, 1.0/100.0);

  glFogi(GL_FOG_MODE, GL_EXP );

  glEnable(GL_FOG);
  
  glBegin(GL_LINES);

  /* X-Direction */

  z = Z_MIN_PLANES;
  
  for(i = 0; i < Z_STEPS_PLANES; i++)
    {
    glVertex3f(-DELTA_Y_PLANES, x_min, z);
    glVertex3f(-DELTA_Y_PLANES, -x_min, z);

    glVertex3f(DELTA_Y_PLANES, x_min, z);
    glVertex3f(DELTA_Y_PLANES, -x_min,  z);
    z += DELTA_Z_PLANES;
    }

  //  z = Z_MIN_PLANES;

  /* z-Direction */

  x = x_min;
  
  for(i = -WORLD_HALF_WIDTH_PLANES; i <= WORLD_HALF_WIDTH_PLANES; i++)
    {
    glVertex3f(-DELTA_Y_PLANES, x, Z_MIN_PLANES);
    glVertex3f(-DELTA_Y_PLANES, x, Z_MAX_PLANES);

    glVertex3f(DELTA_Y_PLANES, x, Z_MIN_PLANES);
    glVertex3f(DELTA_Y_PLANES, x, Z_MAX_PLANES);

    x += DELTA_X_PLANES;
    }
  glEnd();
  glDisable(GL_FOG);
  }

#define Z_MIN_GRID  -600.0
#define Z_MAX_GRID  -10.0

#define DELTA_X_GRID 20.0
#define DELTA_Y_GRID 20.0

#define DELTA_Z_GRID 20.0

#define Z_STEPS_GRID ((int)((Z_MAX_GRID - Z_MIN_GRID)/DELTA_Z_GRID+0.5))

#define TUNNEL_HALF_WIDTH_GRID  1
#define TUNNEL_HALF_HEIGHT_GRID 1

#define SPEED_GRID 0.8

#define WORLD_HALF_HEIGHT_GRID 8
#define WORLD_HALF_WIDTH_GRID  12

static void draw_lines_grid()
  {
  int i, j;
  float x, y, z;

  float x_min = - WORLD_HALF_WIDTH_GRID  * DELTA_X_GRID + 0.5 * DELTA_X_GRID;
  float y_min = - WORLD_HALF_HEIGHT_GRID * DELTA_Y_GRID + 0.5 * DELTA_Y_GRID;

  float x_tunnel_min = - TUNNEL_HALF_WIDTH_GRID * DELTA_X_GRID
    - 0.5 * DELTA_X_GRID;
  float y_tunnel_min = - TUNNEL_HALF_HEIGHT_GRID  * DELTA_Y_GRID
    - 0.5 * DELTA_Y_GRID;

  //  glFogf(GL_FOG_START, 60.0);
  //  glFogf(GL_FOG_END, -Z_MIN_GRID + DELTA_Z_GRID);

  glFogf(GL_FOG_DENSITY, 1.0/200.0);
  glFogf(GL_FOG_START, 400.0);
  glFogf(GL_FOG_END, -Z_MIN_GRID);
  glFogi(GL_FOG_MODE, GL_EXP );

  glEnable(GL_FOG);
    
  //  fprintf(stderr, "Z_MAX: %f\n", Z_MAX);
  
  // Z (= Viewing direction, loop for x and y)
  
  x = x_min;
    
  glBegin(GL_LINES);

  //  fprintf(stderr, "z_min: %f, Z_MAX : %f\n", Z_MIN, Z_MAX);

  //  glNormal3f(0.0, 1.0, 0.0);
  
  for(i = -WORLD_HALF_WIDTH_GRID; i < WORLD_HALF_WIDTH_GRID; i++)
    {
    y = y_min;
    for(j = -WORLD_HALF_HEIGHT_GRID; j < WORLD_HALF_HEIGHT_GRID; j++)
      {
      if(((i < -TUNNEL_HALF_WIDTH_GRID) || (i >= TUNNEL_HALF_WIDTH_GRID))
         || ((j < -TUNNEL_HALF_HEIGHT_GRID) ||
             (j >= TUNNEL_HALF_HEIGHT_GRID)) )
        {
        glVertex3f(x, y, Z_MIN_GRID - DELTA_Z_GRID);
        glVertex3f(x, y, Z_MAX_GRID);
        }
      y += DELTA_Y_GRID;
      }
    x += DELTA_X_GRID;
    }

  //  glNormal3f(1.0, 0.0, 0.0);

/*   glEnd(); */
  
  //#if 0

  // X-Direction

/*   glBegin(GL_LINES); */
  
  z = Z_MIN_GRID;
  for(i = 0; i <= Z_STEPS_GRID; i++)
    {
    y = y_min;
    for(j = -WORLD_HALF_HEIGHT_GRID; j < WORLD_HALF_HEIGHT_GRID; j++)
      {
      if((j < -TUNNEL_HALF_HEIGHT_GRID) || (j >= TUNNEL_HALF_HEIGHT_GRID))
        {
        glVertex3f(x_min,  y, z);
        glVertex3f(-x_min, y, z);
        }
      else
        {
        glVertex3f(x_tunnel_min,  y, z);
        glVertex3f(x_min,         y, z);
        glVertex3f(-x_tunnel_min, y, z);
        glVertex3f(-x_min,        y, z);
        }
      y += DELTA_Y_GRID;
      }
    z += DELTA_Z_GRID;
    }

/*   glEnd(); */
  
  // Y-Direction

/*   glBegin(GL_LINES); */
  
  z = Z_MIN_GRID;
  for(i = 0; i <= Z_STEPS_GRID; i++)
    {
    x = x_min;
    for(j = -WORLD_HALF_WIDTH_GRID; j < WORLD_HALF_WIDTH_GRID; j++)
      {
      if((j < -TUNNEL_HALF_WIDTH_GRID) || (j >= TUNNEL_HALF_WIDTH_GRID))
        {
        glVertex3f(x,  y_min, z);
        glVertex3f(x, -y_min, z);
        }
      else
        {
        glVertex3f(x, y_tunnel_min, z);
        glVertex3f(x, y_min,        z);
        glVertex3f(x, -y_tunnel_min, z);
        glVertex3f(x, -y_min,        z);
        }
      x += DELTA_X_GRID;
      }
    z += DELTA_Z_GRID;
    }

  glEnd();

  glDisable(GL_FOG);
  
  //#endif  
  }

/* Hexagonal grid */

#define DELTA_HEX       20.0
#define DELTA_Z_HEX     20.0

#define SIN_HEX 0.5
#define COS_HEX 0.86602540378443864677

#define SPEED_HEX 0.8

#define Z_MIN_HEX -400.0
#define Z_MAX_HEX 0.0

#define Z_STEPS_HEX ((int)((Z_MAX_PLANES - Z_MIN_PLANES)/DELTA_Z_PLANES+0.5))

#define WORLD_HALF_WIDTH_HEX  6
#define WORLD_HALF_HEIGHT_HEX 3

static void draw_triangle_hex(float x, float y, float z)
  {
  glVertex3f(x, y, z);
  glVertex3f(x, y - DELTA_HEX, z);

  glVertex3f(x, y, z);
  glVertex3f(x+ COS_HEX * DELTA_HEX, y + SIN_HEX * DELTA_HEX, z);

  glVertex3f(x, y, z);
  glVertex3f(x-COS_HEX * DELTA_HEX,  y + SIN_HEX * DELTA_HEX, z);

#if 1
  glVertex3f(x, y, z);
  glVertex3f(x, y, z - DELTA_Z_HEX);

  glVertex3f(x, y - DELTA_HEX, z);
  glVertex3f(x, y - DELTA_HEX, z - DELTA_Z_HEX);
#endif
  
  
  }

//#define NO_Z_LOOP  

static void draw_triangles_hex(float x, float y)
  {
  float z;
  int i;

#ifdef NO_Z_LOOP
  z = -200.0;
#else
  z = Z_MIN_HEX;
  for(i = 0; i < Z_STEPS_HEX; i++) /* Z-loop */
    {
#endif
    draw_triangle_hex(x, y, z);
#ifndef NO_Z_LOOP
    z += DELTA_Z_HEX;
    }
#if 0
  glVertex3f(x, y, Z_MIN_HEX);
  glVertex3f(x, y, Z_MAX_HEX);

  glVertex3f(x, y - DELTA_HEX, Z_MIN_HEX);
  glVertex3f(x, y - DELTA_HEX, Z_MAX_HEX);
#endif
#endif
  }

static void draw_lines_hex()
  {
  int   i, j;
  float x, y;
  
  //  glHint(GL_FOG_HINT, GL_NICEST);
  glFogf(GL_FOG_DENSITY, 1.0/100.0);
  glFogf(GL_FOG_START, 170.0);
  glFogf(GL_FOG_END, -Z_MIN_HEX);
  glFogi(GL_FOG_MODE, GL_EXP );
  //  glFogi(GL_FOG_MODE, GL_LINEAR );
  glEnable(GL_FOG);
    
  //  fprintf(stderr, "Z_MAX: %f\n", Z_MAX);
  
  // Z (= Viewing direction, loop for x and y)
  
  glBegin(GL_LINES);
  
  y = 0.0; /* y-loop */
  for(i = 0; i < WORLD_HALF_HEIGHT_HEX; i++)
    {
    /* Upper half */
#if 1
    x = 0.0; /* x-loop */
    for(j = 0; j < WORLD_HALF_WIDTH_HEX; j++)
      {
      draw_triangles_hex(  x + COS_HEX * DELTA_HEX, y + 0.5 * DELTA_HEX);
      draw_triangles_hex(- x - COS_HEX * DELTA_HEX, y + 0.5 * DELTA_HEX);
      draw_triangles_hex(  x,                       y -       DELTA_HEX);
      if(j)
        draw_triangles_hex(-x,                      y -       DELTA_HEX);
      x += 2.0 * COS_HEX * DELTA_HEX;
      }
#endif

    /* Lower half */

#if 1
    if(i)
      {
      x = 0.0; /* x-loop */
      for(j = 0; j < WORLD_HALF_WIDTH_HEX; j++)
        {
        draw_triangles_hex(  x/* + COS_HEX * DELTA_HEX*/, -(y + DELTA_HEX));
        draw_triangles_hex(- x/* - COS_HEX * DELTA_HEX*/, -(y + DELTA_HEX));
        draw_triangles_hex(  x + COS_HEX * DELTA_HEX,  -(y - (1-SIN_HEX)*DELTA_HEX));
        if(j)
          draw_triangles_hex(-x + COS_HEX * DELTA_HEX, -(y - (1-SIN_HEX)*DELTA_HEX));
        x += 2.0 * COS_HEX * DELTA_HEX;
        }
      }
#endif
    y += 3.0 * DELTA_HEX;
    }
  
  glEnd();

  glDisable(GL_FOG);
  
  }

static void * init_lines(lemuria_engine_t * e)
  {
  int random_value;
  lines_data * data;

#ifdef DEBUG
  fprintf(stderr, "init_lines...");
#endif
  
  data = calloc(1, sizeof(lines_data));


  data->z_start = 0.0;

  data->line_color_start_index = lemuria_random(e, 0, num_line_colors -1);
  data->line_color_end_index = lemuria_random(e, 0, num_line_colors -1);

  lemuria_range_init(e, &data->line_color, 4, 25, 75);

  data->lines_list = glGenLists(1);

  glNewList(data->lines_list, GL_COMPILE);

  random_value = lemuria_random_int(e, 0, 3);

  //  random_value = 3;
  
  switch(random_value)
    {
    case 0:
      draw_lines_grid();
      data->delta_z = DELTA_Z_GRID;
      data->speed = SPEED_GRID;
      break;
    case 1:
      draw_lines_planes();
      data->delta_z = DELTA_Z_PLANES;
      data->speed = SPEED_PLANES;
      break;
    case 2:
      draw_lines_planes_v();
      data->delta_z = DELTA_Z_PLANES;
      data->speed = SPEED_PLANES;
      break;
    case 3:
      draw_lines_hex();
      data->delta_z = DELTA_Z_HEX;
      data->speed = SPEED_HEX;
      break;
    }
  glEndList();
#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  return data;
  }


static void draw_lines(lemuria_engine_t * e, void * user_data)
  {
  lines_data * d = (lines_data*)user_data;

  float line_color[4];
  float fog_color[4];
  
  if(e->beat_detected)
    {
    if(lemuria_range_done(&d->line_color))
      {
      if(lemuria_decide(e, 0.1))
        {
        d->line_color_start_index = d->line_color_end_index;
        d->line_color_end_index = lemuria_random_int(e, 0, num_line_colors-1);
        lemuria_range_init(e, &d->line_color, 4, 25, 75);
        }
      }
    }

  // Set up the color

  lemuria_range_update(&d->line_color);
  
  lemuria_range_get(&d->line_color,
                        line_colors[d->line_color_start_index],
                        line_colors[d->line_color_end_index],
                        line_color);

  fog_color[0] = line_color[0] * 0.3;
  fog_color[1] = line_color[1] * 0.3;
  fog_color[2] = line_color[2] * 0.3;
  fog_color[3] = line_color[3];
    
  glClearColor(fog_color[0], fog_color[1],
               fog_color[2], fog_color[3]);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLineWidth(2.0);

  lemuria_set_perspective(e, 1, 1000.0);
  
  d->z_start += d->speed;

  if(d->z_start > d->delta_z)
    d->z_start -= d->delta_z;
  
  glFogfv(GL_FOG_COLOR, fog_color);
  
  glColor4fv(line_color);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  
  glTranslatef(0.0, 0.0, d->z_start);

  /*  glRotatef(45.0, 0.0, 0.0, 1.0); */

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
  
  glCallList(d->lines_list);

  if(e->antialias)
    {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    //    glEnable(GL_DEPTH_TEST);
    }

  glPopMatrix();

  
/*   draw_lines_grid(d); */
  
  }

effect_plugin_t lines_effect =
  {
    .init =    init_lines,
    .draw =    draw_lines,
    .cleanup = delete_lines,
  };
