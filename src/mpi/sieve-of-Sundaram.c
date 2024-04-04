#include "lib/benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

#define ROOT 0 // MPI main process
#define COLS 20 // Number of cols in the output file

/** @brief 1 GB */
#define _GB 1073741824

#ifdef DEBUG
#define N _GB / 1024
#else
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


/* Parallel */
void sundaram(int _n, bool* primes) {
    int n = (_n - 1) / 2;
    int max_i = (n - 1) / 2;
    bool* marked = (bool*)malloc((max_i + 1) * sizeof(bool));

    // Initialize marked array
    for (int i = 0; i <= max_i; i++)
        marked[i] = false;

    // Sieve of Sundaram
    for (int i = 1; i <= max_i; i++)
        for (int j = i; (i + j + 2 * i * j) <= max_i; j++)
            marked[i + j + 2 * i * j] = true;

    // Process sieve results
    for (int i = 1; i <= max_i; i++) {
        if (!marked[i]) {
            int p = 2 * i + 1;
            int k = (p * p - 1) / 2;
            for (int j = k; j <= n; j += p)
                primes[j] = false;
        }
    }

    free(marked);
}


int main(int argc, char** argv) {
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
	BenchmarkInfo benchmark = NULL;

    serial_sundaram(rank, n, benchmark, dynamic);

	/** @brief Initial time stamp  **/
	if (rank == ROOT)
		benchmark_back("Parallel", benchmark);
	else
		benchmark = benchmark_start("Parallel", rank, dynamic);

	/* PARALLEL ALGORITHM */

    int local_N = n / wsize;  // Divide the range among ranks
    primes = (bool*)malloc(local_N * sizeof(bool));

    sundaram(local_N * (rank + 1), primes);  // Perform sieve locally

    // Gather results from all ranks
    bool* all_primes = (bool*)malloc(n * sizeof(bool));
    MPI_Allgather(primes, local_N, MPI_C_BOOL, all_primes, local_N, MPI_C_BOOL, MPI_COMM_WORLD);

    if (rank == ROOT) {
        // Print prime numbers
        printf("Prime numbers up to %d:\n", n);
        for (int i = 0; i < n; i++) {
            if (all_primes[i])
                printf("%d ", 2 * i + 1);
        }
        printf("\n");
    }

	/** @brief Final time stamp **/
	benchmark_stop(benchmark);

    // ROOT rank writes primes to file
    if (rank == ROOT) {
        // Create txt with the primes
        FILE *fp = fopen("primes.txt", "w");
        if (fp == NULL) {
            fprintf(stderr, "\33[31m""Error opening file.\33[m\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        size_t c = 0;
        for (int i = 0; i < n; i++) {
            if (all_primes[i]) {
                fprintf(fp, "%d ", 2 * i + 1);
                c++;
            }
            if (c != 0 && c % COLS == 0) {
                fprintf(fp, "\n");
            }
        }

        fclose(fp);

        fprintf(stderr, "\33[32m""primes.txt file created successfully.\33[m\n");
    }

    free(primes);
    free(all_primes);

	benchmark_save(wsize, benchmark);
	benchmark_show(wsize, true, benchmark);

    MPI_Finalize();

    return 0;
}
