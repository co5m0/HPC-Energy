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

void printMatrix(float *mat, int len)
{
    for (int i = 0; i < len; i++){
        printf("\t%f", mat[i]);
    }
}

int main(int argc, char **argv)
{
    srand(1);

    int i, j, k, n;
    int nThreads = 1;
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

    double dtime;

    sprintf(file_out_name, "reduce_seq_%d_%d", n, nThreads);

    // --> copy from here
    proc = fork();
    if (proc < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    } else if (proc > 0) {
        dtime = -omp_get_wtime();

        rapl_sysfs_start(rapl);

        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                somma = somma + a[i][j] * b[i][j];
            }
        }

        rapl_sysfs_stop(rapl);
        dtime += omp_get_wtime();

        wait(0);

        //printf("Result sum: %f\n", somma);

        printf("time %f\n", dtime);
        print_file("test_with_power.csv", "DOT-PRODUCT SEQ", dtime, n, rapl_get_energy(rapl), 1);

    } else {  //child
        rapl_power_sysfs(rapl, rapl_power);
        read_power(rapl, rapl_power, 200, 5, file_out_name);

    }
    return 0;
}