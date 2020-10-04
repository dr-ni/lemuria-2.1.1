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

#include <material.h>

void lemuria_set_material(lemuria_material_t * mat, int which)
  {
  glMaterialfv(which, GL_SPECULAR, mat->ref_specular);
  glMaterialfv(which, GL_AMBIENT, mat->ref_ambient);
  glMaterialfv(which, GL_DIFFUSE, mat->ref_diffuse);
  glMateriali(which, GL_SHININESS, mat->shininess);
  }
