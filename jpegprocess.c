#include "jpegprocess.h"

int Width, Height, Depth;
unsigned char *BMap;
unsigned char *grayLevel;

int loadJpg(const char* Name) {
  unsigned char r, g, b;
  int width, height;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  FILE * infile;        /* source file */
  JSAMPARRAY pJpegBuffer;       /* Output row buffer */
  int row_stride;       /* physical row width in output buffer */
  if ((infile = fopen(Name, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", Name);
    return 0;
  }
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);
  width = cinfo.output_width;
  height = cinfo.output_height;

  unsigned char * pDummy = (unsigned char*) malloc(sizeof(unsigned char) * width * height * 3);
  unsigned char * pTest = pDummy;
  if (!pDummy) {
    printf("NO MEM FOR JPEG CONVERT!\n");
    return 0;
  }
  row_stride = width * cinfo.output_components;
  pJpegBuffer = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  grayLevel = (unsigned char*) malloc(sizeof(unsigned char) * width * height);
  unsigned char *countGray = grayLevel;
  
  int x;
  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
    for (x = 0; x < width; x++) {
      //a = 0; // alpha value is not supported on jpg
      r = pJpegBuffer[0][cinfo.output_components * x];
      if (cinfo.output_components > 2) {
        g = pJpegBuffer[0][cinfo.output_components * x + 1];
        b = pJpegBuffer[0][cinfo.output_components * x + 2];
      } else {
        g = r;
        b = r;
      }
      *(pDummy++) = b;
      *(pDummy++) = g;
      *(pDummy++) = r;
      //*(pDummy++) = a;

      *(countGray++) = (unsigned char) (0.2989 * r + 0.5870 * g + 0.1140 * b);
    }
  }
  fclose(infile);
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  BMap = pTest; 
  Height = height;
  Width = width;
  Depth = 32;

  return 1;
}

int writeJpg(char * name, int xres, int yres, unsigned char *img) {
  FILE *ofp;
  struct jpeg_compress_struct cinfo;   /* JPEG compression struct */
  struct jpeg_error_mgr jerr;          /* JPEG error handler */
  JSAMPROW row_pointer[1];             /* output row buffer */
  int row_stride; 
  int i, j, k; 
  unsigned char imgdata[IMAGEPIXELS];                     /* physical row width in output buf */

  if ((ofp = fopen(name, "wb")) == NULL) {
    return IMAGEBADFILE;
  }

  for (i = 0; i < yres; i++)
    for (j = 0; j < xres; j++)
      for (k = 0; k < 3; k++)
        imgdata[i*xres*3 + j*3 + k] = img[(yres-1-i)*xres*3 + j*3 + 2-k];

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, ofp);

  cinfo.image_width = xres;
  cinfo.image_height = yres;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 95, 0);

  jpeg_start_compress(&cinfo, TRUE);

  /* Calculate the size of a row in the image */
  row_stride = cinfo.image_width * cinfo.input_components;

  /* compress the JPEG, one scanline at a time into the buffer */
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = &(imgdata[(yres - cinfo.next_scanline - 1)*row_stride]);
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  fclose(ofp);

  return IMAGENOERR; /* No fatal errors */
}

int writeGrayJpg(char * name, int xres, int yres, unsigned char *grayImg) {
  FILE *ofp;
  struct jpeg_compress_struct cinfo;   /* JPEG compression struct */
  struct jpeg_error_mgr jerr;          /* JPEG error handler */
  JSAMPROW row_pointer[1];             /* output row buffer */
  int row_stride;                      /* physical row width in output buf */
  int i, j, k; 
  unsigned char imgdata[IMAGEPIXELS]; 
  unsigned char img[IMAGEPIXELS];                   

  if ((ofp = fopen(name, "wb")) == NULL) {
    return IMAGEBADFILE;
  }
  
  for (i = 0; i < xres * yres; i++)
  {
    if (grayImg[i] == 255){
      img[i*3] = 255;
      img[i*3 + 1] = 0;
      img[i*3 + 2] = 0;
    }
    else{
      img[i*3] = grayImg[i] / 3;
      img[i*3 + 1] = grayImg[i] / 3;
      img[i*3 + 2] = grayImg[i] / 3;
    }
    //printf("%d ", img[i*3+j]);
  }

  for (i = 0; i < yres; i++)
    for (j = 0; j < xres; j++)
      for (k = 0; k < 3; k++)
        imgdata[i*xres*3 + j*3 + k] = img[(yres-1-i)*xres*3 + j*3 + 2-k];

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, ofp);

  cinfo.image_width = xres;
  cinfo.image_height = yres;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 95, 0);

  jpeg_start_compress(&cinfo, TRUE);

  /* Calculate the size of a row in the image */
  row_stride = cinfo.image_width * cinfo.input_components;

  /* compress the JPEG, one scanline at a time into the buffer */
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = &(imgdata[(yres - cinfo.next_scanline - 1)*row_stride]);
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  fclose(ofp);

  return IMAGENOERR; /* No fatal errors */
}

void freeJpg(){
  free(BMap);
  free(grayLevel);
}

/*
int main()
{
    loadJpg("track.jpg");
    writeJpg("track_test.jpg", Width, Height, (unsigned char*)BMap);
    freeJpg();
    return 0;
}
*/
