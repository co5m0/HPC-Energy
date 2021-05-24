#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>

#include "../../lib/print.h"
#include "../../lib/rapllib.h"

#define N 1024
#define NTHREADS 4

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
    int nThreads;
    float somma = 0;
    
    Rapl_info rapl = new_rapl_info();
    detect_cpu(rapl);
    detect_packages(rapl);
    rapl_sysfs(rapl);

    if (argc == 2)
    {
        n = atoi(argv[1]);
        nThreads = NTHREADS;
    }
    else
    {
        if (argc == 3){
            n = atoi(argv[1]);
            nThreads = atoi(argv[2]);
        }else{
            n = N;
            nThreads = NTHREADS;
        }
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
    rapl_sysfs_start(rapl);

#pragma omp parallel for num_threads(nThreads) private(i, j) reduction(+ \
                                                                       : somma)
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            somma = somma + a[i][j] * b[i][j];
        }
    }

    rapl_sysfs_stop(rapl);
    double end = omp_get_wtime();
    
    double time_spent = (end - begin);
    /*printf("<-------------<\n");
    printMatrix(a,n);
    printf("<-------------<\n");
    printMatrix(b,n);
    */
    printf("Result sum: %f\n", somma);
    printf("Time exec: %f sec, Matrix size: %d, Number Threads: %d\n", time_spent, n, nThreads);
    print_file("reduce_multi", time_spent, n, rapl_get_energy(rapl), nThreads);
}