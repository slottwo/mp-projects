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
#include "lib/benchmark.h"
#include "lib/utils.h"
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

/* Parallel */
void sieve_of_sundaram(
        size_t size, bool* primes, int rank,
        bool dynamic, BenchmarkInfo *benchmark)
{
    size_t n = (size - 1) / 2;
    size_t max_i = (n - 1) / 2;
    bool* marked = (bool*)malloc((max_i + 1) * sizeof(bool));

    *benchmark = benchmark_start("Sundaram MPI cores", rank, dynamic);

    // Initialize marked array
    for (size_t i = 0; i <= max_i; i++)
        marked[i] = false;

    // Sieve of Sundaram
    for (size_t i = 1; i <= max_i; i++)
        for (size_t j = i; (i + j + 2 * i * j) <= max_i; j++)
            marked[i + j + 2 * i * j] = true;

    // Process sieve results
    for (size_t i = 1; i <= max_i; i++) {
        if (!marked[i]) {
            size_t p = 2 * i + 1;
            size_t k = (p * p - 1) / 2;
            for (size_t j = k; j <= n; j += p)
                primes[j] = false;
        }
    }

    benchmark_stop(*benchmark);

    free(marked);
}


int main(int argc, char** argv)
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
    int rank, wsize;
    bool* primes;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

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
    BenchmarkInfo benchmark;

    if (n <= 0) {
        fprintf(stderr, "\33[31m" "Error! Invalid size" "\n\33[m");
        MPI_Abort(MPI_COMM_WORLD, EXIT_ERR_ARG);
    }

    // FIXME -> For some reason can only input for this number
    if (n > 1137)
        n = 1137;
        //MPI_Abort(MPI_COMM_WORLD, 1137);

	/** @start PARALLEL ALGORITHM */
    size_t local_N = 100 / wsize;  // Divide the range among ranks
    primes = (bool*)emalloc(local_N * sizeof(bool));

    sieve_of_sundaram(local_N * (rank + 1), primes, rank, dynamic, &benchmark);  // Perform sieve locally
	/** @end PARALLEL ALGORITHM */

    // Gather results from all ranks
    bool* all_primes = (bool*)emalloc(n * sizeof(bool));

    // TODO -> Use benchmark group to measure time of AllGather
    MPI_Allgather(primes, local_N, MPI_C_BOOL, all_primes, local_N, MPI_C_BOOL, MPI_COMM_WORLD);

    if (rank == 0) {
        // Print prime numbers
        printf("Prime numbers up to %d:\n", n);
        for (long long i = 0; i < n; i++) {
            if (all_primes[i])
                printf("%lld ", 2 * i + 1);
        }
        printf("\n");
    }

    if (rank == ROOT) {
        FILE *fp = efopen(FILE_PRIMES, "w");

        if (fp != NULL)
            fprintf(stderr, "\33[32m" "File " FILE_PRIMES " created successfully!\n" "\33[m");

        // Print prime numbers
        fprintf(fp, "Prime numbers up to %lld:\n", n);

        size_t c = -1;
        for (size_t i = 0; i < n; i++) {
            if (all_primes[i]) {
                fprintf(fp, "%-7lld ", 2 * i + 1);
                c++;
            }
            if (c % COLS == COLS - 1) {
                fprintf(fp, "\n");
                c = -1;
            }
        }
        fprintf(fp, "\n");
        if (fclose(fp))
            fprintf(stderr, "\33[31m""File operation not completed\n""\33[m");
    }

    free(primes);
    free(all_primes);


    benchmark_save(wsize, benchmark);
    benchmark_show(wsize, true, benchmark);

    MPI_Finalize();
    return 0;
}
