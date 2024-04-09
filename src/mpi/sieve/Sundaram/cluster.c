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

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int segment_size = N / size;
    int start = rank * segment_size + 1;
    int end = (rank + 1) * segment_size;

    // Adjust end for the last process to include the remaining numbers
    if (rank == size - 1) {
        end = N;
    }

    bool* primes = (bool*)malloc((end - start + 1) * sizeof(bool));
    for (int i = 0; i <= end - start; i++) {
        primes[i] = false;
    }

    for (int i = 1; i <= end; i++) {
        for (int j = i; (i + j + 2 * i * j) <= end; j++) {
            primes[i + j + 2 * i * j - start] = true;
        }
    }

    // Gather the results from all processes
    bool* all_primes = NULL;
    if (rank == ROOT) {
        all_primes = (bool*)malloc(N * sizeof(bool));
    }

    MPI_Gather(primes, end - start + 1, MPI_C_BOOL, all_primes, end - start + 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

    if (rank == ROOT) {
        // Print the primes
        printf("Primes up to %d:\n", N);
        for (int i = 1; i <= N; i++) {
            if (!all_primes[i - 1]) {
                printf("%d ", i);
            }
        }
        printf("\n");
        free(all_primes);
    }

    free(primes);
    MPI_Finalize();
    return 0;
}
