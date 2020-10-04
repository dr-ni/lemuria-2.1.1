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

#include <light.h>
#include <material.h>

static lemuria_light_t light =
  {
    .ambient =  { 0.2f, 0.2f, 0.4f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 0.0f, 0.0f, 0.0f, 1.0f },
    .position = { 5.0f, 5.0f, 0.0f, 1.0f },
  };

#define NUM_RINGS 150
#define PHI_STEPS 40

#define TUNNEL_DELTA_Z 4.0
#define TUNNEL_RADIUS 20.0

typedef struct _ring_data 
  {
  int do_draw;
  float curvature_plane_x; /* Curvature to the next ring (x-plane) */
  float curvature_plane_y; /* Curvature to the next ring (y_plane) */
  float delta_angle;

  float texture_x, texture_y;
  
  float coords[PHI_STEPS+1][3];
  float normals[PHI_STEPS+1][3];

  struct _ring_data * next; 
  } ring_data;

#define CURV_X_INDEX 0
#define CURV_Y_INDEX 1

typedef struct
  {
  float curvature_1[2];
  float curvature_2[2];

  float * current_curvature;
  float * next_curvature;
  
  ring_data rings[NUM_RINGS];
  ring_data * start_ring;
  
  float ring_coords[PHI_STEPS+1][2]; /* Coordinates of an untransformed ring */
  float ring_normals[PHI_STEPS+1][2]; /* Normals of an untransformed ring */
 
  float speed;
  float z_start;

  float texture_y_start;
  float texture_x_start;
  
  unsigned int texture;
  uint8_t * texture_data;
  
  //  int texture_mode;
  
  lemuria_range_t curvature;

  lemuria_engine_t * engine;

  lemuria_background_t background;
  int rings_to_draw;
  } tunnel_data;

static void draw_ring(tunnel_data * data,
                      ring_data * r_1,
                      ring_data * r_2)
  {
  int i;

  float texture_x_1 = r_1->texture_x;
  float texture_x_2 = r_2->texture_x;
  float texture_y_1 = r_1->texture_y;
  float texture_y_2 = r_2->texture_y;

  float texture_delta_x = 1.0/(float)(PHI_STEPS-1);

  if((!r_1->do_draw) || (!r_2->do_draw))
    return;
  
  if(fabs(texture_x_2 - texture_x_1) > 0.5)
    {
    if(texture_x_2 < texture_x_1)
      texture_x_2 += 1.0;
    else
      texture_x_1 += 1.0;
    }
  
  if(fabs(texture_y_2 < texture_y_1) > 0.5)
    {
    if(texture_y_2 < texture_y_1)
      texture_y_2 += 1.0;
    else
      texture_y_1 += 1.0;
    }

  
  glBegin(GL_QUAD_STRIP);

  glNormal3fv(r_2->normals[0]);
  glTexCoord2f(texture_x_2, texture_y_2);
  glVertex3fv(r_2->coords[0]);

  glNormal3fv(r_1->normals[0]);
  glTexCoord2f(texture_x_1, texture_y_1);
  glVertex3fv(r_1->coords[0]);
  
  for(i = 1; i < PHI_STEPS; i++)
    {
    texture_x_1 += texture_delta_x;
    texture_x_2 += texture_delta_x;
    
    glNormal3fv(r_2->normals[i]);
    glTexCoord2f(texture_x_2, texture_y_2);
    glVertex3fv(r_2->coords[i]);
    
    glNormal3fv(r_1->normals[i]);
    glTexCoord2f(texture_x_1, texture_y_1);
    glVertex3fv(r_1->coords[i]);

    }
  glEnd();
  }

static void init_curvature(lemuria_engine_t * e, tunnel_data * d)
  {
  float * tmp;
  float next_delta, next_phi, next_r;
  float tmp_1[2];
  
  if(!d->current_curvature)
    {
    d->current_curvature = d->curvature_1;
    d->next_curvature = d->curvature_2;
    d->current_curvature[0] = 0.0;
    d->current_curvature[1] = 0.0;
    }
  else
    {
    tmp = d->next_curvature;
    d->next_curvature = d->current_curvature;
    d->current_curvature = tmp;
    }

  if(e->quiet)
    {
    /* Curvatures are already swapped, but we need the one from before */
    
    lemuria_range_get(&(d->curvature),
                      d->next_curvature,
                      d->current_curvature,
                      tmp_1);
    d->current_curvature[0] = tmp_1[0];
    d->current_curvature[1] = tmp_1[1];
    d->next_curvature[CURV_X_INDEX] = 0.0;
    d->next_curvature[CURV_Y_INDEX] = 0.0;
    }
  else if(((d->current_curvature[CURV_X_INDEX] != 0.0) ||
           (d->current_curvature[CURV_Y_INDEX] != 0.0)) &&
          lemuria_decide(e, 0.1))
    {
    d->next_curvature[CURV_X_INDEX] = 0.0;
    d->next_curvature[CURV_Y_INDEX] = 0.0;
    }
  else
    {
    next_delta = lemuria_random(e, 0.0, 0.0005);
    next_phi = lemuria_random(e, 0.0, 2.0* M_PI);
    next_r = sin(next_delta);
    d->next_curvature[CURV_X_INDEX] = next_r * cos(next_phi);
    d->next_curvature[CURV_Y_INDEX] = next_r * sin(next_phi);
    }
  lemuria_range_init(e, &(d->curvature), 2, 100, 300);
  
  }

static void create_ring(lemuria_engine_t * e, tunnel_data * t, ring_data * r)
  {
  float curv[2];

  float phi;
  
  lemuria_range_get(&(t->curvature), t->current_curvature, t->next_curvature,
                    curv);

  phi = atan2(curv[CURV_Y_INDEX], curv[CURV_X_INDEX]);
  
  r->curvature_plane_x = sin(phi);
  r->curvature_plane_y = cos(phi);
  r->delta_angle = asin(sqrt(curv[CURV_X_INDEX]*curv[CURV_X_INDEX]+
                             curv[CURV_Y_INDEX]*curv[CURV_Y_INDEX]));;
  r->texture_x = t->texture_x_start;
  r->do_draw = 1;
  }


#include "gradients/lemuria_tunnel_1.incl"
#include "gradients/lemuria_tunnel_2.incl"
#include "gradients/lemuria_tunnel_3.incl"
#include "gradients/lemuria_monolith_1.incl"

static lemuria_background_data_t tunnel_backgrounds[] =
  {
    {
      .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
      .clouds_gradient = gradient_lemuria_monolith_1,
      .clouds_size = 0
    },
    {
      .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
      .clouds_gradient = gradient_lemuria_tunnel_1,
      .clouds_size = 0
    },
    {
      .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
      .clouds_gradient = gradient_lemuria_tunnel_2,
      .clouds_size = 0
    },
    {
      .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
      .clouds_gradient = gradient_lemuria_tunnel_3,
      .clouds_size = 0
    },
    {
      .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
      .clouds_gradient = gradient_lemuria_tunnel_1,
      .clouds_size = 1
    },
    {
      .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
      .clouds_gradient = gradient_lemuria_tunnel_2,
      .clouds_size = 1
    },
    {
      .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
      .clouds_gradient = gradient_lemuria_tunnel_3,
      .clouds_size = 1
    }
  };

static int num_backgrounds =
sizeof(tunnel_backgrounds)/sizeof(tunnel_backgrounds[0]);

static lemuria_background_data_t goom_background =
  {
    .texture_mode =    LEMURIA_TEXTURE_GOOM,
    .clouds_gradient = (uint8_t)0,
    .clouds_size =     0
  };

static lemuria_background_data_t lemuria_background =
  {
    .texture_mode =    LEMURIA_TEXTURE_LEMURIA,
    .clouds_gradient = (uint8_t*)0,
    .clouds_size = 0
  };

static void * init_tunnel(lemuria_engine_t * e)
  {
  int i;
  int random_number;
  
  tunnel_data * d;

#ifdef DEBUG
  fprintf(stderr, "init_tunnel...");
#endif

  d = calloc(1, sizeof(tunnel_data));
  d->rings_to_draw = NUM_RINGS-1;
  /* Initialize ring coordinates */

  for(i = 0; i <= PHI_STEPS; i++)
    {
    d->ring_coords[i][0] =
      TUNNEL_RADIUS * cos(2.0*M_PI*i/(float)(PHI_STEPS-1));
    d->ring_coords[i][1] =
      TUNNEL_RADIUS * sin(2.0*M_PI*i/(float)(PHI_STEPS-1));

    d->ring_normals[i][0] =
      - cos(2.0*M_PI*i/(float)(PHI_STEPS-1));
    d->ring_normals[i][1] =
      - sin(2.0*M_PI*i/(float)(PHI_STEPS-1));
    }

  /* Initialize curvature */

  init_curvature(e, d);
    
    
  for(i = 0; i < NUM_RINGS - 1; i++)
    {
    d->rings[i].next = &(d->rings[i+1]);
    create_ring(e, d, &d->rings[i]);
    }
  
  create_ring(e, d, &d->rings[NUM_RINGS - 1]);
  d->rings[NUM_RINGS - 1].next = &(d->rings[0]);

  d->start_ring = &(d->rings[0]);

  /* Create the texture */

  // d->texture_mode = lemuria_random_int(0, 1);


  
  if(e->goom)
    random_number = lemuria_random_int(e, 0, 2);
  else
    random_number = lemuria_random_int(e, 0, 1);

  switch(random_number)
    {
    case 0:
      random_number = lemuria_random_int(e, 0, num_backgrounds-1);
      lemuria_background_init(e, &(d->background),
                              &tunnel_backgrounds[random_number]);
      d->speed = 5.0;
      break;
    case 1:
      lemuria_background_init(e, &(d->background), &lemuria_background);
      d->speed = 1.5;
      break;
    case 2:
      lemuria_background_init(e, &(d->background), &goom_background);
      d->speed = 1.5;
      break;
    }
  
  d->texture_y_start = 0.0;
  d->texture_x_start = 0.0;

  d->engine = e; 

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif

  return d;
  }

static void init_matrix(float matrix[3][3])
  {
  matrix[0][0] = 1.0;
  matrix[0][1] = 0.0;
  matrix[0][2] = 0.0;

  matrix[1][0] = 0.0;
  matrix[1][1] = 1.0;
  matrix[1][2] = 0.0;

  matrix[2][0] = 0.0;
  matrix[2][1] = 0.0;
  matrix[2][2] = 1.0;
  }

#define x axis[0]
#define y axis[1]
#define z axis[2]

static void rotate_matrix(float angle, float axis[3],
                          float old_matrix[3][3],
                          float new_matrix[3][3])
  {
  float s;
  float c;
  float tmp_matrix[3][3];
 
  s = sin(angle);
  c = cos(angle);

  /* From the manual page of glRotate (did not verify this) */

  tmp_matrix[0][0] = x*x*(1-c)+c;
  tmp_matrix[0][1] = x*y*(1-c)-z*s;
  tmp_matrix[0][2] = x*z*(1-c)+y*s;

  tmp_matrix[1][0] = y*x*(1-c)+z*s;
  tmp_matrix[1][1] = y*y*(1-c)+c;
  tmp_matrix[1][2] = y*z*(1-c)-x*s;

  tmp_matrix[2][0] = x*z*(1-c)-y*s;
  tmp_matrix[2][1] = y*z*(1-c)+x*s;
  tmp_matrix[2][2] = z*z*(1-c)+c;

  /* Multiply the matrix (manually unrolled) */

  /* First row */
  
  new_matrix[0][0] =
    old_matrix[0][0] * tmp_matrix[0][0] +
    old_matrix[0][1] * tmp_matrix[1][0] +
    old_matrix[0][2] * tmp_matrix[2][0];

  new_matrix[0][1] =
    old_matrix[0][0] * tmp_matrix[0][1] +
    old_matrix[0][1] * tmp_matrix[1][1] +
    old_matrix[0][2] * tmp_matrix[2][1];
  
  new_matrix[0][2] =
    old_matrix[0][0] * tmp_matrix[0][2] +
    old_matrix[0][1] * tmp_matrix[1][2] +
    old_matrix[0][2] * tmp_matrix[2][2];

  /* Second row */

  new_matrix[1][0] =
    old_matrix[1][0] * tmp_matrix[0][0] +
    old_matrix[1][1] * tmp_matrix[1][0] +
    old_matrix[1][2] * tmp_matrix[2][0];

  new_matrix[1][1] =
    old_matrix[1][0] * tmp_matrix[0][1] +
    old_matrix[1][1] * tmp_matrix[1][1] +
    old_matrix[1][2] * tmp_matrix[2][1];

  new_matrix[1][2] =
    old_matrix[1][0] * tmp_matrix[0][2] +
    old_matrix[1][1] * tmp_matrix[1][2] +
    old_matrix[1][2] * tmp_matrix[2][2];

  /* Third row */
  
  new_matrix[2][0] =
    old_matrix[2][0] * tmp_matrix[0][0] +
    old_matrix[2][1] * tmp_matrix[1][0] +
    old_matrix[2][2] * tmp_matrix[2][0];

  new_matrix[2][1] =
    old_matrix[2][0] * tmp_matrix[0][1] +
    old_matrix[2][1] * tmp_matrix[1][1] +
    old_matrix[2][2] * tmp_matrix[2][1];

  new_matrix[2][2] =
    old_matrix[2][0] * tmp_matrix[0][2] +
    old_matrix[2][1] * tmp_matrix[1][2] +
    old_matrix[2][2] * tmp_matrix[2][2];
  }

#undef x
#undef y
#undef z

static void transform(float matrix[3][3], float point[3])
  {
  float tmp[3];

  tmp[0] =
    matrix[0][0] * point[0] +
    matrix[0][1] * point[1] +
    matrix[0][2] * point[2];

  tmp[1] =
    matrix[1][0] * point[0] +
    matrix[1][1] * point[1] +
    matrix[1][2] * point[2];

  tmp[2] =
    matrix[2][0] * point[0] +
    matrix[2][1] * point[1] +
    matrix[2][2] * point[2];

  point[0] = tmp[0];
  point[1] = tmp[1];
  point[2] = tmp[2];
  }

#define TEXTURE_Y_PER_RING 0.005

static float fog_color[4] = { 0.9, 0.9, 1.0, 1.0 };

static void draw_tunnel(lemuria_engine_t * e, void * user_data)
  {
  int i, j;

  float matrices[2][3][3];
  float midpoint[3];

  float rotation_axis[3];
  
  int matrix_index = 0;

  float z_factor;
  float texture_y;

  float color[4];
  
  tunnel_data * data = (tunnel_data*)(user_data);

  ring_data * r;

  if(e->background.mode == EFFECT_FINISH)
    e->background.mode = EFFECT_FINISHING;

  /* Advance */

  lemuria_range_update(&(data->curvature));
  
  if((lemuria_range_done(&(data->curvature)),
      e->beat_detected && lemuria_decide(e, 0.1)) || (e->quiet))
    init_curvature(e, data);
  
  data->z_start += data->speed;

  if(e->background.mode == EFFECT_RUNNING)
    {
    while(data->z_start > TUNNEL_DELTA_Z)
      {
      data->z_start -= TUNNEL_DELTA_Z;
      create_ring(e, data, data->start_ring);
      data->start_ring = data->start_ring->next;
      }
    }
  else
    {
    while(data->z_start > TUNNEL_DELTA_Z)
      {
      data->z_start -= TUNNEL_DELTA_Z;
      data->start_ring->do_draw = 0;
      data->rings_to_draw--;
      data->start_ring = data->start_ring->next;
      }
    }
  
  data->texture_y_start += data->speed * TEXTURE_Y_PER_RING / TUNNEL_DELTA_Z;
  
  if(data->texture_y_start > 1.0)
    data->texture_y_start -= 1.0;

  texture_y = data->texture_y_start;
  
  /* Update rings */
  
  r = data->start_ring;

  r->texture_y = texture_y;
  r->texture_x += 0.01;
  if(r->texture_x > 1.0)
    r->texture_x -= 1.0;
  
  init_matrix(matrices[0]);

  midpoint[0] = 0.0;
  midpoint[1] = 0.0;
  midpoint[2] = data->z_start;

  z_factor = 1.0 - data->z_start / TUNNEL_DELTA_Z;

  r->texture_x = 0.0;
  r->texture_y = texture_y;
  
  rotation_axis[2] = 0.0;

  rotation_axis[0] =
    z_factor * r->curvature_plane_x +
    (1.0 - z_factor) * r->next->curvature_plane_x;

  rotation_axis[1] =
    z_factor * r->curvature_plane_y +
    (1.0 - z_factor) * r->next->curvature_plane_y;
  
  rotate_matrix(z_factor * r->delta_angle,
                rotation_axis,
                matrices[matrix_index],
                matrices[!matrix_index]);

  r = r->next;
  
  transform(matrices[!matrix_index], midpoint);
  
  for(j = 0; j <= PHI_STEPS; j++)
    {
    r->coords[j][0] = data->ring_coords[j][0];
    r->coords[j][1] = data->ring_coords[j][1];
    r->coords[j][2] = 0.0;

    r->normals[j][0] = data->ring_normals[j][0];
    r->normals[j][1] = data->ring_normals[j][1];
    r->normals[j][2] = 0.0;
    
    transform(matrices[!matrix_index], r->coords[j]);
    transform(matrices[!matrix_index], r->normals[j]);
    
    r->coords[j][0] += midpoint[0];
    r->coords[j][1] += midpoint[1];
    r->coords[j][2] += midpoint[2];
    }

  midpoint[2] -= TUNNEL_DELTA_Z;
  matrix_index = !matrix_index;
  r = r->next;

  if(data->background.config->texture_mode != LEMURIA_TEXTURE_CLOUDS)
    {
    data->texture_x_start += 0.001;
    if(data->texture_x_start > 1.0)
      data->texture_x_start -= 1.0;
    }

  
  for(i = 0; i < NUM_RINGS-2; i++)
    {
    rotation_axis[0] = r->curvature_plane_x;
    rotation_axis[1] = r->curvature_plane_y;
    
    rotate_matrix(r->delta_angle, rotation_axis,
                  matrices[matrix_index],
                  matrices[!matrix_index]);

    transform(matrices[!matrix_index], midpoint);

    texture_y += TEXTURE_Y_PER_RING;
    r->texture_y = texture_y;
    
    for(j = 0; j <= PHI_STEPS; j++)
      {
      r->coords[j][0] = data->ring_coords[j][0];
      r->coords[j][1] = data->ring_coords[j][1];
      r->coords[j][2] = 0.0;

      r->normals[j][0] = data->ring_normals[j][0];
      r->normals[j][1] = data->ring_normals[j][1];
      r->normals[j][2] = 0.0;
      
      transform(matrices[!matrix_index], r->coords[j]);
      transform(matrices[!matrix_index], r->normals[j]);

      r->coords[j][0] += midpoint[0];
      r->coords[j][1] += midpoint[1];
      r->coords[j][2] += midpoint[2];
      }
    midpoint[2] -= TUNNEL_DELTA_Z;
    matrix_index = !matrix_index;
    r = r->next;
    }
    
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //  glLineWidth(1.0);

  glShadeModel(GL_SMOOTH);

  // Set up fog

  //  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  if(e->background.mode == EFFECT_FINISHING)
    {
    
    glFogf(GL_FOG_START, 0.5 * ((data->rings_to_draw+1)*TUNNEL_DELTA_Z -
                                data->z_start));
    glFogf(GL_FOG_END, 0.7 * ((data->rings_to_draw+1)*TUNNEL_DELTA_Z -
                                data->z_start));
    }
  else
    {
    glFogf(GL_FOG_START, 0.5 * NUM_RINGS * TUNNEL_DELTA_Z);
    glFogf(GL_FOG_END,   0.7 * NUM_RINGS * TUNNEL_DELTA_Z);
    }
  
  glFogi(GL_FOG_MODE, GL_LINEAR );
  glFogfv(GL_FOG_COLOR, fog_color);
    
  glEnable(GL_FOG);
  glEnable(GL_CULL_FACE);

  glClearColor(fog_color[0], fog_color[1],
               fog_color[2], fog_color[3]);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // Enable lighting
  //  glEnable(GL_LIGHTING);

  glEnable(GL_TEXTURE_2D);

  lemuria_background_set(&(data->background));

  switch(data->background.config->texture_mode)
    {
    case LEMURIA_TEXTURE_CLOUDS:
      glColor4f(1.0, 1.0, 1.0, 1.0);
      //      fprintf(stderr, "Texture data: %p\n", data->texture_data);
      //      lemuria_set_material(&material, GL_FRONT);
      break;
    case LEMURIA_TEXTURE_GOOM:
      //      lemuria_set_material(&material, GL_FRONT);
      glColor4f(1.0, 1.0, 1.0, 1.0);
      break;
    case LEMURIA_TEXTURE_LEMURIA:
      
      //      glMaterialfv(GL_FRONT, GL_SPECULAR, mat->ref_specular);

      color[0] = 1.0;
      color[1] = 1.0;
      color[2] = 1.0;
      color[3] = 1.0;
      
      glMaterialfv(GL_FRONT, GL_AMBIENT,  color);
      glMaterialfv(GL_FRONT, GL_DIFFUSE,  color);
      
      glColor3fv(color);
      glEnable(GL_LIGHTING);
      lemuria_set_light(&light, GL_LIGHT0);
      glEnable(GL_LIGHT0);
    }
  
  lemuria_set_perspective(e, 1, 1000.0);

  r = data->start_ring;

  for(i = 0; i < NUM_RINGS-2; i++)
    {
    draw_ring(data, r, r->next);
    r = r->next;
    }
  
  glDisable(GL_TEXTURE_2D);
  //  glDisable(GL_LIGHTING);
  glDisable(GL_FOG);
  glDisable(GL_CULL_FACE);

  switch(data->background.config->texture_mode)
    {
    case LEMURIA_TEXTURE_CLOUDS:
      break;
    case LEMURIA_TEXTURE_LEMURIA:
      glDisable(GL_LIGHTING);
      break;
    }

  if((e->background.mode == EFFECT_FINISHING) &&
     (!data->start_ring->do_draw))
    e->background.mode = EFFECT_DONE;
  
  //  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  }

static void delete_tunnel(void * data)
  {
  tunnel_data * d = (tunnel_data*)(data);

  lemuria_background_delete(&(d->background));
  free(d);
  }

effect_plugin_t tunnel_effect =
  {
    .init =    init_tunnel,
    .draw =    draw_tunnel,
    .cleanup = delete_tunnel,
  };
