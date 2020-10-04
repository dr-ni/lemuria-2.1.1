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

#include <light.h>
#include <material.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "meshes/icosphere.incl"

typedef struct 
  {
  float sum;
  
  lemuria_range_t line_color;
  
  int line_color_start_index;
  int line_color_end_index;

  unsigned int icosphere_list;
  lemuria_rotator_t rotator;
  lemuria_offset_t offset;
  
  float shape[NUM_ICOSPHERE_VERTICES];
  float vertices[NUM_ICOSPHERE_VERTICES][3];

  float triangle_normals[NUM_ICOSPHERE_TRIANGLES][3];
  float vertex_normals[NUM_ICOSPHERE_VERTICES][3];

  float vertex_average[3];

  float vertex_max;
  
  float decay_factor;
  float blur_factor;

  lemuria_range_t transition_range;

  lemuria_engine_t * engine;
  
  } icosphere_data ;

#define LINE_MIN_RED     0.2
#define LINE_MIN_GREEN   0.2
#define LINE_MIN_BLUE    0.2

#define LINE_MAX_RED     1.0
#define LINE_MAX_GREEN   1.0
#define LINE_MAX_BLUE    1.0

static float line_colors[][4] =
  {
    { 0.3, 0.3, 1.0, 0.5 },
    { LINE_MIN_RED, LINE_MAX_GREEN, LINE_MIN_BLUE, 0.5 },
    { LINE_MIN_RED, LINE_MAX_GREEN, LINE_MAX_BLUE, 0.5 },
    { LINE_MAX_RED, LINE_MIN_GREEN, LINE_MIN_BLUE, 0.5 },
    { LINE_MAX_RED, LINE_MAX_GREEN, LINE_MIN_BLUE, 0.5 },
  };

static int num_line_colors = sizeof(line_colors)/sizeof(float[4]);

static void init_shape(icosphere_data * d)
  {
  int i;
  for(i = 0; i < NUM_ICOSPHERE_VERTICES; i++)
    d->shape[i] = 0.0;
  //  d->shape[0] = 1.0;
  }

#define SPIKES_TO_ADD 3

static void update_shape(lemuria_engine_t * e, icosphere_data * d)
  {
  int i, j;

  int beat_vertex;

  int beat_factor;
  
  // First .run = Decay
  
  float blur_factor;

  d->sum = 0.0;
  
  d->vertex_average[0] = 0.0;
  d->vertex_average[1] = 0.0;
  d->vertex_average[2] = 0.0;

  d->vertex_max = 0.0;
  
  for(i = 0; i < NUM_ICOSPHERE_VERTICES; i++)
    {
    d->shape[i] *= d->decay_factor;

    if(d->vertex_max < d->shape[i])
      d->vertex_max = d->shape[i];
    
    d->sum += d->shape[i];

    d->vertex_average[0] += d->vertices[i][0];
    d->vertex_average[1] += d->vertices[i][1];
    d->vertex_average[2] += d->vertices[i][2];
    }

  d->vertex_average[0] /= NUM_ICOSPHERE_VERTICES;
  d->vertex_average[1] /= NUM_ICOSPHERE_VERTICES;
  d->vertex_average[2] /= NUM_ICOSPHERE_VERTICES;
  
  d->sum /= NUM_ICOSPHERE_VERTICES;
  
  //  fprintf(stderr, "sum: %f\n", sum);

  if((d->sum > 10.0) || d->vertex_max > 100.0)
    {
    d->decay_factor -= 0.01;
    }
  else if((d->sum < 0.2) && e->beat_detected && lemuria_decide(e, 0.1))
    {
    d->decay_factor += lemuria_random(e, -0.01, 0.1);
    
    if(d->decay_factor > 0.99)
      d->decay_factor = 0.99;
    else if(d->decay_factor < 0.8)
      d->decay_factor = 0.8;
    }

  // Second .run = blur
  
  for(i = 0; i < NUM_ICOSPHERE_VERTICES; i++)
    {
    for(j = 0; j < 6; j++)
      {
      if(icoshpere_neighbours[i][j] != -1)
        {
        if(lemuria_decide(e, 0.5))
          blur_factor = 0.010;
        else
          blur_factor = 0.005;

        if(icoshpere_neighbours[icoshpere_neighbours[i][j]][5] == -1)
          blur_factor *= 6.0/5.0;

        d->shape[icoshpere_neighbours[i][j]] += d->shape[i] * blur_factor;
        }
      }
    }
  
  // Third .run = Beats

  if(e->beat_detected)
    {
    for(i = 0; i < SPIKES_TO_ADD; i++)
      {
      beat_vertex = lemuria_random_int(e, 0, NUM_ICOSPHERE_VERTICES-1);
      beat_factor = lemuria_random(e, (float)e->loudness/32768.0,
                                        (float)e->loudness/32768.0 + 2.0);
      if(d->shape[beat_vertex] < beat_factor)
        d->shape[beat_vertex] = beat_factor;
      //#if 0
      for(j = 0; j < 6; j++)
        {
        if((icoshpere_neighbours[beat_vertex][j] != -1) &&
           (d->shape[icoshpere_neighbours[beat_vertex][j]] < 0.1 * beat_factor))
          d->shape[icoshpere_neighbours[beat_vertex][j]] = 0.1 * beat_factor;
        }
      //#endif
      }

    }

  // Fourth .run = update vertices

  for(i = 0; i < NUM_ICOSPHERE_VERTICES; i++)
    {
    d->vertices[i][0] = icosphere_vertices[i][0] * (1.0 + d->shape[i]);
    d->vertices[i][1] = icosphere_vertices[i][1] * (1.0 + d->shape[i]);
    d->vertices[i][2] = icosphere_vertices[i][2] * (1.0 + d->shape[i]);
    }
  
  }

//          (  | A_1 B_1 | )
//          (  | A_2 B_2 | )
//          (              )
//          (  | A_0 B_0 | )
//  A x B = ( -| A_2 B_2 | )
//          (              )
//          (  | A_0 B_0 | )
//          (  | A_1 B_1 | )

static void update_normals(icosphere_data * d)
  {
  int i, j;

  float vec_1[3];
  float vec_2[3];

  float norm_factor;
  
  for(i = 0; i < NUM_ICOSPHERE_TRIANGLES; i++)
    {
    vec_1[0] = d->vertices[icosphere_triangles[i][0]][0] - d->vertices[icosphere_triangles[i][1]][0];
    vec_1[1] = d->vertices[icosphere_triangles[i][0]][1] - d->vertices[icosphere_triangles[i][1]][1];
    vec_1[2] = d->vertices[icosphere_triangles[i][0]][2] - d->vertices[icosphere_triangles[i][1]][2];

    vec_2[0] = d->vertices[icosphere_triangles[i][2]][0] - d->vertices[icosphere_triangles[i][1]][0];
    vec_2[1] = d->vertices[icosphere_triangles[i][2]][1] - d->vertices[icosphere_triangles[i][1]][1];
    vec_2[2] = d->vertices[icosphere_triangles[i][2]][2] - d->vertices[icosphere_triangles[i][1]][2];

    d->triangle_normals[i][0] =   vec_1[1] * vec_2[2] - vec_1[2] * vec_2[1];
    d->triangle_normals[i][1] = - vec_1[0] * vec_2[2] + vec_1[2] * vec_2[0];
    d->triangle_normals[i][2] =   vec_1[0] * vec_2[1] - vec_1[1] * vec_2[0];

    if((d->triangle_normals[i][0]*d->vertices[icosphere_triangles[i][2]][0] +
        d->triangle_normals[i][1]*d->vertices[icosphere_triangles[i][2]][1] +
        d->triangle_normals[i][2]*d->vertices[icosphere_triangles[i][2]][2]) < 0.0)
      {
      d->triangle_normals[i][0] *= -1.0;
      d->triangle_normals[i][1] *= -1.0;
      d->triangle_normals[i][2] *= -1.0;
      }
    }

  norm_factor = 1.0/6.0;
  
  for(i = 0; i < NUM_ICOSPHERE_VERTICES; i++)
    {
    d->vertex_normals[i][0] = 0.0;
    d->vertex_normals[i][1] = 0.0;
    d->vertex_normals[i][2] = 0.0;
    
    for(j = 0; j < 6; j++)
      {
      if(icosphere_triangle_members[i][j] == -1)
        {
        norm_factor = 1.0/5.0;
        break;
        }
      
      d->vertex_normals[i][0] +=
        d->triangle_normals[icosphere_triangle_members[i][j]][0];
      d->vertex_normals[i][1] +=
        d->triangle_normals[icosphere_triangle_members[i][j]][1];
      d->vertex_normals[i][2] +=
        d->triangle_normals[icosphere_triangle_members[i][j]][2];
      }
    d->vertex_normals[i][0] *= norm_factor;
    d->vertex_normals[i][1] *= norm_factor;
    d->vertex_normals[i][2] *= norm_factor;
    }
  
  }

static void delete_icosphere(void * data)
  {
  icosphere_data * d = (icosphere_data*)data;
  lemuria_texture_unref(d->engine, 0);
  glDeleteLists(d->icosphere_list, 1);
  free(d);
  }


static void draw_sphere(icosphere_data * d)
  {
  int i;

  int vertex_1, vertex_2, vertex_3;

  //  glFrontFace(GL_CW);
  
  glEnable(GL_NORMALIZE);
  
  glBegin(GL_TRIANGLES);
  
  for(i = 0; i < NUM_ICOSPHERE_TRIANGLES; i++)
    {
    vertex_1 = icosphere_triangles[i][0];
    vertex_2 = icosphere_triangles[i][1];
    vertex_3 = icosphere_triangles[i][2];

    glTexCoord2f(icosphere_texture_coords[i][0][0],
                 icosphere_texture_coords[i][0][1]);
    
    glNormal3f(d->vertex_normals[vertex_1][0],
               d->vertex_normals[vertex_1][1],
               d->vertex_normals[vertex_1][2]);

    glVertex3f(d->vertices[vertex_1][0],
               d->vertices[vertex_1][1],
               d->vertices[vertex_1][2]);
    
    glTexCoord2f(icosphere_texture_coords[i][1][0],
                 icosphere_texture_coords[i][1][1]);
        
    glNormal3f(d->vertex_normals[vertex_2][0],
               d->vertex_normals[vertex_2][1],
               d->vertex_normals[vertex_2][2]);

    glVertex3f(d->vertices[vertex_2][0],
               d->vertices[vertex_2][1],
               d->vertices[vertex_2][2]);

    glTexCoord2f(icosphere_texture_coords[i][2][0],
                 icosphere_texture_coords[i][2][1]);

    glNormal3f(d->vertex_normals[vertex_3][0],
               d->vertex_normals[vertex_3][1],
               d->vertex_normals[vertex_3][2]);

    glVertex3f(d->vertices[vertex_3][0],
               d->vertices[vertex_3][1],
               d->vertices[vertex_3][2]);
    }

  glEnd();

  glDisable(GL_NORMALIZE);

  //  glFrontFace(GL_CCW);

  }

#if 0

static void draw_sphere_wireframe(icosphere_data * d)
  {
  int i;

  glColor3f(1.0, 0.0, 0.0);

  glBegin(GL_LINES);
  
  for(i = 0; i < NUM_ICOSPHERE_EDGES; i++)
    {
    glVertex3f(d->vertices[icosphere_edges[i][0]][0],
               d->vertices[icosphere_edges[i][0]][1],
               d->vertices[icosphere_edges[i][0]][2]);

    glVertex3f(d->vertices[icosphere_edges[i][1]][0],
               d->vertices[icosphere_edges[i][1]][1],
               d->vertices[icosphere_edges[i][1]][2]);
    }

  
  glEnd();

  glColor3f(0.0, 1.0, 0.0);

  glBegin(GL_LINES);
  
  for(i = 0; i < NUM_ICOSPHERE_VERTICES; i++)
    {
    glVertex3f(d->vertices[i][0],
               d->vertices[i][1],
               d->vertices[i][2]);

    glVertex3f(d->vertices[i][0] + 10.0 * d->vertex_normals[i][0],
               d->vertices[i][1] + 10.0 * d->vertex_normals[i][1],
               d->vertices[i][2] + 10.0 * d->vertex_normals[i][2]);
    }
    
  glEnd();

  
  
  }

#endif

static void * init_icosphere(lemuria_engine_t * e)
  {
  //  int random_value;

  icosphere_data * data;
#ifdef DEBUG
  fprintf(stderr, "init_icosphere...");
#endif

  data = calloc(1, sizeof(icosphere_data));

  data->engine = e;
  lemuria_texture_ref(data->engine, 0);
    
  e->foreground.mode = EFFECT_STARTING;
  lemuria_rotator_init(e, &(data->rotator));
  lemuria_offset_init(e, &(data->offset));
  
  data->line_color_start_index = lemuria_random(e, 0, num_line_colors -1);
  data->line_color_end_index = lemuria_random(e, 0, num_line_colors -1);

  lemuria_range_init(e, &data->line_color, 4, 25, 75);

  init_shape(data);
  data->decay_factor = 0.95;
  lemuria_range_init(e, (&data->transition_range), 1, 100, 200);

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif

  return data;
  }

static lemuria_light_t light = 
  {
    .position = { 5.0f, 5.0f, 10.0f, 1.0f },
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 1.0f, 1.0f, 1.0f, 1.0f }
  };

static lemuria_material_t material =
  {
    .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 128
  };

static void draw_icosphere(lemuria_engine_t * e, void * user_data)
  {
  float line_color[4];
  float scale_start;
  float scale_end;
  float scale_factor;
  
  icosphere_data * d = (icosphere_data*)user_data;

  if(e->foreground.mode == EFFECT_FINISH)
    {
    e->foreground.mode = EFFECT_FINISHING;
    lemuria_range_init(e, (&d->transition_range), 1, 100, 200);
    }

  switch(e->foreground.mode)
    {
    case EFFECT_FINISHING:
      scale_start = M_PI;
      scale_end =   0.0;
      lemuria_range_update((&d->transition_range));

      lemuria_range_get((&d->transition_range),
                        &scale_start, &scale_end,
                        &scale_factor);
      scale_factor = 0.5 * (1.0 - cos(scale_factor));
      if(lemuria_range_done((&d->transition_range)))
        e->foreground.mode = EFFECT_DONE;
      break;
    case EFFECT_STARTING:
      scale_start = 0.0;
      scale_end =   M_PI;
      lemuria_range_update((&d->transition_range));

      lemuria_range_get((&d->transition_range),
                        &scale_start, &scale_end,
                        &scale_factor);
      scale_factor = 0.5 * (1.0 - cos(scale_factor));
      if(lemuria_range_done((&d->transition_range)))
        e->foreground.mode = EFFECT_RUNNING;
         
      break;
    default:
      scale_factor = 1.0;
    }
   
  if(e->beat_detected)
    {
    if(lemuria_decide(e, 0.1))
      lemuria_rotator_change(e, &(d->rotator));

    if(lemuria_decide(e, 0.1))
      lemuria_offset_change(e, &(d->offset));
    if(lemuria_decide(e, 0.1))
      lemuria_offset_kick(&(d->offset));
    }
  
  lemuria_rotator_update(&(d->rotator));
  lemuria_offset_update(&(d->offset));
 
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

  glShadeModel(GL_SMOOTH);
  glEnable(GL_TEXTURE_2D);
  //  glEnable(GL_ALPHA_TEST);

  lemuria_range_update(&d->line_color);
  
  lemuria_range_get(&d->line_color,
                        line_colors[d->line_color_start_index],
                        line_colors[d->line_color_end_index],
                        line_color);
 
  lemuria_set_perspective(e, 1, 10.0);

  // Enable lighting
  glEnable(GL_LIGHTING);
  //  glEnable(GL_CULL_FACE);
  
  // Set up and enable light 0

  lemuria_set_light(&light, GL_LIGHT0);
  glEnable(GL_LIGHT0);

  lemuria_set_material(&material, GL_FRONT);
  
  glColor3f(1.0, 1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  lemuria_offset_translate(&(d->offset));

  glScalef(scale_factor * 0.5 / (1.0+1.8*d->sum),
           scale_factor * 0.5 / (1.0+1.8*d->sum),
           scale_factor * 0.5 / (1.0+1.8*d->sum));

  
  lemuria_rotate(&(d->rotator));

  glTranslatef(-d->vertex_average[0],
               -d->vertex_average[1],
               -d->vertex_average[2]);

  lemuria_texture_bind(e, 0);
  
  // These routines do the actual deformation
  
  update_shape(e, d);
  update_normals(d);

  // This one draws it all
  
  draw_sphere(d);
  //  draw_sphere_wireframe(d);
    
  glPopMatrix();

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  //  glDisable(GL_CULL_FACE);

  }

effect_plugin_t icosphere_effect =
  {
    .init =    init_icosphere,
    .draw =    draw_icosphere,
    .cleanup = delete_icosphere,
  };
