
typedef struct technosophy_light_s
  {
  float  ambient[4];
  float  diffuse[4];
  float  specular[4];
  float  position[4];
  } lemuria_light_t;

void lemuria_set_light(lemuria_light_t * mat, int which);
