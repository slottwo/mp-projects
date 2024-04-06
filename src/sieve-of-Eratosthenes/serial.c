#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

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

    fprintf(stderr, "\033[36m"
                    "Setup\033[m"
                    "\t[%d]\n",
            N);

    bool *non_primes = (bool *)calloc(N, sizeof(bool));
    non_primes[0] = true;
    non_primes[1] = true;

    fprintf(stderr, "\33[2K\033[A\r"
                    "\033[33m"
                    "Loading\n\033[m");

    benchmark = benchmark_start("MPI - Iterative Messages", 0, true);

    int k = 2;
    while (!(k * k > N))
    {
        for (int i = k * k; i < N; i += k)
            non_primes[i] = true;

        do
            k++;
        while (non_primes[k]);
    }

    fprintf(stderr, "\33[2K\033[A\r"
                    "\033[32m"
                    "Done   \n\033[m");

    benchmark_stop(benchmark);

    benchmark_save(1, benchmark);
    benchmark_show(1, true, benchmark);

    free(non_primes);

    return 0;
}