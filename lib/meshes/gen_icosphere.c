#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static float isokaeder_vertices[12][3] = {
  { 0.000000, 0.000000, 1.000000 },
  { 0.894427, 0.000000, 0.447214 },
  { 0.276393, 0.850651, 0.447214 },
  { -0.723607, 0.525731, 0.447214 },
  { -0.723607, -0.525731, 0.447214 },
  { 0.276393, -0.850651, 0.447214 },
  { 0.723607, -0.525731, -0.447214 },
  { 0.723607, 0.525731, -0.447214 },
  { -0.276393, 0.850651, -0.447214 },
  { -0.894427, -0.000000, -0.447214 },
  { -0.276393, -0.850651, -0.447214 },
  { 0.000000, 0.000000, -1.000000 },
};
static int isokaeder_neighbour_indices[12][5] = {
  { 1, 2, 3, 4, 5 },
  { 0, 2, 5, 6, 7 },
  { 0, 3, 1, 7, 8 },
  { 0, 4, 2, 8, 9 },
  { 0, 5, 3, 9, 10 },
  { 0, 1, 4, 10, 6 },
  { 11, 7, 10, 1, 5 },
  { 11, 8, 6, 2, 1 },
  { 11, 9, 7, 3, 2 },
  { 11, 10, 8, 4, 3 },
  { 11, 6, 9, 5, 4 },
  { 6, 7, 8, 9, 10 },
};

typedef struct 
  {
  float coords[3];
  int neighbours[6];
  } vertex_t;

// vertex_1 < vertex_2 !

typedef struct 
  {
  int vertex_1;
  int vertex_2;
  } edge_t;

typedef struct
  {
  int vertex_1;
  int vertex_2;
  int vertex_3;
  } triangle_t;

typedef struct
  {
  vertex_t *   vertices;
  edge_t *     edges;
  triangle_t * triangles;  

  int num_edges;
  int num_vertices;
  int num_triangles;
  } icosphere_t;

int is_neighbour_of(icosphere_t * s, int v1, int v2)
  {
  int i;
  for(i = 0; i < 6; i++)
    {
    if((s->vertices[v1].neighbours[i] != -1) &&
       (s->vertices[v1].neighbours[i] == v2))
      return 1;
    }
  return 0;
  }

int change_neighbour(vertex_t * v, int old, int new)
  {
  int i;
  
  for(i = 0; i < 6; i++)
    {
    if(v->neighbours[i] == old)
      {
      v->neighbours[i] = new;
      return 1;
      }
    }
  return 0;
  }

int edge_is_part_of_triangle(icosphere_t * s, int edge_index, int triangle_index)
  {
  if(((s->edges[edge_index].vertex_1 == s->triangles[triangle_index].vertex_1) &&
      (s->edges[edge_index].vertex_2 == s->triangles[triangle_index].vertex_2)) ||
     ((s->edges[edge_index].vertex_1 == s->triangles[triangle_index].vertex_2) &&
      (s->edges[edge_index].vertex_2 == s->triangles[triangle_index].vertex_3)) ||
     ((s->edges[edge_index].vertex_1 == s->triangles[triangle_index].vertex_1) &&
      (s->edges[edge_index].vertex_2 == s->triangles[triangle_index].vertex_3)))
    return 1;
  return 0;
  }

// Search triangles, must have set up the edges before

int search_triangles(icosphere_t * s)
  {
  int i, j, k;
  s->num_triangles = 0;
#if 0
  for(i = 0; i < s->num_vertices; i++)
    {
    for(j = 0; j < 6; j++)
      {
      if(s->vertices[i].neighbours[j] == -1)
        continue;
      for(k = 0; k < 6; k++)
        {
        if(s->vertices[i].neighbours[k] == -1)
          continue;
        
        if((i < s->vertices[i].neighbours[j]) &&
           (i < s->vertices[i].neighbours[k]) &&
           (s->vertices[i].neighbours[j] < s->vertices[i].neighbours[k]))
          {
          s->triangles[s->num_triangles].vertex_1 = i;
          s->triangles[s->num_triangles].vertex_2 = s->vertices[i].neighbours[j];
          s->triangles[s->num_triangles].vertex_3 = s->vertices[i].neighbours[k];
          s->num_triangles++;
          fprintf(stderr, "Found %d triangles\n", s->num_triangles);
          }
        }
      }
    }
  
#endif

#if 1
  fprintf(stderr, "Searching Triangles\n");
  for(i = 0; i < s->num_vertices; i++)
    {
    for(j = i + 1; j < s->num_vertices; j++)
      {
      for(k = j + 1; k < s->num_vertices; k++)
        {
        if( ( (i < j) && (j < k) ) &&
            is_neighbour_of(s, i, j) &&
            is_neighbour_of(s, j, k) &&
            is_neighbour_of(s, i, k))
          {
          s->triangles[s->num_triangles].vertex_1 = i;
          s->triangles[s->num_triangles].vertex_2 = j;
          s->triangles[s->num_triangles].vertex_3 = k;
          s->num_triangles++;
          }
        }
      }
    fprintf(stderr, "Found %d\n", s->num_triangles);
    }
#endif
    
  fprintf(stderr, "Found %d triangles\n", s->num_triangles);
  }

int search_edges(icosphere_t * s)
  {
  int i, j;

  s->num_edges = 0;

  for(i = 0; i < s->num_vertices; i++)
    {
    for(j = i + 1; j < s->num_vertices; j++)
      {
      if(is_neighbour_of(s, i, j))
        {
        s->edges[s->num_edges].vertex_1 = i;
        s->edges[s->num_edges].vertex_2 = j;
        
        //        fprintf(stderr, "Found edge Nr %d: %d %d\n",
        //                 s->num_edges+1, 
        //                 s->edges[s->num_edges].vertex_1 + 1,
        //                 s->edges[s->num_edges].vertex_2 + 1);
        s->num_edges++;
        }
      }
    }
  fprintf(stderr, "Found %d edges\n", s->num_edges);
  }

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

void subdivide(icosphere_t * s)
  {
  int i, j, k;

  float x_new, y_new, z_new;
  float theta;
  float phi;

  int new_num_vertices = s->num_edges;
  int new_num_edges_divide_edge = s->num_edges;
  int new_num_edges_divide_triangle = 3 * s->num_triangles;

  int new_num_triangles = 3 * s->num_triangles;
  
  int triangle_1, triangle_2;

  int edge_1;
  int edge_2;

  edge_t * divided_edge;
  
  vertex_t * vertex_1;
  vertex_t * vertex_2;
  vertex_t * new_vertex;
  
  s->vertices = realloc(s->vertices,
                        sizeof(vertex_t)*(s->num_vertices+new_num_vertices));
  
  // 1st step: calculate new vertex coordinates AND new neighbours

  // The indices of the new veretices are the same as for the edges,
  // we currently have
  
  for(i = 0; i < new_num_vertices; i++)
    {
    divided_edge = &(s->edges[i]);
    
    // Calculate the midpoints between the old vertices

    vertex_1 = &(s->vertices[divided_edge->vertex_1]);
    vertex_2 = &(s->vertices[divided_edge->vertex_2]);

    new_vertex = &(s->vertices[s->num_vertices + i]);
    
    x_new = 0.5 * (vertex_1->coords[0] + vertex_2->coords[0]);
    y_new = 0.5 * (vertex_1->coords[1] + vertex_2->coords[1]);
    z_new = 0.5 * (vertex_1->coords[2] + vertex_2->coords[2]);

    // Get the corresponding angles in spherical coordinates
    
    phi = atan2(y_new, x_new);

    theta = atan2(sqrt(x_new*x_new + y_new*y_new), z_new);    

    // Make the new vertex sit on the sphere
    
    new_vertex->coords[0] = sin(theta)*cos(phi);
    new_vertex->coords[1] = sin(theta)*sin(phi);
    new_vertex->coords[2] = cos(theta);

    // First neighbours are the vertices of the original line
    
    new_vertex->neighbours[0] = divided_edge->vertex_1;
    new_vertex->neighbours[1] = divided_edge->vertex_2;

    // We must also tell this our neioghbours

    change_neighbour(&(s->vertices[divided_edge->vertex_1]),
                     divided_edge->vertex_2,
                     s->num_vertices + i);

    change_neighbour(&(s->vertices[divided_edge->vertex_2]),
                     divided_edge->vertex_1,
                     s->num_vertices + i);
        
    
    // Now, we have to find the 2 triangles, the divided edge is part of

    triangle_1 = -1;
    triangle_2 = -1;

    for(j = 0; j < s->num_triangles; j++)
      {
      if(edge_is_part_of_triangle(s, i, j))
        {
        if(triangle_1 != -1)
          {
          triangle_2 = j; // Found second triangle
          break;
          }
        else
          triangle_1 = j;
        }
      }
        
    // Now, we have triangle_1 and triangle_2. The last thing we need, are
    // the indides of the other edges if the neighbour triangles

    edge_1 = -1;
    edge_2 = -1;
    
    for(j = 0; j < s->num_edges; j++)
      {

      if((j != i) && (edge_is_part_of_triangle(s, j, triangle_1)))
        {
        if(edge_1 != -1)
          {
          edge_2 = j;
          break;
          }
        else
          edge_1 = j;
        }
      }

    //    fprintf(stderr, "Edge 1: %d Edge 2: %d\n",
    //            edge_1, edge_2);
    
    new_vertex->neighbours[2] = edge_1 + s->num_vertices;
    new_vertex->neighbours[3] = edge_2 + s->num_vertices;

    edge_1 = -1;
    edge_2 = -1;

    for(j = 0; j < s->num_edges; j++)
      {
      if((j != i) && (edge_is_part_of_triangle(s, j, triangle_2)))
        {
        if(edge_1 != -1)
          {
          edge_2 = j;
          break;
          }
        else
          edge_1 = j;
        }
      }
    new_vertex->neighbours[4] = edge_1 + s->num_vertices;
    new_vertex->neighbours[5] = edge_2 + s->num_vertices;
#if 0    
    fprintf(stderr, "Vertex %d, Neighbours: %d %d %d %d %d\n",
            s->num_vertices + i,
            new_vertex->neighbours[0],
            new_vertex->neighbours[1],
            new_vertex->neighbours[2],
            new_vertex->neighbours[3],
            new_vertex->neighbours[4],
            new_vertex->neighbours[5]
            );
#endif
    }

  s->num_vertices += new_num_vertices;

  s->edges =
    realloc(s->edges, sizeof(edge_t) *
            (s->num_edges+new_num_edges_divide_edge+
             new_num_edges_divide_triangle));
  
  s->triangles =
    realloc(s->triangles, sizeof(triangle_t) *
            (s->num_triangles + new_num_triangles));

  fprintf(stderr, "Must have %d vertices, %d triangles and %d edges now\n",
          s->num_vertices,
          s->num_triangles + new_num_triangles,
          (s->num_edges+new_num_edges_divide_edge+
           new_num_edges_divide_triangle));
  
  search_edges(s);
  search_triangles(s);
  }

int vertex_is_part_of_triangle(icosphere_t * s, int vertex_index, int triangle_index)
  {
  int i;
  
  if((s->triangles[triangle_index].vertex_1 == vertex_index) ||
     (s->triangles[triangle_index].vertex_2 == vertex_index) ||
     (s->triangles[triangle_index].vertex_3 == vertex_index))
    return 1;
  return 0;
  }


void print_icosphere(icosphere_t * s)
  {
  int i, j;

  float phi_1,phi_2, phi_3;
  float theta_1, theta_2, theta_3;

  int num_triangles;

  int triangles[6];
  
  printf("#define NUM_ICOSPHERE_VERTICES %d\n", s->num_vertices);
  //  printf("#define NUM_ICOSPHERE_EDGES %d\n", s->num_edges);
  printf("#define NUM_ICOSPHERE_TRIANGLES %d\n", s->num_triangles);

  printf("static float icosphere_vertices[NUM_ICOSPHERE_VERTICES][3] = {\n");

  for(i = 0; i < s->num_vertices; i++)
    {
    printf("{ %f, %f, %f },\n",
           s->vertices[i].coords[0],
           s->vertices[i].coords[1],
           s->vertices[i].coords[2]);
    }
  printf("};\n");

  printf("static int icoshpere_neighbours[NUM_ICOSPHERE_VERTICES][6] = {\n");

  for(i = 0; i < s->num_vertices; i++)
    {
    printf("{ %d, %d, %d, %d, %d, %d },\n",
           s->vertices[i].neighbours[0],
           s->vertices[i].neighbours[1],
           s->vertices[i].neighbours[2],
           s->vertices[i].neighbours[3],
           s->vertices[i].neighbours[4],
           s->vertices[i].neighbours[5]
           );
    }
  printf("};\n");
#if 0
  printf("static int icosphere_edges[NUM_ICOSPHERE_EDGES][2] = {\n");

  for(i = 0; i < s->num_edges; i++)
    {
    printf("{ %d, %d },\n",
           s->edges[i].vertex_1,
           s->edges[i].vertex_2
           );
    }
  printf("};\n");
#endif
  printf("static int icosphere_triangles[NUM_ICOSPHERE_TRIANGLES][3] = {\n");

  for(i = 0; i < s->num_triangles; i++)
    {
    printf("{ %d, %d, %d },\n",
           s->triangles[i].vertex_1,
           s->triangles[i].vertex_2,
           s->triangles[i].vertex_3
           );
    }
  printf("};\n");

  printf("static float icosphere_texture_coords[NUM_ICOSPHERE_TRIANGLES][3][2] = {\n");

  for(i = 0; i < s->num_triangles; i++)
    {
    phi_1 = atan2(s->vertices[s->triangles[i].vertex_1].coords[1],
                  s->vertices[s->triangles[i].vertex_1].coords[0]);
    
    theta_1 = atan2(sqrt(s->vertices[s->triangles[i].vertex_1].coords[0]*
                       s->vertices[s->triangles[i].vertex_1].coords[0] +
                       s->vertices[s->triangles[i].vertex_1].coords[1]*
                       s->vertices[s->triangles[i].vertex_1].coords[1]),
                  s->vertices[s->triangles[i].vertex_1].coords[2]);    

    phi_2 = atan2(s->vertices[s->triangles[i].vertex_2].coords[1],
                  s->vertices[s->triangles[i].vertex_2].coords[0]);
    
    theta_2 = atan2(sqrt(s->vertices[s->triangles[i].vertex_2].coords[0]*
                         s->vertices[s->triangles[i].vertex_2].coords[0] +
                         s->vertices[s->triangles[i].vertex_2].coords[1]*
                         s->vertices[s->triangles[i].vertex_2].coords[1]),
                    s->vertices[s->triangles[i].vertex_2].coords[2]);    

    phi_3 = atan2(s->vertices[s->triangles[i].vertex_3].coords[1],
                  s->vertices[s->triangles[i].vertex_3].coords[0]);
    
    theta_3 = atan2(sqrt(s->vertices[s->triangles[i].vertex_3].coords[0]*
                         s->vertices[s->triangles[i].vertex_3].coords[0] +
                         s->vertices[s->triangles[i].vertex_3].coords[1]*
                         s->vertices[s->triangles[i].vertex_3].coords[1]),
                    s->vertices[s->triangles[i].vertex_3].coords[2]);    

    // Make false values true :-)
#if 0
    if((phi_1 * phi_2 < 0.0) || (phi_1 * phi_3 < 0.0) || (phi_2 * phi_3 < 0.0))
      {
      if(phi_1 < 0.0)
        phi_1 += 2.0 * M_PI;
      if(phi_2 < 0.0)
        phi_2 += 2.0 * M_PI;
      if(phi_3 < 0.0)
        phi_3 += 2.0 * M_PI;
      }
#endif

    if(fabs(phi_2 - phi_1) > M_PI)
      {
      if(phi_2 < phi_1)
        phi_2 += 2.0 * M_PI;
      else
        phi_1 += 2.0 * M_PI;
      }

    if(fabs(phi_3 - phi_1) > M_PI)
      {
      if(phi_3 < phi_1)
        phi_3 += 2.0 * M_PI;
      else
        phi_1 += 2.0 * M_PI;
      }

    if(fabs(phi_3 - phi_2) > M_PI)
      {
      if(phi_3 < phi_2)
        phi_3 += 2.0 * M_PI;
      else
        phi_2 += 2.0 * M_PI;
      }

    
    printf("{ { %f, %f }, { %f, %f }, { %f, %f } },\n",
           phi_1 / (2.0 * M_PI),
           theta_1 / M_PI,
           phi_2 / (2.0 * M_PI),
           theta_2 / M_PI,
           phi_3 / (2.0 * M_PI),
           theta_3 / M_PI
           );
    }
  printf("};\n");
  printf("static int icosphere_triangle_members[NUM_ICOSPHERE_VERTICES][6] = {\n");
  
  for(i = 0; i < s->num_vertices; i++)
    {
    num_triangles = 0;
    triangles[5] = -1;
    
    for(j = 0; j < s->num_triangles; j++)
      {
      if(vertex_is_part_of_triangle(s, i, j))
        {
        triangles[num_triangles] = j;
        num_triangles++;
        }
      }
    printf("{ %d, %d, %d, %d, %d, %d },\n",
           triangles[0],
           triangles[1],
           triangles[2],
           triangles[3],
           triangles[4],
           triangles[5]);
    }
  printf("};\n");
  
  }

// Sort all triangles so they have the proper winding

static int get_quadrant(float angle)
  {
  if((angle > 0.0) && (angle <= 0.5 * M_PI))
    return 1;
  else if(angle > 0.5 * M_PI)
    return 2;
  else if((angle <= 0.0) && (angle > - 0.5 * M_PI))
    return 3;
  else
    return 4;
  }

static void sort_triangles(icosphere_t * s)
  {
  int i;
  float theta_1, phi_1, theta_2, phi_2, theta_3, phi_3;

  int quadrant_1, quadrant_2, quadrant_3;
 
  float test_value;

  float midpoint_phi, midpoint_theta;
  
  vertex_t * tmp_vertex;

  //  float angle_1, angle_2;

  int i_tmp;

  float f_tmp;

  int swap_winding;
  
  fprintf(stderr, "Sorting triangles...");
  
  for(i = 0; i < s->num_triangles; i++)
    {
    tmp_vertex = &s->vertices[s->triangles[i].vertex_1];

    // Calculate the spherical angles
    
    phi_1 = atan2(tmp_vertex->coords[1], tmp_vertex->coords[0]);
    
    theta_1 = atan2(sqrt(tmp_vertex->coords[0]*tmp_vertex->coords[0] +
                         tmp_vertex->coords[1]*tmp_vertex->coords[1]),
                    tmp_vertex->coords[2]);

    tmp_vertex = &s->vertices[s->triangles[i].vertex_2];

    phi_2 = atan2(tmp_vertex->coords[1], tmp_vertex->coords[0]);
    
    theta_2 = atan2(sqrt(tmp_vertex->coords[0]*tmp_vertex->coords[0] +
                         tmp_vertex->coords[1]*tmp_vertex->coords[1]),
                    tmp_vertex->coords[2]);

    tmp_vertex = &s->vertices[s->triangles[i].vertex_3];

    phi_3 = atan2(tmp_vertex->coords[1], tmp_vertex->coords[0]);
    
    theta_3 = atan2(sqrt(tmp_vertex->coords[0]*tmp_vertex->coords[0] +
                         tmp_vertex->coords[1]*tmp_vertex->coords[1]),
                    tmp_vertex->coords[2]);


    // Ensure, that the directions are like x and y
    
    theta_1 = 0.5 * M_PI - theta_1;
    theta_2 = 0.5 * M_PI - theta_2;
    theta_3 = 0.5 * M_PI - theta_3;

    // Special case: The z-axis goes through this triangle
    
    if((fabs(fabs(theta_1) - 0.5 * M_PI) < 0.01) ||
       (fabs(fabs(theta_2) - 0.5 * M_PI) < 0.01) ||
       (fabs(fabs(theta_3) - 0.5 * M_PI) < 0.01))
      {
      // Sort vertices so that the axis goes through vertex_1
            
      if(fabs(fabs(theta_2) - 0.5 * M_PI) < 0.01)
        {
        i_tmp = s->triangles[i].vertex_1;
        s->triangles[i].vertex_1 = s->triangles[i].vertex_2;
        s->triangles[i].vertex_2 = i_tmp;      

        f_tmp = theta_1;
        theta_1 = theta_2;
        theta_2 = f_tmp;

        f_tmp = phi_1;
        phi_1 = phi_2;
        phi_2 = f_tmp;
        }

      if(fabs(fabs(theta_3) - 0.5 * M_PI) < 0.01)
        {
        i_tmp = s->triangles[i].vertex_1;
        s->triangles[i].vertex_1 = s->triangles[i].vertex_3;
        s->triangles[i].vertex_3 = i_tmp;      

        f_tmp = theta_1;
        theta_1 = theta_3;
        theta_3 = f_tmp;

        f_tmp = phi_1;
        phi_1 = phi_3;
        phi_3 = f_tmp;
        }

      swap_winding = 0;

      
      if(phi_2 * phi_3 < 0.0)
        {
        if((phi_2 < 0.0) && (phi_2 < -0.5*M_PI))
          phi_2 += 2.0 * M_PI;

        if((phi_3 < 0.0) && (phi_3 < -0.5*M_PI))
          phi_3 += 2.0 * M_PI;
        
        if(phi_2 > phi_3)
          swap_winding = 1;
        }
      else
        {
        if(phi_2 > phi_3)
          swap_winding = 1;
        }

     
      if(theta_1 < 0.0)
        swap_winding = !swap_winding;
            
      // Now, figure out the winding rule

      if(swap_winding)
        {
        i_tmp = s->triangles[i].vertex_2;
        s->triangles[i].vertex_2 = s->triangles[i].vertex_3;
        s->triangles[i].vertex_3 = i_tmp;      
        }
      
      fprintf(stderr, "Phi_2: %f, Phi_3: %f\n",
              phi_2 / M_PI * 180.0,
               phi_3 / M_PI * 180.0);
       
      //      fprintf(stderr, "Blupp\n");
      }
    else
      {
      if(((phi_1 * phi_2 < 0.0) || (phi_1 * phi_3 < 0.0) || (phi_2 * phi_3 < 0.0)) &&
         (fabs(phi_1) > 0.5 * M_PI))
        {
        
        if(phi_1 < 0.0)
          phi_1 += 2.0 * M_PI;
        
        if(phi_2 < 0.0)
          phi_2 += 2.0 * M_PI;
        
        if(phi_3 < 0.0)
        phi_3 += 2.0 * M_PI;
        }
      
      // Now, find out the winding
      
      midpoint_phi = (phi_1 + phi_2 + phi_3)/3.0;
      midpoint_theta = (theta_1 + theta_2 + theta_3)/3.0;
            
      test_value = (phi_1 - midpoint_phi) * (theta_2 - midpoint_theta) -
        ((phi_2 - midpoint_phi) * (theta_1 -  midpoint_theta));
      
      if(test_value < 0.0)
        {
        i_tmp = s->triangles[i].vertex_1;
        s->triangles[i].vertex_1 = s->triangles[i].vertex_2;
        s->triangles[i].vertex_2 = i_tmp;      
        }
      }
    }
  
  fprintf(stderr, "done\n");
  }

int main(int argc, char ** argv)
  {
  int subdivision;
  int subdivision_max = 4;

  int i, j, k;

  float theta, phi;
  
  // Fill data with icophere;

  icosphere_t icosphere;

  icosphere.vertices = calloc(12, sizeof(vertex_t));
  
  for(i = 0; i < 12; i++)
    {
    for(j = 0; j < 3; j++)
      icosphere.vertices[i].coords[j] = isokaeder_vertices[i][j];

    for(j = 0; j < 5; j++)
      icosphere.vertices[i].neighbours[j] = isokaeder_neighbour_indices[i][j];
    icosphere.vertices[i].neighbours[5] = -1;

    phi = atan2(icosphere.vertices[i].coords[1], icosphere.vertices[i].coords[0]);

    theta = atan2(sqrt(icosphere.vertices[i].coords[0]*icosphere.vertices[i].coords[0] +
                       icosphere.vertices[i].coords[1]*icosphere.vertices[i].coords[1]),
                  icosphere.vertices[i].coords[2]);    
    // Make the new vertex sit on the sphere
    
    icosphere.vertices[i].coords[0] = sin(theta)*cos(phi);
    icosphere.vertices[i].coords[1] = sin(theta)*sin(phi);
    icosphere.vertices[i].coords[2] = cos(theta);

    }
  
  icosphere.num_edges = 0;
  icosphere.num_triangles = 0;
  icosphere.num_vertices = 12;
  
  icosphere.edges = calloc(30, sizeof(edge_t));
  search_edges(&icosphere);

  icosphere.triangles = calloc(20, sizeof(triangle_t));
  search_triangles(&icosphere);

  for(subdivision = 0; subdivision < subdivision_max; subdivision++)
    subdivide(&icosphere);

  sort_triangles(&icosphere);  
  print_icosphere(&icosphere);
  
  }
