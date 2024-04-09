#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi/lib/utils.h"

/** @brief 1 GB */
#define _GB 1073741824

int main(int argc, char *argv[])
{
    size_t N = _GB / 2048;

    /* Parse arguments */
    if (argc > 1)
        while (--argc)
            if (*(*++argv)++ == '-' && **argv == 'N')
                if (--argc)
                    N = atol(*++argv);

    bool *non_primes = (bool *)calloc(N, sizeof(bool));
    non_primes[0] = true;
    non_primes[1] = true;

    clock_t clk0 = clock();

    int i, k = 2;
    while (!(k * k > N))
    {
        for (i = k * k; i < N; i += k)
            non_primes[i] = true;

        do
            k++;
        while (non_primes[k]);
    }

    clock_t clk1 = clock();

    FILE *_fp;
    _fp = fopen(".tmp", "w");
    fprintf(_fp, "%lf %lf\n", (double)clk0 / CLOCKS_PER_SEC, (double)clk1 / CLOCKS_PER_SEC);
    fclose(_fp);

    free(non_primes);

    return 0;
}
