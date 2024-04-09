/**
 * @file utils.c
 * @authors @slottwo, @GersonFeDutra
 * @brief Utilities for this project
 * @version 0.1
 * @date 2024-03-15
 *
 * @copyright Copyright (c) 2024
 *
 */

/** @brief MPI main process */
#define ROOT 0

/** @brief 1 GB */
#define _GB 1073741824

#define min(A, B) (((A) < (B)) ? (A) : (B))
#define max(A, B) (((A) > (B)) ? (A) : (B))

/** @brief Alloc error flag */
#define EXIT_ERR_MEM 1
/** @brief File creation error flag */
#define EXIT_ERR_FILE 2
/** @brief Program arguments error flag */
#define EXIT_ERR_ARG 3

// Crash the app if memory was not created correctly
#define emalloc(S) ({\
        void *p = malloc(S);\
        if (p == NULL) \
            MPI_Abort(MPI_COMM_WORLD, EXIT_ERR_MEM);\
        p;}) // error check malloc
#define ecalloc(N, S) ({\
        void *p = calloc(N, S);\
        if (p == NULL) MPI_Abort(MPI_COMM_WORLD, EXIT_ERR_MEM);\
        p;}) // error check calloc
#define efopen(NAME, MODE) ({\
        FILE *fp = fopen(NAME, MODE);\
        if (fp == NULL) \
            MPI_Abort(MPI_COMM_WORLD, EXIT_ERR_FILE);\
        fp;}) // error check fopen
// Read about GCC compound statements here: <https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html>
// and here: <https://stackoverflow.com/a/3533300>
