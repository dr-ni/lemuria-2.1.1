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

#include <stdlib.h>
#include <lemuria_private.h>
#include <utils.h>
#include <effect.h>

/* Background coordinates */

#define MAX_Y 1.25
#define MAX_X ((MAX_Y*4.0)/3.0)

#define DELTA_X 3.0
#define DELTA_Y 1.0

static float bg_coords[][4][3] =
  {
    {
      { -3.0 * MAX_X, -3.0 * MAX_Y - 3.0 * DELTA_Y, 0.0 },
      {  5.0 * MAX_X, -3.0 * MAX_Y + 5.0 * DELTA_Y, 0.0 },
      {  5.0 * MAX_X,  3.0 * MAX_Y + 5.0 * DELTA_Y, 0.0 },
      { -3.0 * MAX_X,  3.0 * MAX_Y - 3.0 * DELTA_Y, 0.0 }
    },
    {
      { -5.0 * MAX_X, -3.0 * MAX_Y - 5.0 * DELTA_Y, 0.0 },
      {  3.0 * MAX_X, -3.0 * MAX_Y + 3.0 * DELTA_Y, 0.0 },
      {  3.0 * MAX_X,  3.0 * MAX_Y + 3.0 * DELTA_Y, 0.0 },
      { -5.0 * MAX_X,  3.0 * MAX_Y - 5.0 * DELTA_Y, 0.0 }
    },

    /* */
    {
      { -3.0 * MAX_X, -3.0 * MAX_Y + 3.0 * DELTA_Y, 0.0 },
      {  5.0 * MAX_X, -3.0 * MAX_Y - 5.0 * DELTA_Y, 0.0 },
      {  5.0 * MAX_X,  3.0 * MAX_Y - 5.0 * DELTA_Y, 0.0 },
      { -3.0 * MAX_X,  3.0 * MAX_Y + 3.0 * DELTA_Y, 0.0 }
    },
    {
      { -5.0 * MAX_X, -3.0 * MAX_Y + 5.0 * DELTA_Y, 0.0 },
      {  3.0 * MAX_X, -3.0 * MAX_Y - 3.0 * DELTA_Y, 0.0 },
      {  3.0 * MAX_X,  3.0 * MAX_Y - 3.0 * DELTA_Y, 0.0 },
      { -5.0 * MAX_X,  3.0 * MAX_Y + 5.0 * DELTA_Y, 0.0 }
    },

    /* */
    {
      { -3.0 * MAX_X + DELTA_X, -3.0 * MAX_Y, 0.0 },
      {  5.0 * MAX_X + DELTA_X, -3.0 * MAX_Y, 0.0 },
      {  5.0 * MAX_X - DELTA_X,  3.0 * MAX_Y, 0.0 },
      { -3.0 * MAX_X - DELTA_X,  3.0 * MAX_Y, 0.0 }
    },
    {
      { -5.0 * MAX_X + DELTA_X, -3.0 * MAX_Y, 0.0 },
      {  3.0 * MAX_X + DELTA_X, -3.0 * MAX_Y, 0.0 },
      {  3.0 * MAX_X - DELTA_X,  3.0 * MAX_Y, 0.0 },
      { -5.0 * MAX_X - DELTA_X,  3.0 * MAX_Y, 0.0 }
    },
    /* */
    {
      { -3.0 * MAX_X - DELTA_X, -3.0 * MAX_Y, 0.0 },
      {  5.0 * MAX_X - DELTA_X, -3.0 * MAX_Y, 0.0 },
      {  5.0 * MAX_X + DELTA_X,  3.0 * MAX_Y, 0.0 },
      { -3.0 * MAX_X + DELTA_X,  3.0 * MAX_Y, 0.0 }
    },
    {
      { -5.0 * MAX_X - DELTA_X, -3.0 * MAX_Y, 0.0 },
      {  3.0 * MAX_X - DELTA_X, -3.0 * MAX_Y, 0.0 },
      {  3.0 * MAX_X + DELTA_X,  3.0 * MAX_Y, 0.0 },
      { -5.0 * MAX_X + DELTA_X,  3.0 * MAX_Y, 0.0 }
    },
    /* */
    {
      { -5.0 * MAX_X, -3.0 * MAX_Y, 0.0 },
      {  3.0 * MAX_X, -3.0 * MAX_Y, 0.0 },
      {  3.0 * MAX_X,  3.0 * MAX_Y, 0.0 },
      { -5.0 * MAX_X,  3.0 * MAX_Y, 0.0 }
    },
    {
      { -3.0 * MAX_X, -3.0 * MAX_Y, 0.0 },
      {  5.0 * MAX_X, -3.0 * MAX_Y, 0.0 },
      {  5.0 * MAX_X,  3.0 * MAX_Y, 0.0 },
      { -3.0 * MAX_X,  3.0 * MAX_Y, 0.0 }
    },
    {
      { -MAX_X, -3.0 * MAX_Y, 0.0 },
      {  MAX_X, -3.0 * MAX_Y, 0.0 },
      {  MAX_X,  3.0 * MAX_Y, 0.0 },
      { -MAX_X,  3.0 * MAX_Y, 0.0 }
    },
    {
      { -MAX_X, -MAX_Y, 0.0 },
      {  MAX_X, -MAX_Y, 0.0 },
      {  MAX_X,  MAX_Y, 0.0 },
      { -MAX_X,  MAX_Y, 0.0 }
    },
    {
      { -MAX_X, -MAX_Y, 0.0 },
      {  MAX_X, -MAX_Y, 0.0 },
      {  MAX_X,  MAX_Y, 0.0 },
      { -MAX_X,  MAX_Y, 0.0 }
    },
    {
      { -MAX_X, -MAX_Y, 0.0 },
      {  MAX_X, -MAX_Y, 0.0 },
      {  MAX_X,  MAX_Y, 0.0 },
      { -MAX_X,  MAX_Y, 0.0 }
    },
  };

static int num_bg_coords = sizeof(bg_coords)/sizeof(bg_coords[0]);

/* We support all background types */

static lemuria_background_data_t bg_goom =
  {
    .texture_mode =    LEMURIA_TEXTURE_GOOM,
    .clouds_gradient = (uint8_t*)0,
    .clouds_size = 0
  };

static lemuria_background_data_t bg_lemuria =
  {
    .texture_mode = LEMURIA_TEXTURE_LEMURIA,
    (uint8_t *)0,
    clouds_size : 0,
  };

static lemuria_background_data_t bg_xaos =
  {
    .texture_mode = LEMURIA_TEXTURE_XAOS,
    (uint8_t *)0,
    clouds_size : 0,
  };


typedef struct
  {
  lemuria_background_t bg;

  float bg_texture_start;
  int bg_texture_flip;

  int bg_coords_start;
  int bg_coords_end;
  lemuria_range_t bg_coords_range;
    
  } boioioing_data;

static void * init_boioioing(lemuria_engine_t * e)
  {
  //  int i;
  boioioing_data * d;
  int random_number;
#ifdef DEBUG
  fprintf(stderr, "Init boioioing...");
#endif
  d = calloc(1, sizeof(*d));

  /* Initalize background */
    
  if(e->goom)
    random_number = lemuria_random_int(e, 0, 2);
  else
    random_number = lemuria_random_int(e, 1, 2);
  //  random_number = 2;
  switch(random_number)
    {
    case 0:
      //      fprintf(stderr, "Goom\n");
      lemuria_background_init(e,
                              &(d->bg),
                              &(bg_goom));
      break;
    case 1:
      lemuria_background_init(e,
                              &(d->bg),
                              &(bg_xaos));
      break;
    case 2:
      lemuria_background_init(e,
                              &(d->bg),
                              &(bg_lemuria));
      break;
    default:
      break;
    }

  /* Initialize background coordinates */
  
  random_number = lemuria_random_int(e, 0, num_bg_coords-1);
  //  random_number = 0;
  d->bg_coords_start = random_number;
  d->bg_coords_end = random_number;
  lemuria_range_init(e, &(d->bg_coords_range), 3, 0, 1);
#ifdef DEBUG
  fprintf(stderr, "Done\n");
#endif
  return d;
  }

static void draw_boioioing(lemuria_engine_t * e, void * data)
  {
  float bg[4][3];
  int i;
  boioioing_data * d = (boioioing_data*)(data);

  /* Update everything */

  if(e->beat_detected)
    {
    if(lemuria_range_done(&(d->bg_coords_range)) &&
       lemuria_decide(e, 0.2))
      {
      d->bg_coords_start = d->bg_coords_end;
      d->bg_coords_end = lemuria_random_int(e, 0, num_bg_coords-2);
      if(d->bg_coords_end >= d->bg_coords_start)
        d->bg_coords_end++;
      lemuria_range_init(e, &(d->bg_coords_range), 3, 200, 300);
      }
    }

  lemuria_range_update(&(d->bg_coords_range));

  for(i = 0; i < 4; i++)
    {
    lemuria_range_get_cos(&(d->bg_coords_range),
                      bg_coords[d->bg_coords_start][i],
                      bg_coords[d->bg_coords_end][i],
                      bg[i]);
#define BG_FACTOR ((float)(e->width)/(float)(e->height))/(4.0/3.0)
#if 1
    bg[i][0] *= BG_FACTOR;
    bg[i][1] *= BG_FACTOR;
    bg[i][2] *= BG_FACTOR;

#endif
    //    fprintf(stderr, "{ %f %f %f }\n", bg[i][0], bg[i][1], bg[i][2] );
    }
  
  
  glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  lemuria_set_perspective(e, 1, 1000.0);

  lemuria_background_set(&(d->bg));
  lemuria_background_draw(&(d->bg),
                          bg,
                          4,
                          3,
                          &(d->bg_texture_start),
                          0.0,
                          &(d->bg_texture_flip));
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glDisable(GL_TEXTURE_2D);

  }

static void delete_boioioing(void * data)
  {
  boioioing_data * d = (boioioing_data*)(data);
  lemuria_background_delete(&(d->bg));

  free(d);
  }

effect_plugin_t boioioing_effect =
  {
    .init =    init_boioioing,
    .draw =    draw_boioioing,
    .cleanup = delete_boioioing,
  };
