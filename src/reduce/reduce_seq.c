#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "rapllib.h"

#define N 1024

void printMatrix(float **mat, int len)
{
    for (int i = 0; i < len; i++)
    {
        for (int j = 0; j < len; j++)
        {
            printf("\t%f", mat[i][j]);
        }
        printf("\n");
    }
}
int main(int argc, char **argv)
{
    srand(1);

    int i, j, k, n;
    float somma = 0;

    Rapl_info rapl = new_rapl_info();
    detect_cpu(rapl);
    detect_packages(rapl);
    rapl_sysfs(rapl);

    if (argc > 1)
    {
        n = atoi(argv[1]);
    }
    else
    {
        n = N;
    }

    float **a, **b, **c;

    a = (float **)malloc(n * sizeof(float *));
    b = (float **)malloc(n * sizeof(float *));

    for (int x = 0; x < n; x++)
    {
        a[x] = malloc(n * sizeof(float));
        b[x] = malloc(n * sizeof(float));
    }

    //initialization
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            a[i][j] = ((float)rand() * (5) / (float)RAND_MAX - 2);
            b[i][j] = ((float)rand() * (5) / (float)RAND_MAX - 2);
        }
    }
    //printf("RANDOM: %f\n",(float)rand()/ 5 - 2.0);

    //calculate prod
    clock_t begin = clock();
    rapl_sysfs_start(rapl);

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            somma = somma + a[i][j] * b[i][j];
        }
    }

    rapl_sysfs_stop(rapl);
    clock_t end = clock();

    float time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    /*printf("<-------------<\n");
    printMatrix(a, n);
    printf("<-------------<\n");
    printMatrix(b, n);
    */
    printf("Result sum: %f\n", somma);
    printf("Time exec: %f sec, Matrix size: %d\n", time_spent, n);
}