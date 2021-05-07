//#include <cilk/cilk.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
void printMatrix(int mat[][N], int len)
{
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            printf("\t%d", mat[i][j]);
        }
        printf("\n");
    }
}

void matmul_tiled(int a[][N], int b[][N], int c[][N])
{
    int s = 32;
#pragma omp parallel for 
    for (int ih = 0; ih < N; ih += s)
    {
#pragma omp parallel for simd
        for (int jh = 0; jh < N; jh += s)
            for (int kh = 0; kh < N; kh += s)
                for (int il = 0; il < s; il++)
                    for (int kl = 0; kl < s; kl++)
                        for (int jl = 0; jl < s; jl++)
                            c[ih + il][jh + jl] += a[ih + il][kh + kl] * b[kh + kl][jh + jl];
    }
}

int main(int argc, char** argv)
{
    // assert(argc == 2);
    int mnl = N;
    int matA[N][N], matB[N][N], matC[N][N];
    int i, j, k;
    double begin;
    double time_spent;

    for (i = 0; i < mnl; i++) {
        for (j = 0; j < mnl; j++) {
            matA[i][j] = i + j * 7 + 2;
            matB[i][j] = j + i * 9 + 4;
            matC[i][j] = 0;
        }
    }

    /* printMatrix(matA, N);
    printf("<---------------->\n");
    printMatrix(matB, N);
    printf("<---------------->\n"); */
    begin = omp_get_wtime();

/* #pragma omp parallel for
    for (i = 0; i < mnl; i++) {
        for (k = 0; k < mnl; k++)
            for (j = 0; j < mnl; j++) {
                matC[i][j] += matA[i][k] * matB[k][j];
            }
    } */

        matmul_tiled(matA,matB,matC);
    
        

    double end = omp_get_wtime();
    time_spent = (end - begin);
    printf("\nMatrix size %d, time spent %lf s\n\n", N, time_spent);
  
    /* printMatrix(matC, N);
     printf("<---------------->\n"); */
    return 0;
}
