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
#include <utils.h>

/* Switch perspective mode on and off */

void lemuria_set_perspective(lemuria_engine_t * e, int perspective,
                                 float range)
  {
  if(perspective)
    {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 45.0, (float)(e->width)/(float)(e->height), 0.1, range );
    gluLookAt(CAMERA_X, CAMERA_Y, CAMERA_Z,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0 ); 
    }
  else
    {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    }
  }

