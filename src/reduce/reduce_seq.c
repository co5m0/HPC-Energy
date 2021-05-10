#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define N 10

int main(int argc, char **argv)
{
    clock_t begin = clock();
    int i, j, k, n, somma = 0;

    if (argc > 1)
    {
        n = atoi(argv[1]);
    }
    else
    {
        n = N;
    }

    int **a, **b, **c;

    a = (int **)malloc(n * sizeof(int *));
    b = (int **)malloc(n * sizeof(int *));

    for (int x = 0; x < n; x++)
    {
        a[x] = malloc(n * sizeof(int));
        b[x] = malloc(n * sizeof(int));
    }

    //initialization
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            a[i][j] = (int)rand() % 5 - 2;
            b[i][j] = (int)rand() % 5 - 1;
        }
    }

    //calculate prod
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            somma = somma + a[i][j] * b[i][j];
        }
    }
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Result sum: %d\n", somma);
    printf("Time exec: %f sec\n", time_spent);
}