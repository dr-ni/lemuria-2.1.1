typedef struct
  {
  float x;
  float y;
  float z;
  float size;
  float angle; // Rotation angle
  } lemuria_particle_t;

typedef struct
  {
  lemuria_particle_t * particles;
  int num_particles;

  unsigned int texture;
  } lemuria_particle_system_t;

lemuria_particle_system_t *
lemuria_create_particles(lemuria_engine_t * e,
                         int num_particles,
                         int texture_size,
                         unsigned char * texture,
                         float _min_size,
                         float _max_size);

void lemuria_draw_particles(lemuria_particle_system_t *);

void lemuria_destroy_particles(lemuria_particle_system_t *);
