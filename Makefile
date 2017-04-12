OBJS = jpegprocess.o main.o
CC = gcc
DEBUG = -g
CFLAGS = -ljpeg $(DEBUG)
LFLAGS = -Wall -c


p1 : $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -lm -o test 

main.o : jpegprocess.h main.c 
	$(CC) $(pkg-config --cflags --libs python3) $(LFLAGS) main.c $(CFLAGS)

jpegprocess.o : jpegprocess.h jpegprocess.c
	$(CC) $(LFLAGS) jpegprocess.c $(CFLAGS)

clean:
	\rm *.o test
