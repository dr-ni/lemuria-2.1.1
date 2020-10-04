#include <config.h>
#include <stdlib.h>

#include <lemuria.h>
#include <libvisual/libvisual.h>

/* Private context sensitive data goes here, */
typedef struct
  {
  lemuria_engine_t * e;
  } lemuria_t;

/* This function is called before we really start rendering, it's the init function */
static int lv_lemuria_init (VisPluginData *plugin)
  {
  lemuria_t *priv;
  /* Allocate the lemuria private data structure, and register it as a private */
  priv = calloc(1, sizeof(*priv));
  priv->e = lemuria_create();
  
  //priv = visual_mem_new0 (lemuria_t, 1);
  visual_plugin_set_private (plugin, priv);
  return TRUE;
  }

static void lv_lemuria_cleanup (VisPluginData *plugin)
  {
  lemuria_t *priv = (lemuria_t*)visual_plugin_get_private (plugin);
  
  lemuria_destroy(priv->e);
  free(priv);
  }

/* This is used to ask a plugin if it can handle a certain size, and if not, to
 * set the size it wants by putting a value in width, height that represents the
 * required size */
static void lv_lemuria_requisition (VisPluginData *plugin, int *width, int *height)
  {
  int reqw, reqh;
  
  /* Size negotiate with the window */
  reqw = *width;
  reqh = *height;
  
  if (reqw < 256)
    reqw = 256;
  
  if (reqh < 256)
    reqh = 256;
  
  *width = reqw;
  *height = reqh;
  }

static int lv_lemuria_dimension (VisPluginData *plugin, int width, int height)
  {
  lemuria_t *priv = (lemuria_t*)visual_plugin_get_private(plugin);
  
  lemuria_set_size(priv->e, width, height);
  return 0;
  }

/* This is the main event loop, where all kind of events can be handled, more information
 * regarding these can be found at:
 * http://libvisual.sourceforge.net/newdocs/docs/html/union__VisEvent.html 
 */

static int lv_lemuria_events (VisPluginData *plugin, VisEventQueue *events)
  {
  lemuria_t *priv = (lemuria_t*)visual_plugin_get_private(plugin);
  VisEvent ev;
  // VisParamEntry *param;
  
  while (visual_event_queue_poll (events, &ev)) 
    {
    switch (ev.type) 
      {
      case VISUAL_EVENT_KEYDOWN:
        switch(ev.event.keyboard.keysym.sym)
          {
          case VKEY_a:
            if(ev.event.keyboard.keysym.mod & VKMOD_CTRL)
              lemuria_change_effect(priv->e, LEMURIA_EFFECT_FOREGROUND);
            else
              lemuria_next_effect(priv->e, LEMURIA_EFFECT_FOREGROUND);
            break;
          case VKEY_w:
            if(ev.event.keyboard.keysym.mod & VKMOD_CTRL)
              lemuria_change_effect(priv->e, LEMURIA_EFFECT_BACKGROUND);
            else
              lemuria_next_effect(priv->e, LEMURIA_EFFECT_BACKGROUND);
            break;
          case VKEY_t:
            if(ev.event.keyboard.keysym.mod & VKMOD_CTRL)
              lemuria_change_effect(priv->e, LEMURIA_EFFECT_TEXTURE);
            else
              lemuria_next_effect(priv->e, LEMURIA_EFFECT_TEXTURE);
            break;
          case VKEY_F1:
            lemuria_print_help(priv->e);
            break;
          default:
            break;
          }
        break;
      case VISUAL_EVENT_RESIZE:
        lv_lemuria_dimension(plugin, ev.event.resize.width, ev.event.resize.height);
        break;
        
      default: /* to avoid warnings */
        break;
      }
    }
  
  return 0;
  }

/* Using this function we can update the palette when we're in 8bits mode, which
 * we aren't with lemuria, so just ignore :) */
static VisPalette *lv_lemuria_palette (VisPluginData *plugin)
{
        return NULL;
}

/* This is where the real rendering happens! This function is what we call, many times
 * a second to get our graphical frames. */
static void lv_lemuria_render (VisPluginData *plugin, VisVideo *video, VisAudio *audio)
  {
  lemuria_t *priv = (lemuria_t*)visual_plugin_get_private(plugin);
  VisBuffer *pcmbuf;
  float pcm[2][512];
  short pcms_0[512];
  short pcms_1[512];
  short * pcms[2];
  int i;
  pcmbuf = visual_buffer_new ();
  visual_buffer_set_data_pair (pcmbuf, pcm[0], sizeof (pcm[0]));
  visual_audio_get_sample (audio, pcmbuf, VISUAL_AUDIO_CHANNEL_LEFT);
  
  visual_buffer_set_data_pair (pcmbuf, pcm[1], sizeof (pcm[1]));
  visual_audio_get_sample (audio, pcmbuf, VISUAL_AUDIO_CHANNEL_RIGHT);

  /* Ugly, but there really should be a way to get int16_t data from the VisAudio */
  for (i = 0; i < 512; i++)
    {
    pcms_0[i] = pcm[0][i] * 32768.0;
    pcms_1[i] = pcm[1][i] * 32768.0;
    }
  pcms[0] = pcms_0;
  pcms[1] = pcms_1;
  
  lemuria_update_audio(priv->e, pcms);
  lemuria_draw_frame(priv->e);
  }

VISUAL_PLUGIN_API_VERSION_VALIDATOR




/* Main plugin stuff */
/* The get_plugin_info function provides the libvisual plugin registry, and plugin loader
 * with the very basic plugin information */
const VisPluginInfo *get_plugin_info (void)
  {
  /* Initialize the plugin specific data structure
   * with pointers to the functions that represent
   * the plugin interface it's implementation, more info:
   * https://github.com/Libvisual/libvisual/wiki */

  static VisActorPlugin actor = {
		.requisition = lv_lemuria_requisition,
		.palette     = lv_lemuria_palette,
		.render      = lv_lemuria_render,
		.vidoptions.depth = VISUAL_VIDEO_DEPTH_GL
  };

  static VisPluginInfo info = {
		.type = VISUAL_PLUGIN_TYPE_ACTOR,
		.plugname = "lemuria",
		.name = "libvisual Lemuria",
		.author = "B. Plaum (libvisual-0.5 adaption by U. Niethammer)",
		.version = VERSION,
		.about = "OpenGL animations",
		.help =  "If you watch this too long, you'll need some.",
		.license  = VISUAL_PLUGIN_LICENSE_GPL,
		.init = lv_lemuria_init,
		.cleanup = lv_lemuria_cleanup,
		.events = lv_lemuria_events,
		.plugin = &actor
   };
  
  VISUAL_VIDEO_ATTR_OPTIONS_GL_ENTRY(actor.vidoptions, VISUAL_GL_ATTRIBUTE_ALPHA_SIZE, 8);
  VISUAL_VIDEO_ATTR_OPTIONS_GL_ENTRY(actor.vidoptions, VISUAL_GL_ATTRIBUTE_DEPTH_SIZE, 16);
  VISUAL_VIDEO_ATTR_OPTIONS_GL_ENTRY(actor.vidoptions, VISUAL_GL_ATTRIBUTE_DOUBLEBUFFER, 1);
  VISUAL_VIDEO_ATTR_OPTIONS_GL_ENTRY(actor.vidoptions, VISUAL_GL_ATTRIBUTE_RED_SIZE, 8);
  VISUAL_VIDEO_ATTR_OPTIONS_GL_ENTRY(actor.vidoptions, VISUAL_GL_ATTRIBUTE_GREEN_SIZE, 8);
  VISUAL_VIDEO_ATTR_OPTIONS_GL_ENTRY(actor.vidoptions, VISUAL_GL_ATTRIBUTE_BLUE_SIZE, 8);
  return &info;
  }

