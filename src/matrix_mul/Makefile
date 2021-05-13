CC = gcc
CFLAGS = -O2 -Wall 
LFLAGS = -lm -fopenmp
MAINOBJECT = matrix_mul_multi.o
LIBOBJECT = rapllib.o
TARGET = main.out


$(TARGET):	$(MAINOBJECT) $(LIBOBJECT)
	$(CC) -o $(TARGET) $(LIBOBJECT) $(MAINOBJECT) $(LFLAGS)

$(LIBOBJECT):	rapllib.c
	$(CC) $(CFLAGS) -c rapllib.c

clean:	
	rm -f *.o *~ $(TARGET)