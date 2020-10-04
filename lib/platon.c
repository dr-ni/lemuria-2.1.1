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

#include <math.h>

#include <lemuria_private.h>
#include <utils.h>
#include <object.h>
#include <light.h>
#include <material.h>

void lemuria_platon_init(lemuria_engine_t * e,
                         lemuria_object_t * obj);


static lemuria_material_t platon_materials[] =
  {
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.366f, 0.349f, 0.471f, 1.000f },
      .ref_diffuse =  { 0.735f, 0.699f, 0.944f, 1.000f },
      .shininess = 128
    },
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.349f, 0.471f, 0.378f, 1.000f },
      .ref_diffuse =  { 0.700f, 0.945f, 0.758f, 1.000f },
      .shininess = 128
    },
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.471f, 0.415f, 0.349f, 1.000f },
      .ref_diffuse =  { 0.945f, 0.834f, 0.700f, 1.000f },
      .shininess = 128
    },
    
  };

static void draw_platon(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  float speed_norm[3];
  float speed_abs;
  int i;
  glEnable(GL_NORMALIZE);
  glShadeModel(GL_SMOOTH);
 
  //  lemuria_set_perspective(e, 1, 1000.0);

  speed_abs = sqrt(obj->delta_coords[0] * obj->delta_coords[0] +
                   obj->delta_coords[1] * obj->delta_coords[1] +
                   obj->delta_coords[2] * obj->delta_coords[2]);
  
  speed_norm[0] = obj->delta_coords[0] / speed_abs;
  speed_norm[1] = obj->delta_coords[1] / speed_abs;
  speed_norm[2] = obj->delta_coords[2] / speed_abs;
  
  lemuria_set_material(&platon_materials[obj->data.platon.material],
                                         GL_FRONT_AND_BACK);
  
  glMatrixMode(GL_MODELVIEW);
  
  for(i = 0; i < obj->data.platon.num; i++)
    {
    glPushMatrix();
    glTranslatef(-1.5 * speed_norm[0] + i * 3.0 * speed_norm[0],
                 -1.5 * speed_norm[1] + i * 3.0 * speed_norm[1],
                 -1.5 * speed_norm[2] + i * 3.0 * speed_norm[2]);
    lemuria_rotate(&(obj->data.platon.rotator));
    

    switch(obj->data.platon.type)
      {
      case 0:
        lemuriaSolidTetrahedron();
        break;
      case 1:
        lemuriaSolidCube(1.0);
      break;
      case 2:
        lemuriaSolidOctahedron();
        break;
      case 3:
        glScalef(0.7, 0.7, 0.7);
        lemuriaSolidDodecahedron();
        break;
      case 4:
        lemuriaSolidIcosahedron();
        break;
      }
    glPopMatrix();
    }
  glDisable(GL_NORMALIZE);
  }

static void update_platon(lemuria_engine_t * e,
                          lemuria_object_t * obj)
  {
  if(e->beat_detected && lemuria_decide(e, 0.2))
    lemuria_rotator_change(e, &(obj->data.platon.rotator));
  lemuria_rotator_update(&(obj->data.platon.rotator));
  
  }

void lemuria_platon_init(lemuria_engine_t * e,
                         lemuria_object_t * obj)
  {
  lemuria_rotator_init(e, &(obj->data.platon.rotator));
  obj->draw = draw_platon;
  obj->update = update_platon;

  obj->data.platon.material = lemuria_random_int(e, 0,
                                                 sizeof(platon_materials) /
                                                 sizeof(platon_materials[0])-1);
  obj->data.platon.type = lemuria_random_int(e, 0, 4);
  obj->data.platon.num  = lemuria_random_int(e, 3, 6);
  }
