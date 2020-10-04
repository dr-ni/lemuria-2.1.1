
#include "tentacle.h"
#include "particle.h"

typedef enum
  {
    LEMURIA_OBJECT_UFO,
    LEMURIA_OBJECT_SPUTNIK,
    LEMURIA_OBJECT_TEAPOT,
    LEMURIA_OBJECT_TEAPOT_STATIC,
    LEMURIA_OBJECT_SATELLITE,
    LEMURIA_OBJECT_FISH,
    LEMURIA_OBJECT_PLATON,
    LEMURIA_OBJECT_MANTA,
    LEMURIA_OBJECT_BUBBLES,
    LEMURIA_OBJECT_SEAPLANT,
    LEMURIA_NUM_OBJECTS
  } lemuria_object_type_t;

#define FISH_FIN_STEPS 50

#define OBJ_NUM_BUBBLES 20

#define SEAPLANT_NUM_LEAVES    10
#define SEAPLANT_NUM_TENTACLES 2
#define SEAPLANT_TENTACLE_SIZE 20

typedef union
  {
  struct
    {
    int cockpit_color;
    } ufo;
  struct 
    {
    int material;
    lemuria_rotator_t rotator;
    } teapot;
  struct 
    {
    int material;
    int type;
    lemuria_rotator_t rotator;
    int num;
    } platon;
  struct
    {
    float rotation[3];
    } satellite;
  struct
    {
    float rotation[3];
    } sputnik;
  struct
    {
    float phase_t;
    float omega;
    float beta;
    float amplitude;
    int material;
    
    float axis_x;
    float axis_y;
    float axis_z;
    
    float fin_coords[FISH_FIN_STEPS][2][3];
    float fin_normals[FISH_FIN_STEPS][3];
    float fin_angles_cos[FISH_FIN_STEPS];
    float fin_angles_sin[FISH_FIN_STEPS];
    } fish;
  struct
    {
    int material;
    lemuria_tentacle_t t;
    } manta;
  struct
    {
    lemuria_particle_system_t * particles;
    float bubble_speeds[OBJ_NUM_BUBBLES][3];
    } bubbles;
  struct
    {
    lemuria_tentacle_t tentacles[SEAPLANT_NUM_TENTACLES];
    int material;
    float scale_factors[SEAPLANT_NUM_LEAVES];
    } seaplant;
  } lemuria_object_data_t;

typedef struct lemuria_object_s
  {
  float coords[3];
  float delta_coords[3];
  lemuria_object_type_t type;
  lemuria_object_data_t data;
  
  void (*update)(lemuria_engine_t*,struct lemuria_object_s*);
  void (*draw)(lemuria_engine_t*,struct lemuria_object_s*);
  void (*cleanup)(struct lemuria_object_s*);
  } lemuria_object_t;

void lemuria_object_init(lemuria_engine_t*,
                         lemuria_object_t*,
                         lemuria_object_type_t type,
                         float * coords,
                         float * delta_coords);

void lemuria_object_update(lemuria_engine_t*,lemuria_object_t*);
void lemuria_object_draw(lemuria_engine_t*,lemuria_object_t*);
void lemuria_object_cleanup(lemuria_object_t*);

/* This lets OpenGL rotate such that the objects positive X axis will point into the direction,
   where the speed goes */

void lemuria_object_rotate(lemuria_object_t*);
