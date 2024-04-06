/**
 * @file single.c
 * @authors slottwo, GersonFeDutra
 * @brief Sieve of Eratosthenes with message passing multiprocessing
 * @version 2.0
 * @date 2024-04-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <mpi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib/benchmark.h"

/** @brief MPI main process */
#define ROOT 0

/** @brief 1 GB */
#define _GB 1073741824

#ifdef DEBUG
#define N _GB / 1024
#else
#define N _GB
#endif

#ifdef DEBUG
#include "lib/utils.h"
#include <unistd.h> // UNIX only
#endif

int main(int argc, char *argv[])
{

#ifdef DEBUG // Compile with `-D DEBUG`
	// Waits debugger attachment
	{
		int attached = 0;
		while (!attached)
		{
			// Define a label using GDB-specific assembly command
			__asm__("gdb_breakpoint:");
			sleep(3);
		}
	}
#endif

	/* MPI.h Initialization */
	int wsize, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &wsize);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	double _sync = MPI_Wtime();
	MPI_Status status;

	/* Parse arguments */
	bool dynamic = false;
	size_t n = N;
	if (argc > 1)
		while (--argc)
			if (*(*++argv)++ == '-')
				switch (**argv)
				{
				case 'd':
					dynamic = true;
					break;
				case 'n':
					if (--argc)
						n = atol(*++argv);
					break;
				default:
					break;
				}
	BenchmarkInfo benchmark = NULL;

	/* Start */

	bool *NON_PRIMES;
	if (rank == ROOT)
		NON_PRIMES = (bool *)malloc(N * 1UL);

	bool *non_primes = (bool *)calloc(N, 1UL);
	non_primes[0] = true;
	non_primes[1] = true;

	benchmark = benchmark_start("MPI - Single Message", ROOT, dynamic);

	int i, k = 2 + rank;
	while (!(k * k > N))
	{

		for (i = k * k; i < N; i += k)
			non_primes[i] = true;

		do
			k++;
		while (non_primes[k]);
	}

	MPI_Reduce(non_primes, NON_PRIMES, N, MPI_C_BOOL, MPI_LOR, ROOT, MPI_COMM_WORLD);

	/* End */

	benchmark_stop(benchmark);

	free(NON_PRIMES);
	free(non_primes);

	benchmark_save(wsize, benchmark);
	benchmark_show(wsize, true, benchmark);

	MPI_Finalize();
	return 0;
}
