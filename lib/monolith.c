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

//#include "images/monolith_floor1.incl"
// #include "images/archaic_clouds1.incl"

#include "gradients/lemuria_monolith_1.incl"
#include "gradients/lemuria_monolith_2.incl"
#include "gradients/lemuria_monolith_3.incl"

#define MONOLITH_HALF_WIDTH   0.5
#define MONOLITH_HALF_HEIGHT  0.25
#define MONOLITH_HALF_LENGTH  1.0
#define MONOLITH_DELTA_Z  6.0
#define NUM_MONOLITHS     60


typedef struct
  {
  int   texture_x;
  int   texture_y;
  float texture_advance;

  //  char * texture_data; // RGB24

  float coords[4][3];
  
  lemuria_background_data_t * background_data;
  } world_struct;

static void draw_world_struct(world_struct * w,
                              float * texture_start,
                              lemuria_background_t * background,
                              int * flip_y)
  {
  
  // fprintf(stderr, "Created texture image %d\n", glGetError());
  glColor3f(1.0, 0.0, 0.0);

  lemuria_background_draw(background,
                          w->coords,
                          w->texture_x,
                          w->texture_y,
                          texture_start,
                          w->texture_advance,
                          flip_y);
  };
    
typedef struct
  {
  float fog_color[4];
  float fog_density;

  world_struct floor_struct;
  world_struct ceiling_struct;
  } monolith_background;

static lemuria_background_data_t bg_clouds_1 =
  {
    .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
    .clouds_gradient = gradient_lemuria_monolith_1,
    .clouds_size = 0
  };

static lemuria_background_data_t bg_clouds_2 =
  {
    .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
    .clouds_gradient = gradient_lemuria_monolith_2,
    .clouds_size = 0
  };

static lemuria_background_data_t bg_clouds_3 =
  {
    .texture_mode =    LEMURIA_TEXTURE_CLOUDS,
    .clouds_gradient = gradient_lemuria_monolith_3,
    .clouds_size = 0
  };

static lemuria_background_data_t bg_xaos =
  {
    .texture_mode =    LEMURIA_TEXTURE_XAOS,
    .clouds_gradient = (uint8_t*)0,
    .clouds_size = 0
  };

monolith_background backgrounds[] = {
  {
    .fog_color = { 0.8, 0.8, 1.0, 1.0 },
    .fog_density = 1/300.0,
    //    .fog_density = 0.0,
    
    .floor_struct =
    {
      .texture_x =       14,
      .texture_y =       8,
      .texture_advance = 0.005,
      .coords =
      {
        { -300.0, -1.0, -800.0 },
        {  300.0, -1.0, -800.0 },
        {  300.0, -10.0, 0.0 },
        { -300.0, -10.0, 0.0 }
      },
      .background_data = &bg_xaos,
    },
    .ceiling_struct =
    {
      .texture_x =        15,
      .texture_y =        8,
      .texture_advance =  -0.005,

      .coords =
      {
        { -300.0, 1.0, -800.0 },
        {  300.0, 1.0, -800.0 },
        {  300.0, 10.0, 0.0 },
        { -300.0, 10.0, 0.0 }
      },
      .background_data =  (lemuria_background_data_t*)0,
    }
  },
  {
    .fog_color = { 1.0, 0.5, 0.0, 1.0 },
    .fog_density = 1/100.0,
    
    .floor_struct =
    {
      .texture_x =       10,
      .texture_y =       10,
      .texture_advance = 0.005,

      .coords =
      {
        { -150.0, -10.0, -400.0 },
        {  150.0, -10.0, -400.0 },
        {  150.0, -10.0, 0.0 },
        { -150.0, -10.0, 0.0 }
      },
      .background_data =  &bg_clouds_1,
    },

  .ceiling_struct =
    {
      .texture_x =        10,
      .texture_y =        10,
      .texture_advance =  -0.005,

      .coords =
      {
        { -150.0, 10.0, -400.0 },
        {  150.0, 10.0, -400.0 },
        {  150.0, 10.0, 0.0 },
        { -150.0, 10.0, 0.0 }
      },

      .background_data =  (lemuria_background_data_t*)0,
    }
  },
  {
    .fog_color = { 0.0, 1.0, 0.0, 1.0 },
    .fog_density = 1/100.0,
    
    .floor_struct =
    {
      .texture_x =       4.0,
      .texture_y =       1.0,
      .texture_advance = -0.005,

      .coords =
      {
        { -150.0, -10.0, -400.0 },
        {  150.0, -10.0, -400.0 },
        {  150.0, -10.0, 0.0 },
        { -150.0, -10.0, 0.0 }
      },
      .background_data = &bg_clouds_3
    },

  .ceiling_struct =
    {
      .texture_x =        4.0,
      .texture_y =        1.0,
      .texture_advance =  -0.005,
      
      .coords =
      {
        { -150.0, 10.0, -400.0 },
        {  150.0, 10.0, -400.0 },
        {  150.0, 10.0, 0.0 },
        { -150.0, 10.0, 0.0 }
      },

      .background_data =  (lemuria_background_data_t*)0,
    }
  },
  {
    .fog_color = { 0.70, 1.00, 0.94, 1.0 },
    .fog_density = 1/200.0,
    
    .floor_struct =
    {
      .texture_x =       4.0,
      .texture_y =       1.0,
      .texture_advance = -0.005,
    
      .coords =
      {
        { -300.0, -10.0, -1000.0 },
        {  300.0, -10.0, -1000.0 },
        {  300.0, -10.0, 0.0 },
        { -300.0, -10.0, 0.0 }
      },
      .background_data = &bg_clouds_2
    },

  .ceiling_struct =
    {
      .texture_x =        4.0,
      .texture_y =        1.0,
      .texture_advance =  -0.005,
      
      .coords =
      {
        { -300.0, 10.0, -1000.0 },
        {  300.0, 10.0, -1000.0 },
        {  300.0, 10.0, 0.0 },
        { -300.0, 10.0, 0.0 }
      },
      .background_data =  (lemuria_background_data_t*)0,
    }
   },
};

static int
num_backgrounds = sizeof(backgrounds)/sizeof(backgrounds[0]);

static lemuria_background_data_t bg_goom =
  {
    .texture_mode =    LEMURIA_TEXTURE_GOOM,
    .clouds_gradient = (uint8_t*)0,
    .clouds_size = 0
  };
  
monolith_background goom_background_1 =
  {
    .fog_color = { 0.8, 0.8, 1.0, 1.0 },
    .fog_density = 1/300.0,
    //    .fog_density = 0.0,
    
    .floor_struct =
    {
#if 1
      .texture_x =       14,
      .texture_y =       8,
      .texture_advance = 0.005,
      .coords =
      {
        { -300.0, -1.0, -800.0 },
        {  300.0, -1.0, -800.0 },
        {  300.0, -10.0, 0.0 },
        { -300.0, -10.0, 0.0 }
      },
#else
      .texture_x =       3,
      .texture_y =       3,
      .texture_advance = 0.005,
      .coords =
      {
        { -0.8, -1.2,  0.0 },
        {  0.8, -1.2,  0.0 },
        {  0.8,  0.6, 0.0 },
        { -0.8,  0.6, 0.0 }
      },
#endif
      .background_data = &bg_goom,
    },

  .ceiling_struct =
    {
      .texture_x =        15,
      .texture_y =        8,
      .texture_advance =  -0.005,

      .coords =
      {
        { -300.0, 1.0, -800.0 },
        {  300.0, 1.0, -800.0 },
        {  300.0, 10.0, 0.0 },
        { -300.0, 10.0, 0.0 }
      },
      .background_data =  (lemuria_background_data_t*)0,
    }
  };



typedef struct monolith_data_s
  {
  unsigned int monolith_list;

  monolith_background * background;
  
  float speed_forward;
  float speed_backward;

  float start_forward;
  float start_backward;

  void (*call_list_func)(struct monolith_data_s*);

  int color_index;

  unsigned int texture;

  uint8_t * floor_texture_data;
  uint8_t * ceiling_texture_data;
  
  float floor_texture_start;
  float ceiling_texture_start;

  lemuria_engine_t * engine;

  lemuria_background_t * floor;
  lemuria_background_t * ceiling;

  int ceiling_flip_y;
  int floor_flip_y;
    
  } monolith_data;


static void draw_background(lemuria_engine_t * e, monolith_data * m)
  {
  glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  lemuria_background_set(m->floor);
  
  draw_world_struct(&(m->background->floor_struct),
                    &(m->floor_texture_start),m->floor,
                    &(m->floor_flip_y));
#if 1
  
  if(m->ceiling)
    {
    lemuria_background_set(m->ceiling);
    draw_world_struct(&(m->background->ceiling_struct),
                      &(m->ceiling_texture_start),m->ceiling,
                      &(m->ceiling_flip_y));
    }
  else
    draw_world_struct(&(m->background->ceiling_struct),
                      &(m->ceiling_texture_start),m->floor,
                      &(m->ceiling_flip_y));
#endif
    
  //  glDisable(GL_FOG);
  glDisable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  }


static void call_list_1(monolith_data * d)
  {
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(5.0, -3.0, d->start_backward);
  glCallList(d->monolith_list);
  glPopMatrix();

  glPushMatrix();
  glLoadIdentity();
  glTranslatef(-5.0, -3.0, d->start_forward);
  glCallList(d->monolith_list);
  glPopMatrix();

  glPushMatrix();
  glLoadIdentity();
  glTranslatef(5.0, 3.0, d->start_forward);
  glCallList(d->monolith_list);
  glPopMatrix();

  glPushMatrix();
  glLoadIdentity();
  glTranslatef(-5.0, 3.0, d->start_backward);
  glCallList(d->monolith_list);
  glPopMatrix();
  }

static void call_list_2(monolith_data * d)
  {
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(5.0, 3.0, d->start_backward);
  glCallList(d->monolith_list);
  glPopMatrix();

  glPushMatrix();
  glLoadIdentity();
  glTranslatef(-5.0, 3.0, d->start_forward);
  glCallList(d->monolith_list);
  glPopMatrix();


  glPushMatrix();
  glLoadIdentity();

  glTranslatef(10.0, -5.0, -50.0);
  glRotatef(90.0, 0.0, 1.0, 0.0);
  
  glTranslatef(0.0, 0.0, 20 + d->start_backward);
  glCallList(d->monolith_list);
  glPopMatrix();

  glPushMatrix();
  glLoadIdentity();

  glTranslatef(10.0, -5.0, -40.0);
  glRotatef(90.0, 0.0, 1.0, 0.0);
  
  glTranslatef(0.0, 0.0, 20 + d->start_forward);
  glCallList(d->monolith_list);
  glPopMatrix();
  }

static void call_list_3(monolith_data * d)
  {
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(5.0, -3.0, d->start_backward);
  glCallList(d->monolith_list);
  glPopMatrix();

  glPushMatrix();
  glLoadIdentity();
  glTranslatef(-5.0, -3.0, d->start_forward);
  glCallList(d->monolith_list);
  glPopMatrix();


  glPushMatrix();
  glLoadIdentity();

  glTranslatef(10.0, 5.0, -50.0);
  glRotatef(90.0, 0.0, 1.0, 0.0);
  
  glTranslatef(0.0, 0.0, 20 + d->start_backward);
  glCallList(d->monolith_list);
  glPopMatrix();

  glPushMatrix();
  glLoadIdentity();

  glTranslatef(10.0, 5.0, -40.0);
  glRotatef(90.0, 0.0, 1.0, 0.0);
  
  glTranslatef(0.0, 0.0, 20 + d->start_forward);
  glCallList(d->monolith_list);
  glPopMatrix();
  }


static lemuria_light_t light =
  {
    .ambient =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .diffuse =  { 0.5f, 0.5f, 0.5f, 1.0f },
    .specular = { 0.0f, 0.0f, 0.0f, 1.0f},
    .position = { 1.0f, 0.0f, 10.0f, 1.0f }
  };

typedef struct _color_setup
  {
  lemuria_material_t monolith_material;
  float fog_color[4];
  float sky_color[4];
  float floor_color[4];
  } color_setup;

static color_setup monolith_colors[] =
  {
    {
      .monolith_material =
      {
        .ref_specular = { 0.0f, 0.0f, 0.0f, 0.0f },
        .ref_ambient =  { 0.2f, 0.2f, 0.2f, 1.0f },
        .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
        .shininess = 128
      },
      .fog_color =   { 0.6, 0.2, 0.2, 1.0 },
      .sky_color =   { 0.2, 0.2, 1.0, 1.0 },
      .floor_color = { 0.2, 0.2, 0.2, 1.0 },
    },
    {
      .monolith_material =
      {
        .ref_specular = { 0.0f, 0.0f, 0.0f, 0.0f },
        .ref_ambient =  { 0.2f, 0.2f, 0.4f, 1.0f },
        .ref_diffuse =  { 1.0f, 1.0f, 1.0f, 1.0f },
        .shininess = 128
      },
      .fog_color =   { 0.6, 0.0, 0.0, 1.0 },
      .sky_color =   { 0.0, 0.0, 1.0, 1.0 },
      .floor_color = { 0.0, 0.0, 1.0, 1.0 },
    }

    
    
  };



#define SKY_X_MIN -100.0
#define SKY_X_MAX  100.0

#define SKY_Z_MIN -300.0
#define SKY_Z_MAX 0.0

#define SKY_Y 10.0

static void draw_monolith(lemuria_engine_t * e, void * user_data)
  {

  monolith_data * data = (monolith_data*)(user_data);

  data->start_forward += data->speed_forward;
  if(data->start_forward > MONOLITH_DELTA_Z)
    data->start_forward -= MONOLITH_DELTA_Z;

  data->start_backward -= data->speed_backward;
  if(data->start_backward < MONOLITH_DELTA_Z)
    data->start_backward += MONOLITH_DELTA_Z;
  
  glShadeModel(GL_FLAT);

  //  glEnable(GL_CULL_FACE);
  
  lemuria_set_perspective(e, 1, 1000.0);
  
  lemuria_set_light(&light, GL_LIGHT0);
  glEnable(GL_LIGHT0);
  
  // Enable lighting
  glEnable(GL_LIGHTING);
  
  lemuria_set_material(&(monolith_colors[data->color_index].monolith_material),
                       GL_FRONT_AND_BACK);

  glMatrixMode(GL_MODELVIEW);

  /* Enable fog */

  glFogf(GL_FOG_DENSITY, data->background->fog_density);
  glFogi(GL_FOG_MODE, GL_EXP );
  glFogfv(GL_FOG_COLOR,  data->background->fog_color);
  glEnable(GL_FOG);
  
  if(e->antialias)
    {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_SMOOTH);

    if(e->antialias > 1)
      glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    else
      glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
        
    glEnable(GL_CULL_FACE);
    data->call_list_func(data);
    glDisable(GL_CULL_FACE);

    glDisable(GL_POLYGON_SMOOTH);
    
    draw_background(e, data);
    
    
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);

    glColor4f(data->background->fog_color[0],
              data->background->fog_color[1],
              data->background->fog_color[2],
              data->background->fog_color[3]);

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
    glClearColor(data->background->fog_color[0],
                 data->background->fog_color[1],
                 data->background->fog_color[2],
                 data->background->fog_color[3]);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    draw_background(e, data);

    glEnable(GL_CULL_FACE);
    data->call_list_func(data);
    glDisable(GL_CULL_FACE);

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    

    }
  
 
  }

static void * init_monolith(lemuria_engine_t * e)
  {
  int i;
  float z = 0.0;
  int random_number;
  monolith_data * data;

#ifdef DEBUG
  fprintf(stderr, "init_monolith...");
#endif
  
  data = calloc(1, sizeof(monolith_data));

  
  i = lemuria_random_int(e, 0, 2);
  //  i = 2;
  switch(i)
    {
    case 0:
      data->call_list_func = call_list_1;
      break;
    case 1:
      data->call_list_func = call_list_2;
      break;
    case 2:
      data->call_list_func = call_list_3;
      break;
    }

#if 1
  if(e->goom && lemuria_decide(e, 0.3))
    {
    data->background = &goom_background_1;
    }
  else
    {
    random_number = lemuria_random_int(e, 0, num_backgrounds-1);
    //    random_number = 0;
    data->background = &(backgrounds[random_number]);
    }
#else
  random_number = 0;
  data->background = &(backgrounds[random_number]);
  //  data->background = &goom_background_1;
#endif
  data->floor = calloc(1, sizeof(lemuria_background_t));
  
  lemuria_background_init(e, data->floor,
                          data->background->floor_struct.background_data);
  
  if(data->background->ceiling_struct.background_data)
    {
    data->ceiling = calloc(1, sizeof(lemuria_background_t));
  
    lemuria_background_init(e, data->ceiling,
                            data->background->ceiling_struct.background_data);
    }
  else
    data->ceiling = (lemuria_background_t*)0;
  
  data->speed_forward  = 0.10;
  data->speed_backward = 0.10;
  
  data->start_forward = 0.0;
  data->start_backward = 0.0;

  data->color_index = lemuria_random(e, 0, sizeof(monolith_colors)/
                                     sizeof(monolith_colors[0])-1);

  data->color_index = 1;
  data->monolith_list = glGenLists(1);
  glNewList(data->monolith_list, GL_COMPILE);

  glBegin(GL_QUADS);
  
  for(i = 0; i < NUM_MONOLITHS; i++)
    {
    // Front Face

    glNormal3f(0.0, 0.0,1.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               -MONOLITH_HALF_HEIGHT,
               z + MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, 0.0,1.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                -MONOLITH_HALF_HEIGHT,
                z + MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, 0.0,1.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                MONOLITH_HALF_HEIGHT,
                z + MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, 0.0,1.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               MONOLITH_HALF_HEIGHT,
               z + MONOLITH_HALF_LENGTH);

    // Back Face
    
    glNormal3f(0.0, 0.0,-1.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               -MONOLITH_HALF_HEIGHT,
               z - MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, 0.0,-1.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               MONOLITH_HALF_HEIGHT,
               z - MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, 0.0,-1.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                MONOLITH_HALF_HEIGHT,
                z - MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, 0.0,-1.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                -MONOLITH_HALF_HEIGHT,
                z - MONOLITH_HALF_LENGTH); 

    // Top Face

    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               MONOLITH_HALF_HEIGHT,
               z - MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               MONOLITH_HALF_HEIGHT,
               z + MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                MONOLITH_HALF_HEIGHT,
                z + MONOLITH_HALF_LENGTH); 

    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                MONOLITH_HALF_HEIGHT,
                z - MONOLITH_HALF_LENGTH);

    // Bottom Face

    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               -MONOLITH_HALF_HEIGHT,
               z - MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                -MONOLITH_HALF_HEIGHT,
                z - MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                -MONOLITH_HALF_HEIGHT,
                z + MONOLITH_HALF_LENGTH);

    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               -MONOLITH_HALF_HEIGHT,
               z + MONOLITH_HALF_LENGTH);

    // Right face

    glNormal3f(1.0,0.0,0.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                -MONOLITH_HALF_HEIGHT,
                z - MONOLITH_HALF_LENGTH);

    glNormal3f(1.0,0.0,0.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                MONOLITH_HALF_HEIGHT,
                z - MONOLITH_HALF_LENGTH);

    glNormal3f(1.0,0.0,0.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                MONOLITH_HALF_HEIGHT,
                z + MONOLITH_HALF_LENGTH);

    glNormal3f(1.0,0.0,0.0);
    glVertex3f( MONOLITH_HALF_WIDTH,
                -MONOLITH_HALF_HEIGHT,
                z + MONOLITH_HALF_LENGTH);

    // Left Face

    glNormal3f(-1.0,0.0,0.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               -MONOLITH_HALF_HEIGHT,
               z - MONOLITH_HALF_LENGTH);

    glNormal3f(-1.0,0.0,0.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               -MONOLITH_HALF_HEIGHT,
               z + MONOLITH_HALF_LENGTH);

    glNormal3f(-1.0,0.0,0.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               MONOLITH_HALF_HEIGHT,
               z + MONOLITH_HALF_LENGTH);

    glNormal3f(-1.0,0.0,0.0);
    glVertex3f(-MONOLITH_HALF_WIDTH,
               MONOLITH_HALF_HEIGHT,
               z - MONOLITH_HALF_LENGTH);
    z -= MONOLITH_DELTA_Z;
    }
  glEnd();
  glEndList();

  data->engine = e;
#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  
  return data;
  }

static void delete_monolith(void * e)
  {
  monolith_data * data = (monolith_data*)(e);


  if(data->floor)
    {
    lemuria_background_delete(data->floor);
    free(data->floor);
    }
  if(data->ceiling)
    {
    lemuria_background_delete(data->ceiling);
    free(data->ceiling);
    }
  glDeleteLists(data->monolith_list, 1);
  free(data);
  }

effect_plugin_t monolith_effect =
  {
    .init =    init_monolith,
    .draw =    draw_monolith,
    .cleanup = delete_monolith,
  };
