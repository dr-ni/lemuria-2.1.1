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

#include "images/archaic_desert.incl"
#include "images/gates_floor.incl"
#include "images/mystic_floor.incl"

#include "gradients/lemuria_archaic_1.incl"

#define SKY_X_MIN -500.0
#define SKY_X_MAX  500.0

#define Z_MIN -600.0

typedef struct
  {
  float fog_color[4];
  
  int texture_size_x;
  int texture_size_y;
  float texture_advance;

  lemuria_background_data_t background;
  } ceiling_data;

typedef struct 
  {
  lemuria_material_t material;
    
  lemuria_light_t light;


  float delta_z;
    
  float ceil_coords[4][3];
  
  float eye_height;

  // Textures are RGB24
  
  int floor_texture_width;
  int floor_texture_height;
  char * floor_texture_data;

  float floor_texture_x;
  float floor_texture_y;

  float fog_density;
  
  void (*draw_func)(float z);
  } object_data;

typedef struct archaic_data_s
  {
  object_data * obj;
  ceiling_data * ceil;

  unsigned int archaic_list;
  float archaic_delta_z;
  
  float speed;

  float start;

  float floor_texture_start;

  
  //  void (*call_list_func)(struct monolith_data_s*);

  unsigned int world_texture;
  int ceiling_texture_flip;
  float ceiling_texture_start;

  lemuria_background_t background;
  } archaic_data;

static void get_normal(float ** vertices, float * normal)
  {
  float diff_1[3];
  float diff_2[3];

  float norm;
  
  diff_1[0] = vertices[1][0] - vertices[0][0];
  diff_1[1] = vertices[1][1] - vertices[0][1];
  diff_1[2] = vertices[1][2] - vertices[0][2];

  diff_2[0] = vertices[2][0] - vertices[0][0];
  diff_2[1] = vertices[2][1] - vertices[0][1];
  diff_2[2] = vertices[2][2] - vertices[0][2];

  normal[0] =   diff_1[1] * diff_2[2] - diff_1[2] * diff_2[1];

  normal[1] = - diff_1[0] * diff_2[2] + diff_1[2] * diff_2[0];

  normal[2] =   diff_1[0] * diff_2[1] - diff_1[1] * diff_2[0];

  norm = sqrt(normal[0] * normal[0] +
              normal[1] * normal[1] +
              normal[2] * normal[2]);

  normal[0] /= norm;
  normal[1] /= norm;
  normal[2] /= norm;
  }

// Obelisk dimensions

#define OBELISK_WIDTH_1  1.2
#define OBELISK_WIDTH_2  0.8

#define OBELISK_HEIGHT_1 10.0
#define OBELISK_HEIGHT_2 0.8

#define OBELISK_OFFSET   5.0

#define OBELISK_EYE_HEIGHT 3.0

static void draw_obelisk(float z)
  {
  float vertices[4][3];
  float normal[3];

  float * vertices_ptr[4];
  vertices_ptr[0] = vertices[0];
  vertices_ptr[1] = vertices[1];
  vertices_ptr[2] = vertices[2];
  vertices_ptr[3] = vertices[3];
    
  /* Front face */

  vertices[0][0] = OBELISK_OFFSET - OBELISK_WIDTH_1 * 0.5;
  vertices[0][1] = -OBELISK_EYE_HEIGHT;
  vertices[0][2] = OBELISK_WIDTH_1 * 0.5 + z;

  vertices[1][0] = OBELISK_OFFSET + OBELISK_WIDTH_1 * 0.5;
  vertices[1][1] = -OBELISK_EYE_HEIGHT;
  vertices[1][2] = OBELISK_WIDTH_1 * 0.5 + z;

  vertices[2][0] = OBELISK_OFFSET + OBELISK_WIDTH_2 * 0.5;
  vertices[2][1] = -OBELISK_EYE_HEIGHT + OBELISK_HEIGHT_1;
  vertices[2][2] = OBELISK_WIDTH_2 * 0.5 + z;

  vertices[3][0] = OBELISK_OFFSET - OBELISK_WIDTH_2 * 0.5;
  vertices[3][1] = -OBELISK_EYE_HEIGHT + OBELISK_HEIGHT_1;
  vertices[3][2] = OBELISK_WIDTH_2 * 0.5 + z;

  get_normal(vertices_ptr, normal);

  glBegin(GL_QUADS);
  
  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);

  /* Invert x for the other side */

  vertices[0][0] *= -1.0;
  vertices[1][0] *= -1.0;
  vertices[2][0] *= -1.0;
  vertices[3][0] *= -1.0;
  normal[0] *= -1.0;

  
  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);

  /* Next Quad */

  vertices[0][0] = OBELISK_OFFSET - OBELISK_WIDTH_1 * 0.5;
  vertices[0][1] = -OBELISK_EYE_HEIGHT;
  vertices[0][2] = - OBELISK_WIDTH_1 * 0.5 + z;

  vertices[1][0] = OBELISK_OFFSET - OBELISK_WIDTH_1 * 0.5;
  vertices[1][1] = -OBELISK_EYE_HEIGHT;
  vertices[1][2] = + OBELISK_WIDTH_1 * 0.5 + z;

  vertices[2][0] = OBELISK_OFFSET - OBELISK_WIDTH_2 * 0.5;
  vertices[2][1] = -OBELISK_EYE_HEIGHT + OBELISK_HEIGHT_1;
  vertices[2][2] = + OBELISK_WIDTH_2 * 0.5 + z;

  vertices[3][0] = OBELISK_OFFSET - OBELISK_WIDTH_2 * 0.5;
  vertices[3][1] = -OBELISK_EYE_HEIGHT + OBELISK_HEIGHT_1;
  vertices[3][2] = - OBELISK_WIDTH_2 * 0.5 + z;

  get_normal(vertices_ptr, normal);

  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);

  /* Invert x for the other side */

  vertices[0][0] *= -1.0;
  vertices[1][0] *= -1.0;
  vertices[2][0] *= -1.0;
  vertices[3][0] *= -1.0;
  normal[0] *= -1.0;
  
  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
    
  glEnd();

  /* Make the top pyramids */
  
  vertices[0][0] = OBELISK_OFFSET - OBELISK_WIDTH_2 * 0.5;
  vertices[0][1] = -OBELISK_EYE_HEIGHT + OBELISK_HEIGHT_1;
  vertices[0][2] = OBELISK_WIDTH_2 * 0.5 + z;

  vertices[1][0] = OBELISK_OFFSET + OBELISK_WIDTH_2 * 0.5;
  vertices[1][1] = -OBELISK_EYE_HEIGHT + OBELISK_HEIGHT_1;
  vertices[1][2] = OBELISK_WIDTH_2 * 0.5 + z;
  
  vertices[2][0] = OBELISK_OFFSET;
  vertices[2][1] = -OBELISK_EYE_HEIGHT + OBELISK_HEIGHT_1 + OBELISK_HEIGHT_2;
  vertices[2][2] = z;

  get_normal(vertices_ptr, normal);
  
  glBegin(GL_TRIANGLES);

  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);

  /* Invert x for the other side */

  vertices[0][0] *= -1.0;
  vertices[1][0] *= -1.0;
  vertices[2][0] *= -1.0;
  normal[0] *= -1.0;

  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);

  /* Second triangle */

  vertices[0][0] = OBELISK_OFFSET - OBELISK_WIDTH_2 * 0.5;
  vertices[0][1] = -OBELISK_EYE_HEIGHT + OBELISK_HEIGHT_1;
  vertices[0][2] = -OBELISK_WIDTH_2 * 0.5 + z;

  vertices[1][0] = OBELISK_OFFSET - OBELISK_WIDTH_2 * 0.5;
  vertices[1][1] = -OBELISK_EYE_HEIGHT + OBELISK_HEIGHT_1;
  vertices[1][2] = +OBELISK_WIDTH_2 * 0.5 + z;
  
  vertices[2][0] = OBELISK_OFFSET;
  vertices[2][1] = -OBELISK_EYE_HEIGHT + OBELISK_HEIGHT_1 + OBELISK_HEIGHT_2;
  vertices[2][2] = z;

  
  get_normal(vertices_ptr, normal);
  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);

  vertices[0][0] *= -1.0;
  vertices[1][0] *= -1.0;
  vertices[2][0] *= -1.0;
  normal[0] *= -1.0;

  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  
  glEnd();
  
  }

static ceiling_data ceilings[] =
  {
    {
      .fog_color = { 0.8, 0.8, 1.0, 1.0 },
      //      .fog_color = { 0.0, 0.0, 0.0, 0.0 },
      .texture_size_x = 9,
      .texture_size_y = 3,
      .texture_advance = 0.005,
      .background =
      {
        .texture_mode = LEMURIA_TEXTURE_GOOM,
        .clouds_gradient = gradient_lemuria_archaic_1,
        .clouds_size = 0
      },
      
    },
    {
      .fog_color = { 0.47, 0.48, 0.78, 1.0 },
      //      .fog_color = { 0.0, 0.0, 0.0, 0.0 },
      .texture_size_x = 10,
      .texture_size_y = 2,
      .texture_advance = -0.005,
      .background =
      {
        .texture_mode = LEMURIA_TEXTURE_CLOUDS,
        .clouds_gradient = gradient_lemuria_archaic_1,
        .clouds_size = 0
                
      },
      
    },
  };

static int num_ceilings = sizeof(ceilings)/sizeof(ceilings[0]);

static object_data obelisk_data =
  {
  .material = {
    .ref_specular = { 0.0f, 0.0f, 0.0f, 0.0f },
    .ref_ambient =  { 0.2f, 0.2f, 0.2f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 128
  },
  
  .light = {
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .specular = { 0.0f, 0.0f, 0.0f, 0.0f },
    .position = { 0.0f, 0.0f, 0.0f, 1.0f }
  },
  

  .delta_z = 20.0,
  

  .ceil_coords =
  {
    { -SKY_X_MAX, -1.0, -600.0 },
    {  SKY_X_MAX, -1.0, -600.0 },
    {  SKY_X_MAX, 15.0, 0.0 },
    { -SKY_X_MAX, 15.0, 0.0 },
  },

  .eye_height = OBELISK_EYE_HEIGHT,

  .floor_texture_width = archaic_desert_width,
  .floor_texture_height = archaic_desert_height,
  .floor_texture_data = archaic_desert_data,

  .floor_texture_x = 20.0,
  .floor_texture_y = 10.0,
  .fog_density = 1.0/ 200.0,
  
  .draw_func = draw_obelisk
  };

/*
 *  Gates is the plural of gate and is not related to any
 *  living person :-)
 */

#define GATE_THICKNESS 1.0
#define GATE_HEIGHT_1  10.0
#define GATE_HEIGHT_2  12.0
#define GATE_WIDTH_1   10.0
#define GATE_WIDTH_2   14.0
#define GATE_EYE_HEIGHT 3.0

static void draw_gates(float z)
  {
  float vertices[4][3];
  float normal[3];

  /* Inner quads */

  vertices[0][0] =  GATE_WIDTH_1 * 0.5;
  vertices[0][1] = -GATE_EYE_HEIGHT;
  vertices[0][2] = -0.5 * GATE_THICKNESS + z;

  vertices[1][0] =  GATE_WIDTH_1 * 0.5;
  vertices[1][1] = -GATE_EYE_HEIGHT;
  vertices[1][2] =  0.5 * GATE_THICKNESS + z;

  vertices[2][0] =  GATE_WIDTH_1 * 0.5;
  vertices[2][1] = -GATE_EYE_HEIGHT + GATE_HEIGHT_1;
  vertices[2][2] =  0.5 * GATE_THICKNESS + z;

  vertices[3][0] =  GATE_WIDTH_1 * 0.5;
  vertices[3][1] = -GATE_EYE_HEIGHT + GATE_HEIGHT_1;
  vertices[3][2] = -0.5 * GATE_THICKNESS + z;

  normal[0] = -1.0;
  normal[1] = 0.0;
  normal[2] = 0.0;

  glBegin(GL_QUADS);
  
  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);

  /* Invert x for the other side */

  vertices[0][0] *= -1.0;
  vertices[1][0] *= -1.0;
  vertices[2][0] *= -1.0;
  vertices[3][0] *= -1.0;
  normal[0]      *= -1.0;
  
  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);

  /* Front faces */

  vertices[0][0] =  GATE_WIDTH_1 * 0.5;
  vertices[0][1] = -GATE_EYE_HEIGHT;
  vertices[0][2] =  0.5 * GATE_THICKNESS + z;

  vertices[1][0] =  GATE_WIDTH_2 * 0.5;
  vertices[1][1] = -GATE_EYE_HEIGHT;
  vertices[1][2] =  0.5 * GATE_THICKNESS + z;

  vertices[2][0] =  GATE_WIDTH_2 * 0.5;
  vertices[2][1] = -GATE_EYE_HEIGHT + GATE_HEIGHT_1;
  vertices[2][2] =  0.5 * GATE_THICKNESS + z;

  vertices[3][0] =  GATE_WIDTH_1 * 0.5;
  vertices[3][1] = -GATE_EYE_HEIGHT + GATE_HEIGHT_1;
  vertices[3][2] =  0.5 * GATE_THICKNESS + z;

  normal[0] = 0.0;
  normal[1] = 0.0;
  normal[2] = 1.0;
  
  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);

  /* Invert x for the other side */

  vertices[0][0] *= -1.0;
  vertices[1][0] *= -1.0;
  vertices[2][0] *= -1.0;
  vertices[3][0] *= -1.0;
    
  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);

  /* */ 

  vertices[0][0] = -GATE_WIDTH_1 * 0.5;
  vertices[0][1] = -GATE_EYE_HEIGHT + GATE_HEIGHT_1;
  vertices[0][2] =  -0.5 * GATE_THICKNESS + z;

  vertices[1][0] =  GATE_WIDTH_1 * 0.5;
  vertices[1][1] = -GATE_EYE_HEIGHT  + GATE_HEIGHT_1;
  vertices[1][2] =  -0.5 * GATE_THICKNESS + z;

  vertices[2][0] =  GATE_WIDTH_1 * 0.5;
  vertices[2][1] = -GATE_EYE_HEIGHT + GATE_HEIGHT_1;
  vertices[2][2] =  0.5 * GATE_THICKNESS + z;

  vertices[3][0] = -GATE_WIDTH_1 * 0.5;
  vertices[3][1] = -GATE_EYE_HEIGHT + GATE_HEIGHT_1;
  vertices[3][2] =  0.5 * GATE_THICKNESS + z;

  normal[0] = 0.0;
  normal[1] = -1.0;
  normal[2] = 0.0;

  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);
  
  /* */

  vertices[0][0] = -GATE_WIDTH_2 * 0.5;
  vertices[0][1] = -GATE_EYE_HEIGHT + GATE_HEIGHT_1;
  vertices[0][2] =  0.5 * GATE_THICKNESS + z;

  vertices[1][0] =  GATE_WIDTH_2 * 0.5;
  vertices[1][1] = -GATE_EYE_HEIGHT  + GATE_HEIGHT_1;
  vertices[1][2] =  0.5 * GATE_THICKNESS + z;

  vertices[2][0] =  GATE_WIDTH_2 * 0.5;
  vertices[2][1] = -GATE_EYE_HEIGHT + GATE_HEIGHT_2;
  vertices[2][2] =  0.5 * GATE_THICKNESS + z;

  vertices[3][0] = -GATE_WIDTH_2 * 0.5;
  vertices[3][1] = -GATE_EYE_HEIGHT + GATE_HEIGHT_2;
  vertices[3][2] =  0.5 * GATE_THICKNESS + z;

  normal[0] = 0.0;
  normal[1] = 0.0;
  normal[2] = 1.0;

  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);
  
  glEnd();
  }

static object_data gates_data =
  {
  .material = {
    .ref_specular = { 0.0f, 0.0f, 0.0f, 1.0f },
    .ref_ambient =  { 0.3f, 0.3f, 0.3f, 1.0f },
    .ref_diffuse =  { 0.3f, 0.3f, 0.3f, 1.0f },
    .shininess = 128
  },
  
  .light = {
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 0.0f, 0.0f, 0.0f, 1.0f },
    .position = { 0.0f, 0.0f, 0.0f, 1.0f }
  },
  

  .delta_z = 15.0,
  

  .ceil_coords =
  {
    { -SKY_X_MAX, 0.0, Z_MIN },
    {  SKY_X_MAX, 0.0, Z_MIN },
    {  SKY_X_MAX, 10.0, 0.0 },
    { -SKY_X_MAX, 10.0, 0.0 },
  },
  .eye_height = 3.0,

  .floor_texture_width =  gates_floor_width,
  .floor_texture_height = gates_floor_height,
  .floor_texture_data =   gates_floor_data,

  .floor_texture_x = 15.0,
  .floor_texture_y = 10.0,
  .fog_density = 1.0/ 200.0,
  
  .draw_func = draw_gates
  };

static void draw_quad_mirror(float vertices[4][3], float normal[3])
  {
  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);

  /* Other side */
  
  glNormal3f(-normal[0], normal[1], normal[2]);
  glVertex3f(-vertices[3][0], vertices[3][1], vertices[3][2]);
  glVertex3f(-vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(-vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(-vertices[0][0], vertices[0][1], vertices[0][2]);
  }

#if 0
static void draw_quad(float vertices[4][3], float normal[3])
  {
  glNormal3f(normal[0], normal[1], normal[2]);
  glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);
  glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);
  glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);
  glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);
  }
#endif
#define CYLINDER_PHI_STEPS 20

static void draw_cylinder(float radius, float y_min, float y_max,
                          float z, float x)
  {
  //  float phi_1 = 0.0;
  float phi_2;

  float vertices[4][3];
  float normal_1[3];
  float normal_2[3];

  int i;
  
  vertices[0][0] = x;
  vertices[0][1] = y_min;
  vertices[0][2] = z + radius;

  vertices[3][0] = vertices[0][0];
  vertices[3][1] = y_max;
  vertices[3][2] = vertices[0][2];

  vertices[1][1] = y_min;
  vertices[2][1] = y_max;

  normal_1[0] = 1.0;
  normal_1[1] = 0.0;
  normal_1[2] = 0.0;

  normal_2[1] = 0.0;

  glBegin(GL_QUADS);
  
  for(i = 0; i < CYLINDER_PHI_STEPS; i++)
    {
    phi_2 = (float)i * 2.0 * M_PI/ (float)(CYLINDER_PHI_STEPS-1);

    normal_2[0] = sin(phi_2);
    normal_2[2] = cos(phi_2);
    
    vertices[1][0] = x + radius * normal_2[0];
    vertices[1][2] = z + radius * normal_2[2];
    
    vertices[2][0] = vertices[1][0];
    vertices[2][2] = vertices[1][2];
    
    glNormal3f(normal_1[0], normal_1[1], normal_1[2]);
    glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);

    glNormal3f(normal_2[0], normal_2[1], normal_2[2]);
    glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);

    glNormal3f(normal_2[0], normal_2[1], normal_2[2]);
    glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);

    glNormal3f(normal_1[0], normal_1[1], normal_1[2]);
    glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);

    normal_1[0] = normal_2[0];
    normal_1[2] = normal_2[2];
    
    vertices[0][0] = vertices[1][0];
    vertices[0][2] = vertices[1][2];
    
    vertices[3][0] = vertices[2][0];
    vertices[3][2] = vertices[2][2];
    }

  glEnd();
  
  vertices[1][0] = x + radius;
  vertices[2][1] = 0.0;
  vertices[3][2] = y_min;
  }

#define MYSTIC_WIDTH_1 1.5
#define MYSTIC_WIDTH_2 1.0

#define MYSTIC_HEIGHT_1 2.0
#define MYSTIC_HEIGHT_2 10.0

#define MYSTIC_EYE_HEIGHT 5.0

#define MYSTIC_OFFSET 9.0

#define MYSTIC_PHI_STEPS 10

#define MYSTIC_CEILING_HEIGHT 15.0

#define MYSTIC_DELTA_Z 10.0

static void draw_mystic(float z)
  {
  float vertices[4][3];
  float normal[3];

  float z_arc[MYSTIC_PHI_STEPS+1];
  float y_arc[MYSTIC_PHI_STEPS+1];

  float z_n_arc[MYSTIC_PHI_STEPS+1];
  float y_n_arc[MYSTIC_PHI_STEPS+1];
    
  int i;
  
  /* Cylinders */

  draw_cylinder(0.5 * MYSTIC_WIDTH_2, -MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1,
                -MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1 + MYSTIC_HEIGHT_2, z,
                MYSTIC_OFFSET);

  draw_cylinder(0.5 * MYSTIC_WIDTH_2, -MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1,
                -MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1 + MYSTIC_HEIGHT_2, z,
                -MYSTIC_OFFSET);
  
  /* Lower block */

  glBegin(GL_QUADS);
  
  vertices[0][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[0][1] = - MYSTIC_EYE_HEIGHT;
  vertices[0][2] = z + MYSTIC_WIDTH_1 * 0.5;

  vertices[1][0] = MYSTIC_OFFSET + 0.5 * MYSTIC_WIDTH_1;
  vertices[1][1] = - MYSTIC_EYE_HEIGHT;
  vertices[1][2] = z + MYSTIC_WIDTH_1 * 0.5;

  vertices[2][0] = MYSTIC_OFFSET + 0.5 * MYSTIC_WIDTH_1;
  vertices[2][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1;
  vertices[2][2] = z + MYSTIC_WIDTH_1 * 0.5;

  vertices[3][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[3][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1;
  vertices[3][2] = z + MYSTIC_WIDTH_1 * 0.5;

  normal[0] = 0.0;
  normal[1] = 0.0;
  normal[2] = 1.0;

  draw_quad_mirror(vertices, normal);
  
  vertices[0][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[0][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1;
  vertices[0][2] = z + MYSTIC_WIDTH_1 * 0.5;

  vertices[1][0] = MYSTIC_OFFSET + 0.5 * MYSTIC_WIDTH_1;
  vertices[1][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1;
  vertices[1][2] = z + MYSTIC_WIDTH_1 * 0.5;

  vertices[2][0] = MYSTIC_OFFSET + 0.5 * MYSTIC_WIDTH_1;
  vertices[2][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1;
  vertices[2][2] = z - MYSTIC_WIDTH_1 * 0.5;

  vertices[3][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[3][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1;
  vertices[3][2] = z - MYSTIC_WIDTH_1 * 0.5;

  normal[0] = 0.0;
  normal[1] = 1.0;
  normal[2] = 0.0;
  
  draw_quad_mirror(vertices, normal);

  vertices[0][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[0][1] = - MYSTIC_EYE_HEIGHT;
  vertices[0][2] = z - MYSTIC_WIDTH_1 * 0.5;

  vertices[1][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[1][1] = - MYSTIC_EYE_HEIGHT;
  vertices[1][2] = z + MYSTIC_WIDTH_1 * 0.5;

  vertices[2][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[2][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1;
  vertices[2][2] = z + MYSTIC_WIDTH_1 * 0.5;

  vertices[3][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[3][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1;
  vertices[3][2] = z - MYSTIC_WIDTH_1 * 0.5;

  normal[0] = 0.0;
  normal[1] = 1.0;
  normal[2] = 0.0;
  
  draw_quad_mirror(vertices, normal);

  vertices[3][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[3][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1 + MYSTIC_HEIGHT_2;
  vertices[3][2] = z + MYSTIC_WIDTH_1 * 0.5;

  vertices[2][0] = MYSTIC_OFFSET + 0.5 * MYSTIC_WIDTH_1;
  vertices[2][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1 + MYSTIC_HEIGHT_2;
  vertices[2][2] = z + MYSTIC_WIDTH_1 * 0.5;

  vertices[1][0] = MYSTIC_OFFSET + 0.5 * MYSTIC_WIDTH_1;
  vertices[1][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1 + MYSTIC_HEIGHT_2;
  vertices[1][2] = z - MYSTIC_WIDTH_1 * 0.5;

  vertices[0][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[0][1] = - MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1 + MYSTIC_HEIGHT_2;
  vertices[0][2] = z - MYSTIC_WIDTH_1 * 0.5;

  normal[0] = 0.0;
  normal[1] = 1.0;
  normal[2] = 0.0;
  
  draw_quad_mirror(vertices, normal);

  for(i = 0; i <= MYSTIC_PHI_STEPS; i++)
    {
    z_n_arc[i] =   cos((float)i/(float)MYSTIC_PHI_STEPS * M_PI);
    y_n_arc[i] = - sin((float)i/(float)MYSTIC_PHI_STEPS * M_PI);
    
    z_arc[i] = z + 0.5 * MYSTIC_DELTA_Z -
      0.5 * (MYSTIC_DELTA_Z - MYSTIC_WIDTH_1) *
      cos((float)i/(float)MYSTIC_PHI_STEPS * M_PI);
    y_arc[i] = -MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1 + MYSTIC_HEIGHT_2 +
      0.5 * (MYSTIC_DELTA_Z - MYSTIC_WIDTH_1) *
      sin((float)i/(float)MYSTIC_PHI_STEPS * M_PI);
    }

  normal[0] = -1.0;
  normal[1] = 0.0;
  normal[2] = 0.0;
  
  for(i = 0; i < MYSTIC_PHI_STEPS; i++)
    {
    vertices[0][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
    vertices[0][1] = y_arc[i];
    vertices[0][2] = z_arc[i];
    
    vertices[1][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
    vertices[1][1] = y_arc[i+1];
    vertices[1][2] = z_arc[i+1];
    
    vertices[2][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
    vertices[2][1] = MYSTIC_CEILING_HEIGHT;
    vertices[2][2] = z_arc[i+1];
    
    vertices[3][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
    vertices[3][1] = MYSTIC_CEILING_HEIGHT;
    vertices[3][2] = z_arc[i];

    draw_quad_mirror(vertices, normal);
    }

  vertices[0][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[0][1] = -MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1 + MYSTIC_HEIGHT_2;
  vertices[0][2] = z - 0.5 * MYSTIC_WIDTH_1;

  vertices[1][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[1][1] = -MYSTIC_EYE_HEIGHT + MYSTIC_HEIGHT_1 + MYSTIC_HEIGHT_2;
  vertices[1][2] = z + 0.5 * MYSTIC_WIDTH_1;

  vertices[2][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[2][1] = MYSTIC_CEILING_HEIGHT;
  vertices[2][2] = z + 0.5 * MYSTIC_WIDTH_1;

  vertices[3][0] = MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1;
  vertices[3][1] = MYSTIC_CEILING_HEIGHT;
  vertices[3][2] = z - 0.5 * MYSTIC_WIDTH_1;

  draw_quad_mirror(vertices, normal);
 
  for(i = 0; i < MYSTIC_PHI_STEPS; i++)
    {
    glNormal3f(0.0, y_n_arc[i], z_n_arc[i]);

    glVertex3f(MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1, y_arc[i], z_arc[i]);
    glVertex3f(MYSTIC_OFFSET + 0.5 * MYSTIC_WIDTH_1, y_arc[i], z_arc[i]);

    glNormal3f(0.0, y_n_arc[i+1], z_n_arc[i+1]);

    glVertex3f(MYSTIC_OFFSET + 0.5 * MYSTIC_WIDTH_1, y_arc[i+1], z_arc[i+1]);
    glVertex3f(MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1, y_arc[i+1], z_arc[i+1]);

    glNormal3f(0.0, y_n_arc[i], z_n_arc[i]);

    glVertex3f(-MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1, y_arc[i], z_arc[i]);
    glVertex3f(-MYSTIC_OFFSET + 0.5 * MYSTIC_WIDTH_1, y_arc[i], z_arc[i]);

    glNormal3f(0.0, y_n_arc[i+1], z_n_arc[i+1]);

    glVertex3f(-MYSTIC_OFFSET + 0.5 * MYSTIC_WIDTH_1, y_arc[i+1], z_arc[i+1]);
    glVertex3f(-MYSTIC_OFFSET - 0.5 * MYSTIC_WIDTH_1, y_arc[i+1], z_arc[i+1]);
    }

  glEnd();
  }

static object_data mystic_data =
  {
  .material = {
    .ref_specular = { 0.0f, 0.0f, 0.0f, 1.0f },
    .ref_ambient =  { 0.2f, 0.2f, 0.2f, 1.0f },
    .ref_diffuse =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .shininess = 128
  },
  
  .light = {
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .specular = { 0.0f, 0.0f, 0.0f, 1.0f},
    .position = { 10.0f, 10.0f, 0.0f, 1.0f }
  },
  
  .delta_z = MYSTIC_DELTA_Z,
  

  .ceil_coords =
  {
    { -SKY_X_MAX, 15.0, Z_MIN },
    {  SKY_X_MAX, 15.0, Z_MIN },
    {  SKY_X_MAX, 15.0, 0.0 },
    { -SKY_X_MAX, 15.0, 0.0 },
  },
  .eye_height = MYSTIC_EYE_HEIGHT,

  .floor_texture_width =  mystic_floor_width,
  .floor_texture_height = mystic_floor_height,
  .floor_texture_data =   mystic_floor_data,

  .floor_texture_x = 200.0,
  .floor_texture_y = 200.0,
  
  .draw_func = draw_mystic
  };

static void draw_ceiling(archaic_data * data)
  {
  lemuria_background_set(&(data->background));
    
  lemuria_background_draw(&(data->background),
                          data->obj->ceil_coords,
                          data->ceil->texture_size_x,
                          data->ceil->texture_size_y,
                          &(data->ceiling_texture_start),
                          data->ceil->texture_advance,
                          &(data->ceiling_texture_flip));
  }

static void draw_floor(archaic_data * data)
  {
  if(data->obj->floor_texture_data)
    {
    glBindTexture(GL_TEXTURE_2D, data->world_texture);
    }
  else
    {
    glDisable(GL_TEXTURE_2D);
    }
 
  glBegin(GL_QUADS);
  //  glNormal3f(0.0, 1.0, 0.0);
  
  glTexCoord2f(0.0, data->floor_texture_start);
  glVertex3f(SKY_X_MIN, -data->obj->eye_height, 0.0);

  glTexCoord2f(data->obj->floor_texture_x,
               data->floor_texture_start);
  glVertex3f(SKY_X_MAX, -data->obj->eye_height, 0.0);

  glTexCoord2f(data->obj->floor_texture_x,
               data->obj->floor_texture_y + data->floor_texture_start);
  glVertex3f(SKY_X_MAX, -data->obj->eye_height, Z_MIN);

  glTexCoord2f(0.0, data->obj->floor_texture_y + data->floor_texture_start);
  glVertex3f(SKY_X_MIN, -data->obj->eye_height, Z_MIN);
  glEnd();
  }

static void draw_background(archaic_data* data)
  {
  //  glColor4f(1.0, 1.0, 1.0, 1.0);
  glShadeModel(GL_FLAT);
  
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
  glEnable(GL_TEXTURE_2D);

  // Draw Floor
    
  draw_ceiling(data);

  // Draw Ceiling
    
  draw_floor(data);
  
  glDisable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  }

static void draw_foreground(archaic_data* data)
  {
  lemuria_set_material(&data->obj->material, GL_FRONT);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);

  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0.0, 0.0, data->start);
  glCallList(data->archaic_list);
  glPopMatrix();
  glDisable(GL_LIGHTING);
  }

static void draw_archaic(lemuria_engine_t * e, void * user_data)
  {
  archaic_data * data = (archaic_data*)(user_data);

  data->start += data->speed;
  if(data->start > data->obj->delta_z)
    data->start -= data->obj->delta_z;

  data->floor_texture_start +=
    data->obj->floor_texture_y * data->speed / (-Z_MIN);
  if(data->floor_texture_start > 1.0)
    data->floor_texture_start -= 1.0;
      
  //  glEnable(GL_CULL_FACE);
  
  lemuria_set_perspective(e, 1, 1000.0);
  
  // Set up and enable light 0

  lemuria_set_light(&(data->obj->light), GL_LIGHT0);
    
  glEnable(GL_LIGHT0);

  glFogf(GL_FOG_DENSITY, data->obj->fog_density);
  glFogi(GL_FOG_MODE, GL_EXP );
  glFogfv(GL_FOG_COLOR, data->ceil->fog_color);
  
  glEnable(GL_FOG);

  if(e->antialias)
    {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
    //    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_POLYGON_SMOOTH);
    
    if(e->antialias > 1)
      glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    else
      glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

    draw_foreground(data);

    glDisable(GL_POLYGON_SMOOTH);

    draw_background(data);
    
    glDisable(GL_FOG);
    glColor4f(data->ceil->fog_color[0], data->ceil->fog_color[1],
              data->ceil->fog_color[2], data->ceil->fog_color[3]);
    
    glBegin(GL_QUADS);
    glVertex3f(-10.0, -10.0, 0.0);
    glVertex3f( 10.0, -10.0, 0.0);
    glVertex3f( 10.0,  10.0, 0.0);
    glVertex3f(-10.0,  10.0, 0.0);
    glEnd();
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    }
  else
    {
    glClearColor(data->ceil->fog_color[0], data->ceil->fog_color[1],
                 data->ceil->fog_color[2], data->ceil->fog_color[3]);
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    draw_background(data);
    glClear(GL_DEPTH_BUFFER_BIT);
    draw_foreground(data);
    glDisable(GL_FOG);
    
    }
    

  //  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  //  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  }

static void * init_archaic(lemuria_engine_t * e)
  {
  int i;
  int num_objects;
  float z;
  //  int result;
  archaic_data * data;
#ifdef DEBUG
  fprintf(stderr, "init_archaic...");
#endif

  data = calloc(1, sizeof(archaic_data));

  // mystic is currently disabled
  
  //  i = lemuria_random_int(0, 2);
  i = lemuria_random_int(e, 0, 1);
  //  i = 2;
  switch(i)
    {
    case 0:
      data->obj = &gates_data;
      break;
    case 1:
      data->obj = &obelisk_data;
      break;
    case 2:
      data->obj = &mystic_data;
      break;
    }

  if(e->goom)
    i = lemuria_random_int(e, 0, num_ceilings - 1);
  else
    i = lemuria_random_int(e, 1, num_ceilings - 1);
  //  i = 2;
  data->ceil = &ceilings[i];

  lemuria_background_init(e, &(data->background), &(data->ceil->background));
  
  
  data->floor_texture_start = 0.0;
  data->ceiling_texture_start = 0.0;

  // Create texture
                          
  if(data->obj->floor_texture_data)
    {
    glGenTextures(1, &(data->world_texture));
    glBindTexture(GL_TEXTURE_2D, data->world_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3,
                 data->obj->floor_texture_width,
                 data->obj->floor_texture_height,
                 0, GL_RGB, GL_UNSIGNED_BYTE,
                 data->obj->floor_texture_data);
    
    }

  // Set up the objects
  
  num_objects = -Z_MIN / data->obj->delta_z;
  
  data->speed  = 0.16;

  data->start = 0.0;
  
  data->archaic_list = glGenLists(1);
  glNewList(data->archaic_list, GL_COMPILE);

  z = 0.0;
  
  for(i = 0; i < num_objects; i++)
    {
    data->obj->draw_func(z);
    z -= data->obj->delta_z;
    }
  
  glEndList();

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  
  return data;
  }

static void delete_archaic(void * e)
  {
  archaic_data * data = (archaic_data*)(e);
  glDeleteLists(data->archaic_list, 1);

  if(data->obj->floor_texture_data)
    glDeleteTextures(1, &(data->world_texture));

  lemuria_background_delete(&(data->background));
  free(data);
  }

effect_plugin_t archaic_effect =
  {
    .init =    init_archaic,
    .draw =    draw_archaic,
    .cleanup = delete_archaic,
  };
