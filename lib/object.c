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

#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <stdio.h>
#include <lemuria_private.h>
#include <utils.h>
#include <object.h>

extern void lemuria_ufo_init(lemuria_engine_t * e,
                             lemuria_object_t * obj);

extern void lemuria_sputnik_init(lemuria_engine_t * e,
                                 lemuria_object_t * obj);

extern void lemuria_teapot_init(lemuria_engine_t * e,
                                lemuria_object_t * obj);

extern void lemuria_satellite_init(lemuria_engine_t * e,
                                   lemuria_object_t * obj);

extern void lemuria_fish_init(lemuria_engine_t * e,
                              lemuria_object_t * obj);

extern void lemuria_manta_init(lemuria_engine_t * e,
                              lemuria_object_t * obj);

extern void lemuria_platon_init(lemuria_engine_t * e,
                                lemuria_object_t * obj);

extern void lemuria_obj_bubbles_init(lemuria_engine_t * e,
                                     lemuria_object_t * obj);

extern void lemuria_seaplant_init(lemuria_engine_t * e,
                                  lemuria_object_t * obj);


float light_ambient[4]  = { 0.3f, 0.3f, 0.3f, 1.0f };
float light_diffuse[4]  = { 0.6f, 0.6f, 0.6f, 1.0f };
float light_specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

void lemuria_object_init(lemuria_engine_t * e,
                         lemuria_object_t * obj,
                         lemuria_object_type_t type,
                         float * coords,
                         float * delta_coords)
  {
  obj->type = type;
  memcpy(obj->coords, coords, 3*sizeof(float));
  memcpy(obj->delta_coords, delta_coords, 3*sizeof(float));
  switch(type)
    {
    case LEMURIA_OBJECT_UFO:
      lemuria_ufo_init(e, obj);
      break;
    case LEMURIA_OBJECT_SPUTNIK:
      lemuria_sputnik_init(e, obj);
      break;
    case LEMURIA_OBJECT_TEAPOT:
    case LEMURIA_OBJECT_TEAPOT_STATIC:
      lemuria_teapot_init(e, obj);
      break;
    case LEMURIA_OBJECT_SATELLITE:
      lemuria_satellite_init(e, obj);
      break;
    case LEMURIA_OBJECT_FISH:
      lemuria_fish_init(e, obj);
      break;
    case LEMURIA_OBJECT_PLATON:
      lemuria_platon_init(e, obj);
      break;
    case LEMURIA_OBJECT_MANTA:
      lemuria_manta_init(e, obj);
      break;
    case LEMURIA_OBJECT_BUBBLES:
      lemuria_obj_bubbles_init(e, obj);
      break;
    case LEMURIA_OBJECT_SEAPLANT:
      lemuria_seaplant_init(e, obj);
      break;
    case LEMURIA_NUM_OBJECTS:
      break;
    }
  }

void lemuria_object_update(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  obj->coords[0] += obj->delta_coords[0];
  obj->coords[1] += obj->delta_coords[1];
  obj->coords[2] += obj->delta_coords[2];
  if(obj->update)
    obj->update(e, obj);
  }

void lemuria_object_draw(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  float light_position[4];

  //  light_position[3] = 0.0;

  light_position[0] = 10.0;
  light_position[1] = 10.0;
  light_position[2] = 10.0;
  light_position[3] = 0.0;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(obj->coords[0], obj->coords[1], obj->coords[2]);
    
  /* Set up light */

  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glEnable(GL_LIGHT0);

  light_position[0] = -10.0;
  light_position[1] = -10.0;

  glLightfv(GL_LIGHT1, GL_POSITION, light_position);
  glLightfv(GL_LIGHT1, GL_AMBIENT,  light_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glEnable(GL_LIGHT1);
  
  glEnable(GL_LIGHTING);
    
  obj->draw(e, obj);

  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHT1);
  glPopMatrix();
    
  }

void lemuria_object_cleanup(lemuria_object_t * obj)
  {
  if(obj->cleanup)
    obj->cleanup(obj);
  memset(obj, 0, sizeof(*obj));
  }
                           
void lemuria_object_rotate(lemuria_object_t * obj)
  {
  float angle_z;
  float angle_y;

  angle_y = -atan2(obj->delta_coords[2], obj->delta_coords[0]);

  angle_z =  atan2(obj->delta_coords[1], cos(angle_y) * obj->delta_coords[0]);
  //  fprintf(stderr, "%f %f %f, Angle_y: %f, Angle_z: %f\n",
  //          obj->delta_coords[0], obj->delta_coords[1], obj->delta_coords[2],
  //          180.0*angle_y/M_PI, 180.0*angle_z/M_PI);
  glRotatef(180.0*angle_y/M_PI, 0.0, 1.0, 0.0);
  glRotatef(180.0*angle_z/M_PI, 0.0, 0.0, 1.0);
  
  }
