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
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../../lib/print.h"
#include "../../lib/rapllib.h"

#define N 1024
#define NTHREADS 4

int main(int argc, char **argv) {
    srand(1);
    int i, j, k, n, x;
    int nThreads;
    // --> declare proc var
    int proc;
    // --> declare file output file var
    char file_out_name[25];

    Rapl_info rapl = new_rapl_info();
    // --> declare rapl_power var
    Rapl_power_info rapl_power = new_rapl_power_info();
    detect_cpu(rapl);
    detect_packages(rapl);

    if (argc > 1) {
        n = atoi(argv[1]);
        nThreads = atoi(argv[2]);
    } else {
        n = N;
        nThreads = NTHREADS;
    }

    // --> change "multi" with file type
    sprintf(file_out_name, "multi_%d_%d", n, nThreads);

    float **a, **b, **c;
    
#pragma omp parallel num_threads(1)
    {
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
    }

    // --> copy from here
    proc = fork();
    if (proc < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    } else if (proc > 0) {  //parent
                            // --> to here before, paste before rapl_sysfs and omp_get_wtime
        rapl_sysfs(rapl);

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
        // --> wait for child
        wait(0);
        printf("Time exec: %f sec, Matrix size: %d, Number Threads: %d\n", time_spent, n, nThreads);
        print_file("test_mult.csv", __FILE__, time_spent, n, rapl_get_energy(rapl), nThreads);

        // --> copy from here
    } else {  //child
        rapl_power_sysfs(rapl, rapl_power);
        read_power(rapl, rapl_power, 500, 10, file_out_name);
    }
    // --> to here
    return 0;
}
