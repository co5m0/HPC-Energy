#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 256
#define THRESHOLD 150

void matmul_tiled(int a[][N], int b[][N], int c[][N])
{
    int s = 16;
#pragma omp parallel for
    for (int ih = 0; ih < N; ih += s)
    {
#pragma omp parallel for
        for (int jh = 0; jh < N; jh += s)
            for (int kh = 0; kh < N; kh += s)
                for (int il = 0; il < s; il++)
                    for (int kl = 0; kl < s; kl++)
                        for (int jl = 0; jl < s; jl++)
                            c[ih + il][jh + jl] += a[ih + il][kh + kl] * b[kh + kl][jh + jl];
    }
}
void matmultrec(int mf, int ml, int nf, int nl, int pf, int pl,
                double **A, double **B, double **C)

// Dimensions: A[mf..ml][pf..pl] B[pf..pl][nf..nl] C[mf..ml][nf..nl]

{
    if ((ml - mf) * (nl - nf) * (pl - pf) < THRESHOLD)
        matmult(mf, ml, nf, nl, pf, pl, A, B, C);
    else
    {
#pragma omp task firstprivate(mf, ml, nf, nl, pf, pl)
        {
            matmultrec(mf, mf + (ml - mf) / 2, nf, nf + (nl - nf) / 2, pf, pf + (pl - pf) / 2, A, B, C); // C11 += A11*B11
            matmultrec(mf, mf + (ml - mf) / 2, nf, nf + (nl - nf) / 2, pf + (pl - pf) / 2, pl, A, B, C); // C11 += A12*B21
        }
#pragma omp task firstprivate(mf, ml, nf, nl, pf, pl)
        {
            matmultrec(mf, mf + (ml - mf) / 2, nf + (nl - nf) / 2, nl, pf, pf + (pl - pf) / 2, A, B, C); // C12 += A11*B12
            matmultrec(mf, mf + (ml - mf) / 2, nf + (nl - nf) / 2, nl, pf + (pl - pf) / 2, pl, A, B, C); // C12 += A12*B22
        }
#pragma omp task firstprivate(mf, ml, nf, nl, pf, pl)
        {
            matmultrec(mf + (ml - mf) / 2, ml, nf, nf + (nl - nf) / 2, pf, pf + (pl - pf) / 2, A, B, C); // C21 += A21*B11
            matmultrec(mf + (ml - mf) / 2, ml, nf, nf + (nl - nf) / 2, pf + (pl - pf) / 2, pl, A, B, C); // C21 += A22*B21
        }
#pragma omp task firstprivate(mf, ml, nf, nl, pf, pl)
        {
            matmultrec(mf + (ml - mf) / 2, ml, nf + (nl - nf) / 2, nl, pf, pf + (pl - pf) / 2, A, B, C); // C22 += A21*B12
            matmultrec(mf + (ml - mf) / 2, ml, nf + (nl - nf) / 2, nl, pf + (pl - pf) / 2, pl, A, B, C); // C22 += A22*B22
        }
#pragma omp taskwait
    }
}

void matmul_deq(int a[][N], int b[][N], int c[][N])
{

    if (N < 30)
    {
        return matmul_tiled(a, b, c);
    }

    static int i = 0, j = 0, k = 0;

    if (i >= N)
    {
        return;
    }
    else if (i < N)
    {
        if (j < N)
        {
            if (k < N)
            {
                c[i][j] += a[i][k] * b[k][j];
                k++;
                matmul_deq(a, b, c);
            }
            k = 0;
            j++;
            matmul_deq(a, b, c);
        }
        j = 0;
        i++;
        matmul_deq(a, b, c);
    }
}

int main()
{
    int a[N][N], b[N][N], c[N][N] = {0};
    int i, j, k;
    // clock_t start, end;
    double time_spent;
    // init matrix A e B
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            a[i][j] = (i + j * 7 + 2) % 256;
            b[i][j] = (j + i * 9 + 4) % 256;
        }
    }
    double start = omp_get_wtime();
    // start = clock();
    //matmul_tiled(a, b, c);

#pragma omp parallel
    {
#pragma omp single
        {
            matmul_deq(a, b, c);
        }
    }

    // end = clock();
    // time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    double end = omp_get_wtime();
    // printf("\ntime: %lfms\n", (time_spent)*1000);
    printf("\ntime: %lfms\n", (end - start) * 1000);

    // matmul_deq(a, b, c);
}
