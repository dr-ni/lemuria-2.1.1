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
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <lemuria_private.h>
#include <utils.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define BEAT_MAX 200

/* Config values from bursk */

#define BEAT_SENSITIVITY 4

typedef struct
  {
  int32_t beathistory[BEAT_MAX];
  int     beatbase;
  int32_t aged;           /* smoothed out loudness */

  int32_t lowest;         /* quietest point in current beat */
  int     elapsed;        /* frames since last beat */
  int     isquiet;        /* was previous frame quiet */
  int     prevbeat;       /* period of previous beat */
  
  } lemuria_analysis;

static int detect_beat(lemuria_analysis * a,
                       int32_t loudness, int *thickref, int *quietref)
  {
  int             beat, i, j;
  int32_t         total;
  int             sensitivity;
  
  /* Incorporate the current loudness into history */
  a->aged = (a->aged * 7 + loudness) >> 3;
  a->elapsed++;
  
  /* If silent, then clobber the beat */
  if (a->aged < 2000 || a->elapsed > BEAT_MAX)
    {
    a->elapsed = 0;
    a->lowest = a->aged;
    memset(a->beathistory, 0, sizeof a->beathistory);
    }
  else if (a->aged < a->lowest)
    a->lowest = a->aged;
  
  
  /* Beats are detected by looking for a sudden loudness after a lull.
   * They are also limited to occur no more than once every 15 frames,
   * so the beat flashes don't get too annoying.
         */
  j = (a->beatbase + a->elapsed) % BEAT_MAX;
  a->beathistory[j] = loudness - a->aged;
  beat = FALSE;
  if (a->elapsed > 15 && a->aged > 2000 && loudness * 4 > a->aged * 5)
    {
    /* Compute the average loudness change, assuming this is beat */                for (i = BEAT_MAX / a->elapsed, total = 0;
                                                                                         --i > 0;
                                                                                         j = (j + BEAT_MAX - a->elapsed) % BEAT_MAX)
      {
      total += a->beathistory[j];
      }
    total = total * a->elapsed / BEAT_MAX;
    
    /* Tweak the sensitivity to emphasize a consistent rhythm */
    sensitivity = BEAT_SENSITIVITY;
    i = 3 - abs(a->elapsed - a->prevbeat)/2;
    if (i > 0)
      sensitivity += i;
    
    /* If average change is significantly positive, this is a beat.
     */
    if (total * sensitivity > a->aged)
      {
      a->prevbeat = a->elapsed;
      a->beatbase = (a->beatbase + a->elapsed) % BEAT_MAX;
      a->lowest = a->aged;
      a->elapsed = 0;
      beat = TRUE;
      }
    }
  
  /* Thickness is computed from the difference between the instantaneous
   * loudness and the a->aged loudness.  Thus, a sudden increase in volume
   * will produce a thick line, regardless of rhythm.
   */
  if (a->aged < 1500)
    *thickref = 0;
  else
    {
    *thickref = loudness * 2 / a->aged;
    if (*thickref > 3)
      *thickref = 3;
    }
  
  /* Silence is computed from the a->aged loudness.  The quietref value is
   * set to TRUE only at the start of silence, not throughout the silent
   * period.  Also, there is some hysteresis so that silence followed
   * by a slight noise and more silence won't count as two silent
   * periods -- that sort of thing happens during many fade edits, so
   * we have to account for it.
   */
  if (a->aged < (a->isquiet ? 1500 : 500))
    {
    /* Quiet now -- is this the start of quiet? */
    *quietref = !a->isquiet;
    a->isquiet = TRUE;
    }
  else
    {
    *quietref = FALSE;
    a->isquiet = FALSE;
    }
  
  /* return the result */
  return beat;
  }

void lemuria_analysis_perform(lemuria_engine_t * e)
  {
  int i, imin, imax, start;
  int32_t delta_sum;

  lemuria_analysis * a = (lemuria_analysis*)e->analysis;

  /* Find the maximum and minimum, with the restriction that
   * the minimum must occur after the maximum.
   */
  for (i = 1, imin = imax = 0, delta_sum = 0; i < 127 / 2; i++)
    {
    if (e->time_buffer_read[0][i] < e->time_buffer_read[0][imin])
      imin = i;
    if (e->time_buffer_read[0][i] > e->time_buffer_read[0][imax])
      imin = imax = i;
    delta_sum += abs(e->time_buffer_read[0][i] - e->time_buffer_read[0][i - i]);
    }
  
  /* Triggered sweeps start halfway between min & max */
  start = (imax + imin) / 2;
  
  /* Compute the loudness.  We don't want to do a full spectrum analysis
   * to do this, but we can guess the low-frequency sound is proportional
   * to the maximum difference found (because loud low frequencies need
   * big signal changes), and that high-frequency sound is proportional
   * to the differences between adjacent samples.  We want to be sensitive
   * to both of those, while ignoring the mid-range sound.
   *
   * Because we have only one low-frequency difference, but hundreds of
   * high-frequency differences, we need to give more weight to the
   * low-frequency difference (even though each high-frequency difference
   * is small).
   */
  
  e->loudness = (((int32_t)e->time_buffer_read[0][imax] -
                  (int32_t)e->time_buffer_read[0][imin]) * 60 + delta_sum) / 75;

  e->beat_detected = detect_beat(a, e->loudness, &(e->thickness),
                                 &(e->quiet));
/*   if(e->beat_detected) */
/*     fprintf(stderr, "Beat Detected, %d %d %d\n", e->loudness, e->thickness, */
/*             e->quiet); */
  
  }

void * lemuria_analysis_init()
  {
  lemuria_analysis * ret = calloc(1, sizeof(lemuria_analysis));  
  return ret;
  }

void lemuria_analysis_cleanup(void * _a)
  {
  lemuria_analysis * a = (lemuria_analysis *)_a;
  free(a);
  }

