/**
 * @file sieve-of-Eratosthenes.c
 * @authors slottwo, GersonFeDutra
 * @brief Sieve of Eratosthenes with message passing multiprocessing
 * @version 0.1
 * @date 2024-03-15
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lib/benchmark.h"

/** @brief 1 GB */
#define _GB 1073741824
//#define N _GB
#define N _GB / 1024

#define ROOT 0 // MPI main process

int main(int argc, char const *argv[])
{
    /* MPI.h Initialization */
    int wsize, rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;

    BenchmarkInfo benchmark = NULL;

    /* SERIAL ALGORITHM */
    if (rank == ROOT) {
        /** @brief Initial time stamp **/
        fprintf(stderr, "\033[33m"
						"Loading...\n\033[m");
        bool *non_primes = (bool *)calloc(N, sizeof(bool));
		fprintf(stderr, "\33[2K\033[A\r"); // clear line

        benchmark = benchmark_start("Serial", ROOT);

        int k = 2;
        int max_k = k;
        do
        {
            if (k * k > N)
                break;

            for (int i = k * k; i < N; i += k)
                non_primes[i] = true;

            do
                k++;
            while (non_primes[k]);
        }
        while (!(k * k > N));


        /** @brief Final time stamp **/
        benchmark_stop(benchmark);

        free(non_primes);
        benchmark_show(wsize, true, benchmark);
    }

    /** @brief Wait serial execution at ROOT **/
    MPI_Barrier(MPI_COMM_WORLD);

    /** @brief Initial time stamp  **/
    if (rank == ROOT)
        benchmark_back("Parallel", benchmark);
    else
        ;//benchmark = benchmark_start("Parallel", rank);

    MPI_Finalize();
    return 0;
    /* PARALLEL ALGORITHM */

    //int size = N / wsize + (N % wsize != 0);
    bool *non_primes = (bool *)calloc(N, sizeof(bool));

    /* START */
    int k = 2;
    do
    {
        for (int i = k * k; i < N; i += k)
            non_primes[i] = true;

        do
            k++;
        while (non_primes[k]);
    }
    while (!(k * k > N));
    /* END */

    if (rank == ROOT)
        /** @brief Final time stamp **/
        benchmark_stop(benchmark);

    free(non_primes);

    benchmark_show(wsize, true, benchmark);

    MPI_Finalize();
    return 0;
}
