#ifndef __TENTACLE_H_
#define __TENTACLE_H_

/* Tentacle engine */

typedef struct
  {
  int num_points;
  struct
    {
    float coords[2];
    float normals[2];
    float angle;
    } * points;
  float frequency;
  float wavelength;
  float phase;
  float segment_length;
  float amplitude;
  } lemuria_tentacle_t;

void lemuria_tentacle_init(lemuria_tentacle_t * t,
                           int num_points, float length,
                           float frequency, float wavelength,
                           float phase, float amplitude);

void lemuria_tentacle_cleanup(lemuria_tentacle_t * t);

void lemuria_tentacle_update(lemuria_tentacle_t * t);

#endif // __TENTACLE_H_
