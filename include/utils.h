/*
 *  Utility structures
 */

/*
 *  Rotator: This lets foreground objects rotate
 */

typedef struct
  {
  /*
   *  These angles are rotations around the x- y- and z-axis
   */
  
  float angle[3];
  
  /* These are the steps, which are added at each frame */

  float delta_angle[3];
    
  /* The following let us rotate to a certain position */

  int turn_frame;
  int turn_end;
  
  float angle_start[3];
  float angle_end[3];
  
  } lemuria_rotator_t;

/* Initialize with reasonable values */

void lemuria_rotator_init(lemuria_engine_t * e,
                          lemuria_rotator_t * r);

/* Make an identity transform */

void lemuria_rotator_reset(lemuria_rotator_t * r);

/* Let the rotation go torarwds a given rotation */

void lemuria_rotator_turnto(lemuria_rotator_t * r,
                            float angle_x,
                            float angle_y,
                            float angle_z,
                            int num_frames);

int lemuria_rotator_done(lemuria_rotator_t * r);
  

/* Change program */

void lemuria_rotator_change(lemuria_engine_t * e,
                            lemuria_rotator_t * r);

/* Update the current values */

void lemuria_rotator_update(lemuria_rotator_t * r);

/* Rotate */

void lemuria_rotate(lemuria_rotator_t * r);

/* Get rotation matrix (for rotating things manually) */

void lemuria_rotate_get_matrix(lemuria_rotator_t * r, float matrix[3][3]);

/*
 * Float iterator:
 * Maintains an internal counter so that a floating point
 * Variable can be changed linearly
 */

typedef struct
  {
  int count;
  int count_max;
  int num_values;
  } lemuria_range_t;

/*
 *  Initialize the iterator with a frame number value anywhere between
 *  min_steps and max_steps
 */

void lemuria_range_init(lemuria_engine_t * e,
                        lemuria_range_t * r,
                        int num_values,
                        int min_steps, int max_steps);

void lemuria_range_update(lemuria_range_t * r);

void lemuria_range_get(lemuria_range_t * r,
                       float * min, float * max, float * ret);

void lemuria_range_get_cos(lemuria_range_t * r,
                           float * min, float * max, float * ret);

void lemuria_range_get_n(lemuria_range_t * r,
                         float * min, float * max,
                         float * ret, int n);

void lemuria_range_get_cos_n(lemuria_range_t * r,
                             float * min, float * max,
                             float * ret, int n);


int lemuria_range_done(lemuria_range_t * r);

/* Utility functions */

/* Deliver a number between min and max */

float lemuria_random(lemuria_engine_t * e, float min, float max);

int lemuria_random_int(lemuria_engine_t * e, int min, int max);


/* return 1 with a probability of p */

int lemuria_decide(lemuria_engine_t * e, float p);

/* Switch perspective mode on and off */

void lemuria_set_perspective(lemuria_engine_t * e,
                                 int perspective,
                                 float range);

/* Sound analysis */

void * lemuria_analysis_init();

void lemuria_analysis_cleanup(void *);

void lemuria_analysis_perform(lemuria_engine_t * e);

/* Scaler class: Lets objects grow and shrink */

typedef struct
  {
  float scale_min;
  float scale_max;

  float scale_start;
  float scale_end;

  lemuria_range_t scale_range;
  } lemuria_scale_t;

void lemuria_scale_init(lemuria_engine_t * e,
                        lemuria_scale_t *,
                        float _min_scale, float _max_scale);

void lemuria_scale_update(lemuria_scale_t *);

/* Change program */

void lemuria_scale_change(lemuria_engine_t * e,
                          lemuria_scale_t *);

/* Get the current scale factor */

float lemuria_scale_get(lemuria_scale_t *);

/* Offset */

typedef struct
  {
  float axis_start[2];
  float axis_end[2];
  
  lemuria_rotator_t rotator;
  lemuria_range_t axis_range;

  float phi;
  float delta_phi;
  float delta_phi_fac;
  
  float offset[3];
  
  } lemuria_offset_t;

void lemuria_offset_init(lemuria_engine_t * e,
                         lemuria_offset_t* off);

void lemuria_offset_kick(lemuria_offset_t* off);

void lemuria_offset_change(lemuria_engine_t * e,
                           lemuria_offset_t* off);

void lemuria_offset_update(lemuria_offset_t* off);

void lemuria_offset_translate(lemuria_offset_t* off);

float * lemuria_offset_get(lemuria_offset_t* off);

void lemuria_offset_reset(lemuria_engine_t * e,
                          lemuria_offset_t* off);


// Generic background engine

#define LEMURIA_TEXTURE_LEMURIA       0
#define LEMURIA_TEXTURE_CLOUDS        1
#define LEMURIA_TEXTURE_GOOM          2
#define LEMURIA_TEXTURE_XAOS          3
#define LEMURIA_TEXTURE_CLOUDS_ROTATE 4

typedef struct
  {
  int texture_mode;
  uint8_t * clouds_gradient;
  int clouds_size; /* 0: Small, 1: large */
  } lemuria_background_data_t;

typedef struct 
  {
  lemuria_background_data_t * config;
  
  unsigned int texture;
  int texture_flip_y;

  uint8_t * image;
  
  lemuria_engine_t * engine;

  /* For clouds with rotating palettes */

  int delta_palette_index;
  int palette_index;
  } lemuria_background_t;

void lemuria_background_init(lemuria_engine_t * e,
                             lemuria_background_t * b,
                             lemuria_background_data_t * d);

/* This is only used for LEMURIA_TEXTURE_CLOUDS_ROTATE */

void lemuria_background_update(lemuria_background_t * b);

void lemuria_background_set(lemuria_background_t * b);

void lemuria_background_delete(lemuria_background_t * b);

/*
 * For the following function, the coords must be like:
 *
 *  3                        2
 *  --------------------------
 *  \                        /
 *   \                      /
 *    \                    /
 *     \__________________/
 *      0                1
 */

void lemuria_background_draw(lemuria_background_t * b,
                             float coords[4][3],
                             int texture_size_x,
                             int texture_size_y,
                             float * texture_offset_y,
                             float texture_advance_y, int * flip_y);

void lemuria_get_rotation_matrix(float angle, float axis[3],
                                 float matrix[3][3]);

/*
 *  Stuff taken from freeglut
 */

void lemuriaWireCube( GLdouble dSize );
void lemuriaSolidCube( GLdouble dSize );
void lemuriaSolidSphere(GLdouble radius, GLint slices, GLint stacks);
void lemuriaWireSphere(GLdouble radius, GLint slices, GLint stacks);
void lemuriaSolidCone( GLdouble base, GLdouble height, GLint slices, GLint stacks );
void lemuriaWireCone( GLdouble base, GLdouble height, GLint slices, GLint stacks);
void lemuriaSolidCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks);
void lemuriaWireCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks);
void lemuriaWireTorus( GLdouble dInnerRadius, GLdouble dOuterRadius, GLint nSides, GLint nRings );
void lemuriaSolidTorus( GLdouble dInnerRadius, GLdouble dOuterRadius, GLint nSides, GLint nRings );
void lemuriaWireDodecahedron( void );
void lemuriaSolidDodecahedron( void );
void lemuriaWireOctahedron( void );
void lemuriaSolidOctahedron( void );
void lemuriaWireTetrahedron( void );
void lemuriaSolidTetrahedron( void );
void lemuriaWireIcosahedron( void );
void lemuriaSolidIcosahedron( void );
void lemuriaWireRhombicDodecahedron( void );
void lemuriaSolidRhombicDodecahedron( void );
void lemuriaWireSierpinskiSponge ( int num_levels, GLdouble offset[3], GLdouble scale );
void lemuriaSolidSierpinskiSponge ( int num_levels, GLdouble offset[3], GLdouble scale );

void lemuriaWireTeapot( GLdouble size );
void lemuriaSolidTeapot( GLdouble size );
