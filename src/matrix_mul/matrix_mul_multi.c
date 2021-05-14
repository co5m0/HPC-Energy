/**
 * 
 * gcc -O2 -Wall  -c rapllib.c
 * 
 * gcc -o main.out ../../lib/rapllib.o  matrix_mul_multi.c -lm -fopenmp
 * 
 * sudo ./main.out
 * 
 **/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../../lib/print.h"
#include "../../lib/rapllib.h"

#define N 1024
#define NTHREADS 4

int main(int argc, char **argv) {
    srand(1);
    int i, j, k, n, x;
    int nThreads;

    Rapl_info rapl = new_rapl_info();
    detect_cpu(rapl);
    detect_packages(rapl);
    rapl_sysfs(rapl);

    if (argc > 1) {
        n = atoi(argv[1]);
        nThreads = atoi(argv[2]);
    } else {
        n = N;
        nThreads = NTHREADS;
    }

    float **a, **b, **c;

    a = (float **)malloc(n * sizeof(float *));
    b = (float **)malloc(n * sizeof(float *));
    c = (float **)malloc(n * sizeof(float *));

    for (x = 0; x < n; x++) {
        a[x] = malloc(n * sizeof(float));
        b[x] = malloc(n * sizeof(float));
        c[x] = malloc(n * sizeof(float));
    }

    //initialization
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            a[i][j] = ((float)rand() * (5) / (float)RAND_MAX - 2);
            b[i][j] = ((float)rand() * (5) / (float)RAND_MAX - 2);
        }
    }

    //calculate prod
    double begin = omp_get_wtime();
    rapl_sysfs_start(rapl);

#pragma omp parallel private(i, j, k) num_threads(nThreads)
    {
#pragma omp for
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                c[i][j] = 0;
                for (k = 0; k < n; k++) {
                    c[i][j] = c[i][j] + a[i][k] * b[k][j];
                }
            }
        }
    }

    rapl_sysfs_stop(rapl);
    double end = omp_get_wtime();
    double time_spent = (end - begin);
    printf("Time exec: %f sec, Matrix size: %d, Number Threads: %d\n", time_spent, n, nThreads);
    print_file(__FILE__, time_spent, n, rapl_get_energy(rapl), nThreads);
}