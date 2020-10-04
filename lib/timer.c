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

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <stdio.h>

#include <lemuria_private.h>

#define SECONDS_PER_DAY      86400
#define MILLISECONDS_PER_DAY 86400000

#define MAX_FRAME_RATE  40.0

static void lemuria_gettime(int32_t * ret)
  {
  struct timeval tm;
  gettimeofday(&tm, NULL);
  
  *ret = (tm.tv_sec % SECONDS_PER_DAY) * 1000 + tm.tv_usec / 1000;
  }

void lemuria_wait(lemuria_engine_t * e)
  {
  int32_t current_time;
  int32_t next_frame_time;
  int32_t diff_time;

  struct timeval delay_duration;
  
  if(e->last_frame_time == -1)
    {
    lemuria_gettime(&e->last_frame_time);
    //    fprintf(stderr, "%d\n", e->last_frame_time);
    return;
    }
  else
    {
    next_frame_time = e->last_frame_time + (int)(1000.0/MAX_FRAME_RATE + 0.5);
    
    lemuria_gettime(&current_time);

    // Especially for psychedelic parties, we must prevent 
    // lemuria from hanging up at midnight

    //    fprintf(stderr, "%d %d %d", current_time, next_frame_time, e->last_frame_time);
        
    if(next_frame_time > MILLISECONDS_PER_DAY)
      next_frame_time -= MILLISECONDS_PER_DAY;
    
    diff_time = next_frame_time - current_time;
    //    fprintf(stderr, " %d\n", diff_time);
    if(diff_time > 5)
      {
      delay_duration.tv_sec = diff_time /  1000;
      delay_duration.tv_usec = (diff_time % 1000) * 1000;
      
      select(0,  NULL,  NULL, NULL, &delay_duration);
      e->last_frame_time = next_frame_time;
      }
    else
      e->last_frame_time = current_time;
    }
  
  }
