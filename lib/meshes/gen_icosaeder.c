#include <stdio.h>
#include <math.h>

void print_vertex(float x, float y, float z)
  {
  printf("  { %f, %f, %f },\n", x, y, z);
  }

void print_neighbour_indices(int i_1, int i_2, int i_3,
                             int i_4, int i_5)
  {
  printf("  { %d, %d, %d, %d, %d },\n",
         i_1 - 1,
         i_2 - 1,
         i_3 - 1,
         i_4 - 1,
         i_5 - 1);
  }

int main(int argc, char ** argv)
  {
  float phi;

  float theta;
  
  int i;

  float R_to_a = 0.25 * sqrt(2.0 * (5.0 + sqrt(5.0)));

  theta = 2.0 * asin(1.0/(R_to_a * 2.0));
    
  printf("static float vertices[12][3] = {\n");

  print_vertex(0.0, 0.0, 1.0);
  
  for(i = 0; i < 5; i++)
    {
    phi = 2.0 * M_PI * i / 5.0;

    print_vertex(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    }

  theta =  M_PI - theta;

  for(i = 0; i < 5; i++)
    {
    phi = (2.0 * M_PI * i / 5.0) - 2.0 * M_PI / 10.0;
    
    print_vertex(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    }

  print_vertex(0.0, 0.0, -1.0);
  
  printf("};\n");
  
  printf("static int neighbour_indices[12][5] = {\n");
  
  /* Vertex 1 */
  print_neighbour_indices(2, 3, 4, 5, 6);
  
  /* Vertex 2 */
  print_neighbour_indices(1, 3, 6, 7, 8);

  /* Vertex 3 */
  print_neighbour_indices(1, 4, 2, 8, 9);

  /* Vertex 4 */
  print_neighbour_indices(1, 5, 3, 9, 10);

  /* Vertex 5 */
  print_neighbour_indices(1, 6, 4, 10, 11);

  /* Vertex 6 */
  print_neighbour_indices(1, 2, 5, 11, 7);

  
  /* Vertex 7 */
  print_neighbour_indices(12, 8, 11, 2, 6);

  /* Vertex 8 */
  print_neighbour_indices(12, 9, 7,  3, 2);

  /* Vertex 9 */
  print_neighbour_indices(12, 10, 8, 4, 3);

  /* Vertex 10 */
  print_neighbour_indices(12, 11, 9, 5, 4);

  /* Vertex 11 */
  print_neighbour_indices(12, 7, 10, 6, 5);

  /* Vertex 12 */

  print_neighbour_indices(7, 8, 9, 10, 11);
  
  printf("};\n");

  }
