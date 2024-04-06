#pragma once
#include <stdbool.h>
/* Benchmark library for Open MPI */

//extern struct benchmark_info;
typedef struct benchmark_info *BenchmarkInfo;

/* Change the dynamic mode:
 * If dynamic is off will print rank on each individual line.
 * Use dynamic mode when splitting thread buffers. */
void set_dynamic(bool, BenchmarkInfo);

/* Turn the current execution group the serial for comparison. */
void set_group_as_serial(int source, BenchmarkInfo benchmark);

/* Start the benchmark timer register from base execution. */
BenchmarkInfo benchmark_start(const char *, int rank, bool dynamic);

/* Start the benchmark, receives base execution timings. */
BenchmarkInfo benchmark_start_from(const char *name, int rank, bool dynamic, double start_time, double end_time);

/* Starts next benchmark register. */
double benchmark_next(const char *, BenchmarkInfo);

/* Stop the benchmark register. */
double benchmark_stop(BenchmarkInfo);

/* Continues to next execution. */
void benchmark_back(const char *name, BenchmarkInfo);

/* Clear the benchmark registers. */
void benchmark_clear(BenchmarkInfo);

/* Starts sequence of executions on group mode. */
BenchmarkInfo benchmark_group_start(const char *name, const char *first, int rank, bool dynamic);

/* Next sequence. */
void benchmark_group_next(const char *name, const char *next, BenchmarkInfo);

/* Generates benchmark for registered executions. */
void benchmark_show(int wsize, bool clear, BenchmarkInfo);

/* Saves data as json for chrome tracing visual benchmark. */
void benchmark_save(int wsize, BenchmarkInfo);
