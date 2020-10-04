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
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include <config.h>
#include <lemuria_private.h>


// #define GOOM_WIDTH  256
// #define GOOM_HEIGHT 256

static char * goom2_dlls[] =
  {
    /* Look here in the hope, that gmerlin_goom was
       installed under the same prefix */
    PREFIX"/lib/libgoom2.so.0",
    /* Let ld.so try to find it */
    "libgoom2.so.0",
    /* Hopefully it won't be there */
    "/usr/lib/xmms-goom/libgoom2.so",
    (char*)0
  };

typedef struct
  {
  /* Goom2 */

  void * goom2_instance;
    
  void * (*goom2_init)(uint32_t resx, uint32_t resy);

  char * (*goom2_update)(void *goomInfo, int16_t data[2][512],
                             int forceMode, float fps,
                             char *songTitle, char *message);
  void (*goom2_close) (void *goomInfo);
  
  void * module;
  uint32_t texture;
  int refcount;
  } goom_t;

void lemuria_goom_create(lemuria_engine_t * e)
  {
  int i = 0;
  goom_t * goom;  

  goom = calloc(1, sizeof(goom_t));

  while(goom2_dlls[i])
    {
    if((goom->module = dlopen(goom2_dlls[i], RTLD_NOW)) &&
       (goom->goom2_init = dlsym(goom->module, "goom_init")) &&
       (goom->goom2_update = dlsym(goom->module, "goom_update")) &&
       (goom->goom2_close = dlsym(goom->module, "goom_close")))
      {
      goom->goom2_instance = goom->goom2_init(GOOM_SIZE, GOOM_SIZE);
      e->goom = goom;
      fprintf(stderr, "Found goom-2k4 (DLL: %s)\n", goom2_dlls[i]);
      return;
      }
    i++;
    }
  return;
  }

void lemuria_goom_update(lemuria_engine_t * e)
  {
  char * goom_image;
  int i;
  
  goom_t * goom = (goom_t*)(e->goom);

  if(!goom || !goom->refcount)
    return;
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, goom->texture);

  goom_image = goom->goom2_update(goom->goom2_instance, e->time_buffer_read, 0, -1,
                              (char*)0, (char*)0);
  
  for(i = 0; i < GOOM_SIZE*GOOM_SIZE; i++)
    goom_image[4*i+3] = 0xff;
    
  glTexImage2D(GL_TEXTURE_2D, 0, 4, GOOM_SIZE, GOOM_SIZE,
               0, GL_RGBA, GL_UNSIGNED_BYTE, goom_image);
  glDisable(GL_TEXTURE_2D);
  
  }

void lemuria_goom_destroy(lemuria_engine_t * e)
  {
  goom_t * goom = (goom_t*)(e->goom);
  
  if(!goom)
    return;
  
  //  glDeleteTextures(1, &(goom->texture));

  goom->goom2_close(goom->goom2_instance);
  dlclose(goom->module);
  free(e->goom);
  e->goom = (void*)0;
  }

void lemuria_goom_bind(lemuria_engine_t * e)
  {
  glBindTexture(GL_TEXTURE_2D, ((goom_t*)e->goom)->texture);
  }

void lemuria_goom_ref(lemuria_engine_t * e)
  {
  goom_t * goom = (goom_t*)e->goom;
  if(!goom->refcount)
    {
    glGenTextures(1, &(goom->texture));
    glBindTexture(GL_TEXTURE_2D, goom->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 4,
                 GOOM_SIZE,
                 GOOM_SIZE,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 (char*)0);
    }
  goom->refcount++;
  }

int lemuria_goom_refcount(lemuria_engine_t * e)
  {
  return ((goom_t*)e->goom)->refcount;
  }

void lemuria_goom_unref(lemuria_engine_t * e)
  {
  goom_t * goom = (goom_t*)e->goom;
  goom->refcount--;
  if(!goom->refcount)
    glDeleteTextures(1, &(goom->texture));
  }

