#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>
#define N 1024
#define NTHREADS 4

void printMatrix(double **mat, int len)
{
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            printf("\t%f", mat[i][j]);
        }
        printf("\n");
    }
}
int main(int argc, char **argv)
{
srand(1);
    
    int i, j, k, n;
    int nThreads;
    double somma = 0;
    if (argc > 1)
    {
        n = atoi(argv[1]);
        nThreads = atoi(argv[2]);
    }
    else
    {
        n = N;
        nThreads = NTHREADS;
    }

    double **a, **b, **c;

    a = (double **)malloc(n * sizeof(double *));
    b = (double **)malloc(n * sizeof(double *));

    for (int x = 0; x < n; x++)
    {
        a[x] = malloc(n * sizeof(double));
        b[x] = malloc(n * sizeof(double));
    }

    //initialization
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            a[i][j] = ((double) rand()*(5)/(double)RAND_MAX-2);
            b[i][j] = ((double) rand()*(5)/(double)RAND_MAX-2);
            
        }
    }
    //printf("RANDOM: %f\n",(double)rand()/ 5 - 2.0);

    //calculate prod
    double begin = omp_get_wtime();
    #pragma omp parallel for num_threads(nThreads) private(i,j) reduction(+ \
                                                                        : somma)
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            somma = somma + a[i][j] * b[i][j];
        }
    }
    double end = omp_get_wtime();
    double time_spent = (end - begin);
    /*printf("<-------------<\n");
    printMatrix(a,n);
    printf("<-------------<\n");
    printMatrix(b,n);
    */
    printf("Result sum: %f\n", somma);
    printf("Time exec: %f sec, Matrix size: %d, Number Threads: %d\n", time_spent,n,nThreads);
}