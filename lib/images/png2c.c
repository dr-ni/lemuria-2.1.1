#include <stdlib.h>
#include <png.h>

#include <stdio.h>

#include <string.h> 

int main(int argc, char ** argv)
  {
  png_structp png_ptr;
  png_infop info_ptr;
  png_infop end_info;

  int png_bit_depth;
  int png_color_type;

  int width, height;
  int bytes_per_pixel;
  unsigned char * image;
  unsigned char ** image_rows;

  char output_base[1024];
  char * pos;
  FILE * file;

  int i, linepos;
  
  file = fopen(argv[1], "rb");
  if(!file)
    return 0;

  strcpy(output_base, argv[1]);
  pos = strrchr(output_base, '.');
  *pos = '\0';
  
  png_byte signature[8];

  fread(signature, 1, 8, file);

  if(png_sig_cmp(signature, 0, 8))
    return 0;

  png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING, (png_voidp)0,
     NULL, NULL);

  if (!png_ptr)
    return 0;

  setjmp(png_jmpbuf(png_ptr));

    info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    return 0;
    }

  end_info = png_create_info_struct(png_ptr);
  if(!end_info)
    {
    png_destroy_read_struct(&png_ptr, &info_ptr,
                            (png_infopp)NULL);
    return 0;
    }
  png_init_io(png_ptr, file);
  png_set_sig_bytes(png_ptr, 8);

  // Now, read info from the file
  
  png_read_info(png_ptr, info_ptr);

  width  = png_get_image_width(png_ptr,  info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  png_bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  png_color_type = png_get_color_type(png_ptr, info_ptr);

  switch(png_color_type)
    {
    case PNG_COLOR_TYPE_GRAY:       //  (bit depths 1, 2, 4, 8, 16)
      fprintf(stderr, "Grayscale png: %d x %d\n", width, height);
      if(png_bit_depth < 8)
        png_set_gray_1_2_4_to_8(png_ptr);
      bytes_per_pixel = 1;
      break;
    case PNG_COLOR_TYPE_RGB:        //  (bit_depths 8, 16)
      fprintf(stderr, "RGB png: %d x %d\n", width, height);
      if(png_bit_depth == 16)
        png_set_strip_16(png_ptr);
      bytes_per_pixel = 3;
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:  //  (bit_depths 8, 16)
      fprintf(stderr, "RGBA png: %d x %d\n", width, height);
      if(png_bit_depth == 16)
        png_set_strip_16(png_ptr);
      bytes_per_pixel = 4;
      break;
    }

  image = calloc(width * height * bytes_per_pixel, 1);
  image_rows = calloc(height, sizeof(char*));

  for(i = 0; i < height; i++)
    image_rows[i] = &(image[i*width*bytes_per_pixel]);

  setjmp(png_jmpbuf(png_ptr));
  png_read_image(png_ptr, image_rows);
  png_read_end(png_ptr, end_info);

  printf("#define %s_width %d\n", output_base, width);
  printf("#define %s_height %d\n", output_base, height);

  printf("static char %s_data[] = {\n");
  linepos = 0;

  for(i = 0; i < width*height*bytes_per_pixel; i++)
    {
    linepos++;
    if(!(linepos % 10))
      {
      printf("\n");
      linepos = 0;
      }
    printf("0x%02x, ", (int)(image[i]));
    }
  printf("};\n");
  }
