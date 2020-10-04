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

#define Z_STEPS   20
#define PHI_STEPS 50

#define MAX_NUM_VALUES               4
#define NUM_VALUES_TUBE_TORUS        3
#define NUM_VALUES_TUBE_SPHERE       3
#define NUM_VALUES_TUBE_TUBE_WRAPPED 4

static lemuria_light_t light =
  {
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .position = { 30.0f, 30.0f, 10.0f, 1.0f },
  };

static lemuria_material_t material =
  {
    .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_ambient =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
    .shininess = 50
  };

#define EXIT_ROTATE   (1<<0) // Rotation is ready
#define EXIT_DEFORM   (1<<1) // Deformation is ready
#define EXIT_DEFORM_2 (1<<2) // Another deformation is running

typedef struct tube_data_s
  {
  lemuria_rotator_t rotator;

  lemuria_scale_t scale_xy;
  lemuria_scale_t scale_z;
    
  void (*get_coords)(struct tube_data_s *, float,
                     float*, float*, float*, float*); 

  int flip_texture;
  float scale_start;
  float scale_end;
  
  lemuria_range_t deform_range;
  lemuria_range_t transition_range;

  float min_values[MAX_NUM_VALUES];
  float max_values[MAX_NUM_VALUES];
  float values[MAX_NUM_VALUES];
  
  int scale_duration;
  int scale_iteration;

  int exit_status;
  lemuria_engine_t * engine;
  } tube_data;

#define TUBE_RADIUS 0.5

#define TUBE_WRAPPED_RADIUS 0.7

#define TUBE_LENGTH 1.7

#define TORUS_LARGE_RADIUS 0.6
#define TORUS_SMALL_RADIUS 0.15

#define SPHERE_RADIUS 0.6

static int debug_now = 1;

static void get_coords_tube(struct tube_data_s * d, float z_norm,
                            float* r_ret, float* z_ret,
                            float * z_n_ret, float * r_n_ret)
  {
  *r_ret = TUBE_RADIUS;
  *z_ret = TUBE_LENGTH * z_norm - TUBE_LENGTH * 0.5;
  *z_n_ret = 0.0;
  *r_n_ret = 1.0;
  }

#if 0
static void get_coords_tube_wrapped(struct tube_data_s * d, float z_norm,
                                    float* r_ret, float* z_ret,
                                    float * z_n_ret, float * r_n_ret)
  {
  *r_ret = TUBE_WRAPPED_RADIUS;
  *z_ret = TUBE_LENGTH * 0.5 - TUBE_LENGTH * z_norm;
  *z_n_ret = 0.0;
  *r_n_ret = -1.0;
  }
static void coords_tube_tube_wrapped_init(struct tube_data_s * d)
  {
  float curved_way = fabs(TUBE_WRAPPED_RADIUS - TUBE_RADIUS) * M_PI;

  d->min_values[0] = 0.0;
  d->max_values[0] = (TUBE_LENGTH + curved_way)/curved_way;

  d->min_values[1] = - (curved_way * curved_way)/TUBE_LENGTH;
  d->max_values[1] = 1.0;

  d->max_values[2] = 0.0;
  d->min_values[2] = (TUBE_LENGTH + curved_way)/curved_way;

  d->max_values[3] = - (curved_way * curved_way)/TUBE_LENGTH;
  d->min_values[3] = 1.0;
  }
  
static void get_coords_tube_tube_wrapped(struct tube_data_s * d,
                                         float z_norm, float* r_ret,
                                         float* z_ret, float * z_n_ret,
                                         float * r_n_ret)
  {
  float norm = TUBE_LENGTH + M_PI * fabs(TUBE_WRAPPED_RADIUS -
                                         TUBE_RADIUS);
  
  
  if(d->values[0] < 1.0)
    {
    
    }
  else if(z_norm < TUBE_LENGTH/norm)
    {
    
    }
  else
    {
    
    }
    
  
  }

#endif

static void get_coords_torus(struct tube_data_s * d, float z_norm,
                             float* r_ret, float* z_ret,
                             float * z_n_ret, float * r_n_ret)
  {
  
  float poloidal_angle = -(z_norm) * M_PI * 2.0;
  
  float cos_angle = cos(poloidal_angle);
  float sin_angle = sin(poloidal_angle);
  
  *r_ret = TORUS_LARGE_RADIUS + cos_angle * TORUS_SMALL_RADIUS;
  *z_ret = TORUS_SMALL_RADIUS * sin_angle;

  *r_n_ret = - cos_angle;
  *z_n_ret = - sin_angle;
  }

static void coords_tube_torus_init(struct tube_data_s * d)
  {
  // Radius
  
  d->min_values[0] = TUBE_RADIUS;
  d->max_values[0] = TORUS_LARGE_RADIUS - TORUS_SMALL_RADIUS;

  // "Height" of the thing

  d->min_values[1] = TUBE_LENGTH;
  d->max_values[1] = TORUS_SMALL_RADIUS * 2 * M_PI;

  // Angle range

  d->min_values[2] = 0.0;
  d->max_values[2] = M_PI;
  }

static void coords_torus_tube_init(struct tube_data_s * d)
  {
  // Radius
  
  d->max_values[0] = TUBE_RADIUS;
  d->min_values[0] = TORUS_LARGE_RADIUS - TORUS_SMALL_RADIUS;

  // Curvature

  d->max_values[1] = TUBE_LENGTH;
  d->min_values[1] = TORUS_SMALL_RADIUS * 2 * M_PI;

  // Angle range

  d->max_values[2] = 0.0;
  d->min_values[2] = M_PI;
  }

static void get_coords_tube_torus(struct tube_data_s * d, float z_norm,
                                  float* r_ret, float* z_ret,
                                  float * z_n_ret, float * r_n_ret)
  {
  float curvature_radius;

  float angle;
  float sin_angle;
  float cos_angle;

  // Check if effect is over

  if(lemuria_range_done(&d->deform_range))
    {
    if(d->min_values[2] < d->max_values[2])
      d->get_coords = get_coords_torus;
    else
      d->get_coords = get_coords_tube;
    d->get_coords(d, z_norm, r_ret, z_ret, z_n_ret, r_n_ret);
    return;
    }
  
  // Zero curvature -> tube
  
  if(d->values[2] == 0.0)
    {
    get_coords_tube(d, z_norm, r_ret, z_ret, z_n_ret, r_n_ret);
    return;
    }

  angle = d->values[2]*(-1.0 + z_norm * 2.0);
  sin_angle = sin(angle);
  cos_angle = cos(angle);
  
  curvature_radius = d->values[1] / (2.0 * d->values[2]);
  
  *r_ret = d->values[0] + (1.0 - cos_angle) * curvature_radius;
  *z_ret = sin_angle * curvature_radius;;

  *r_n_ret = cos_angle;
  *z_n_ret = -sin_angle;
  }

static void get_coords_sphere(struct tube_data_s * d, float z_norm,
                              float* r_ret, float* z_ret,
                              float * z_n_ret, float * r_n_ret)
  {
  float theta;

  float cos_theta;
  float sin_theta;

  theta = M_PI - z_norm * M_PI;

  cos_theta = cos(theta);
  sin_theta = sin(theta);

  *r_ret = SPHERE_RADIUS * sin_theta;
  *z_ret = SPHERE_RADIUS * cos_theta;

  *z_n_ret = cos_theta;
  *r_n_ret = sin_theta;
  }

static void coords_tube_sphere_init(struct tube_data_s * d)
  {
  // Angle between the normal vector at z = z_min and the x-y - plane
  
  d->min_values[0] = 0.0;
  d->max_values[0] = 0.5 * M_PI;

  // "Height" of the thing

  d->min_values[1] = TUBE_LENGTH;
  d->max_values[1] = SPHERE_RADIUS * M_PI;

  // Radius

  d->min_values[2] = TUBE_RADIUS;
  d->max_values[2] = SPHERE_RADIUS;
  }

static void coords_sphere_tube_init(struct tube_data_s * d)
  {
  // Angle between the normal vector at z = z_min and the x-y - plane
  
  d->max_values[0] = 0.0;
  d->min_values[0] = 0.5 * M_PI;

  // "Height" of the thing

  d->max_values[1] = TUBE_LENGTH;
  d->min_values[1] = SPHERE_RADIUS * M_PI;

  // Radius

  d->max_values[2] = TUBE_RADIUS;
  d->min_values[2] = SPHERE_RADIUS;
  }

static void get_coords_tube_sphere(struct tube_data_s * d, float z_norm,
                                   float* r_ret, float* z_ret,
                                   float * z_n_ret, float * r_n_ret)
  {
  float curvature_radius;
  float angle;
  float cos_angle;
  float sin_angle;

  if(lemuria_range_done(&d->deform_range))
    {
    if(d->min_values[0] < d->max_values[0])
      d->get_coords = get_coords_sphere;
    else
      d->get_coords = get_coords_tube;
    d->get_coords(d, z_norm, r_ret, z_ret, z_n_ret, r_n_ret);
    return;
    }

  if(d->values[0] == 0.0)
    {
    get_coords_tube(d, z_norm, r_ret, z_ret, z_n_ret, r_n_ret);
    return;
    }

  angle = d->values[0]*(-1.0 + 2.0 * z_norm);

  cos_angle = cos(angle);
  sin_angle = sin(angle);
    
  curvature_radius = d->values[1] / (2.0 * d->values[0]);

  *r_ret = d->values[2] - curvature_radius * (1.0 - cos_angle);
  *z_ret = curvature_radius * sin_angle;

  *r_n_ret = cos_angle;
  *z_n_ret = sin_angle;
  }


static void change_deform(lemuria_engine_t * e,
                          tube_data * data, int do_exit)
  {
  int num_values = MAX_NUM_VALUES;
  if(data->get_coords == get_coords_tube)
    {
    if(do_exit)
      {
      data->exit_status |= EXIT_DEFORM;
      return;
      }
    if(lemuria_decide(e, 0.5))
      {
      data->get_coords = get_coords_tube_torus;
      coords_tube_torus_init(data);
      num_values = NUM_VALUES_TUBE_TORUS;
      }
    else
      {
      data->get_coords = get_coords_tube_sphere;
      coords_tube_sphere_init(data);
      num_values = NUM_VALUES_TUBE_SPHERE;
      }
    }
  else if(data->get_coords == get_coords_torus)
    {
    if(do_exit)
      {
      data->exit_status |= EXIT_DEFORM;
      return;
      }
    data->get_coords = get_coords_tube_torus;
    coords_torus_tube_init(data);
    num_values = NUM_VALUES_TUBE_TORUS;
    }
  else if(data->get_coords == get_coords_sphere)
    {
    data->get_coords = get_coords_tube_sphere;
    coords_sphere_tube_init(data);
    num_values = NUM_VALUES_TUBE_SPHERE;
    }
  else
    {
    data->exit_status |= EXIT_DEFORM_2;
    }

  if(!(data->exit_status & EXIT_DEFORM_2))
    lemuria_range_init(e, &(data->deform_range), num_values, 150, 250);
  }

static void draw_tube_mesh(lemuria_engine_t * e, tube_data * data)
  {
  int i, j;

  /*
   *  First index:  z
   *  Second index: phi
   */
  
  float x_11;
  float x_12;
  float x_21;
  float x_22;
  
  float y_11;
  float y_12;
  float y_21;
  float y_22;

  float z_1;
  float z_2;
  
  float phi_1 = 0.0;
  float phi_2;

  float r_1;
  float r_2;

  float z_n_1;
  float z_n_2;

  float r_n_1;
  float r_n_2;
  
  //  float r;

  float texture_x_1;
  float texture_x_2;

  float texture_y_1 = 0.0;
  float texture_y_2 = 0.0;

  float cos_phi_1 = 1.0;
  float sin_phi_1 = 0.0;
  
  float cos_phi_2;
  float sin_phi_2;

  //  glColor3f(1.0, 1.0, 1.0);

  glBegin(GL_QUADS);

  data->flip_texture = 1;

  data->get_coords(data, 0.0,
                   &r_1, &z_1, &z_n_1, &r_n_1);
  
  for(i = 0; i < Z_STEPS; i++)
    {
    texture_y_2 = (float)(i+1)/(float)(Z_STEPS);
    
    data->get_coords(data, texture_y_2,
                     &r_2, &z_2, &z_n_2, &r_n_2);
    
    x_11 = r_1;
    x_21 = r_2;

    y_11 = 0.0;
    y_21 = 0.0;

    texture_x_1 = 0.0;

    phi_1 = 0.0;
    
    for(j = 0; j < PHI_STEPS; j++)
      {
      texture_x_2 = (float)(j+1)/(float)(PHI_STEPS);
      
      phi_2 = 2.0 * M_PI * texture_x_2;

      cos_phi_2 = cos(phi_2);
      sin_phi_2 = sin(phi_2);

      x_12 = r_1 * cos_phi_2;
      x_22 = r_2 * cos_phi_2;
      
      y_12 = r_1 * sin_phi_2;
      y_22 = r_2 * sin_phi_2;
      
      glTexCoord2f(texture_x_1, texture_y_1);
      glNormal3f(cos_phi_1 * r_n_1,
                 sin_phi_1 * r_n_1, z_n_1);

      glVertex3f(x_11, y_11, z_1);

      glTexCoord2f(texture_x_2, texture_y_1);
      glNormal3f(cos_phi_2 * r_n_1,
                 sin_phi_2 * r_n_1, z_n_1);

      glVertex3f(x_12, y_12, z_1);

      glTexCoord2f(texture_x_2, texture_y_2);
      glNormal3f(cos_phi_2 * r_n_2,
                 sin_phi_2 * r_n_2, z_n_2);

      glVertex3f(x_22, y_22, z_2);
      
      glTexCoord2f(texture_x_1, texture_y_2);
      glNormal3f(cos_phi_1 * r_n_2,
                 sin_phi_1 * r_n_2, z_n_2);

      glVertex3f(x_21, y_21, z_2);

      x_21 = x_22;
      y_21 = y_22;

      x_11 = x_12;
      y_11 = y_12;

      phi_1 = phi_2;
      
      texture_x_1 = texture_x_2;

      cos_phi_1 = cos_phi_2;
      sin_phi_1 = sin_phi_2;
      }
    debug_now = 0;
    texture_y_1 = texture_y_2;
    z_1 = z_2;
    r_1 = r_2;
    r_n_1 = r_n_2;
    z_n_1 = z_n_2;
    }
  glEnd();
  }

#define OFFSET_FACTOR 4.0

static void draw_tube(lemuria_engine_t * e, void * user_data)
  {
  //  int done_before;
  int done_after;
  float scale_xy;
  float scale_z;

  float offset_z = 0.0;
  float offset_z_start;
  float offset_z_end;

  int exit_before;

  int exit_deform_2 = 0;
  
  tube_data * data = (tube_data*)(user_data);

  if(e->foreground.mode == EFFECT_FINISH)
    {
    e->foreground.mode = EFFECT_FINISHING;
    data->exit_status = 0;

    change_deform(e, data, 1);
    lemuria_rotator_turnto(&(data->rotator),
                           0.0,
                           0.0,
                           0.0,
                           lemuria_random_int(e, 100, 200));
    }
  
  switch(e->foreground.mode)
    {
    case EFFECT_STARTING:
      lemuria_range_update(&data->transition_range);
      offset_z_start = 0.5 * M_PI;
      offset_z_end   = 0.0;
      
      lemuria_range_get(&data->transition_range,
                        &offset_z_start,
                        &offset_z_end,
                        &offset_z);
      offset_z = OFFSET_FACTOR * sin(offset_z);
      if(lemuria_range_done(&data->transition_range))
        {
        e->foreground.mode = EFFECT_RUNNING;
        lemuria_rotator_change(e, &data->rotator);
        change_deform(e, data, 0);
        }
      
      break;
    case EFFECT_FINISHING:

      exit_before = data->exit_status;
      
      // We had a running deformation before, check if it's finished
      
      if(data->exit_status & EXIT_DEFORM_2)
        {
        lemuria_range_update(&data->deform_range);
        done_after =  lemuria_range_done(&data->deform_range);
        if(done_after)
          {
          exit_deform_2 = 1;
          }
        }
      // Check if the deformation is done
      else if(!(data->exit_status & EXIT_DEFORM))
        {
        lemuria_range_update(&data->deform_range);
        done_after =  lemuria_range_done(&data->deform_range);
        
        if(done_after)
          data->exit_status |= EXIT_DEFORM;
        }

      if(!(data->exit_status & EXIT_ROTATE))
        {
        //        done_before = lemuria_rotator_done(&data->rotator);
        lemuria_rotator_update(&data->rotator);
        
        if(lemuria_rotator_done(&data->rotator))
          data->exit_status |= EXIT_ROTATE;
        }
      
      if(data->exit_status == (EXIT_DEFORM | EXIT_ROTATE))
        {
        if(data->exit_status != exit_before)
          {
          lemuria_range_init(e, &data->transition_range, 1, 100, 200);
          }
              
        lemuria_range_update(&data->transition_range);
        
        offset_z_start = 0.0;
        offset_z_end   = 0.5 * M_PI;
        lemuria_range_get(&data->transition_range,
                          &offset_z_start,
                          &offset_z_end,
                          &offset_z);
        offset_z = OFFSET_FACTOR *sin(offset_z);
        if(lemuria_range_done(&data->transition_range))
          {
          e->foreground.mode = EFFECT_DONE;
          }
           
        }
      else
        lemuria_rotator_update(&(data->rotator));
      break;
    case EFFECT_RUNNING:
      lemuria_range_update(&data->deform_range);
      lemuria_rotator_update(&(data->rotator));
      lemuria_scale_update(&(data->scale_xy));
      lemuria_scale_update(&(data->scale_z));

      if(e->beat_detected)
        {
        if(lemuria_decide(e, 0.1))
          lemuria_rotator_change(e, &(data->rotator));
        
        if(lemuria_decide(e, 0.1))
          lemuria_scale_change(e, &(data->scale_xy));
        
        if(lemuria_decide(e, 0.1))
          lemuria_scale_change(e, &(data->scale_z));
        
        if(lemuria_range_done(&(data->deform_range)) &&
           lemuria_decide(e, 0.02))
          change_deform(e, data, 0);
        }
      break;
    }

  scale_xy = lemuria_scale_get(&(data->scale_xy));
  scale_z = lemuria_scale_get(&(data->scale_z));
  
  lemuria_range_get(&data->deform_range,
                    data->min_values, data->max_values, data->values);
  
  glShadeModel(GL_SMOOTH);
  //  glEnable(GL_POLYGON_SMOOTH);

  glEnable(GL_NORMALIZE);
    
/*   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_LIGHTING_MODE_HP, */
/*             GL_TEXTURE_POST_SPECULAR_HP); */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  
  glEnable(GL_TEXTURE_2D);
  
  // Enable lighting
  
  lemuria_set_perspective(e, 1, 20.0);
  
  // Set up and enable light 0

  lemuria_set_light(&light, GL_LIGHT0);
  glEnable(GL_LIGHT0);
  
  // All materials hereafter have full specular reflectivity
  // with a high shine

  lemuria_set_material(&material, GL_FRONT_AND_BACK);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
  
  glEnable(GL_LIGHTING);
  
  glMatrixMode(GL_MODELVIEW); 
  glPushMatrix();
  glLoadIdentity();
  //  fprintf(stderr, "scale_xy: %f, scale_z: %f\n", scale_xy, scale_z);

  glTranslatef(0.0, 0.0, offset_z);
  lemuria_rotate(&(data->rotator));

  glScalef(scale_xy, scale_xy, scale_z);

  lemuria_texture_bind(e, 0);

  /* Now, draw the tube */
  
  draw_tube_mesh(e, data);

  glPopMatrix();
  
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);

  glDisable(GL_COLOR_MATERIAL);


  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glDisable(GL_NORMALIZE);

  if(exit_deform_2)
    {
    data->exit_status &= ~EXIT_DEFORM_2;
    change_deform(e, data, 1);

    if(data->exit_status == (EXIT_DEFORM | EXIT_ROTATE))
      lemuria_range_init(e, &data->transition_range, 1, 100, 200);
    }
    
  }

static void * init_tube(lemuria_engine_t * e)
  {
  int index;
  
  tube_data * data;

#ifdef DEBUG
  fprintf(stderr, "init_tube...");
#endif

  
  data = calloc(1, sizeof(tube_data));

  data->engine = e;
  lemuria_texture_ref(data->engine, 0);

  lemuria_rotator_reset(&(data->rotator));
  e->foreground.mode = EFFECT_STARTING;

  lemuria_scale_init(e, &(data->scale_xy), 0.2, 1.0);
  lemuria_scale_init(e, &(data->scale_z),  0.2, 1.0);
  
  //  data->get_coords = get_coords_tube_torus;
#if 0
  index = lemuria_random_int(e, 0, 5);
#else
  index = lemuria_random_int(e, 0, 1);
#endif
  
  switch(index)
    {
    case 0:
      data->get_coords = get_coords_tube;
      break;
    case 1:
      data->get_coords = get_coords_torus;
      break;
    case 2:
      data->get_coords = get_coords_tube_sphere;
      coords_tube_sphere_init(data);
      break;
    case 3:
      data->get_coords = get_coords_sphere;
      break;
    case 4:
      data->get_coords = get_coords_tube_sphere;
      coords_sphere_tube_init(data);
      break;
    case 5:
      data->get_coords = get_coords_tube_torus;
      coords_torus_tube_init(data);
      break;
    case 6:
      data->get_coords = get_coords_tube_torus;
      coords_tube_torus_init(data);
      break;
    }
  
  lemuria_range_init(e, &(data->deform_range), 3, 50, 100);
  lemuria_range_init(e, &(data->transition_range), 1, 100, 200);

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  
  return data;
  }

static void delete_tube(void * e)
  {
  tube_data * data = (tube_data*)(e);
  lemuria_texture_unref(data->engine, 0);
  free(data);
  }

effect_plugin_t tube_effect =
  {
    .init =    init_tube,
    .draw =    draw_tube,
    .cleanup = delete_tube
  };
