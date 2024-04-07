#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#include "../lib/benchmark.h"

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
    // int wsize, rank;
    MPI_Init(&argc, &argv);
    // MPI_Comm_size(MPI_COMM_WORLD, &wsize);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    double _sync = MPI_Wtime();
    // MPI_Status status;

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

    FILE *tmp;

    // fprintf(stderr, "\033[36m"
    //                 "Setup\033[m"
    //                 "\t[%d]\n",
    //         n);

    bool *non_primes = (bool *)calloc(N, sizeof(bool));
    non_primes[0] = true;
    non_primes[1] = true;

    // fprintf(stderr, "\33[2K\033[A\r"
    //                 "\033[33m"
    //                 "Loading\n\033[m");

    tmp = fopen(".tmp", "w");
    fprintf(tmp, "%f\n", MPI_Wtime());
    // fclose(tmp);

    int k = 2;
    while (!(k * k > n))
    {
        for (int i = k * k; i < n; i += k)
            non_primes[i] = true;

        do
            k++;
        while (non_primes[k]);
    }

    // tmp = fopen(".tmp", "a+");
    fprintf(tmp, "%f", MPI_Wtime());
    fclose(tmp);

    // fprintf(stderr, "\33[2K\033[A\r"
    //                 "\033[32m"
    //                 "Done   \n\033[m");

    free(non_primes);

    return 0;
}
