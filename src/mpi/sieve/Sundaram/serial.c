/**
 * @file sieve-of-Sundaram.c
 * @authors @slottwo, @GersonFeDutra
 * @brief Sieve of Sundaram with message passing multiprocessing
 * @version 0.1
 * @date 2024-03-15
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "../../lib/benchmark.h"
#include "../../lib/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

#define COLS 10 // Number of cols in the output file
#define FILE_PRIMES "primes.txt"

#ifdef DEBUG
#include <unistd.h> // UNIX only
// Upper limit for primes
#define N _GB / 1024
#else
// Upper limit for primes
#define N _GB
#endif

void serial_sundaram(int rank, int n, BenchmarkInfo benchmark, bool dynamic)
{
	/* SERIAL ALGORITHM */
	if (rank == ROOT) {
		/** @brief Initial time stamp **/
		fprintf(stderr, "\033[33m"
						"Loading...\n\033[m");
		bool *non_primes = (bool *)calloc(n, sizeof(bool));
		fprintf(stderr, "\33[2K\033[A\r"); // clear line

		benchmark = benchmark_start("Serial", ROOT, true);

		int k = 2;
		int max_k = k;
		do {
			if (k * k > n)
				break;

			for (int i = k * k; i < n; i += k)
				non_primes[i] = true;

			do
				k++;
			while (non_primes[k]);
		} while (!(k * k > n));

		/** @brief Final time stamp **/
		benchmark_stop(benchmark);
		set_dynamic(dynamic, benchmark);

		free(non_primes);
	}

	/** @brief Wait serial execution at ROOT **/
	MPI_Barrier(MPI_COMM_WORLD);
}
