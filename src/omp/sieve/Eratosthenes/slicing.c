#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

/** @brief 1 GB */
#define _GB 1073741824

/** @brief Max numbers on output file */
#define _L ((N > 10000) ? N : 10000)

int main(int argc, char *argv[])
{
    size_t N = _GB;
    int NTHREADS = omp_get_num_procs() / 2;

    /* Parse arguments */
    if (argc > 1)
        while (--argc)
            if (*(*++argv)++ == '-')
                switch (**argv)
                {
                case 'N':
                    if (--argc)
                        N = atol(*++argv);
                    break;
                case 'p':
                    if (--argc)
                        NTHREADS = atol(*++argv);
                    break;
                default:
                    break;
                }

    bool *non_primes = (bool *)calloc(N, sizeof(bool));
    non_primes[0] = true;
    non_primes[1] = true;

    clock_t clk = clock();

    /* Start */

#pragma omp parallel num_threads(NTHREADS)
    {
        int k = 2 + omp_get_thread_num();
        while (!(k * k > N))
        {
            for (int i = k * k; i < N; i += k)
                non_primes[i] = true;

            do
                k++;
            while (non_primes[k]);
        }
    }

    /* End */

    clk = clock() - clk;

    FILE *log;
    log = fopen("bin/log/omp_slicing", "a+");
    if (log == NULL)
        exit(1);
    fprintf(log, "%d %d %d\n", N, clk, NTHREADS);
    fclose(log);

    // FILE *out;
    // out = fopen("bin/out/omp_slicing", "w");
    // if (out == NULL)
    //     exit(1);
    // for (i = 0; i < _L; i++)
    //     if (!non_primes[i])
    //         fprintf(out, "%d ", i);
    // fclose(out);

    free(non_primes);

    return 0;
}
