
typedef struct lemuria_material_s
  {
  float ref_specular[4];
  float ref_ambient[4];
  float ref_diffuse[4];
  int shininess;
  } lemuria_material_t;

void lemuria_set_material(lemuria_material_t * mat, int which);

