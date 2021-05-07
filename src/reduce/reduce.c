#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>

#define N 10000

double f(double x)
{
    return (4.0 / (1.0 + x * x));
}

double CalcPi(int n, int nThreads)
{
    const double fH = 1.0 / (double)n;
    double fSum = 0.0;
    double fX;
    int i;

#pragma omp parallel for num_threads(nThreads) private(fX, i) reduction(+ \
                                                                        : fSum)
    for (i = 0; i < n; i++)
    {
        fX = fH * ((double)i + 0.5);
        fSum += f(fX);
    }
    return fH * fSum;
}

int main(int argc, char **argv)
{
    int nThreads;
    double result;
    double start = omp_get_wtime();

    if (argc < 2)
    {
        nThreads = 1;
    }
    else
    {
        nThreads = atoi(argv[1]);
    }

    result = CalcPi(N, nThreads);
    double end = omp_get_wtime();

    printf("Result %lf \n", result);
    printf("Time ex of %d thread(s): %lf msec\n", nThreads, (end - start) * 1000);
}