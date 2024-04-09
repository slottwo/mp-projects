/**
 * @file iterative.c
 * @authors slottwo, GersonFeDutra
 * @brief Sieve of Eratosthenes with message passing multiprocessing
 * @version 1.0
 * @date 2024-04-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "../../lib/benchmark.h"
#include "../../lib/utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

/** @brief 1 GB */
#define _GB 1073741824

/** @brief Max numbers on output file */
#define _L ((N > 10000) ? N : 10000)

int main(int argc, char *argv[])
{
    /* MPI.h Initialization */
    int wsize, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;
    BenchmarkInfo benchmark = NULL;

    size_t N = _GB/2048;

    /* Parse arguments */
    if (argc > 1)
        while (--argc)
            if (*(*++argv)++ == '-' && **argv == 'N')
                if (--argc)
                    N = atol(*++argv);

    FILE *_fp = efopen(".tmp", "r");
    double serial_start, serial_end;
    fscanf(_fp, "%f %f\n", &serial_start, &serial_end);
    fclose(_fp);

    bool *NON_PRIMES = (bool *)calloc(N, sizeof(bool));
    bool *non_primes = (bool *)calloc(N, sizeof(bool));
    non_primes[0] = true;
    non_primes[1] = true;

    benchmark = benchmark_start_from("MPI Iterative Messages", rank, false, serial_start, serial_end);

    /* Start */

    for (int i = 4; i < N; i += 2)
        non_primes[i] = true;

    MPI_Barrier(MPI_COMM_WORLD);

    int i, k = 3 + rank;
    while (!(k * k > N))
    {

        for (i = k * k; i < N; i += k)
            non_primes[i] = true;

        MPI_Allreduce(non_primes, NON_PRIMES, N, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);

        do
            k++;
        while (NON_PRIMES[k]);
    }

    /* End */
    benchmark_stop(benchmark);

    free(non_primes);
    free(NON_PRIMES);

    benchmark_save_to(wsize, benchmark, "iterative-trace.json");
    benchmark_show(wsize, true, benchmark);

    MPI_Finalize();

    return 0;
}
