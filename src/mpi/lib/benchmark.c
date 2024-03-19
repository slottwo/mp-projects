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
void print_list(List l, size_t i)
{
    printf("<node%d>:{start_time: %f, end_time: %f, name: %s", i, l->start_time, l->end_time, l->name);

    if (l->next == NULL)
        printf("}\n");
    else {
        printf(",\n");
        print_list(l->next, i + 1);
    }
}
typedef struct info {
    List list;
    List tail;
    bool stopped;
    int rank;
}* Info;
void print_info(Info info)
{
    printf("info:{stopped: %d, rank: %d, list:[\n",info->stopped, info->rank);
    print_list(info->list, 0);
    printf("], tail:[\n");
    print_list(info->tail, 0);
    printf("]\n");
    printf("\n");
}
struct group_node {
    const char *name;
    Info list;
    struct group_node *next;
};
void print_group(struct group_node *g)
{
    printf("{group:{name: %s, info-list:[\n");
    print_info(g->list);
    printf("], next->[");

    if (g->next != NULL) {
        printf(",\n");
        print_group(g->next);
    }
    else
        printf("]}\n");
}
struct benchmark_info {
    struct group_node *list;
    struct group_node *tail;
    double base_time;
    Info current;
};
void print_bench(BenchmarkInfo b)
{
    printf("full benchmark{base_time: %f, group-list:(", b->base_time);
    print_group(b->list);
    printf("), tail-list:(");
    print_group(b->tail);
    printf("), current-info:|");
    print_info(b->current);
    printf("|\n");
}
static unsigned using_groups = 0;

static Info _create_info(const char *name, int rank)
{
    // Adds info node
    Info info = malloc(sizeof(struct info));
    info->rank = rank;
    info->stopped = false;

    List next = malloc(sizeof(struct node));
    info->tail = info->list = next;
    next->name = name;
    next->next = NULL;
    next->start_time = MPI_Wtime();

    return info;
}

BenchmarkInfo benchmark_start(const char *name, int rank)
{
    static const char *DEFAULT = "Default";

    printf("%s Δt: ...", name);
    fflush(stdout);

    BenchmarkInfo benchmark = malloc(sizeof(struct benchmark_info));
    benchmark->base_time = 0.0;

    // Force group creation
    struct group_node *group = malloc(sizeof(struct group_node));
    benchmark->tail = benchmark->list = group;
    group->name = DEFAULT;
    group->next = NULL;

    benchmark->current = _create_info(name, benchmark->current->rank);
    group->list = benchmark->current;

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

    Info info = benchmark->current;
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

    Info info = benchmark->current;

    print_bench(benchmark);

    info->tail->end_time = time;

    info->stopped = true;

    return time;
}

void benchmark_back(const char *name, BenchmarkInfo benchmark)
{
    printf("%s time: ...", name);
    fflush(stdout);

    Info info = benchmark->current;

    // debug
    //print_info(info);

    // Add next node
    List node = malloc(sizeof(struct node));
	info->tail = info->tail->next = node;

    node->start_time = node->end_time = 0.0;
	node->name = name;
	node->next = NULL;
	node->start_time = MPI_Wtime();
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

    benchmark->current = _create_info(next, benchmark->current->rank);

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

static void _benchmark_sequence(int wsize, bool clear, Info info)
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

static double _benchmark_group(int wsize, bool clear, Info info)
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
