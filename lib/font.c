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

#include <lemuria_private.h>

#include <stdio.h>

#include <stdlib.h>
#include <string.h>


#include <utils.h>

#include "font.incl"

/* Text support */

#define MAX_LINES 16
#define MAX_CHARS 1024

/* Fade out modes */

#define FADE_ALPHA    0
#define FADE_ROTATE_1 1
#define FADE_ROTATE_2 2
#define FADE_ROTATE_3 3

#define FADE_MAX      3

typedef struct
  {
  struct
    {
    char * text;
    float ll_x;
    float ll_y;
    float width;
    }lines[MAX_LINES];
  char text[MAX_CHARS];
  int frames;

  lemuria_range_t fade_range;
  int fade_mode;
    
  int num_lines;
  float size;
  } text_struct;

typedef struct
  {
  text_struct text;
  float height; /* Font height (in texture coord units)*/ 
  unsigned int texture;
  } font_struct;

void lemuria_font_init(lemuria_engine_t * e)
  {
  font_struct * f;

  unsigned char * buffer;
  int i;
  unsigned char * src;
  unsigned char * dst;
  
  f = calloc(1, sizeof(*f));

  buffer = malloc(FONT_TEXTURE_SIZE * FONT_TEXTURE_SIZE * 2);
  src = font_pixels;
  dst = buffer;
  
  for(i = 0; i < FONT_TEXTURE_SIZE * FONT_TEXTURE_SIZE; i++)
    {
    *dst = 0xff;
    dst++;
    *dst = *src;
    dst++;
    src++;
    }
  
  glGenTextures(1, &(f->texture));

  glBindTexture(GL_TEXTURE_2D, f->texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, 2,
               FONT_TEXTURE_SIZE,
               FONT_TEXTURE_SIZE,
               0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
               buffer);

  free(buffer);
    
  f->height = texture_coords[0][3] - texture_coords[0][1]; 
  
  e->font = f;
  }

void lemuria_font_destroy(lemuria_engine_t * e)
  {
  
  }

/* Utility functions */
/* Put one character to the screen, return the x-coordinate of
   the next character */

static float put_char(font_struct * f, char c,
                      float ll_x, float ll_y, float size)
  {
  float width, height;
  int char_index;

  char_index = c - 0x20;
  if((char_index < 0) || (char_index >= 96))
    return 0.0;
  
  width  =  size*(texture_coords[char_index][2]-texture_coords[char_index][0]);
  height = -size*(texture_coords[char_index][3]-texture_coords[char_index][1]);
    
  glBegin(GL_QUADS);
  
  glTexCoord2f(texture_coords[char_index][0], texture_coords[char_index][1]);
  glVertex2f(ll_x, ll_y);

  glTexCoord2f(texture_coords[char_index][2], texture_coords[char_index][1]);
  glVertex2f(ll_x + width, ll_y);

  glTexCoord2f(texture_coords[char_index][2], texture_coords[char_index][3]);
  glVertex2f(ll_x + width, ll_y + height);

  glTexCoord2f(texture_coords[char_index][0], texture_coords[char_index][3]);
  glVertex2f(ll_x, ll_y + height);
    
  glEnd();
  return ll_x + width;
  }

static void put_line(font_struct * f, char * line,
                     float ll_x, float ll_y, float size)
  {
  char * pos;
  pos = line;
  while(*pos != '\0')
    {
    ll_x = put_char(f, *pos, ll_x, ll_y, size);
    pos++;
    }
  }

static float line_width(const char * line)
  {
  float ret;
  const char * pos;

  int char_index;

  pos = line;
  ret = 0.0;
  
  while(*pos != '\0')
    {
    char_index = *pos - 0x20;
    if((char_index < 0) || (char_index >= 96))
      {
      pos++;
      continue;

      }
    ret += (texture_coords[char_index][2] - texture_coords[char_index][0]);
    pos++;
    }
  return ret;
  }

/* Text output */

void lemuria_put_text(lemuria_engine_t * e,
                      char * text,
                      float justify_h,
                      float justify_v,
                      float pos_x,
                      float pos_y,
                      float size,
                      int frames_to_live)
  {
  int i;
  
  char * pos;
  float max_width;

  font_struct * f;    
  /* Copy text */
  
  if(strlen(text) > MAX_CHARS)
    {
    fprintf(stderr, "Set MAX_CHARS to %d in %s\n",
            strlen(text)+1, __FILE__);
    return;
    }
  f = (font_struct*)(e->font);

  /* Check for free text structure */
  
  strcpy(f->text.text, text);

  pos = f->text.text;

  f->text.size = size;
  f->text.frames = frames_to_live;
  f->text.lines[0].text = pos;
  f->text.num_lines = 1;

  while(1)
    {
    if(!(pos = strchr(pos, '\n')))
      break;
    else
      {
      
      *pos = '\0';
      pos++;
      f->text.lines[f->text.num_lines].text =
        pos;
      f->text.num_lines++;
      }
    }

  /* Get the line widths */
  max_width = 0.0;
  
  for(i = 0; i < f->text.num_lines; i++)
    {
    f->text.lines[i].width =
      line_width(f->text.lines[i].text);
    if(max_width < f->text.lines[i].width)
      max_width = f->text.lines[i].width;
    }
  /* Now, calculate the positions of the lines */
  
  for(i = 0; i < f->text.num_lines; i++)
    {
    f->text.lines[i].ll_x = pos_x -
      f->text.size * justify_h * f->text.lines[i].width;
    f->text.lines[i].ll_y = pos_y +
      f->text.size * (-justify_v * f->text.num_lines * f->height
                      + (i+1) * f->height);
    }
  
  
  }

static void draw_text_normal(font_struct * f)
  {
  int i;
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, f->texture);
  glColor3f(1.0, 1.0, 1.0);

  for(i = 0; i < f->text.num_lines; i++)
    {
    put_line(f, f->text.lines[i].text, 
             f->text.lines[i].ll_x,
             f->text.lines[i].ll_y,
             f->text.size);
    }
  
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  }

static void draw_text_alpha(font_struct * f, float progress)
  {
  int i;
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, f->texture);
  glColor4f(1.0, 1.0, 1.0, 1.0 - progress);
  
  for(i = 0; i < f->text.num_lines; i++)
    {
    put_line(f, f->text.lines[i].text, 
             f->text.lines[i].ll_x,
             f->text.lines[i].ll_y,
             f->text.size);
    }
  
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  }

static void draw_text_rotate_1(font_struct * f, float progress)
  {
  int i;

  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, f->texture);
  glColor3f(1.0, 1.0, 1.0);
  
  glPushMatrix();

  glRotatef(- progress * 90.0, 1.0, 0.0, 0.0);
  
  for(i = 0; i < f->text.num_lines; i++)
    {
    put_line(f, f->text.lines[i].text, 
             f->text.lines[i].ll_x,
             f->text.lines[i].ll_y,
             f->text.size);
    }

  glPopMatrix();
  
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  }

static void draw_text_rotate_2(font_struct * f, float progress)
  {
  int i;

  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, f->texture);
  glColor3f(1.0, 1.0, 1.0);
  
  glPushMatrix();

  glRotatef(progress * 600.0, 0.0, 1.0, 0.0);
  glScalef(1.0 - progress, 1.0 - progress, 1.0 - progress);
    
  for(i = 0; i < f->text.num_lines; i++)
    {
    put_line(f, f->text.lines[i].text, 
             f->text.lines[i].ll_x,
             f->text.lines[i].ll_y,
             f->text.size);
    }

  glPopMatrix();
  
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  }

static void draw_text_rotate_3(font_struct * f, float progress)
  {
  int i;

  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, f->texture);
  glColor3f(1.0, 1.0, 1.0);
  
  glPushMatrix();

  glRotatef(progress * 600.0, 1.0, 0.0, 0.0);
  glScalef(1.0 - progress, 1.0 - progress, 1.0 - progress);
    
  for(i = 0; i < f->text.num_lines; i++)
    {
    put_line(f, f->text.lines[i].text, 
             f->text.lines[i].ll_x,
             f->text.lines[i].ll_y,
             f->text.size);
    }

  glPopMatrix();
  
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  }

void lemuria_text_update(lemuria_engine_t * e)
  {
  font_struct * f;
  float min = 0.0;
  float max = 1.0;
  float progress;

  f = ((font_struct*)(e->font));
  
  if(f->text.frames <= 0)
    {
    if(lemuria_range_done(&(f->text.fade_range)))
      return;

    lemuria_range_update(&(f->text.fade_range));
    lemuria_range_get(&(f->text.fade_range), &min, &max, &progress);
    
    switch(f->text.fade_mode)
      {
      case FADE_ALPHA:
        draw_text_alpha(f, progress);
        break;
      case FADE_ROTATE_1:
        draw_text_rotate_1(f, progress);
        break;
      case FADE_ROTATE_2:
        draw_text_rotate_2(f, progress);
        break;
      case FADE_ROTATE_3:
        draw_text_rotate_3(f, progress);
        break;
      }
    }
  else
    draw_text_normal(f);
  
  f->text.frames--;

  /* Switch to fade out mode */
    
  if(!f->text.frames)
    {
    lemuria_range_init(e, &(f->text.fade_range), 1, 50, 100);
    f->text.fade_mode = lemuria_random_int(e, 0, FADE_MAX);
    }
  }

