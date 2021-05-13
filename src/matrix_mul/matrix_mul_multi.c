#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>
#define N 1024
#define NTHREADS 4

int main(int argc, char **argv)
{
    srand(1);
    int i, j, k, n, x;
    int nThreads;

    if (argc > 1)
    {
        n = atoi(argv[1]);
        nThreads = atoi(argv[2]);
    }
    else
    {
        n = N;
        nThreads = NTHREADS;
    }

    float **a, **b, **c;

    a = (float **)malloc(n * sizeof(float *));
    b = (float **)malloc(n * sizeof(float *));
    c = (float **)malloc(n * sizeof(float *));

    for (x = 0; x < n; x++)
    {
        a[x] = malloc(n * sizeof(float));
        b[x] = malloc(n * sizeof(float));
        c[x] = malloc(n * sizeof(float));
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

    //calculate prod
    double begin = omp_get_wtime();
#pragma omp parallel private(i, j, k) num_threads(nThreads)
    {
#pragma omp for
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                c[i][j] = 0;
                for (k = 0; k < n; k++)
                {
                    c[i][j] = c[i][j] + a[i][k] * b[k][j];
                }
            }
        }
    }
    double end = omp_get_wtime();
    /*
 printf("MATRIX - A\n");
 for (i=0;i<n;i++) {
    for(j=0;j<n;j++){ 
        printf("%f ", a[i][j]);                
    }
        printf("\n");
 }
printf("\n"); 


  printf("MATRIX - B\n");
 for (i=0;i<n;i++) {
    for(j=0;j<n;j++) {
        printf("%f ", b[i][j]);                
    }
     printf("\n");
 } 

 printf("\n");
printf("MATRIX - C (RESULT) \n");
for (i=0;i<n;i++) {
    for(j=0;j<n;j++) {
        printf("%f ", c[i][j]);                
    }    
        printf("\n");
} 
*/

    double time_spent = (end - begin);
    printf("Time exec: %f sec, Matrix size: %d, Number Threads: %d\n", time_spent, n, nThreads);
}