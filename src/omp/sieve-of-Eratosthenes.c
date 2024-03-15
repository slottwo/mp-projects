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
    // bool *primes;

    // int size = N / wsize + (N % wsize != 0);
    // bool *non_primes = (bool *)calloc(size, sizeof(bool));

    // /** @brief Initial time stamp */
    // double t_init;
    // if (rank == 0)
    //     t_init = MPI_Wtime();

    // /* SERIAL ALGORITHM
    // int k = 2;
    // while (true)
    // {
    //     for (int i = k * k; i < N; i += k)
    //         non_primes[i] = true;

    //     do
    //         k++;
    //     while (non_primes[k]);

    //     if (k * k > N)
    //         break;
    // }
    //  */
    
    // /* INICIO */

    // /* FIM */

    // if (rank == 0)
    //     printf("Δt: %f\n", MPI_Wtime() - t_init);

    // free(primes);

    return 0;
}
