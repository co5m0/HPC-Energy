//#include <cilk/cilk.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rapllib.h"

#define N 1024
#define S 32
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

void matmul_tiled(float **a, float **b, float **c, int n, int s)
{

#pragma omp parallel for
    for (int ih = 0; ih < n; ih += s)
    {
#pragma omp parallel for simd
        for (int jh = 0; jh < n; jh += s)
            for (int kh = 0; kh < n; kh += s)
                for (int il = 0; il < s; il++)
                    for (int kl = 0; kl < s; kl++)
                        for (int jl = 0; jl < s; jl++)
                            c[ih + il][jh + jl] += a[ih + il][kh + kl] * b[kh + kl][jh + jl];
    }
}

int main(int argc, char **argv)
{
    srand(1);
    int n, s;
    float **a, **b, **c;
    int i, j, k, x;
    double begin;
    double time_spent;
       
    Rapl_info rapl = new_rapl_info();
    detect_cpu(rapl);
    detect_packages(rapl);
    rapl_sysfs(rapl);

    if (argc > 1)
    {
        n = atoi(argv[1]);
        s = atoi(argv[2]);
    }
    else
    {
        n = N;
        s = S;
    }

    a = (float **)malloc(n * sizeof(float *));
    b = (float **)malloc(n * sizeof(float *));
    c = (float **)malloc(n * sizeof(float *));

    for (x = 0; x < n; x++)
    {
        a[x] = malloc(n * sizeof(float));
        b[x] = malloc(n * sizeof(float));
        c[x] = malloc(n * sizeof(float));
    }

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            a[i][j] = ((float)rand() * (5) / (float)RAND_MAX - 2);
            b[i][j] = ((float)rand() * (5) / (float)RAND_MAX - 2);
        }
    }

    /* printMatrix(a, n);
    printf("<---------------->\n");
    printMatrix(b, n);
    printf("<---------------->\n"); 
    */
    
    begin = omp_get_wtime();
    rapl_sysfs_start(rapl);

    matmul_tiled(a, b, c, n, s);

    rapl_sysfs_stop(rapl);
    double end = omp_get_wtime();
    
    time_spent = (end - begin);
    //printMatrix(c, N);
    //printf("<---------------->\n");
    printf("Time exec: %f sec, Matrix size: %d, Tile size: %d  s\n\n", time_spent, n, s);
    return 0;
}
