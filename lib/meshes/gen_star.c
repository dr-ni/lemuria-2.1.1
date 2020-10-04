#include <math.h>
#include <stdio.h>

#define TAN_18 0.324919696
#define SIN_36 0.587785252
#define COS_36 0.809016994

#define R 1.0
#define r (R*(TAN_18 / (SIN_36 + COS_36 * TAN_18)))

int main(int argc, char ** argv)
  {
  float phi;
  //  float cos_phi;
  //  float sin_phi;

  int i;

  phi = 0.0;

  printf("static float star_coords[11][3] =\n  {\n");
  for(i = 0; i < 5; i++)
    {
    printf("    { %f, %f, 0.0 },\n", R * cos(phi), R*sin(phi));

    phi += M_PI / 5.0;
    
    printf("    { %f, %f, 0.0 },\n", r * cos(phi), r*sin(phi));

    phi += M_PI / 5.0;
    
    }

  printf("    { %f, %f, 0.0 },\n", R * cos(phi), r*sin(phi));
  printf("  };\n");
  }
