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

#include <stdlib.h>

#include <GL/gl.h>

#include <lemuria_private.h>
#include <utils.h>

/* Deliver a number between min and max */

float lemuria_random(lemuria_engine_t * e, float min, float max)
  {
  return min + (float)rand()*(max - min)/(float)(RAND_MAX);
  }

int lemuria_random_int(lemuria_engine_t * e, int min, int max)
  {
  int den = max - min + 1;
  return (rand() % den) + min;
  }

/* return 1 with a probability of p */

int lemuria_decide(lemuria_engine_t * e, float p)
  {
  float rand_f = (float)(rand())/(float)(RAND_MAX) ;
  return rand_f < p ? 1 : 0;
  }
