/**
 * @file sieve-of-Eratosthenes.c
 * @authors slottwo, GersonFeDutra
 * @brief Sieve of Eratosthenes with shared memory multiprocessing
 * @version 0.1
 * @date 2024-03-15
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/** @brief 1 GB */
#define N 1073741824

int main(int argc, char const *argv[])
{
    int NTHREADS = omp_get_num_procs() / 2;

    /** @brief Initial time stamp */
    double t;

    bool *primes;
    bool *non_primes;

    /* SERIAL ALGORITHM */

    t = omp_get_wtime();

    non_primes = (bool *)calloc(N, sizeof(bool));

    int k = 2;
    while (true)
    {
        for (int i = k * k; i < N; i += k)
            non_primes[i] = true;

        do
            k++;
        while (non_primes[k]);

        if (k * k > N)
            break;
    }

    printf("Δt: %f\n", omp_get_wtime() - t);

    /* PARALLEL ALGORITHM */

    t = omp_get_wtime();

#pragma omp parallel num_threads(NTHREADS)
    {
    }

    printf("Δt: %f\n", omp_get_wtime() - t);

    // End

    return 0;
}
