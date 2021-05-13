#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "rapllib.h"

#define N 1024

int main(int argc, char **argv)
{
    srand(1);
    int i, j, k, n, x;

    Rapl_info rapl = new_rapl_info();
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
    clock_t begin = clock();
    rapl_sysfs_start(rapl);

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

    rapl_sysfs_stop(rapl);
    clock_t end = clock();
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

    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Time exec: %f sec, Matrix size: %d\n", time_spent, n);
}