#include <stdio.h>
#include <math.h>

void print_vertex(float x, float y, float z)
  {
  printf("  { %f, %f, %f },\n", x, y, z);
  }

void print_neighbour_indices(int i_1, int i_2, int i_3)
  {
  printf("  { %d, %d, %d },\n", i_1 - 1, i_2 - 1, i_3 - 1);
  }

static int num_lines = 0;

void print_line_indices(int i_1, int i_2)
  {
  num_lines++;
  printf("  { %d, %d },\n", i_1 - 1, i_2 - 1);
  }

int main(int argc, char ** argv)
  {
  float phi;
  float theta_1, theta_2;

  float theta;
  
  int i;

  float R_to_a = 0.25 * sqrt(6.0 * (3.0 + sqrt(5.0)));

  theta_1 = asin(1.0/(R_to_a * 2.0 * sin(36.0 / 180.0 * M_PI)));

  theta_2 = theta_1 + 2.0 * asin(1.0/(2.0 * R_to_a));
    
  printf("static float dodekaeder_vertices[20][3] = {\n");

/*   fprintf(stderr, "theta_1: %f, theta_2: %f, delta_theta: %f\n", */
/*           theta_1 * 180.0 / M_PI, */
/*           theta_2 * 180.0 / M_PI, */
/*           (theta_2 - theta_1) * 180.0 / M_PI); */
          
  // Print the upper pentangle

  theta = theta_1;
  
  for(i = 0; i < 5; i++)
    {
    phi = 2.0 * M_PI * i / 5.0;

    print_vertex(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    }

  theta =  theta_2;
  
  for(i = 0; i < 5; i++)
    {
    phi = 2.0 * M_PI * (2*i) / 10.0;

    fprintf(stderr, "phi 1: %f\n", phi / M_PI * 180.0);
    print_vertex(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    
    phi = 2.0 * M_PI * (2*i+1) / 10.0;

    fprintf(stderr, "phi 2: %f\n", phi / M_PI * 180.0);
            
    print_vertex(sin(M_PI - theta) * cos(phi),
                 sin(M_PI - theta) * sin(phi),
                 cos(M_PI - theta));
    }

  theta = M_PI - theta_1;

  for(i = 0; i < 5; i++)
    {
    phi = 2.0 * M_PI * (2*i+1) / 10.0;
    
    print_vertex(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    }
      
  printf("};\n");
#if 0  
  printf("static int dodekaeder_neighbour_indices[20][3] = {\n");
  
  /* Vertex 1 */
  print_neighbour_indices(2, 5, 6);

  /* Vertex 2 */
  print_neighbour_indices(1, 3, 8);

  /* Vertex 3 */
  print_neighbour_indices(2, 4, 10);

  /* Vertex 4 */
  print_neighbour_indices(3, 5, 12);

  /* Vertex 5 */
  print_neighbour_indices(1, 4, 14);

  /* Vertex 6 */
  print_neighbour_indices(1, 7, 15);

  /* Vertex 7 */
  print_neighbour_indices(6, 8, 16);

  /* Vertex 8 */
  print_neighbour_indices(2, 7, 9);

  /* Vertex 9 */
  print_neighbour_indices(8, 10, 17);

  /* Vertex 10 */
  print_neighbour_indices(3, 9, 11);

  /* Vertex 11 */
  print_neighbour_indices(10, 12, 18);

  /* Vertex 12 */
  print_neighbour_indices(4, 11, 13);

  /* Vertex 13 */
  print_neighbour_indices(12, 14, 19);

  /* Vertex 14 */
  print_neighbour_indices(5, 13, 15);

  /* Vertex 15 */
  print_neighbour_indices(6, 14, 20);

  /* Vertex 16 */
  print_neighbour_indices(7, 17, 20);

  /* Vertex 17 */
  print_neighbour_indices(9, 16, 18);

  /* Vertex 18 */
  print_neighbour_indices(11, 17, 19);

  /* Vertex 19 */
  print_neighbour_indices(13, 18, 20);

  /* Vertex 20 */
  print_neighbour_indices(15, 16, 19);

  printf("};\n");
#endif

  printf("static int dodekaeder_line_indices[30][2] = {\n");
  
  /* Vertex 1 -> */
  //  print_neighbour_indices(2, 5, 6);

  print_line_indices(1, 2);
  print_line_indices(1, 5);
  print_line_indices(1, 6);
  
  /* Vertex 2 ->  */
  // print_neighbour_indices(1, 3, 8);

  print_line_indices(2, 3);
  print_line_indices(2, 8);

  /* Vertex 3 ->  */
  // print_neighbour_indices(2, 4, 10);

  print_line_indices(3, 4);
  print_line_indices(3, 10);

  /* Vertex 4 ->  */
  //  print_neighbour_indices(3, 5, 12);
  print_line_indices(4, 5);
  print_line_indices(4, 12);

  /* Vertex 5 ->  */
  // print_neighbour_indices(1, 4, 14);

  print_line_indices(5, 14);
    
  /* Vertex 6 ->  */
  // print_neighbour_indices(1, 7, 15);

  print_line_indices(6, 7);
  print_line_indices(6, 15);
  
  /* Vertex 7 ->  */
  // print_neighbour_indices(6, 8, 16);

  print_line_indices(7, 8);
  print_line_indices(7, 16);
  
  /* Vertex 8 ->  */
  // print_neighbour_indices(2, 7, 9);

  print_line_indices(8, 9);
  
  /* Vertex 9 ->  */
  // print_neighbour_indices(8, 10, 17);

  print_line_indices(9, 10);
  print_line_indices(9, 17);
  
  /* Vertex 10 ->  */
  // print_neighbour_indices(3, 9, 11);

  print_line_indices(10, 11);

  /* Vertex 11 ->  */
  // print_neighbour_indices(10, 12, 18);

  print_line_indices(11, 12);
  print_line_indices(11, 18);

  /* Vertex 12 ->  */
  // print_neighbour_indices(4, 11, 13);

  print_line_indices(12, 13);

  /* Vertex 13 ->  */
  //  print_neighbour_indices(12, 14, 19);

  print_line_indices(13, 14);
  print_line_indices(13, 19);
    
  /* Vertex 14 ->  */
  //  print_neighbour_indices(5, 13, 15);

  print_line_indices(14, 15);

  /* Vertex 15 ->  */
  //  print_neighbour_indices(6, 14, 20);

  print_line_indices(15, 20);

  /* Vertex 16 ->  */
  //  print_neighbour_indices(7, 17, 20);

  print_line_indices(16, 17);
  print_line_indices(16, 20);

  /* Vertex 17 ->  */
  //  print_neighbour_indices(9, 16, 18);

  print_line_indices(17, 18);

  /* Vertex 18 -> */
  // print_neighbour_indices(11, 17, 19);

  print_line_indices(18, 19);

  /* Vertex 19 -> */
  // print_neighbour_indices(13, 18, 20);

  print_line_indices(19, 20);

  /* Vertex 20 -> */
  //  print_neighbour_indices(15, 16, 19);
  
  printf("};\n");

  fprintf(stderr, "Num lines: %d\n", num_lines);
  }
