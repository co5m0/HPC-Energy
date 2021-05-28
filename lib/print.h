#include <stdio.h>
#include <string.h>
#define MYFILE "prova.csv"

void print_file(char *name_csv, char *name, double time, int size, double energy, int nThreads)
{
    FILE *f;
    int s;

    //File opening .csv
    if ((f = fopen(name_csv, "a+")) == NULL)
    {
        printf("Errore opening file...\n");
    }

    //File size calculation to check if it is empty
    fseek(f, 0, SEEK_END);
    s = ftell(f);

    //If it is empty it is initialized with the label names
    if (s == 0)
    {
        printf("Empty File... Initialization!\n");
        fprintf(f, "%s| %s| %s| %s| %s\n", "NAMEFILE", "TIME", "SIZE", "ENERGY", "NTHREAD");
    }

    /* //Saving values ​​in csv file
    fprintf(f, "%s, %f, %d, %f, %d\n", name, time, size, energy, nThreads); */

    int time_1 = (int)time;
    int time_2 = (time - time_1) * 1000;

    int energy_1 = (int)energy;
    int energy_2 = (energy - energy_1) * 1000;
    //Saving values ​​in csv file
    fprintf(f, "%s| %d,%d| %d| %d,%d| %d\n", name, time_1, time_2, size, energy_1, energy_2, nThreads);

    fclose(f);
}