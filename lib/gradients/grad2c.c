#include <stdio.h>
#include <stdlib.h>

//#include <glib.h>

#include <libgimp/gimp.h>

// #include <libgimp/gimpintl.h>


/*
 *  Gimp plugin for saving Gimp color gradients into
 *  C include files for use by lemuria
 */

static void      query          (void);
static void      run            (const gchar   *name,
                                 gint    nparams,
                                 const GimpParam  *param,
                                 gint    *nreturn_vals,
                                 GimpParam  **return_vals);

GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static void query()
  {
  static GimpParamDef args[]=
    {
      { GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive" },
      { GIMP_PDB_IMAGE, "image", "Input image (unused)" },
      { GIMP_PDB_DRAWABLE, "drawable", "Input drawable" }
    };
  static gint nargs = sizeof (args) / sizeof (args[0]);
  
  gimp_install_procedure ("plug_in_gradsave",
                          "Save gradient for use by lemuria ",
                          "LaberLaber",
                          "Burkhard Plaum",
                          "Burkhard Plaum",
                          "2002",
//                          "<Image>/Filters/Colors/Map/Save Gradient for lemuria",
                           "<Image>/Filters/Map/Save Gradient for lemuria",
                          "RGB*, GRAY*",
                          GIMP_PLUGIN,
                          nargs, 0,
                          args, NULL);
  }

#define NSTEPS 256

#define __CLAMP(i) if(i>255)i=255

static void
run (const gchar   *name,
     gint    nparams,
     const GimpParam  *param,
     gint    *nreturn_vals,
     GimpParam  **return_vals)
  {
  char * gradient_name;
  char * filename;
  FILE * output_file;
  double * gradient_data;
  int i;
  int r, g, b, a;
  
  //Some stuff pasted from another plugin
  
  static GimpParam values[1];
//  GimpRunModeType run_mode;
  GimpPDBStatusType status = GIMP_PDB_SUCCESS;

//  run_mode = param[0].data.d_int32;

  //  INIT_I18N();

  *nreturn_vals = 1;
  *return_vals = values;

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
  
  // Do the actual work here

  gradient_name = gimp_gradients_get_gradient();

  filename = g_strdup_printf("%s/%s.incl", DIRECTORY, gradient_name);

  fprintf(stderr, "Filename: %s\n", filename);
  output_file = fopen(filename, "w");
  gradient_data = gimp_gradients_sample_uniform (NSTEPS, FALSE);

  fprintf(output_file, "uint8_t gradient_%s[%d] = {\n", gradient_name,
          NSTEPS*4);
    
  for(i = 0; i < NSTEPS; i++)
    {
    r = (int)(gradient_data[i*4]   * NSTEPS+0.5);
    g = (int)(gradient_data[i*4+1] * NSTEPS+0.5);
    b = (int)(gradient_data[i*4+2] * NSTEPS+0.5);
    a = (int)(gradient_data[i*4+3] * NSTEPS+0.5);

    __CLAMP(r);
    __CLAMP(g);
    __CLAMP(b);
    __CLAMP(a);
        
    fprintf(output_file, "    0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
            r, g, b, a);
    
    }
  fprintf(output_file, "  };\n");
  fclose(output_file);
  g_free(gradient_data);
  g_free(filename);
  }


MAIN ()

 
