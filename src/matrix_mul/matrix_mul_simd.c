//#include <cilk/cilk.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define S 32
void printMatrix(double **mat, int len)
{
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < len; j++) {
            printf("\t%f", mat[i][j]);
        }
        printf("\n");
    }
}

void matmul_tiled(double **a, double **b, double **c, int n, int s)
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

int main(int argc, char** argv)
{
    
    int n,s;
    double **a, **b, **c;
    int i, j, k,x;
    double begin;
    double time_spent;

  if(argc>1){
        n = atoi(argv[1]);
        s = atoi(argv[2]);
            }else{
        n=N;
        s=S;
    }

a = (double**)malloc(n*sizeof(double*));
b = (double**)malloc(n*sizeof(double*));
c = (double**)malloc(n*sizeof(double*));

for( x=0;x<n;x++){
    a[x]=malloc(n*sizeof(double));
    b[x]=malloc(n*sizeof(double));
    c[x]=malloc(n*sizeof(double));
}

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            a[i][j] =  (double)rand()/5-2.0; //min -2 and max 2
            b[i][j] = (double)rand()/5-1.0; //min -1 and max 3
        }
    }

    /* printMatrix(a, n);
    printf("<---------------->\n");
    printMatrix(b, n);
    printf("<---------------->\n"); 
    */
    begin = omp_get_wtime();
    matmul_tiled(a,b,c,n,s);
    double end = omp_get_wtime();
    time_spent = (end - begin);
    //printMatrix(c, N);
    //printf("<---------------->\n"); 
    printf("\nMatrix size %d, tile size %d time spent %lf s\n\n", n,s, time_spent);
    return 0;
}
