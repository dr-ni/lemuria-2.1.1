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
#include <lemuria_private.h>
#include <utils.h>
#include <effect.h>

/* This is really dirty */

#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include "xaos/src/include/filter.h"
#include "xaos/src/include/ui_helper.h"

/* Another dirty trick: This is normally in ui.c,
   which we don't have */

// uih_context *uih;

struct lemuria_xaos_s
  {
  struct palette     *palette;
  struct image       *image;
  struct uih_context *uih;

  int refcount; /* Render only if needed */

  unsigned char * image_buffer;
  uint32_t texture;
  int bytes_per_pixel;
  };

// #define XAOS_SIZE 256

/* Dump palette */
#if 0
static void dump_palette(struct palette * p)
  {
  int i;
  fprintf(stderr, "start: %d, end: %d, max: %d, pixels : %p\n",
          p->start, p->end, p->maxentries, p->pixels);
  for(i = 0; i < 256; i++)
    {
    fprintf(stderr, "%08x\n", p->pixels[i]);
    }
  }
#endif

#if 0
static int passfunc(struct uih_context * uih,
                    int display, const char * text, float percent)
  {
  fprintf(stderr, "Passfunc %f\n", percent);
  return 0;
  }
#endif
/* Initialize and delete */

void lemuria_xaos_create(lemuria_engine_t * e)
  {
  e->xaos = calloc(1, sizeof(*(e->xaos)));
  //  tl_init();
  glGenTextures(1, &(e->xaos->texture));
  glBindTexture(GL_TEXTURE_2D, e->xaos->texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* Create palette */

  /* TRUECOLOR24 is also possible */
  
  e->xaos->palette =
    createpalette (0, 255, TRUECOLOR, 0, 0, NULL, NULL, NULL, NULL, NULL);
  e->xaos->bytes_per_pixel = bytesperpixel(TRUECOLOR);

  /* Create image */
  
  e->xaos->image_buffer = malloc(3 * XAOS_SIZE *
                                 XAOS_SIZE);

  e->xaos->image =
    create_image_mem (XAOS_SIZE, XAOS_SIZE,
                      2, e->xaos->palette,
                      1.0, 1.0);
  /* Create ui context */

  e->xaos->uih = uih_mkcontext(0, e->xaos->image,
                               NULL,/*int (*passfunc) (struct uih_context *, int, CONST char *, float),*/
                               NULL,/*void (*longwait) (struct uih_context *),*/
                               NULL/*void (*updatemenus) (struct uih_context * c, CONST char *)*/);
  
  uih_autopilot_on (e->xaos->uih);

  uih_constantframetime(e->xaos->uih, 1000000 / 30);
  //  uih_setformula(e->xaos->uih, lemuria_random_int(0, nformulas-1));
  //  uih_mkpalette(e->xaos->uih);
  //  uih_mkdefaultpalette(e->xaos->uih);
  }

void lemuria_xaos_destroy(lemuria_engine_t * e)
  {
  glDeleteTextures(1, &(e->xaos->texture));
  free(e->xaos);
  }

/* Make next picture */

//static long double angle = 0;


void lemuria_xaos_update(lemuria_engine_t * e)
  {
  unsigned char * src, *dst;
  int i;
  if(!e->xaos->refcount)
    return;
  // e->xaos->uih->fcontext->version++;
  //  tl_process_group (syncgroup, NULL);
  uih_update (e->xaos->uih, 0, 0, 0);
  
  uih_prepare_image(e->xaos->uih);

  //  fprintf(stderr, "lemuria_xaos_update %d\n", e->xaos->uih->recalculatemode);

  //  uih_animate_image(e->xaos->uih);
  uih_drawwindows(e->xaos->uih);
  
  if(e->xaos->uih->errstring)
    {
    fprintf(stderr, "Error: %s\n", e->xaos->uih->errstring);
    }
  
  //  fprintf(stderr, "\n");
  
  src = e->xaos->uih->image->currlines[0];
  dst = e->xaos->image_buffer;
    
  for(i = 0; i < XAOS_SIZE*XAOS_SIZE; i++)
    {
    dst[0] = src[2];
    dst[1] = src[1];
    dst[2] = src[0];
    dst += 3;
    src += e->xaos->bytes_per_pixel;
    }

  //  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, e->xaos->texture);
  glTexImage2D(GL_TEXTURE_2D, 0, 3,
               XAOS_SIZE,
               XAOS_SIZE,
               0, GL_RGB, GL_UNSIGNED_BYTE,
               e->xaos->image_buffer);
  //  glDisable(GL_TEXTURE_2D);

  
  uih_displayed (e->xaos->uih);
  
  }

/* Ref, unref */

void lemuria_xaos_ref(lemuria_engine_t * e)
  {
  int random_number;
#ifdef DEBUG
  fprintf(stderr, "lemuria_xaos_ref\n");
#endif
  if(!e->xaos->refcount)
    {
    random_number =
      lemuria_random_int(e, 0,
                         nformulas - 1);
    uih_setformula(e->xaos->uih, random_number);

    //    uih_setformula(e->xaos->uih, 2);
    uih_mkpalette(e->xaos->uih);
    //    dump_palette(e->xaos->uih->palette);
    }
  e->xaos->refcount++;
  }

void lemuria_xaos_unref(lemuria_engine_t * e)
  {
#ifdef DEBUG
  fprintf(stderr, "lemuria_xaos_unref\n");
#endif
  e->xaos->refcount--;
  }

/* Bind the xaos texture */

void lemuria_xaos_bind(lemuria_engine_t * e)
  {
  glBindTexture(GL_TEXTURE_2D, e->xaos->texture);
  }

/* Background Effect, just for testing */

typedef struct
  {
  lemuria_engine_t * engine;
  } xaos_data;

static void * init_xaos(lemuria_engine_t * e)
  {
  xaos_data * ret = calloc(1, sizeof(*ret));
  lemuria_xaos_ref(e);
  ret->engine = e;
  return ret;
  }

#define XAOS_HALF_WIDTH 1.0
#define XAOS_HALF_HEIGHT 1.0

static void draw_xaos(lemuria_engine_t * e, void * data)
  {
  glClearColor(0.0, 0.0, 0.4, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  lemuria_set_perspective(e, 1, 1000.0);
  lemuria_xaos_bind(e);
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-XAOS_HALF_WIDTH, -XAOS_HALF_HEIGHT, 0.0);
  
  glTexCoord2f(1.0, 0.0);
  glVertex3f( XAOS_HALF_WIDTH, -XAOS_HALF_HEIGHT, 0.0);
  
  glTexCoord2f(1.0, 1.0);
  glVertex3f( XAOS_HALF_WIDTH, XAOS_HALF_HEIGHT, 0.0);
  
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-XAOS_HALF_WIDTH, XAOS_HALF_HEIGHT, 0.0);
  
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  }

static void delete_xaos(void * d)
  {
  xaos_data * data = (xaos_data*)d;
  lemuria_xaos_unref(data->engine);
  free(data);
  }

effect_plugin_t xaos_effect =
  {
    .init =    init_xaos,
    .draw =    draw_xaos,
    .cleanup = delete_xaos,
  };
