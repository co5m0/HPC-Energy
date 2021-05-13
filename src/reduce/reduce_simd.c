#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>
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
    double begin = omp_get_wtime();

#pragma omp parallel
    {
#pragma omp private(i, j) simd reduction(+ \
                                         : somma)
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                somma = somma + a[i][j] * b[i][j];
            }
        }
    }

    double end = omp_get_wtime();
    double time_spent = (end - begin);
    /*printf("<-------------<\n");
    printMatrix(a,n);
    printf("<-------------<\n");
    printMatrix(b,n);
    */
    free(a);
    free(b);
    printf("Result sum: %f\n", somma);
    printf("Time exec: %f sec, Matrix size: %d\n", time_spent, n);
}