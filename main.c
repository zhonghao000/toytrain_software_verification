#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <jpeglib.h>
#include <time.h>
#include <math.h>
#include <string.h>
//#include <conio.h>
#include <python3.4m/Python.h>
#include "jpegprocess.h"

#define HISTOGRAM_NUMBER 256
#define CLASS_SMALLEST_MEAN 0
#define CLASS_GREATEST_MEAN 255
#define INVALID_NUM -1
#define NORMAL 0
#define SLOW 1
#define STOP 2
#define TRACKGRAYLIMIT 100
#define Pi 3.141592654

int state = 0;

int report_way()
{
  clock_t start = clock(), diff;

  /* read the image from disk */
  char fileName[] = "track.jpg";
  loadJpg(fileName);
  printf("The width is: %d\nThe height is:%d\n\n", Width, Height);

  /* step 1: convert RGB to greyscale, which is done in loadJPG */

  /* step 2: color detection of the bottom line */
  /* step 2.1: Histogram calculation */
  /* initialize the histogram */
  int histo[HISTOGRAM_NUMBER] = {0};
  int i, j, k; //loop index
  printf("Grey level at the bottom line:\n");
  for (i = 0 + (Height - 1) * Width; i < Height * Width; i++)
  {
    histo[grayLevel[i]] ++;
    printf("%d  ", grayLevel[i]);
  }

  printf("\n\nThe histogram for the bottom line:\n");
  for (i = 0; i < HISTOGRAM_NUMBER; i++)
    printf("%d  ", histo[i]);

  /* step 2.2: K-means */
  /* Divide the hidtogram into two classes.
   * One class (class 1) contains the pixels having relatively high grayscales, potentially the background pixels.
   * The other class (class 2) contains the pixels having relatively low grayscales, potentially the track pixels.
   */
  int class1Mean = CLASS_SMALLEST_MEAN; 
  int class2Mean = CLASS_GREATEST_MEAN;
  int class1MeanPre = INVALID_NUM;
  int class2MeanPre = INVALID_NUM;
  int sum1, sum2, class1Num, class2Num;
  while (class1Mean != class1MeanPre && class2Mean != class2MeanPre)
  {
    sum1 = sum2 = 0;
    class1Num = class2Num = 0;
    for (i = 0; i < 256; i++)
    {
      if (histo[i] == 0)
        continue;
      if (abs(class1Mean - i) < abs(class2Mean - i)) {
        sum1 += i * histo[i];
        class1Num += histo[i];
      }
      else {
        sum2 += i * histo[i];
        class2Num += histo[i];
      } 
    }
    class1MeanPre = class1Mean;
    class2MeanPre = class2Mean;
    class1Mean = sum1 / class1Num;
    class2Mean = sum2 / class2Num;
  }
  printf("\n\nCalculated class mean values:\nclass1Mean = %d, class2Mean = %d\n", class1Mean, class2Mean);

  /* step 2.3 & 2.4: Scan pixels & eliminate outliers */
  /* Scan the pixels from the left to the right of the bottom line. 
   * Store the x-coordinates of the pixels whose graysclae is closer to the mean value of class 1 to array class1Cordinate[].
   * Use quatile method to eliminate the outliers in class1Coordinate[].
   * Store the refined candidates into array refinedClass1Cordinate[].
   */
  int rowBase = (Height - 1) * Width;
  const int imageWidth = Width;
  int class1Coordinate[imageWidth];
  int refinedClass1Coordinate[imageWidth];
  for (i = 0; i < Width; i ++){
    class1Coordinate[i] = INVALID_NUM;
    refinedClass1Coordinate[i] = INVALID_NUM;
  }
  int *class1CoorPtr = class1Coordinate;
  for (i = rowBase; i < rowBase + Width; i++) {
    if (abs(grayLevel[i] - class1Mean) > abs(grayLevel[i] - class2Mean))
      continue;
    else 
      *(class1CoorPtr++) = i - rowBase;
  }
  int maximumLimit = ((int) (1.5 * ((double) (class1Coordinate[class1Num * 3 / 4] - class1Coordinate[class1Num / 4])))) + class1Coordinate[class1Num * 3 / 4];
  int minimumLimit = -((int) (1.5 * ((double) (class1Coordinate[class1Num * 3 / 4] - class1Coordinate[class1Num / 4])))) + class1Coordinate[class1Num / 4];
  int *refinedClass1CoorPtr = refinedClass1Coordinate;
  for (i = 0; i < class1Num; i++)
  {
    if (class1Coordinate[i] >= minimumLimit && class1Coordinate[i] <= maximumLimit)
      *(refinedClass1CoorPtr++) = class1Coordinate[i];
  }
  printf("\nThe x-coordinates of pixels belong to class 1:\n");
  for (i = 0; i < (int)(class1CoorPtr - class1Coordinate); i++)
    printf("%d ", class1Coordinate[i]);
  printf("\n\nThe x-coordinates of pixels belong to class 1 after refined:\n");
  for (i = 0; i < (int)(refinedClass1CoorPtr - refinedClass1Coordinate); i++)
    printf("%d ", refinedClass1Coordinate[i]);

  /* step 3: scanning above and boundary detection */

  int rightmostCoorIndex = (int)(refinedClass1CoorPtr - refinedClass1Coordinate) - 1;
  int trackLeftPotentialCoor = refinedClass1Coordinate[0] - 0.1 * (refinedClass1Coordinate[rightmostCoorIndex] - refinedClass1Coordinate[0]);
  int trackRightPotentialCoor = refinedClass1Coordinate[rightmostCoorIndex] + 0.1 * (refinedClass1Coordinate[rightmostCoorIndex] - refinedClass1Coordinate[0]);
  int prevClass1Num = class1Num;
  int inconsistencyCount = 0;
  int temp;
  printf("\n\nThe boundaries of the track in each line:\n");
  for (i = Height - 2; i >= 0; i--)
  {
    if (trackLeftPotentialCoor < 0 || trackRightPotentialCoor > Width)
      break;
    sum1 = sum2 = 0;
    class1CoorPtr = class1Coordinate;
    for (j = trackLeftPotentialCoor; j <= trackRightPotentialCoor; j++)
    {
      if (abs(grayLevel[i * Width + j] - class1Mean) > abs(grayLevel[i *Width + j] - class2Mean))
        continue;
      else {
        *(class1CoorPtr++) = j;
        //sum1 += grayLevel[i * Width + j];
      }
    }
    maximumLimit = ((int) (1 * ((double) (class1Coordinate[class1Num * 3 / 4] - class1Coordinate[class1Num / 4])))) + class1Coordinate[class1Num * 3 / 4];
    minimumLimit = -((int) (1 * ((double) (class1Coordinate[class1Num * 3 / 4] - class1Coordinate[class1Num / 4])))) + class1Coordinate[class1Num / 4];
    refinedClass1CoorPtr = refinedClass1Coordinate;
    for (j = 0; j < class1Num; j++)
    {
      if (class1Coordinate[j] >= minimumLimit && class1Coordinate[j] <= maximumLimit)
        *(refinedClass1CoorPtr++) = class1Coordinate[j];
    }
    class1Num = (int)(refinedClass1CoorPtr - refinedClass1Coordinate);
    if (prevClass1Num - class1Num > 100){
      inconsistencyCount ++;
      if (inconsistencyCount > 10)
        break;
      continue;
    }
    k = class1Num - 1; //what if class1Num == 0?
    if (trackLeftPotentialCoor == refinedClass1Coordinate[0] || trackRightPotentialCoor == refinedClass1Coordinate[k])
      break;

    class2Num = 0;
    for (j = 0; j < Width; j++)
    {
      if (j < refinedClass1Coordinate[0] || j > refinedClass1Coordinate[k]){
        sum2 += grayLevel[i * Width + j];
        class2Num += 1;
      }
      else {
        sum1 += grayLevel[i * Width + j];
      }
    }
    temp = sum1 / (Width - class2Num);
    if (temp > TRACKGRAYLIMIT)
    {
      inconsistencyCount++;
      if (inconsistencyCount > 100)
        break;
      continue;
    }
    inconsistencyCount = 0;
    prevClass1Num = class1Num;
    class1Mean = temp;
    class2Mean = sum2 / class2Num;
    trackLeftPotentialCoor = refinedClass1Coordinate[0] - 0.5 * (refinedClass1Coordinate[k] - refinedClass1Coordinate[0]);
    trackRightPotentialCoor = refinedClass1Coordinate[k] + 0.5 * (refinedClass1Coordinate[k] - refinedClass1Coordinate[0]);
    printf("line %d: (%d, %d), C1mean %d, C2mean %d\n", Height - 1 - i, refinedClass1Coordinate[0], refinedClass1Coordinate[k], class1Mean, class2Mean);
    if (Height - 1 - i == 563)
      temp = 0;
    
  }


  diff = clock() - start;
  int msec = diff * 1000 / CLOCKS_PER_SEC;
  printf("\n\nTime taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
  /*
  printf("\nAfter classified:");
  i = 0;
  while(refinedClass1Cordinate[i] != 0) {
    printf("%d  ", grayLevel[refinedClass1Cordinate[i]]);
    i ++;
  }
  */
  free(BMap);
  free(grayLevel);
  return 0;
}

int gaussianFilter[5][5] = {{2, 4, 5, 4, 2},
                            {4, 9, 12, 9, 4},
                            {5, 12, 15, 12, 5},
                            {4, 9, 12, 9, 4},
                            {2, 4, 5, 4, 2}};

int matrixConv(int pixelIndex, unsigned char* gray)
{
  int yCoor = pixelIndex / Width;
  int xCoor = pixelIndex % Width;
  int gaussFilterSum, sum, i, j;
  sum = gaussFilterSum = 0;

  for (i = -2; i < 3; i++)
    for (j = -2; j < 3; j++)
    {
      if (yCoor + i >= 0 && yCoor + i < Height && xCoor + j >= 0 && xCoor + j < Width)
      {
        sum += gaussianFilter[i+2][j+2] * gray[(yCoor+i)*Width+xCoor+j];
        gaussFilterSum += gaussianFilter[i+2][j+2];
        //printf("(%d, %d)", i, j);
      }
    }
  
  if (gaussFilterSum == 0)
    return 0;
  else 
    return (sum / gaussFilterSum);
}

int Gx[3][3] = {{-1, 0, 1},
                {-2, 0, 2},
                {-1, 0, 1}};

int Gy[3][3] = {{-1, -2, -1},
                {0, 0, 0},
                {1, 2, 1}};

void gradientCal(unsigned char* pixel, int pixelIndex, double* gradStrenth, int* gradAngle)
{
  int yCoor = pixelIndex / Width;
  int xCoor = pixelIndex % Width;
  double GxSum = 0;
  double GySum = 0;
  double angle = 90;
  int i, j;

  for (i = -1; i < 2; i++)
    for (j = -1; j < 2; j++)
    {
      if (yCoor + i >= 0 && yCoor + i < Height && xCoor + j >= 0 && xCoor + j < Width)
      {
        GxSum += Gx[i+1][j+1] * pixel[(yCoor+i)*Width+xCoor+j];
        GySum += Gy[i+1][j+1] * pixel[(yCoor+i)*Width+xCoor+j];
      }
    }
  gradStrenth[pixelIndex] = sqrt(GxSum*GxSum + GySum*GySum);
  if (GySum != 0)
    angle = atan(GySum / GxSum) * 180 / Pi;

  if (angle <= 67.5 && angle > 22.5)
    gradAngle[pixelIndex] = 45;
  else if (angle <= 22.5 && angle > -22.5)
    gradAngle[pixelIndex] = 0;
  else if (angle <= -22.5 && angle > -67.5)
    gradAngle[pixelIndex] = -45;
  else
    gradAngle[pixelIndex] = 90;
}

void create_marks_csv(char *filename,unsigned char*a){
  printf("\n Creating %s.csv file",filename);
  FILE *fp;
  int i,j;
  filename=strcat(filename,".csv");
  fp=fopen(filename,"w+");
  //fprintf(fp,"Student Id, Physics, Chemistry, Maths");
  for(i=0;i<480;i++){
    for(j=0;j<640;j++)
        fprintf(fp,",%d ",a[i*640 + j]);
    fprintf(fp,"\n");
  }
  fclose(fp);
  printf("\n %sfile created",filename);
}

void canny(unsigned char* smoothGray){
  int i, j;
  /* step 1: use Gaussian filter to smooth the image */
  //printf("%d\n%d\n%d\n%d", matrixConv(0), matrixConv(639), matrixConv(306560), matrixConv(307199));
  
  for (i = 0; i < Width * Height; i++){
    smoothGray[i] = (unsigned char) matrixConv(i, grayLevel);
  }

  /* step 2: find the intensity gradient of the image */
  
  double *gradStrenth = (double*)malloc(IMAGEPIXELS * sizeof(double));
  int *gradAngle = (int*)malloc(IMAGEPIXELS * sizeof(double));
  for (i = 0; i < Width * Height; i++)
    gradientCal(smoothGray, i, gradStrenth, gradAngle);

 
  //for (i = 0; i < Width * Height; i++)
  //if(smoothGray[i] < 100)
  //  smoothGray[i] = 0;

  for (i = 0; i < Height; i++)
  for (j = 0; j < Width; j++)
  {
    
    if (gradAngle[i*Width + j] == 90){
      if (i - 1 >= 0 && gradStrenth[i*Width+j] < gradStrenth[(i-1)*Width+j]){
        smoothGray[i*Width+j] = 0;
        continue;
      }
      if (i + 1 < Height && gradStrenth[i*Width+j] < gradStrenth[(i+1)*Width+j]){
        smoothGray[i*Width+j] = 0;
        continue;
      }
      else smoothGray[i*Width+j] =(unsigned char)(gradStrenth[i*Width+j]);
    }
    
    if (gradAngle[i*Width + j] == 45){
      if (i - 1 >= 0 && j + 1 < Width && gradStrenth[i*Width+j] < gradStrenth[(i-1)*Width+j+1]){
        smoothGray[i*Width+j] = 0;
        continue;
      }
      if (i + 1 < Height && j - 1 >= 0 && gradStrenth[i*Width+j] < gradStrenth[(i+1)*Width+j-1]){
        smoothGray[i*Width+j] = 0;
        continue;
      }
      else smoothGray[i*Width+j] =(unsigned char)(gradStrenth[i*Width+j]);
    }

    if (gradAngle[i*Width + j] == 0){
      if (j - 1 >= 0 && gradStrenth[i*Width+j] < gradStrenth[(i)*Width+j-1]){
        smoothGray[i*Width+j] = 0;
        continue;
      }
      if (j + 1 < Width && gradStrenth[i*Width+j] < gradStrenth[i*Width+j+1]){
        smoothGray[i*Width+j] = 0;
        continue;
      }
      else smoothGray[i*Width+j] =(unsigned char)(gradStrenth[i*Width+j]);
    }
    if (gradAngle[i*Width + j] == -45){
      if (i - 1 >= 0 && j - 1 >= 0 && gradStrenth[i*Width+j] < gradStrenth[(i-1)*Width+j-1]){
        smoothGray[i*Width+j] = 0;
        continue;
      }
      if (i + 1 < Height && j + 1 < Width && gradStrenth[i*Width+j] < gradStrenth[(i+1)*Width+j+1]){
        smoothGray[i*Width+j] = 0;
        continue;
      }
      else smoothGray[i*Width+j] =(unsigned char)(gradStrenth[i*Width+j]);
    }
    
  }

  for (i = 0; i < Width * Height; i++)
  {
    if (smoothGray[i] < 100)
      smoothGray[i] = 0 ;
    //printf("%d ", (unsigned char) gradStrenth[i]);
  }
  free(gradStrenth);
  free(gradAngle);
}

int main()
{
  int i, j, k, testTime;
  unsigned char *smoothGray = (unsigned char*) malloc(IMAGEPIXELS * sizeof(unsigned char));
  int rowBase;
  int class1Coordinate[640];
  int coorMidean[480];
  int class1Num;
  int maximumLimit, minimumLimit;
  int *class1CoorPtr;
  int bendSum;
  int classNumber[480];
  char testBuffer[50] = "straight1.jpg";
  //canny(smoothGray);
  
  clock_t start = clock(), diff;
  //loadJpg(testBuffer);
  //printf("width = %d, height = %d\n", Width, Height);
  testTime = 0;
  while (testTime < 15){
  loadJpg(testBuffer);
  start = clock();
  state = 0;
  for (i = 0; i < Width * Height; i ++)
    if (grayLevel[i] < 100)
      smoothGray[i] = 0;
    else
      smoothGray[i] = 200;
  //writeGrayJpg("step1.jpg", Width, Height, smoothGray);

  bendSum = 0;
  for (j = Height / 5; j < Height / 2; j++)
  {
    rowBase = (Height - 1 - j) * Width;
    class1Num = 0;
    class1CoorPtr = class1Coordinate;
    for (i = rowBase; i < rowBase + Width; i++) 
    {
      if (smoothGray[i] == 0)
      {
        *(class1CoorPtr++) = i - rowBase;
        class1Num += 1;
      }
    }
    maximumLimit = ((int) (0.5 * ((double) (class1Coordinate[class1Num * 3 / 4] - class1Coordinate[class1Num / 4])))) + class1Coordinate[class1Num * 3 / 4];
    minimumLimit = -((int) (0.5 * ((double) (class1Coordinate[class1Num * 3 / 4] - class1Coordinate[class1Num / 4])))) + class1Coordinate[class1Num / 4];
    if (maximumLimit > Width)
      maximumLimit = Width;
    if (minimumLimit < 0)
      minimumLimit = 0;
    classNumber[j] = 0;
    for (i = rowBase; i < rowBase + Width; i++) 
    {
      if (i - rowBase < minimumLimit || i - rowBase > maximumLimit)
      {
        smoothGray[i] = 200;
      }
      else if (smoothGray[i] == 0){
        classNumber[j] += 1;
      }
    }
    if (j > 5 + Height / 5)
    {
      if (classNumber[j] < classNumber[j-6] / 2)
      {
        state = 2;
        break;
      }
    }
    //printf("%d:%d\n", 480-j, classNumber[j]);
    coorMidean[j] = (maximumLimit + minimumLimit)/2;
    //printf("%d ", coorMidean[i]);
    if (j >= 4 + Height / 5)
    {
      for (k = 1; k < 5; k++)
        coorMidean[j] += coorMidean[j - k];
      coorMidean[j] /= 5;
      bendSum += coorMidean[j] - coorMidean[j-4];
      if (j >= 34 + Height / 5)
        bendSum -= coorMidean[j-30] - coorMidean[j-34];
      //if (testTime == 10)
        //printf("%d:%d %d\n",480 - j, coorMidean[j] - coorMidean[j-4], bendSum);
      if (abs(bendSum) >= 50)
      {
        state = 1;
        if (abs(bendSum) >= 150)
          break;
      }
      
    }
    
    //printf("%d ", class1Num);
  }
  //printf("status = %d\n", state);
  diff = clock() - start;
  int msec = diff * 1000 / CLOCKS_PER_SEC;
  printf("picture name: %s, status = %d, time taken: %d seconds %d milliseconds\n\n", testBuffer, state, msec/1000, msec%1000);

  if (testTime < 15){
  for (i = 0; i < 240 * 640; i++)
    smoothGray[i] = 0;
  for (; i < 480 * 640; i++)
    if (smoothGray[i] == 0)
      smoothGray[i] = 200;
    else
      smoothGray[i] = 0;
  i = 0;
  while(testBuffer[i] != '\0')
    i++;
  testBuffer[i-1] = 'G';
  testBuffer[i-2] = 'P';
  testBuffer[i-3] = 'J';
  writeGrayJpg(testBuffer, Width, Height, smoothGray);
  testBuffer[i-1] = 'g';
  testBuffer[i-2] = 'p';
  testBuffer[i-3] = 'j';
  }
  testTime ++;
  if (testTime < 5)
    testBuffer[8] = ((char) ((testTime) %5 + 1)) + '0';
  if (testTime == 5)
    strcpy(testBuffer, "bend1.jpg");
  if (testTime > 5 && testTime < 10)
    testBuffer[4] = ((char) ((testTime) %5 + 1)) + '0';
  if (testTime == 10)
    strcpy(testBuffer, "obstacle1.jpg");
  if (testTime > 10 && testTime < 15)
    testBuffer[8] = ((char) ((testTime) %5 + 1)) + '0';
  }

  /*
  printf("status = %d\n", state);
  diff = clock() - start;
  int msec = diff * 1000 / CLOCKS_PER_SEC;
  printf("Time taken %d seconds %d milliseconds\n\n", msec/1000, msec%1000);
  */
  //writeGrayJpg("step4.jpg", Width, Height, smoothGray);
  /*
  char file[20] = "testChar";
  create_marks_csv(file,smoothGray);
  writeGrayJpg("step4.jpg", Width, Height, smoothGray);
    //printf("%d ", smoothGray[i]);
  */

  freeJpg();
  free(smoothGray);

  return 0;
}
