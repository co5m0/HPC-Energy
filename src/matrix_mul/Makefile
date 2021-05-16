CC = gcc
CFLAGS = -O2 -Wall 
LFLAGS = -O2 -lm -fopenmp
MAINOBJECT_MULTI = matrix_mul_multi.o
MAINOBJECT_SEQ = matrix_mul_seq.o
MAINOBJECT_SIMD = matrix_mul_simd.o
MAINOBJECT_SINGLE = matrix_mul_single.o
MAINOBJECT_TIL = matrix_mul_til.o
LIBOBJECT = ../../lib/rapllib.o
TARGET_MULTI = multi.out
TARGET_SEQ	 = seq.out
TARGET_SIMD	 = simd.out
TARGET_SINGLE= single.out
TARGET_TIL	 = til.out

all: multi.out seq.out simd.out single.out

multi.out:	matrix_mul_multi.o $(LIBOBJECT)
	$(CC) -o $(TARGET_MULTI) $(LIBOBJECT) $(MAINOBJECT_MULTI) $(LFLAGS)

seq.out:	matrix_mul_seq.o $(LIBOBJECT)
	$(CC) -o $(TARGET_SEQ) $(LIBOBJECT) $(MAINOBJECT_SEQ) $(LFLAGS)

simd.out:	matrix_mul_simd.o $(LIBOBJECT)
	$(CC) -o $(TARGET_SIMD) $(LIBOBJECT) $(MAINOBJECT_SIMD) $(LFLAGS)

single.out:	matrix_mul_single.o $(LIBOBJECT)
	$(CC) -o $(TARGET_SINGLE) $(LIBOBJECT) $(MAINOBJECT_SINGLE) $(LFLAGS)

# til.out:	matrix_mul_til.o $(LIBOBJECT)
# 	$(CC) -o $(TARGET_TIL) $(LIBOBJECT) $(MAINOBJECT_TIL) $(LFLAGS)

clean:	
	rm -f *.o *~ *.out