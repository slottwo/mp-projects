#pragma once
#include <stdbool.h>
/* Benchmark library for Open MPI */

//extern struct benchmark_info;
typedef struct benchmark_info *BenchmarkInfo;

/* Start the benchmark timer register from base execution. */
BenchmarkInfo benchmark_start(const char *, int rank);

/* Starts next benchmark register. */
double benchmark_next(const char *, BenchmarkInfo);

/* Stop the benchmark register. */
double benchmark_stop(BenchmarkInfo);

/* Continues to next execution. */
void benchmark_back(const char *name, BenchmarkInfo);

/* Clear the benchmark registers. */
void benchmark_clear(BenchmarkInfo);

/* Starts sequence of executions on group mode. */
BenchmarkInfo benchmark_group_start(const char *name, const char *first, int rank);

/* Next sequence. */
void benchmark_group_next(const char *name, const char *next, BenchmarkInfo);

/* Generates benchmark for registered executions. */
void benchmark_show(int wsize, bool clear, BenchmarkInfo);
