#include "benchmark.h"
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

typedef struct node {
    double start_time;
    double end_time;
    const char *name;
    struct node *next;
}* List;
struct info {
    List list;
    List tail;
    bool stopped;
    int rank;
};
struct group_node {
    const char *name;
    struct info *list;
    struct group_node *next;
};
struct benchmark_info {
    struct group_node *list;
    struct group_node *tail;
    double base_time;
    struct info *current;
};
static unsigned using_groups = 0;

static void _start(const char *name, BenchmarkInfo benchmark)
{
    struct info *info = malloc(sizeof(struct info));
    info->stopped = false;

    // Adds info node
    info->tail = info->list = malloc(sizeof(struct node));
    info->tail->name = name;
    info->tail->next = NULL;
    info->tail->start_time = MPI_Wtime();

    benchmark->current = info;
}

BenchmarkInfo benchmark_start(const char *name, int rank)
{
    static const char *DEFAULT = "Default";

    printf("%s Δt: ...", name);
    fflush(stdout);

    BenchmarkInfo benchmark = malloc(sizeof(struct benchmark_info));

    // Force group creation
    struct group_node *group = malloc(sizeof(struct group_node));
    benchmark->tail = benchmark->list = group;
    benchmark->tail->name = DEFAULT;
    benchmark->tail->next = NULL;

    _start(name, benchmark);
    benchmark->tail->list = benchmark->current;

    return benchmark;
}

BenchmarkInfo benchmark_group_start(const char *name, const char *first, int rank)
{
    printf("Group %s:\n", name);

    BenchmarkInfo benchmark = benchmark_start(first, rank);
    benchmark->tail->name = name;
    using_groups = 1;

    return benchmark;
}

double benchmark_next(const char *name, BenchmarkInfo benchmark)
{
    double time = MPI_Wtime();

    printf("\b\b\b%g\n", time); // Elapsed time
    printf("%s time: ...", name);
    fflush(stdout);

    struct info *info = benchmark->current;
    info->tail->end_time = time; // Store time

    // Add a info node
    info->tail = info->tail->next = malloc(sizeof(struct node));
    info->tail->name = name;
    info->tail->start_time = time;
    info->tail->next = NULL;

    return time;
}

double benchmark_stop(BenchmarkInfo benchmark)
{
    double time = MPI_Wtime();

    printf("\b\b\b%g\n", time);

    struct info *info = benchmark->current;
    info->tail->end_time = time;
    info->stopped = true;

    return time;
}

void benchmark_back(const char *name, BenchmarkInfo benchmark)
{
    printf("%s time: ...", name);
    fflush(stdout);

    struct info *info = benchmark->current;
    info->stopped = false;

    // Add next node
	info->tail = info->tail->next = malloc(sizeof(struct node));

    MPI_Finalize();
    return;

	info->tail->name = name;
	info->tail->next = NULL;

	info->tail->start_time = MPI_Wtime();
}

static double _group_total_time(struct group_node *g)
{
    double total = 0.0f;
    for (struct node *n = g->list->list; n != NULL; n = n->next)
        total += n->end_time - n->start_time;

    printf("Group [%s] total time: %.3f\n", g->name, total);

    return total;
}

void benchmark_group_next(const char *name, const char *next, BenchmarkInfo benchmark)
{
    if (!benchmark->current->stopped)
        benchmark_stop(benchmark);

    // Calculate group time:
    struct group_node *n = benchmark->tail;
    benchmark->base_time = _group_total_time(n);

    printf("\nGroup %s:\n", name);

    _start(next, benchmark);

    // Add next group node
    benchmark->tail = benchmark->tail->next = malloc(sizeof(struct group_node));
    benchmark->tail->list = benchmark->current;
    benchmark->tail->name = name;
    benchmark->tail->next = NULL;

    n->next = benchmark->tail;
}

void print_efficiency(double t_serial, double t_parallel, unsigned threads_used)
{
	double speedup = t_serial / t_parallel;

    printf("Speedup: %.4f\n", speedup);
	printf("Efficiency: %.4f\n", speedup / threads_used);
}

static void _benchmark_sequence(int wsize, bool clear, struct info *info)
{
    const char *fast_name = info->list->name;
    double serial_time, fast_time;
    double time = info->list->end_time - info->list->start_time;
    fast_time = serial_time = time;

	List next = info->list->next;

    if (clear)
        free(info->list);

	while (next != NULL) {
        printf("\n* %s *", next->name);
		time = next->start_time - next->end_time;

		if (time <= fast_time) {
            fast_name = next->name;
            fast_time = time;
        }

        print_efficiency(serial_time, time, wsize);

        info->list = next;
		next = next->next;

        if (clear)
            free(info->list);
	}

    printf("\nFastest: %s\n", fast_name);
}


static double _benchmark_group(int wsize, bool clear, struct info *info)
{
    double total = 0.0;

    List next;
    for (List n = info->list; n != NULL; n = next) {
        double time = next->end_time - next->start_time;
        total += time;
        printf("\t%s %g\n", n->name, time);

        next = n->next;
        if (clear)
            free(n);
    }

    return total;
}

void benchmark_show(int wsize, bool clear, BenchmarkInfo benchmark)
{
    if (benchmark == NULL) {
        fprintf(stderr, "\33[31m""Benchmark info is null""\33m\n");
        return;
    }

    if (!benchmark->current->stopped)
        benchmark_stop(benchmark);

    printf("\n\nThreads used: %u\n", wsize);

    if (using_groups) {
        double serial_time = benchmark->base_time;
        double fast_time = serial_time;
        const char *fast_name = benchmark->list->name;

        for (struct group_node *g = benchmark->list; g != NULL; g = g->next) {
            benchmark->current = g->list;
            double total = 0.0;

            printf("Group [%s]:\n", g->name);
            double time = _benchmark_group(wsize, clear, benchmark->current);
            printf("\tTotal time: %g\n", time);

            if (g != benchmark->list)
                print_efficiency(serial_time, time, wsize);

            if (time <= fast_time) {
                fast_name = g->name;
                fast_time = time;
            }
        }

        printf("\nFastest: %s\n", fast_name);
    }
    else
        _benchmark_sequence(wsize, clear, benchmark->list->list);
}

void benchmark_clear(BenchmarkInfo benchmark)
{
    struct group_node *next_group;
    for (struct group_node *g = benchmark->list; g != NULL; g = next_group) {
        benchmark->current = g->list;

        List next = benchmark->current->list->next;

        free(benchmark->current->list);

        while (next != NULL) {
            benchmark->current->list = next;
            next = next->next;
            free(benchmark->current->list);
        }

        next_group = g->next;
        free(g);
    }

    free(benchmark);
}
