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
int nThreads;

void gemm(float *restrict a, float *restrict b, float *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            for (int j = 0; j < n; j++) {
                c[i * n + j] += a[i * n + k] * b[k * n + j];
            }
        }
    }
}

void gemm_tlp(float *restrict a, float *restrict b, float *restrict c, int n) {
#pragma omp parallel for num_threads(nThreads)
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            for (int j = 0; j < n; j++) {
                c[i * n + j] += a[i * n + k] * b[k * n + j];
            }
        }
    }
}

void gemm_tlp_simd(float *restrict a, float *restrict b, float *restrict c, int n) {
#pragma omp parallel for num_threads(nThreads)
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            __m128 a4 = _mm_set1_ps(a[i * n + k]);
            for (int j = 0; j < n; j += 4) {
                __m128 c4 = _mm_load_ps(&c[i * n + j]);
                __m128 b4 = _mm_load_ps(&b[k * n + j]);
                c4 = _mm_add_ps(_mm_mul_ps(a4, b4), c4);
                _mm_store_ps(&c[i * n + j], c4);
            }
        }
    }
}

int main(int argc, char **argv) {
    // int n = 2048;
    int n;
    // --> declare proc var
    int proc;
    // --> declare file output file var
    char file_out_name[25];

    if (argc > 1) {
        n = atoi(argv[1]);
        nThreads = atoi(argv[2]);
    } else {
        n = N;
        nThreads = NTHREADS;
    }

    float *a = _mm_malloc(n * n * sizeof *a, 64);
    float *b = _mm_malloc(n * n * sizeof *b, 64);
    //float *c1 = _mm_malloc(n * n * sizeof *c1, 64);
    //float *c2 = _mm_malloc(n * n * sizeof *c2, 64);
    float *c3 = _mm_malloc(n * n * sizeof *c3, 64);

    Rapl_info rapl = new_rapl_info();
    Rapl_power_info rapl_power = new_rapl_power_info();
    detect_cpu(rapl);
    detect_packages(rapl);
    rapl_sysfs(rapl);

#pragma omp parallel for num_threads(1)
    for (int i = 0; i < n * n; i++) a[i] = 1.0 * i;
#pragma omp parallel for num_threads(1)
    for (int i = 0; i < n * n; i++) b[i] = 1.0 * i;
    //memset(c1, 0, n * n * sizeof *c1);
    //memset(c2, 0, n * n * sizeof *c2);
#pragma omp parallel num_threads(1)
    {
    memset(c3, 0, n * n * sizeof *c3);
    }
    double dtime;

    

    sprintf(file_out_name, "simd_%d_%d", n, nThreads);

    proc = fork();
    if (proc < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    } else if (proc > 0) {
        dtime = -omp_get_wtime();
        rapl_sysfs_start(rapl);
        gemm_tlp_simd(a, b, c3, n);
        rapl_sysfs_stop(rapl);
        dtime += omp_get_wtime();

        wait(0);

        printf("time %f\n", dtime);
        print_file("test_with_power.csv", "SIMD", dtime, n, rapl_get_energy(rapl), nThreads);
    } else {  //child
        rapl_power_sysfs(rapl, rapl_power);
        read_power(rapl, rapl_power, 200, 7, file_out_name);
    }

    return 0;
}
