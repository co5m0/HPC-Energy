#include <omp.h>
#include <stdio.h>
#include <string.h>
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
    if (argc > 1) {
        n = atoi(argv[1]);
        nThreads = atoi(argv[2]);
    } else {
        n = N;
        nThreads = NTHREADS;
    }
    float *a = _mm_malloc(n * n * sizeof *a, 64);
    float *b = _mm_malloc(n * n * sizeof *b, 64);
    float *c1 = _mm_malloc(n * n * sizeof *c1, 64);
    float *c2 = _mm_malloc(n * n * sizeof *c2, 64);
    float *c3 = _mm_malloc(n * n * sizeof *c2, 64);
    Rapl_info rapl = new_rapl_info();
    detect_cpu(rapl);
    detect_packages(rapl);
    rapl_sysfs(rapl);
    for (int i = 0; i < n * n; i++) a[i] = 1.0 * i;
    for (int i = 0; i < n * n; i++) b[i] = 1.0 * i;
    memset(c1, 0, n * n * sizeof *c1);
    memset(c2, 0, n * n * sizeof *c2);
    memset(c3, 0, n * n * sizeof *c3);
    double dtime;

    dtime = -omp_get_wtime();
    rapl_sysfs_start(rapl);
    gemm(a, b, c1, n);
    rapl_sysfs_stop(rapl);
    dtime += omp_get_wtime();
    printf("time %f\n", dtime);
    print_file("SEQ", dtime, n, rapl_get_energy(rapl), nThreads);

    dtime = -omp_get_wtime();
    rapl_sysfs_start(rapl);
    gemm_tlp(a, b, c2, n);
    rapl_sysfs_stop(rapl);
    dtime += omp_get_wtime();
    printf("time %f\n", dtime);
    print_file("MULTI", dtime, n, rapl_get_energy(rapl), nThreads);

    dtime = -omp_get_wtime();
    rapl_sysfs_start(rapl);
    gemm_tlp_simd(a, b, c3, n);
    rapl_sysfs_stop(rapl);
    dtime += omp_get_wtime();
    printf("time %f\n", dtime);
    print_file("SIMD", dtime, n, rapl_get_energy(rapl), nThreads);

    printf("error %d\n", memcmp(c1, c2, n * n * sizeof *c1));
    printf("error %d\n", memcmp(c1, c3, n * n * sizeof *c1));
}