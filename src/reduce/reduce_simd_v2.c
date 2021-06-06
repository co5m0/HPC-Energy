#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <x86intrin.h>

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

    int i, j, k, n, nThreads;
    float somma = 0;

    // --> declare proc var
    int proc;
    // --> declare file output file var
    char file_out_name[25];
    
    Rapl_info rapl = new_rapl_info();
    Rapl_power_info rapl_power = new_rapl_power_info();
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

#pragma omp parallel for num_threads(1)
    for (int x = 0; x < n; x++)
    {
        a[x] = malloc(n * sizeof(float));
        b[x] = malloc(n * sizeof(float));
    }

#pragma omp parallel for num_threads(1)
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
    double dtime;

    sprintf(file_out_name, "reduce_simd_%d_%d", n, nThreads);

    proc = fork();
    if (proc < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    } else if (proc > 0) {
        dtime = -omp_get_wtime();

        rapl_sysfs_start(rapl);

#pragma omp parallel private(i, j)
    {
#pragma omp for simd reduction(+ \
                               : somma)
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                somma = somma + a[i][j] * b[i][j];
            }
        }
    }

        rapl_sysfs_stop(rapl);
        dtime += omp_get_wtime();

        wait(0);

        //printf("Result sum: %f\n", somma);

        printf("time %f\n", dtime);
        print_file("test_with_power.csv", "DOT-PRODUCT SIMD", dtime, n, rapl_get_energy(rapl), nThreads);
        
    } else {  //child
        rapl_power_sysfs(rapl, rapl_power);
        read_power(rapl, rapl_power, 200, 5, file_out_name);
    }
    return 0;
}