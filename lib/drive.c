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
#include "gradients/lemuria_drive_1.incl"
#include "gradients/lemuria_drive_2.incl"
 
#include <light.h>
#include <material.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


// #define DRAW_MESH


#define DELTA_Z      5.0
#define NUM_SEGMENTS 100

#define SEGMENT_STEPS  20
#define SEGMENT_RADIUS 10.0
#define CAMERA_HEIGHT  5.0

#define SPEED_MIN     1.5
#define SPEED_MAX     3.0

#define DELTA_PHI_MIN 0.001
#define DELTA_PHI_MAX DELTA_Z / (2 * M_PI * SEGMENT_RADIUS)

#define TEXTURE_DELTA_Y 0.02

#define NUM_LAST_ANGLES 3

#define SKY_RADIUS      (DELTA_Z*(float)NUM_SEGMENTS)
#define SKY_HEIGHT      (0.7*SKY_RADIUS)


#define SKY_PHI_STEPS   20
#define SKY_THETA_STEPS 10
#define SKY_THETA_START (30.0 / 180.0 * M_PI) 
#define SKY_THETA_END   (91.0 / 180.0 * M_PI) 

#define FOG_RADIUS (0.90 * SKY_RADIUS)

#define FOG_THETA_START (80.0 / 180.0 * M_PI)
#define FOG_THETA_END   (91.0 / 180.0 * M_PI)
#define FOG_CUTOFF_INDEX (SKY_THETA_STEPS-3)

#define FOG_START (DELTA_Z * NUM_SEGMENTS * 0.4)
#define FOG_END   (DELTA_Z * NUM_SEGMENTS * 0.95)

static float fog_color[3] = { 1.0, 1.0, 1.0 };

static lemuria_background_data_t bg_drive_1 =
  {
    .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
    .clouds_gradient = gradient_lemuria_drive_1,
    .clouds_size = 0
  };

static lemuria_background_data_t bg_drive_2 =
  {
    .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
    .clouds_gradient = gradient_lemuria_drive_2,
    .clouds_size = 0
  };

static lemuria_background_data_t sky_goom =
  {
    .texture_mode =    LEMURIA_TEXTURE_GOOM,
    .clouds_gradient = (uint8_t*)0,
    .clouds_size = 0
  };

static lemuria_background_data_t sky_lemuria =
  {
    .texture_mode = LEMURIA_TEXTURE_LEMURIA,
    (uint8_t *)0,
    clouds_size : 0,
  };
#if 0
static lemuria_background_data_t sky_xaos =
  {
    .texture_mode = LEMURIA_TEXTURE_XAOS,
    (uint8_t *)0,
    clouds_size : 0,
  };
#endif

#ifndef DRAW_MESH
static lemuria_light_t light_0 =
  {
    .ambient =  { 0.2f, 0.2f, 0.2f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .position = { 15.0f, 10.0f, 0.0f, 0.0f },
  };
static lemuria_material_t material =
  {
    .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_ambient =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 50
  };

#endif // DRAW_MESH


typedef struct
  {
  float delta_phi;
  float texture_y_before;
  float texture_y_after;
  } segment_data;

typedef struct
  {
  float delta_phi_start;
  float delta_phi_end;
  
  lemuria_range_t curvature_range;
  lemuria_range_t speed_range;
  float z_start;
  float speed_start;
  float speed_end;
  segment_data segments[NUM_SEGMENTS];
    
  float segment_coords[SEGMENT_STEPS][2];        /* Only x-y */
  float segment_normals[SEGMENT_STEPS][2];       /* Only x-y */

  float coords[NUM_SEGMENTS][SEGMENT_STEPS][2];  /* Only x-z */
  float normals[NUM_SEGMENTS][SEGMENT_STEPS][2]; /* Only x-z */
  
  int start_segment;
  int end_segment;

  lemuria_background_t drive_background;
  lemuria_background_t sky_background;
  float texture_y;
  //  float last_angles[NUM_LAST_ANGLES];

  float angle;
    
  //  float delta_phi_max 
  } drive_data;


static void update_segment_coords(float phi, float x, float z,
                                  drive_data * d,
                                  float coords[SEGMENT_STEPS][2],
                                  float normals[SEGMENT_STEPS][2])
  {
  int i;

  float cos_phi;
  float sin_phi;

  sin_phi = sin(phi);
  cos_phi = cos(phi);
  
  for(i = 0; i < SEGMENT_STEPS; i++)
    {
    /* X */
    coords[i][0] = d->segment_coords[i][0] * cos_phi + x;
    /* Z */
    coords[i][1] = z - sin_phi * d->segment_coords[i][0];
    /* X */
    normals[i][0] = d->segment_normals[i][0] * cos_phi;
    /* Z */
    normals[i][1] = - sin_phi * d->segment_normals[i][0];
    }
  }

/* We assume, that z_start < delta_z */

#define CURVATURE_RADIUS(d_phi) DELTA_Z/d_phi

static void update_coords(drive_data * d)
  {
  int index, i;
  float x;
  float z;
  float cos_phi;
  float sin_phi;
  
  float x_u, z_u;
  float x_t, z_t;
    
  float curvature_radius = 0;
  float phi;
  
  index = d->start_segment;
  
  phi = - d->z_start /
    DELTA_Z * d->segments[d->start_segment].delta_phi;

  cos_phi = cos(phi);
  sin_phi = sin(phi);

  if(d->segments[d->start_segment].delta_phi != 0.0)
    {
    curvature_radius =
      CURVATURE_RADIUS(d->segments[d->start_segment].delta_phi);
    z = -curvature_radius * sin_phi;
    x = curvature_radius * (1.0 - cos_phi);
    }
  else
    {
    x = 0;
    z = d->z_start;
    }

  //  fprintf(stderr, "Coords: %f %f\n", x, z);

  d->end_segment = (d->start_segment) ? d->start_segment-1 : NUM_SEGMENTS - 1;
  
  index = d->start_segment;
  for(i = 0; i < NUM_SEGMENTS; i++)
    {
    //    if(!i)fprintf(stderr, "Coords: %f %f\n", x, z);
    
    update_segment_coords(phi, x, z,
                          d,
                          d->coords[index],
                          d->normals[index]);

    if(d->segments[index].delta_phi != 0)
      {
      curvature_radius =
        CURVATURE_RADIUS(d->segments[index].delta_phi);
      x_u = curvature_radius * (1.0 -
                                cos(d->segments[index].delta_phi));
      
      z_u = curvature_radius * sin(d->segments[index].delta_phi);
      x_t =   cos_phi * x_u + sin_phi * z_u;
      z_t = - sin_phi * x_u + cos_phi * z_u;
      
      x += x_t;
      z -= z_t;
      }
    else
      {
      x += DELTA_Z * sin_phi;
      z -= DELTA_Z * cos_phi;
      }
        
    phi += d->segments[index].delta_phi;

    if(fabs(phi) > 60.0/ 180.0 * M_PI)
      {
      d->end_segment = index;
      break;
      }
    
    cos_phi = cos(phi);
    sin_phi = sin(phi);
    index++;
    if(index >= NUM_SEGMENTS)
      index = 0;
    }
  //  fprintf(stderr, "Phi: %f i: %d\n", 180.0 * phi / M_PI, i);
  
  
  }

#ifndef DRAW_MESH
static void draw_sky(drive_data * d,
                     float texture_x_start)
  {
  int i, j;
  float phi;
  float theta_2;

  float sin_phi;
  float cos_phi;
  float sin_theta_1;
  float cos_theta_1;

  float sin_theta_2;
  float cos_theta_2;
  float texture_x;
  float texture_y_1;
  float texture_y_2;

  /* Draw Sky */
  glEnable(GL_TEXTURE_2D);
  lemuria_background_set(&(d->sky_background));
  glColor3f(1.0, 1.0, 1.0);

  cos_theta_1 = cos(SKY_THETA_START);
  sin_theta_1 = sin(SKY_THETA_START);
  
  texture_y_1 = 1.0;

  glFogi(GL_FOG_MODE, GL_LINEAR );
  glFogfv(GL_FOG_COLOR, fog_color);
  glFogf(GL_FOG_START, 0.9 * SKY_RADIUS);
  glFogf(GL_FOG_END,   SKY_RADIUS);
  glEnable(GL_FOG);
  
  for(i = 0; i < SKY_THETA_STEPS; i++)
    {
    glBegin(GL_QUAD_STRIP);
    theta_2 = SKY_THETA_START + (SKY_THETA_END - SKY_THETA_START) *
      (float)i / (float)(SKY_THETA_STEPS-1);

    texture_y_2 = 1.0 - (float)i / (float)(SKY_THETA_STEPS-1);
    
    sin_theta_2 = sin(theta_2);
    cos_theta_2 = cos(theta_2);
    
    for(j = 0; j < SKY_PHI_STEPS; j++)
      {
      phi =  M_PI - M_PI * (float)j / (float)(SKY_PHI_STEPS-1);

      texture_x = texture_x_start + (float)j / (float)(SKY_PHI_STEPS-1);
              
      sin_phi = sin(phi);
      cos_phi = cos(phi);

      glTexCoord2f(texture_x, texture_y_1);
                         
      glVertex3f(SKY_RADIUS * cos_phi * sin_theta_1,
                 SKY_HEIGHT * cos_theta_1,
                 -SKY_RADIUS * sin_phi * sin_theta_1);

      glTexCoord2f(texture_x, texture_y_2);
      glVertex3f(SKY_RADIUS * cos_phi * sin_theta_2,
                 SKY_HEIGHT * cos_theta_2,
                 -SKY_RADIUS * sin_phi * sin_theta_2);
      }
    texture_y_1 = texture_y_2;
    sin_theta_1 = sin_theta_2;
    cos_theta_1 = cos_theta_2;
    glEnd();
    
    }
  glDisable(GL_TEXTURE_2D);
#if 0
  /* Draw Fog */

  cos_theta_1 = cos(FOG_THETA_START);
  sin_theta_1 = sin(FOG_THETA_START);
  
  alpha_1 = 0.0;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
   
  for(i = 0; i < SKY_THETA_STEPS; i++)
    {
    alpha_2 = (float)i / (float)(FOG_CUTOFF_INDEX);
    if(alpha_2 > 1.0)
      alpha_2 = 1.0;

    glBegin(GL_QUAD_STRIP);
    theta_2 = FOG_THETA_START + (FOG_THETA_END - FOG_THETA_START) *
      (float)i / (float)(SKY_THETA_STEPS-1);

    sin_theta_2 = sin(theta_2);
    cos_theta_2 = cos(theta_2);
    
    for(j = 0; j < SKY_PHI_STEPS; j++)
      {
      phi =  M_PI - M_PI * (float)j / (float)(SKY_PHI_STEPS-1);

      sin_phi = sin(phi);
      cos_phi = cos(phi);
      glColor4f(fog_color[0], fog_color[1], fog_color[2], alpha_1);
      glVertex3f(FOG_RADIUS * cos_phi * sin_theta_1,
                 FOG_RADIUS * cos_theta_1,
                 -FOG_RADIUS * sin_phi * sin_theta_1);

      glColor4f(fog_color[0], fog_color[1], fog_color[2], alpha_2);
      glVertex3f(FOG_RADIUS * cos_phi * sin_theta_2,
                 FOG_RADIUS * cos_theta_2,
                 -FOG_RADIUS * sin_phi * sin_theta_2);
      }
    sin_theta_1 = sin_theta_2;
    cos_theta_1 = cos_theta_2;
    alpha_1 = alpha_2;
    glEnd();
    
    }
  glEnable(GL_DEPTH_TEST);

  glDisable(GL_BLEND);
#endif
  }

#endif

#ifdef DRAW_MESH

static void
draw_segment(drive_data * d,
             int index_1, int index_2)
  {
  int i;

  if(d->segments[index_1].delta_phi == 0.0)
    glColor3f(1.0, 1.0, 0.0);
  else
    glColor3f(0.0, 1.0, 0.0);
  
  glLineWidth(1.0);
  
  glBegin(GL_LINE_STRIP);
  for(i = 0; i < SEGMENT_STEPS; i++)
    {
    glVertex3f(d->coords[index_1][i][0],
               d->segment_coords[i][1],
               d->coords[index_1][i][1]);
    }
  glEnd();

  glBegin(GL_LINES);
  for(i = 0; i < SEGMENT_STEPS; i++)
    {
    glVertex3f(d->coords[index_1][i][0],
               d->segment_coords[i][1],
               d->coords[index_1][i][1]);
    glVertex3f(d->coords[index_2][i][0],
               d->segment_coords[i][1],
               d->coords[index_2][i][1]);
    }
  glEnd();

  glColor3f(1.0, 0.0, 0.0);
  
  glBegin(GL_LINES);
  for(i = 0; i < SEGMENT_STEPS; i++)
    {
    glVertex3f(d->coords[index_1][i][0],
               d->segment_coords[i][1],
               d->coords[index_1][i][1]);
    glVertex3f(d->coords[index_1][i][0]+d->normals[index_1][i][0],
               d->segment_coords[i][1]+d->segment_normals[i][1],
               d->coords[index_1][i][1]+ d->normals[index_1][i][1]);
    }
  glEnd();
  
  }

#else

static void
draw_segment(drive_data * d,
             int index_1, int index_2)
  {
  int i;
  float texture_x;
  
  glBegin(GL_QUAD_STRIP);
  for(i = 0; i < SEGMENT_STEPS; i++)
    {
    texture_x = (float)i / (float)(SEGMENT_STEPS-1);

    glTexCoord2f(texture_x, d->segments[index_1].texture_y_after);
    glNormal3f(d->normals[index_1][i][0],
               d->segment_normals[i][1],
               d->normals[index_1][i][1]);
    
    glVertex3f(d->coords[index_1][i][0],
               d->segment_coords[i][1],
               d->coords[index_1][i][1]);

    glTexCoord2f(texture_x, d->segments[index_2].texture_y_before);
    glNormal3f(d->normals[index_2][i][0],
               d->segment_normals[i][1],
               d->normals[index_2][i][1]);
    
    glVertex3f(d->coords[index_2][i][0],
               d->segment_coords[i][1],
               d->coords[index_2][i][1]);
    }
  glEnd();
  
  }

#endif

static void draw_drive(lemuria_engine_t * e, void * user_data)
  {
  int index_1;
  int index_2;
  int i;
  float delta_phi;
  float angle;
  float speed;
  float speed_norm;
  
  drive_data * d = (drive_data*)(user_data);

  lemuria_range_update(&(d->curvature_range));
  lemuria_range_update(&(d->speed_range));

  /* Change stuff */

  lemuria_range_get(&(d->speed_range),
                    &(d->speed_start),
                    &(d->speed_end),
                    &speed);
  speed_norm = speed / SPEED_MAX;
  //  fprintf(stderr, "Speed: %f speed_max: %f (Speed/speed_max)^2: %f\n",
  //          speed, SPEED_MAX, speed_norm * speed_norm);
  
  //  fprintf(stderr, "Speed: %f %f\n", speed, SPEED_MAX);

  if(e->quiet)
    {
    d->speed_start = speed;
    d->speed_end = SPEED_MIN;
    lemuria_range_init(e, &(d->speed_range),
                       1, 100, 200);
    }
  else if(e->beat_detected)
    {
    if(lemuria_range_done(&(d->speed_range)) &&
       lemuria_decide(e, 0.2))
      {
      d->speed_start = d->speed_end;
      d->speed_end =
        SPEED_MIN + (lemuria_random(e, 0.0, 0.5) +
                     0.5 * (float)e->loudness/32768.0)*(SPEED_MAX - SPEED_MIN);
      lemuria_range_init(e, &(d->speed_range),
                         1, 100, 200);
      }
    
    if(lemuria_range_done(&(d->curvature_range)) &&
       lemuria_decide(e, 0.2))
      {
      d->delta_phi_start = d->delta_phi_end;

      //      if(d->delta_phi_end > 0.0)

      if(lemuria_decide(e, 0.3))
        d->delta_phi_end = 0.0;
      else
        {
        // d->delta_phi_end = lemuria_random(DELTA_PHI_MIN, DELTA_PHI_MAX);
        d->delta_phi_end =
          DELTA_PHI_MIN +
          (lemuria_random(e, 0.0, 0.5) +
           0.5 * (float)e->loudness / 32768.0)*(DELTA_PHI_MAX - DELTA_PHI_MIN);
        if(lemuria_decide(e, 0.5))
          d->delta_phi_end *= -1.0;
        }
      //  fprintf(stderr, "New phi: %f\n", d->delta_phi_end * 180.0 / M_PI);
        lemuria_range_init(e, &(d->curvature_range), 2, 50, 100);
      }
    }
  
  /* Create new rings if necessary */

  
  d->z_start += speed;

  lemuria_range_get(&(d->curvature_range),
                    &(d->delta_phi_start),
                    &(d->delta_phi_end),
                    &delta_phi);
  
  if(fabs(delta_phi) < DELTA_PHI_MIN)
    delta_phi = 0.0;
  
  while(d->z_start >= DELTA_Z)
    {
    d->segments[d->start_segment].delta_phi = delta_phi;
    d->texture_y += TEXTURE_DELTA_Y;
    d->segments[d->start_segment].texture_y_before = d->texture_y;
    if(d->texture_y > 1.0)
      d->texture_y -= 1.0;
    d->segments[d->start_segment].texture_y_after = d->texture_y;
    
    d->start_segment++;
    if(d->start_segment >= NUM_SEGMENTS)
      d->start_segment = 0;
    d->z_start -= DELTA_Z;
    }
  update_coords(d);
  
  /* Set up Opengl */

  glClearColor(0.0, 0.0, 0.0, 0.0);
  //  glDisable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  lemuria_set_perspective(e, 1, 1000.0);

  /* Set up light and materials */
#ifndef DRAW_MESH
  glShadeModel(GL_SMOOTH);
#endif
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  index_1 = d->start_segment;
  index_2 = index_1 + 1;
  if(index_2 >= NUM_SEGMENTS)
    index_2 = 0;
  
  /* Do curvature transformation */
  
  angle =
    atan2(20.0 * (speed_norm * speed_norm) * d->segments[d->start_segment].delta_phi,
          DELTA_PHI_MAX);

  /*  / (SPEED_MAX * SPEED_MAX) */
  
  
  d->angle = 0.8 * d->angle + 0.2 * angle;
  
  /* */
  
  glTranslatef(0.0, SEGMENT_RADIUS - CAMERA_HEIGHT, 0.0);

  glRotatef(d->angle * 180.0 / M_PI, 0.0, 0.0, 1.0);

  glTranslatef(0.0, CAMERA_HEIGHT - SEGMENT_RADIUS, 0.0);

#ifndef DRAW_MESH
  draw_sky(d, 0.0);
  glFogi(GL_FOG_MODE, GL_LINEAR );
  glFogfv(GL_FOG_COLOR, fog_color);
  glFogf(GL_FOG_START, FOG_START);
  glFogf(GL_FOG_END,   FOG_END);
  
  glEnable(GL_FOG);

  /* We set the light after we rotated stuff */
  lemuria_set_material(&material, GL_FRONT);
  glEnable(GL_LIGHTING);
  lemuria_background_set(&(d->drive_background));

  lemuria_set_light(&light_0, GL_LIGHT0);
  glEnable(GL_LIGHT0);
  glEnable(GL_TEXTURE_2D);
#endif

  //  lemuria_set_light(&light_1, GL_LIGHT1);
  //  glEnable(GL_LIGHT1);
  
  //  fprintf(stderr, "Segments: %d %d\n", d->start_segment, d->end_segment);
  
  for(i = 0; i < NUM_SEGMENTS-1; i++)
    {
    draw_segment(d, index_1, index_2);
    
    if(index_2 == d->end_segment)
      break;
    
    index_1++;
    if(index_1 >= NUM_SEGMENTS)
      index_1 = 0;

    index_2++;
    if(index_2 >= NUM_SEGMENTS)
      index_2 = 0;
    
    }
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHT1);
  glDisable(GL_FOG);
  
  }

static void init_coords(drive_data * d)
  {
  int i;
  float phi;
  
  for(i = 0; i < SEGMENT_STEPS; i++)
    {
    phi = - M_PI + (float)i/(float)(SEGMENT_STEPS-1) * M_PI;
    d->segment_coords[i][0] = SEGMENT_RADIUS * cos(phi);
    d->segment_coords[i][1] = SEGMENT_RADIUS * sin(phi) +
      SEGMENT_RADIUS - CAMERA_HEIGHT;
    d->segment_normals[i][0] = - cos(phi);
    d->segment_normals[i][1] = - sin(phi);
    }

  d->texture_y = 0.0;
  for(i = 0; i < NUM_SEGMENTS; i++)
    {

    d->texture_y += TEXTURE_DELTA_Y;
    d->segments[i].texture_y_before = d->texture_y;

    if(d->texture_y > 1.0)
      d->texture_y -= 1.0;
    d->segments[i].texture_y_after = d->texture_y;
    }
  }

static void * init_drive(lemuria_engine_t * e)
  {
  drive_data * d;
  int random_number;
  d = calloc(1, sizeof(*d));
  init_coords(d);

  d->delta_phi_start = 0.0;
  d->delta_phi_end   = 0.0;

  lemuria_range_init(e, &(d->curvature_range),
                     1, 10, 10);

  random_number = lemuria_random_int(e, 0, 1);

  //  random_number = 1;
  
  switch(random_number)
    {
    case 0:
      lemuria_background_init(e,
                              &(d->drive_background),
                              &(bg_drive_1));
      break;
    case 1:
      lemuria_background_init(e,
                              &(d->drive_background),
                              &(bg_drive_2));
      break;
    }
  

  if(e->goom)
    random_number = lemuria_random_int(e, 0, 1);
  else
    random_number = 1;
  
  //  random_number = 1;
  
  switch(random_number)
    {
    case 0:
      lemuria_background_init(e,
                              &(d->sky_background),
                              &(sky_goom));
      break;
    case 1:
      lemuria_background_init(e,
                              &(d->sky_background),
                              &(sky_lemuria));
      break;
    }
  
  d->speed_start = 0.0;
  d->speed_end = SPEED_MIN + (e->loudness / 32768.0)*(SPEED_MAX - SPEED_MIN);
  lemuria_range_init(e, &(d->speed_range),
                     1, 50, 60);
  
  return d;
  }

static void delete_drive(void * data)
  {
  drive_data * d = (drive_data*)(data);
  lemuria_background_delete(&(d->sky_background));
  lemuria_background_delete(&(d->drive_background));
  free(d);
  }


effect_plugin_t drive_effect =
  {
    .init =    init_drive,
    .draw =    draw_drive,
    .cleanup = delete_drive,
  };
