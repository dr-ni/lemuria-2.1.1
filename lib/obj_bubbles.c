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

#include <stdio.h>
#include <GL/gl.h>

#include <lemuria_private.h>
#include <utils.h>
#include <object.h>

#include "images/object_bubble.incl"

#define MIN_SIZE 0.2
#define MAX_SIZE 1.0

void lemuria_obj_bubbles_init(lemuria_engine_t * e,
                              lemuria_object_t * obj);


static void update_bubbles(lemuria_engine_t * e,
                           lemuria_object_t * obj)
  {
  int i;
  for(i = 0; i < OBJ_NUM_BUBBLES; i++)
    {
    obj->data.bubbles.bubble_speeds[i][0] *= 0.95;
    obj->data.bubbles.bubble_speeds[i][2] *= 0.95;
        
    obj->data.bubbles.particles->particles[i].x += obj->data.bubbles.bubble_speeds[i][0];
    obj->data.bubbles.particles->particles[i].y += obj->data.bubbles.bubble_speeds[i][1];
    obj->data.bubbles.particles->particles[i].z += obj->data.bubbles.bubble_speeds[i][2];

    if(obj->data.bubbles.particles->particles[i].y > 20.0)
      {
      obj->data.bubbles.particles->particles[i].y = 0.0;
      obj->data.bubbles.bubble_speeds[i][0] = lemuria_random(e, -0.05, 0.05);
      obj->data.bubbles.bubble_speeds[i][1] = lemuria_random(e, 0.15, 0.25);
      obj->data.bubbles.bubble_speeds[i][2] = lemuria_random(e, -0.05, 0.05);
      }
    }
  }

static void cleanup_bubbles(lemuria_object_t * obj)
  {
  lemuria_destroy_particles(obj->data.bubbles.particles);
  }

static void draw_bubbles(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  lemuria_draw_particles(obj->data.bubbles.particles);
  }

void lemuria_obj_bubbles_init(lemuria_engine_t * e,
                              lemuria_object_t * obj)
  {
  int i;
  obj->data.bubbles.particles =
    lemuria_create_particles(e, OBJ_NUM_BUBBLES,
                             object_bubble_width,
                             (uint8_t*)object_bubble_data,
                             MIN_SIZE,
                             MAX_SIZE);

  for(i = 0; i < OBJ_NUM_BUBBLES; i++)
    {
    obj->data.bubbles.particles->particles[i].y =
      lemuria_random(e, 0.0, 20.0);

    obj->data.bubbles.bubble_speeds[i][0] = lemuria_random(e, -0.05, 0.05);
    obj->data.bubbles.bubble_speeds[i][1] = lemuria_random(e, 0.15, 0.25);
    obj->data.bubbles.bubble_speeds[i][2] = lemuria_random(e, -0.05, 0.05);
    }
  
  
  obj->update  = update_bubbles;
  obj->draw    = draw_bubbles;
  obj->cleanup = cleanup_bubbles;
  
  }
