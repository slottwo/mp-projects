#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

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

    if (rank == ROOT)
    {

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

        // int NTHREADS = omp_get_num_procs() / 2;
        int NTHREADS = wsize;

        bool *non_primes = (bool *)calloc(n, sizeof(bool));
        non_primes[0] = true;
        non_primes[1] = true;

        benchmark = benchmark_start_from_file("OMP - Slicing", ROOT, dynamic);

        /* Start */

#pragma omp parallel num_threads(NTHREADS)
        {
            int k = 2 + omp_get_thread_num();
            while (!(k * k > n))
            {
                for (int i = k * k; i < n; i += k)
                    non_primes[i] = true;

                do
                    k++;
                while (non_primes[k]);
            }
        }

        /* End */

        benchmark_stop(benchmark);

        free(non_primes);

        benchmark_save(wsize, benchmark);
        benchmark_show(wsize, true, benchmark);
    }

    MPI_Finalize();
    return 0;
}
