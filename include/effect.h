/* typedef struct technosophy_engine_s technosophy_engine_t */

struct effect_plugin_s
  {
  void * (*init)();
  void (*draw)(lemuria_engine_t *, void * data);
  void (*cleanup)(void*);
  };

/* Manage effects (effect.c) */

void lemuria_manage_effects(lemuria_engine_t * engine);

