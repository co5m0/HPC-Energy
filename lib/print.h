#include <stdio.h>
#define MYFILE "test.csv"

void print_file(char *name, double time, int size, double energy, int nThreads) {
    FILE *f;
    int s;

    //File opening .csv
    if ((f = fopen(MYFILE, "a+")) == NULL) {
        printf("Errore opening file...\n");
    }

    //File size calculation to check if it is empty
    fseek(f, 0, SEEK_END);
    s = ftell(f);

    //If it is empty it is initialized with the label names
    if (s == 0) {
        printf("Empty File... Initialization!\n");
        fprintf(f, "%s, %s, %s, %s\n", "NAMEFILE", "TIME", "SIZE", "ENERGY", "NTHREAD");
    }

    //Saving values ​​in csv file
    fprintf(f, "%s, %f, %d, %f, %d\n", name, time, size, energy, nThreads);
    fclose(f);
}
