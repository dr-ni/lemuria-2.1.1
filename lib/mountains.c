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

#include <string.h>

#include <light.h>
#include <material.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <particle.h>


#define X_MAX       100.0
#define Z_START    -500.0      

#define FREQ_BANDS  40
#define Z_STEPS     60

#define X_STEPS    (2*FREQ_BANDS-1)

#define HEIGHT_BASE_MOUNTAINS   -10.0
#define HEIGHT_BASE_CAVE        -30.0

#define HEIGHT_FACTOR 100.0 // Maximum height of a mountain

#define MODE_MOUNTAINS 0
#define MODE_CAVE      1

// #define CEILING_MODE_NONE     0
// #define CEILING_MODE_LEMURIA  1
// #define CEILING_MODE_GOOM     2

// #define DRAW_WIREFRAME

static const float delta_z = -Z_START/(float)Z_STEPS;
static const float delta_x = 2.0*X_MAX/(float)X_STEPS;

static float line_colors[][4] =
  {
    { 1.0,  0.90, 0.55, 1.0 },
    { 0.66, 0.67, 1.0, 1.0 },
    { 1.0,  0.74, 1.0, 1.0 },
  };

static int num_line_colors = sizeof(line_colors)/sizeof(line_colors[0]);

static float lemuria_colors[][4] =
  {
    { 1.0, 0.0, 1.0, 1.0 },
    { 0.0, 1.0, 1.0, 1.0 },
    { 1.0, 1.0, 0.0, 1.0 },
  };

static int num_lemuria_colors = sizeof(lemuria_colors)/sizeof(lemuria_colors[0]);

typedef struct _line_data
  {
  float heights[X_STEPS+1];
  float normals[X_STEPS+1][3];
  float color[4];
  struct _line_data * next;
  } line_data;

// Ceiling

typedef struct
  {
  float x;
  float y_min;
  float y_max;
  float z_min;
  float z_max;
  float fog_start;
  float fog_end;
  float fog_color[4];
  lemuria_background_data_t background;
  
  } ceiling_data_t;

static ceiling_data_t ceilings[] =
  {
    {
      .x =      800.0,
      .y_min =  -10.0,
      .y_max =  100.0,
      .z_min = -1500.0,
      .z_max =  0.0,
      
      .fog_start = 1300.0,
      .fog_end =   1500.0,
      .fog_color = { 0.0, 0.0, 0.0, 1.0 },
      .background =
      {
        .texture_mode = LEMURIA_TEXTURE_LEMURIA,
        (uint8_t *)0,
        clouds_size : 0,
      },
    },

    // The one with goom MUST be the last one!!

    {
      .x =      800.0,
      .y_min = -10.0,
      .y_max =  100.0,
      .z_min = -1500.0,
      .z_max =  0.0,
      
      .fog_start = 1300.0,
      .fog_end =   1500.0,
      .fog_color = { 0.0, 0.0, 0.0, 1.0 },
      .background =
      {
        .texture_mode = LEMURIA_TEXTURE_GOOM,
        (uint8_t *)0,
        clouds_size : 0,
      },
    },
    
  };

static int num_ceilings = sizeof(ceilings)/sizeof(ceilings[0]);

typedef struct
  {
  line_data lines[Z_STEPS+1];

  line_data * new_line;
  line_data * normal_line;
  line_data * normal_line2;
  
  int freq_tab[FREQ_BANDS];

  float weight_table[FREQ_BANDS];
  
  float speed;
  float z_start;

  int first_index;

  lemuria_range_t color_range;

  lemuria_range_t angle_range;

  int start_color;
  int end_color;

  int mode;
  //  int ceiling_mode;
  
  float height_base;

  float angle_end;

  // Ceiling stuff


  lemuria_engine_t * engine;

  int lemuria_color_start;
  int lemuria_color_end;

  lemuria_range_t lemuria_color_range;
  
  lemuria_background_t ceiling;

  ceiling_data_t * ceiling_data;
  
  } mountains_data;

#define LOG10_2 0.301029996

static void * init_mountains(lemuria_engine_t * e)
  {
  int i, j;
  float f_tmp;
  int i_tmp;
  mountains_data * d;

#ifdef DEBUG
  fprintf(stderr, "init_mountains...");
#endif

  d = calloc(1, sizeof(mountains_data));
  

  d->z_start = 0;

  d->start_color = lemuria_random_int(e, 0, num_line_colors-1);
  d->end_color = lemuria_random_int(e, 0, num_line_colors-1);
  lemuria_range_init(e, &d->color_range, 4, 50, 100);
  
  d->mode = lemuria_random_int(e, 0, 1);
  //  d->mode = MODE_CAVE;
  switch(d->mode)
    {
    case MODE_MOUNTAINS:
      d->height_base = HEIGHT_BASE_MOUNTAINS;

      if(e->goom)
        i_tmp = lemuria_random_int(e, 0, num_ceilings-1);
      else
        i_tmp = lemuria_random_int(e, 0, num_ceilings-2);
           
      d->ceiling_data = &(ceilings[i_tmp]);
      lemuria_background_init(e, &(d->ceiling),
                               &(d->ceiling_data->background));

      d->lemuria_color_start = lemuria_random_int(e, 0, num_lemuria_colors-2);
      d->lemuria_color_end   = lemuria_random_int(e, 0, num_lemuria_colors-2);
      lemuria_range_init(e, &(d->lemuria_color_range), 4, 100, 200);
      break;
    case MODE_CAVE:
      d->height_base = HEIGHT_BASE_CAVE;
      break;
    }

  
  for(i = 0; i <= Z_STEPS; i++)
    {
    if(i < Z_STEPS)
      d->lines[i].next = &(d->lines[i+1]);
    else
      d->lines[i].next = &(d->lines[0]);
    
    for(j = 0; j <= X_STEPS; j++)
      {
      d->lines[i].heights[j] = d->height_base;
      d->lines[i].normals[j][0] = 0.0;
      d->lines[i].normals[j][1] = 1.0;
      d->lines[i].normals[j][2] = 0.0;
      memcpy(d->lines[i].color, line_colors[d->start_color], 4*sizeof(float));
      }
    
    }

  // Intialize the frequency table
    
  for(i = 0; i < FREQ_BANDS; i++)
    {
    f_tmp = pow(2.0, (i+1)*8.0/FREQ_BANDS);
    i_tmp = (int)(f_tmp+0.5);
    if(i_tmp > 256)
      i_tmp = 256;
    
    d->freq_tab[i] = i_tmp;

    if((i) && (d->freq_tab[i-1] == d->freq_tab[i]))
      {
      d->freq_tab[i]++;
      }
    
    d->weight_table[i] = HEIGHT_FACTOR*sqrt((float)(i+1));
    
    //    fprintf(stderr, "d->freq_tab[%d]: %d %f\n",i, d->freq_tab[i], f_tmp);
    }

  d->speed = 5.0;
  d->new_line     = &(d->lines[2]);
  d->normal_line  = &(d->lines[1]);
  d->normal_line2 = &(d->lines[0]);

  d->angle_end = 0.0;
  lemuria_range_init(e, &d->angle_range, 1, 100, 200);

  d->engine = e;

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  
  return d;
  } 

static void draw_ceiling(lemuria_engine_t * e, ceiling_data_t * c,
                         lemuria_background_t * b)
  {
  //  fprintf(stderr, "Draw Goom\n");
  
  glFogf(GL_FOG_START,  c->fog_start);
  glFogf(GL_FOG_END,    c->fog_end);
  glFogfv(GL_FOG_COLOR, c->fog_color);
  
  glFogi(GL_FOG_MODE,   GL_LINEAR );
  glEnable(GL_FOG);
  
  glEnable(GL_TEXTURE_2D);

  lemuria_background_set(b);
  
  //  glColor3f(1.0, 1.0, 0.0);
  
  glBegin(GL_QUADS);

  glTexCoord2f(0.0, 0.0);
  glVertex3f(-c->x, c->y_min, c->z_min);

  glTexCoord2f(1.0, 0.0);
  glVertex3f( c->x, c->y_min, c->z_min);

  glTexCoord2f(1.0, 1.0);
  glVertex3f( c->x, c->y_max, c->z_max);

  glTexCoord2f(0.0, 1.0);
  glVertex3f(-c->x, c->y_max, c->z_max);

  glEnd();
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_FOG);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  
  }

#if 0
static void draw_ceiling_lemuria(lemuria_engine_t * e, mountains_data * d)
  {
  float color[4];

  
  //  fprintf(stderr, "Draw Lemuria\n");

  //  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glFogf(GL_FOG_START, LEMURIA_FOG_START);
  glFogf(GL_FOG_END, LEMURIA_FOG_END);
  glFogi(GL_FOG_MODE, GL_LINEAR );
  glFogfv(GL_FOG_COLOR,  lemuria_fog_color);
  
  glEnable(GL_FOG);
  
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, e->texture);

  glColor4fv(color);

  
  glBegin(GL_QUADS);

  glTexCoord2f(0.0, 0.0);
  glVertex3f(-LEMURIA_X, LEMURIA_Y_MIN, LEMURIA_Z_MIN);

  glTexCoord2f(1.0, 0.0);
  glVertex3f( LEMURIA_X, LEMURIA_Y_MIN, LEMURIA_Z_MIN);

  glTexCoord2f(1.0, 1.0);
  glVertex3f( LEMURIA_X, LEMURIA_Y_MAX, LEMURIA_Z_MAX);

  glTexCoord2f(0.0, 1.0);
  glVertex3f(-LEMURIA_X, LEMURIA_Y_MAX, LEMURIA_Z_MAX);

  glEnd();
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_FOG);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  
  }
#endif

static void create_line(lemuria_engine_t * e,
                        mountains_data * d)
  {
  int i, j, start_index;

  int sum_l;
  int sum_r;

  float vec_x[3];
  float vec_z[3];
  float absolute;
  
  // Now, loop through all frequency bands
    
  start_index = 0;  

  // Set up the color

  if(lemuria_range_done(&d->color_range) && e->beat_detected &&
     lemuria_decide(e, 0.1))
    {
    d->start_color = d->end_color;
    d->end_color = lemuria_random_int(e, 0, num_line_colors-2);
    if(d->end_color >= d->start_color)
      d->end_color++;
    lemuria_range_init(e, &d->color_range, 4, 50, 100);
    }
  
  lemuria_range_update(&d->color_range);

  lemuria_range_get(&d->color_range, line_colors[d->start_color],
                    line_colors[d->end_color], d->new_line->color);
  
  for(i = 0; i < FREQ_BANDS; i++)
    {
    sum_r = 0;
    sum_l = 0;
    for(j = start_index; j < d->freq_tab[i]; j++)
      {
      sum_l += abs(e->freq_buffer_read[0][j]);
      sum_r += abs(e->freq_buffer_read[1][j]);
      }
    //    fprintf(stderr, "sum: %d %d\n", sum_l, sum_r);
    
    d->new_line->heights[i] =
      d->height_base +
      d->weight_table[i]*(float)sum_l /
      ((float)(d->freq_tab[i]-start_index)*32768.0);
    
    d->new_line->heights[X_STEPS-i] =
      d->height_base +
      d->weight_table[i]*(float)sum_r /
      ((float)(d->freq_tab[i]-start_index)*32768.0);
    start_index = d->freq_tab[i];
    
    }
 
  // Now, calculate the normals

  vec_x[0] = 2.0 * delta_x;
  vec_x[2] = 0.0;

  vec_z[0] = 0.0;
  vec_z[2] = 2.0 * delta_z;

  //
  // Cross product:
  //
  // ( ax )   ( bx )   ( ay * bz - az * by )
  // ( ay ) X ( by ) = ( az * bx - ax * bz )
  // ( az )   ( bz )   ( ax * by - ay * bx )
  
  for(i = 1; i < X_STEPS; i++)
    {
    vec_x[1] = d->normal_line->heights[i+1] - d->normal_line->heights[i-1];
    vec_z[1] = d->normal_line2->heights[i]  - d->new_line->heights[i];
    d->normal_line->normals[i][0] = vec_z[1] * vec_x[2] - vec_z[2] * vec_x[1];
    d->normal_line->normals[i][1] = vec_z[2] * vec_x[0] - vec_z[0] * vec_x[2];
    d->normal_line->normals[i][2] = vec_z[0] * vec_x[1] - vec_z[1] * vec_x[0];

    absolute = sqrt(d->normal_line->normals[i][0]*
                    d->normal_line->normals[i][0]+
                    d->normal_line->normals[i][1]*
                    d->normal_line->normals[i][1]+
                    d->normal_line->normals[i][2]*
                    d->normal_line->normals[i][2]);
    d->normal_line->normals[i][0] /= absolute;
    d->normal_line->normals[i][1] /= absolute;
    d->normal_line->normals[i][2] /= absolute;
    }

  d->new_line = d->new_line->next;
  d->normal_line = d->normal_line->next;
  d->normal_line2 = d->normal_line2->next;
    
  }



static void draw_band(mountains_data * d, line_data * near_line,
                      float z_1, float z_2)
  {
  int i;


  float x_1 = -X_MAX;
  float x_2;

  line_data * far_line = near_line->next;
  
  //  fprintf(stderr, "Draw band: %d %d\n", index_1, index_2);

#ifdef DRAW_WIREFRAME
  
  glBegin(GL_LINES);

  
  for(i = 0; i < X_STEPS; i++)
    {
    x_2 = (i+1) * 2.0 * X_MAX / (X_STEPS) - X_MAX;

    glColor4f(0.0, 1.0, 0.0, 1.0);

    glVertex3f(x_1, near_line->heights[i],  z_1);
    glVertex3f(x_2, near_line->heights[i+1], z_1);
    glVertex3f(x_2, near_line->heights[i+1], z_1);
    glVertex3f(x_2, far_line->heights[i+1], z_2);

    glColor4f(1.0, 0.0, 0.0, 1.0);

    glVertex3f(x_1,
               near_line->heights[i],
               z_1);
    glVertex3f(x_1 + 3.0 * near_line->normals[i][0],
               near_line->heights[i] + 3.0 * near_line->normals[i][1],
               z_1 + 3.0 * near_line->normals[i][2]);
    
    
    x_1 = x_2;
    }
  glEnd();
#else

  glBegin(GL_QUAD_STRIP);
    
  for(i = 0; i < X_STEPS; i++)
    {
    x_2 = (i+1) * 2.0 * X_MAX / (X_STEPS) - X_MAX;

    glColor4fv(near_line->color);

    glNormal3f(near_line->normals[i][0],
               near_line->normals[i][1],
               near_line->normals[i][2]);
    glVertex3f(x_1, near_line->heights[i] ,  z_1);

    glColor4fv(far_line->color);
    glNormal3f(far_line->normals[i][0],
               far_line->normals[i][1],
               far_line->normals[i][2]);
    glVertex3f(x_1, far_line->heights[i],    z_2);

    glColor4fv(near_line->color);
    glNormal3f(near_line->normals[i+1][0],
               near_line->normals[i+1][1],
               near_line->normals[i+1][2]);
    glVertex3f(x_2, near_line->heights[i+1], z_1);

    glColor4fv(far_line->color);
    glNormal3f(far_line->normals[i+1][0],
               far_line->normals[i+1][1],
               far_line->normals[i+1][2]);
    glVertex3f(x_2, far_line->heights[i+1],  z_2);
        
    x_1 = x_2;
    }
  glEnd();
  
#endif  
  
  
  }

static lemuria_material_t material =
  {
    .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_ambient =  { 1.0f, 1.0f, 0.0f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 0.0f, 1.0f },
    .shininess = 128
  };

static lemuria_light_t light =
  {
    .ambient =  { 0.2f, 0.2f, 0.2f, 1.0f },
    .diffuse =  { 0.8f, 0.8f, 0.8f, 1.0f },
    .specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .position = { 0.0f, 0.0f, 2.0f, 1.0f }
  };

static void draw_mountains(lemuria_engine_t * e, void * user_data)
  {
  line_data * near_line;
  float z_1, z_2;
  int i;
  float angle, angle_start = 0.0;
  mountains_data * d = (mountains_data*)(user_data);
  float color[4];
  lemuria_set_perspective(e, 1, 2000.0);

  glClearColor(0.0, 0.0, 0.1, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
  // Draw Ceiling

  if(d->mode != MODE_CAVE)
    {
    // Change the colors for lemuria ceiling

    if(d->ceiling.config->texture_mode == LEMURIA_TEXTURE_LEMURIA)
      {
      if(e->beat_detected && lemuria_range_done(&(d->lemuria_color_range)) &&
         lemuria_decide(e, 0.02))
        {
        d->lemuria_color_start = d->lemuria_color_end;
        d->lemuria_color_end   = lemuria_random_int(e, 0, num_lemuria_colors-2);
        if(d->lemuria_color_end >= d->lemuria_color_start)
          d->lemuria_color_end++;
        lemuria_range_init(e, &(d->lemuria_color_range), 4, 100, 200);
        //    fprintf(stderr, "Changing Color\n");
        }
      else
        lemuria_range_update(&(d->lemuria_color_range));
      
      lemuria_range_get(&(d->lemuria_color_range),
                        lemuria_colors[d->lemuria_color_start],
                        lemuria_colors[d->lemuria_color_end],
                        color);
      glColor4fv(color);
      }
    else
      {
      glColor3f(1.0, 1.0, 1.0);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      }
    
    draw_ceiling(e, d->ceiling_data, &(d->ceiling)); 

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glClear(GL_DEPTH_BUFFER_BIT);
    }
  
  d->z_start += d->speed;
  while(d->z_start > delta_z)
    {
    create_line(e, d);
    d->z_start -= delta_z;
    }
  
  near_line = d->new_line;


  if(d->mode == MODE_CAVE)
    {
    if(lemuria_range_done(&(d->angle_range)) &&
       e->beat_detected && lemuria_decide(e, 0.03))
      {
      if(lemuria_decide(e, 0.5))
        d->angle_end = -360.0;
      else
        d->angle_end = 360.0;
      lemuria_range_init(e, &(d->angle_range), 1, 200, 300);
      }
    else
      lemuria_range_update(&(d->angle_range));

    lemuria_range_get_cos(&(d->angle_range), &angle_start,
                          &(d->angle_end), &angle);
    }
  else
    angle = 0.0;
  
  glEnable(GL_COLOR_MATERIAL);
  //  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
  glLineWidth(1.0);


#ifndef DRAW_WIREFRAME

  lemuria_set_material(&material, GL_FRONT);
  lemuria_set_light(&light, GL_LIGHT0);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glShadeModel(GL_SMOOTH);

#endif

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glRotatef(angle, 0.0, 0.0, 1.0);

  z_1 = d->z_start;
  for(i = 0; i < Z_STEPS; i++)
    {
    z_2 = z_1 - delta_z;
    draw_band(d, near_line, z_1, z_2);
    z_1 = z_2;
    near_line = near_line->next;
    }

  glPopMatrix();
    
  if(d->mode == MODE_CAVE)
    {
    glPushMatrix();
    glLoadIdentity();
    glScalef(1.0, -1.0, 1.0);
    glRotatef(-angle, 0.0, 0.0, 1.0);
    
    z_1 = d->z_start;
    near_line = d->new_line;
    
    
    for(i = 0; i < Z_STEPS; i++)
      {
      z_2 = z_1 - delta_z;
      draw_band(d, near_line, z_1, z_2);
      z_1 = z_2;
      near_line = near_line->next;
      }
    glPopMatrix();
    }
  
  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glDisable(GL_COLOR_MATERIAL);


  }

static void delete_mountains(void * e)
  {
  mountains_data * data = (mountains_data*)(e);

  if(data->mode != MODE_CAVE)
    {
    lemuria_background_delete(&(data->ceiling));
    }

  free(data);
  }

effect_plugin_t mountains_effect =
  {
    .init =    init_mountains,
    .draw =    draw_mountains,
    .cleanup = delete_mountains,
  };
