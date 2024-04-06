#include <omp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/** @brief 1 GB */
#define _GB 1073741824

#define N 1000000

int main(int argc, char const *argv[])
{
    int NTHREADS = omp_get_num_procs() / 2;

    /** @brief Initial time stamp */
    double t;

    /* Start */

    t = omp_get_wtime();

    bool *non_primes = (bool *)calloc(N, sizeof(bool));

    non_primes[0] = true;
    non_primes[1] = true;

#pragma omp parallel num_threads(NTHREADS)
    {
        int k = 2 + omp_get_thread_num();
        while (!(k * k > N))
        {
            for (int i = k * k; i < N; i += k)
                non_primes[i] = true;

            do
            {
                k++;
            } while (non_primes[k]);
        }
    }

    printf("Δt: %f\n", omp_get_wtime() - t);

    /* End */

    return 0;
}
