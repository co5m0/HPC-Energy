CC = gcc
CFLAGS = -O2 -Wall
LFLAGS = -lm
MAINOBJECT = test.o
LIBOBJECT = rapllib.o
TARGET = main.out


$(TARGET):	$(MAINOBJECT) $(LIBOBJECT)
	$(CC) -o $(TARGET) $(LIBOBJECT) $(MAINOBJECT) $(LFLAGS)

$(LIBOBJECT):	rapllib.c
	$(CC) $(CFLAGS) -c rapllib.c

clean:	
	rm -f *.o *~ $(TARGET)