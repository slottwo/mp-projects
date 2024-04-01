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

#include "lib/benchmark.h"
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/** @brief 1 GB */
#define _GB 1073741824

#ifdef DEBUG
#define N _GB / 1024
#else
#define N _GB
#endif

#define ROOT 0 // MPI main process

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
		while (!attached) {
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
			if (*(*++argv)++ == '-') {
				switch (**argv) {
					case 'd': dynamic = true; break;
					case 'n': {
						if (--argc)
							n = atol(*++argv);
					} break;
					default: break;
				}
			}
	BenchmarkInfo benchmark = NULL;

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

	/** @brief Initial time stamp  **/
	if (rank == ROOT)
		benchmark_back("Parallel", benchmark);
	else
		benchmark = benchmark_start("Parallel", rank, dynamic);

	/* PARALLEL ALGORITHM */

	// int size = n / wsize + (n % wsize != 0);
	bool *non_primes = (bool *)calloc(n, sizeof(bool));

	/* START */
	int k = 2;
	do {
		for (int i = k * k; i < n; i += k)
			non_primes[i] = true;

		do
			k++;
		while (non_primes[k]);
	} while (!(k * k > n));
	/* END */

	/** @brief Final time stamp **/
	benchmark_stop(benchmark);

	free(non_primes);

	benchmark_save(wsize, benchmark);
	benchmark_show(wsize, true, benchmark);

	MPI_Finalize();
	return 0;
}
