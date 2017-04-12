#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <jpeglib.h>
#include <time.h>

#define IMAGEBADFILE -1
#define IMAGENOERR 0
#define IMAGEPIXELS 1000000

extern int Width, Height, Depth;
extern unsigned char *BMap;
extern unsigned char *grayLevel;

extern int loadJpg(const char* Name);

extern int writeJpg(char * name, int xres, int yres, unsigned char *img);

extern int writeGrayJpg(char * name, int xres, int yres, unsigned char *grayImg);

extern void freeJpg();

