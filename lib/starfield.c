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

#include <material.h>
#include <object.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Things, which float around in outer space...

#define OBJECT_Z_START 0.0
#define OBJECT_Z_END   -500.0

#define TRANSITION_TIME 80

// Maximum number of objects

#define MAX_NUM_SPACE_OBJECTS 1

static void space_object_init(lemuria_engine_t * e,
                              lemuria_object_t * obj)
  {
  float radius;
  float angle;
  //  float tmp;
  int random_number;
  float coords[3];
  float delta_coords[3];
  float speed = 0.0;
  lemuria_object_type_t type = LEMURIA_OBJECT_UFO;

  radius = lemuria_random(e, 5.0, 10.0);
  angle =  lemuria_random(e, 0.0, 2.0*M_PI);

  
  //  angle = -0.5 * M_PI;
  
  coords[0] = radius * cos(angle);
  coords[1] = radius * sin(angle);
  delta_coords[0] = 0.0;
  delta_coords[1] = 0.0;
    
  /*  fprintf(stderr, "space_object_init\n"); */
    
  // #if 0
  if(lemuria_decide(e, 0.5))
    {
    random_number = lemuria_random_int(e, 0, 2);

    switch(random_number)
      {
      case 0:
        type = LEMURIA_OBJECT_SATELLITE;
        speed = lemuria_random(e, 0.3, 1.0);
        break;
      case 1:
        type = LEMURIA_OBJECT_UFO;
        speed = lemuria_random(e, 0.3, 1.0);
        break;
      case 2:
        speed = lemuria_random(e, 0.3, 1.0);
        type = LEMURIA_OBJECT_SPUTNIK;
        break;
      }
    }
  else
    {
    random_number = lemuria_random_int(e, 0, 1);
    switch(random_number)
      {
      case 0:
        speed = lemuria_random(e, 0.1, 0.2);
        type = LEMURIA_OBJECT_TEAPOT;
        break;
      case 1:
        speed = lemuria_random(e, 0.2, 0.5);
        type = LEMURIA_OBJECT_FISH;
        break;
      }
    }
  
#if 1
  if(lemuria_decide(e, 0.5))
    {
    coords[2] = OBJECT_Z_END;
    delta_coords[2] = speed;
    }
  else
    {
    coords[2] = 0.0;
    delta_coords[2] = -speed;
    }
  //  type = LEMURIA_OBJECT_FISH;
#else

  coords[0] =   0.0;
  coords[1] =   0.0;
  coords[2] =   0.0;
  
  delta_coords[0] =  0.0;
  delta_coords[1] =  0.0;
  delta_coords[2] =  -0.5;
  
  //  delta_coords[2] = -speed;
 
  
#endif
  
  lemuria_object_init(e, obj, type, coords, delta_coords);
  //  fprintf(stderr, "Space object init\n");
  
  }

#define NUM_STARS 2000

#define X_MAX 400.0
#define Y_MAX 300.0

#define Z_MIN -1000.0
#define Z_MAX -1.0

#define STAR_SIZE_MIN 1.0
#define STAR_SIZE_MAX 3.0

#define COLOR_MIN 0.8
#define COLOR_MAX 1.0

#define SPEED_FRAMES_MIN 400

typedef struct
  {
  float x, y, z;
  float size;
  float red;
  float green;
  float blue;
  } star_data;

typedef struct
  {
  star_data * stars;
  float speed;

  float start_speed;
  float end_speed;
  
  lemuria_range_t speed_range;
  
  int speed_frames;

  lemuria_object_t space_objects[MAX_NUM_SPACE_OBJECTS];

  int num_active_space_objects;

  int do_transition;
  lemuria_range_t transition_range;
  
  } starfield_data;

static void create_star(lemuria_engine_t * e, star_data * d)
  {
  d->x = lemuria_random(e, -X_MAX, X_MAX);
  d->y = lemuria_random(e, -Y_MAX, Y_MAX);
  d->z = Z_MIN;
  d->size = lemuria_random(e, STAR_SIZE_MIN, STAR_SIZE_MAX);

  d->red   = lemuria_random(e, COLOR_MIN, COLOR_MAX);
  d->green = lemuria_random(e, COLOR_MIN, COLOR_MAX);
  d->blue  = lemuria_random(e, COLOR_MIN, COLOR_MAX);
  }

static void * init_starfield(lemuria_engine_t * e)
  {
  int i;
  starfield_data * d;

#ifdef DEBUG
  fprintf(stderr, "init_starfield...");
#endif

  d = calloc(1, sizeof(starfield_data));

  d->stars = calloc(NUM_STARS, sizeof(star_data));

/*   space_object_init(&(d->space_objects[0])); */
/*   d->num_active_space_objects = 1; */
  
  for(i = 0; i < NUM_STARS; i++)
    {
    create_star(e, &d->stars[i]);
    d->stars[i].z = lemuria_random(e, Z_MIN, Z_MAX);
    }
  d->speed = 0.30;

#ifdef DEBUG
  fprintf(stderr, "done\n");
#endif
  e->background.mode = EFFECT_STARTING;
  d->do_transition = 1;
  lemuria_range_init(e, &(d->transition_range),
                     1, TRANSITION_TIME, TRANSITION_TIME);

  return d;
  }

static void draw_starfield_normal(lemuria_engine_t * e, void * user_data)
  {
  int i;
  
  starfield_data * data = (starfield_data*)(user_data);

  if((e->beat_detected) && lemuria_decide(e, 0.5) &&
     (e->background.mode == EFFECT_RUNNING))
    {
    if(lemuria_range_done(&(data->speed_range)) &&
       (data->speed_frames > SPEED_FRAMES_MIN))
      {
      lemuria_range_init(e, &(data->speed_range), 1, 20, 30);
      data->start_speed = data->speed;
      data->end_speed = 0.3 +
        lemuria_random(e, 0.0, 1.0 * (float)(e->loudness/32768.0));
      data->speed_frames = 0;
      }
    else if(data->num_active_space_objects < MAX_NUM_SPACE_OBJECTS)
      {
      for(i = 0; i < MAX_NUM_SPACE_OBJECTS; i++)
        {
        if(!(data->space_objects[i].draw))
          {
          space_object_init(e, &(data->space_objects[i]));
          data->num_active_space_objects++;
          break;
          }
        }
      }
    }

  data->speed_frames++;
  
  lemuria_range_update(&(data->speed_range));
  lemuria_range_get(&(data->speed_range), &(data->start_speed),
                        &(data->end_speed), &(data->speed));
  
  glClearColor(0.0, 0.0, 0.1, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  lemuria_set_perspective(e, 1, 1000.0);

  if(e->antialias)
    {
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    if(e->antialias > 1)
      glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    else
      glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    }
  
  for(i = 0; i < NUM_STARS; i++)
    {
    /* Adjust coordinates */

    data->stars[i].z += data->speed;
    if(data->stars[i].z > Z_MAX)
      create_star(e, &data->stars[i]);
    
    glPointSize(data->stars[i].size);

    glBegin(GL_POINTS);

    glColor3f(data->stars[i].red, data->stars[i].green, data->stars[i].blue);
    glVertex3f(data->stars[i].x, data->stars[i].y, data->stars[i].z);

    glEnd();
    }
  if(e->antialias)
    {
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_BLEND);
    }
  
  if(data->num_active_space_objects)
    {
    for(i = 0; i < MAX_NUM_SPACE_OBJECTS; i++)
      {
      if(data->space_objects[i].draw)
        {
        lemuria_object_update(e, &(data->space_objects[i]));
        if((data->space_objects[i].coords[2] < OBJECT_Z_END) ||
           (data->space_objects[i].coords[2] > 0.0))
          {
          lemuria_object_cleanup(&(data->space_objects[i]));
          data->num_active_space_objects--;
          }
        }

      if(data->space_objects[i].draw)
        {
        lemuria_object_draw(e, &data->space_objects[i]);
        }
      }
    }

  if(!data->do_transition &&
     e->background.mode == EFFECT_FINISHING)
    {
    data->do_transition = 1;
    lemuria_range_init(e, &(data->transition_range),
                       1, TRANSITION_TIME, TRANSITION_TIME);
    }
  
  //  draw_ufo(e);
  }

#define PHASE_2 0.6

static void draw_starfield_transition(lemuria_engine_t * e, void * user_data)
  {
  float progress;
  float min, max;
  float delta_z1, delta_z2;
  int i;
  float bg_color[3];
  starfield_data * data = (starfield_data*)(user_data);
  lemuria_range_update(&(data->transition_range));
    
  if(e->background.mode == EFFECT_STARTING)
    {
    min = 1.0;
    max = 0.0;
    }
  else
    {
    min = 0.0;
    max = 1.0;
    }
  
  lemuria_range_get(&(data->transition_range), &min, &max, &progress);
  if(progress < PHASE_2)
    {
    delta_z1 = 0.0;
    delta_z2 = (Z_MAX - Z_MIN) * (progress / PHASE_2);
    bg_color[0] = 0.0;
    bg_color[1] = 0.0;
    bg_color[2] = 0.1;
    }
  else
    {
    progress = (progress - PHASE_2 ) / (1.0 - PHASE_2);
    delta_z1 = (Z_MAX - Z_MIN) * progress;
    delta_z2 = (Z_MAX - Z_MIN);

    bg_color[0] = progress;
    bg_color[1] = progress;
    bg_color[2] = 0.1 + 0.9 * progress;
    }

  glShadeModel(GL_SMOOTH);
  glClearColor(bg_color[0], bg_color[1], bg_color[2], 1.0);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  lemuria_set_perspective(e, 1, 1000.0);

  if(e->antialias)
    {
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    if(e->antialias > 1)
      glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    else
      glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    }

  //  glEnable(GL_COLOR_MATERIAL);
  
  for(i = 0; i < NUM_STARS; i++)
    {
    /* Adjust coordinates */

    //    data->stars[i].z += data->speed;
    
    glLineWidth(data->stars[i].size / 2.0);

    glBegin(GL_LINES);

    glColor3fv(bg_color);
    glVertex3f(data->stars[i].x, data->stars[i].y,
               data->stars[i].z + delta_z1);

    glColor3f(data->stars[i].red, data->stars[i].green, data->stars[i].blue);
    glVertex3f(data->stars[i].x, data->stars[i].y,
               data->stars[i].z + delta_z2);

    glEnd();
    }

  //  glDisable(GL_COLOR_MATERIAL);
  
  if(e->antialias)
    {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    }
  if(lemuria_range_done(&(data->transition_range)))
    {
    if(e->background.mode == EFFECT_FINISHING)
      e->background.mode = EFFECT_DONE;
    else
      {
      e->background.mode = EFFECT_RUNNING;
      data->do_transition = 0;
      }
    }
  
  }

static void draw_starfield(lemuria_engine_t * e, void * user_data)
  {
  starfield_data * data = (starfield_data*)(user_data);

  if(e->background.mode == EFFECT_FINISH)
    {
    e->background.mode = EFFECT_FINISHING;
    if(!data->num_active_space_objects)
      {
      data->do_transition = 1;
      lemuria_range_init(e, &(data->transition_range),
                         1, TRANSITION_TIME, TRANSITION_TIME);
      }
    }
  if(data->do_transition)
    draw_starfield_transition(e, user_data);
  else
    draw_starfield_normal(e, user_data);
  }


static void delete_starfield(void * data)
  {
  int i;
  starfield_data * d = (starfield_data*)(data);

  for(i = 0; i < MAX_NUM_SPACE_OBJECTS; i++)
    {
    if(d->space_objects[i].draw)
      lemuria_object_cleanup(&(d->space_objects[i]));
    }
  free(d->stars);
  free(d);
  }

effect_plugin_t starfield_effect =
  {
    .init =    init_starfield,
    .draw =    draw_starfield,
    .cleanup = delete_starfield,
  };
