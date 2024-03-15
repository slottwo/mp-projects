/**
 * @file 0.c
 * @authors slottwo, GersonFeDutra
 * @brief Numerical Integration
 * @version 0.1
 * @date 2024-03-15
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 1000000

int main(int argc, char const *argv[])
{
    // MPI.h Initialization

    int w_size, rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &w_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;

    /** @brief Initial time stamp */
    double t_init;
    if (rank == 0)
        t_init = MPI_Wtime();

    /* INICIO */

    /* FIM */

    if (rank == 0)
        printf("Δt: %f\n", MPI_Wtime() - t_init);

    MPI_Finalize();
    return 0;
}
