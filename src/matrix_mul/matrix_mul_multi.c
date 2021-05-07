//padding size = 64 bytes / size of type of data
//ex. int --> 64/4 = 16 bytes of padding

#include <stdio.h>
#include <omp.h>
#include <time.h>

#define nThreads 7
#define N 100000

int main()
{

    int a[N];

    for (int i = 0; i < N; i++)
    {
        a[i] = 1;
    }

    double start = omp_get_wtime();
    int sum = 0;
    int s = 0;
    int result[nThreads * 16];

#pragma omp parallel num_threads(nThreads)
    {
        int t = omp_get_thread_num();
#pragma omp for
        for (int i = 0; i < N; i++)
        {
            result[t * 16] += a[i];
        }
        //printf("Result %d\n", result[t * 16]);
    }

    for (int i = 0; i < nThreads; i++)
    {
        sum += result[i * 16];
    }
    printf("Sum %d\n", sum);
    double end = omp_get_wtime();
    double start2 = omp_get_wtime();

#pragma omp parallel for reduction(+ \
                                   : s)
    for (int i = 0; i < N; i++)
    {
        s = s + a[i];
    }
    printf("s: %d \n", s);

    double end2 = omp_get_wtime();
    printf("Normal Time ex: %lf\n", (end - start));
    printf("Reduction Time ex: %lf\n", (end2 - start2));
}