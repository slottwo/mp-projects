#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/** @brief 1 GB */
#define _GB 1073741824

/** @brief Max numbers on output file */
#define _L ((N > 10000) ? N : 10000)

int main(int argc, char *argv[])
{
    size_t N = _GB;

    /* Parse arguments */
    if (argc > 1)
        while (--argc)
            if (*(*++argv)++ == '-' && **argv == 'N')
                if (--argc)
                    N = atol(*++argv);

    // fprintf(stderr, "\033[36m"
    //                 "Setup\033[m"
    //                 "\t[%d]\n",
    //         N);

    bool *non_primes = (bool *)calloc(N, sizeof(bool));
    non_primes[0] = true;
    non_primes[1] = true;

    // fprintf(stderr, "\33[2K\033[A\r"
    //                 "\033[33m"
    //                 "Loading\n\033[m");

    clock_t clk = clock();

    int i, k = 2;
    while (!(k * k > N))
    {
        for (i = k * k; i < N; i += k)
            non_primes[i] = true;

        do
            k++;
        while (non_primes[k]);
    }

    clk = clock() - clk;

    FILE *log;
    log = fopen("bin/log/serial", "a+");
    if (log == NULL)
        exit(1);
    fprintf(log, "%d %d\n", N, clk);
    fclose(log);

    // FILE *out;
    // out = fopen("bin/out/serial", "w");
    // if (out == NULL)
    //     exit(1);
    // for (i = 0; i < _L; i++)
    //     if (!non_primes[i])
    //         fprintf(out, "%d ", i);
    // fclose(out);

    // fprintf(stderr, "\33[2K\033[A\r"
    //                 "\033[32m"
    //                 "Done   \n\033[m");

    free(non_primes);

    return 0;
}
