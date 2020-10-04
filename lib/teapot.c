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

void lemuria_teapot_init(lemuria_engine_t * e,
                         lemuria_object_t * obj);


static lemuria_material_t teapot_materials[] =
  {
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.219f, 0.416f, 0.000f, 1.000f },
      .ref_diffuse =  { 0.439f, 0.833f, 0.000f, 1.000f },
      .shininess = 128
    },
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.502f, 0.000f, 0.283f, 1.000f },
      .ref_diffuse =  { 1.000f, 0.000f, 0.564f, 1.000f },
      .shininess = 128
    },
    {
      .ref_specular = { 1.0f, 1.0f, 1.0f, 1.0f },
      .ref_ambient =  { 0.502f, 0.366f, 0.000f, 1.000f },
      .ref_diffuse =  { 1.000f, 0.729f, 0.000f, 1.000f },
      .shininess = 128
    },
    
  };

static void draw_teapot(lemuria_engine_t * e, lemuria_object_t * obj)
  {
  glShadeModel(GL_SMOOTH);
 
  //  lemuria_set_perspective(e, 1, 1000.0);

  lemuria_set_material(&teapot_materials[obj->data.teapot.material],
                                         GL_FRONT_AND_BACK);

  
  lemuria_rotate(&(obj->data.teapot.rotator));
  
  lemuriaSolidTeapot(2.0);
    
  }

static void update_teapot(lemuria_engine_t * e,
                          lemuria_object_t * obj)
  {
  if(obj->type == LEMURIA_OBJECT_TEAPOT)
    lemuria_rotator_update(&(obj->data.teapot.rotator));
  }

void lemuria_teapot_init(lemuria_engine_t * e,
                         lemuria_object_t * obj)
  {
  lemuria_rotator_init(e, &(obj->data.teapot.rotator));
  obj->draw = draw_teapot;
  obj->update = update_teapot;

  obj->data.teapot.material = lemuria_random_int(e, 0,
                                                 sizeof(teapot_materials) /
                                                 sizeof(teapot_materials[0])-1);
  }
