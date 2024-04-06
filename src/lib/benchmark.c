#include "benchmark.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <assert.h>

#define NAME_BUFF_SIZE 256
#define JSON_NODE_SIZE 256 + NAME_BUFF_SIZE
#define PROMPT_CLR "\33[35m"
#define MEMERR 43 // Memory allocation error
#define ROOT 0 // MPI main process

#define min(A, B) (((A) < (B)) ? (A) : (B))
#define max(A, B) (((A) > (B)) ? (A) : (B))

// Crash the app if memory was not created correctly
#define emalloc(S) ({\
        void *p = malloc(S);\
        if (p == NULL) MPI_Abort(MPI_COMM_WORLD, MEMERR);\
        p;}) // error check malloc
#define ecalloc(N, S) ({\
        void *p = calloc(N, S);\
        if (p == NULL) MPI_Abort(MPI_COMM_WORLD, MEMERR);\
        p;}) // error check calloc
// Read about GCC compound statements here: <https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html>
// and here: <https://stackoverflow.com/a/3533300>

typedef struct node {
    double start_time;
    double end_time;
    const char *name;
    struct node *next;
}* List;
typedef struct info {
    List list;
    List tail;
    bool stopped;
    int n;
}* Info;
struct group_node {
    const char *name;
    double base_time;
    Info list;
    struct group_node *next;
};
struct benchmark_info {
    struct group_node *list;
    struct group_node *tail;
    struct group_node *serial;
    int rank;
    bool dynamic;
    Info current;
    bool has_serial;
    struct {
        double start;
        double end;
    } serial_time;
};
static unsigned using_groups = 0;

void set_dynamic(bool value, BenchmarkInfo benchmark)
{
    benchmark->dynamic = value;
}

void set_group_as_serial(int source, BenchmarkInfo benchmark)
{
    benchmark->serial = benchmark->tail;
}

static Info _create_info(const char *name)
{
    // Adds info node
    Info info = emalloc(sizeof(struct info));
    info->stopped = false;
    info->n = 1;

    List next = emalloc(sizeof(struct node));
    info->tail = info->list = next;
    next->name = name;
    next->next = NULL;
    next->start_time = MPI_Wtime();

    return info;
}

BenchmarkInfo _benchmark_start(const char *name, int rank, bool dynamic)
{
    static const char *DEFAULT = "Default";

    if (dynamic) {
        printf("%s Δt: ...", name);
        fflush(stdout);
    }
    else
        fprintf(stderr, PROMPT_CLR "[%d] %s Δt: Loading...\33[m\n", rank, name);

    BenchmarkInfo benchmark = emalloc(sizeof(struct benchmark_info));
    benchmark->dynamic = dynamic;
    benchmark->rank = rank;
    benchmark->serial = NULL;
    benchmark->has_serial = true;

    // Force group creation
    struct group_node *group = emalloc(sizeof(struct group_node));
    benchmark->tail = benchmark->list = group;
    group->name = DEFAULT;
    group->base_time = 0.0;
    group->next = NULL;

    benchmark->current = _create_info(name);
    group->list = benchmark->current;

    return benchmark;
}

BenchmarkInfo benchmark_start(const char *name, int rank, bool dynamic)
{
    return _benchmark_start(name, rank, dynamic);
}

BenchmarkInfo benchmark_start_from(const char *name, int rank, bool dynamic, double start_time, double end_time)
{
    BenchmarkInfo benchmark = _benchmark_start(name, rank, dynamic);
    benchmark->has_serial = false;
    benchmark->serial_time.start = start_time;
    benchmark->serial_time.end = end_time;
    return benchmark;
}

BenchmarkInfo benchmark_group_start(const char *name, const char *first, int rank, bool dynamic)
{
    if (dynamic)
        printf("Group %s:\n", name);
    else
        printf("[%d] Group %s:\n", rank, name);

    BenchmarkInfo benchmark = benchmark_start(first, rank, dynamic);
    benchmark->tail->name = name;
    using_groups = 1;

    return benchmark;
}

double benchmark_stop(BenchmarkInfo benchmark)
{
    double time = MPI_Wtime();
    Info info = benchmark->current;

    if (benchmark->dynamic)
        printf("\b\b\b%g\n",
                time - info->tail->start_time); // Elapsed time
    else
        printf("[%d] %s Δt: %g\n",
                benchmark->rank,
                info->tail->name,
                time - info->tail->start_time); // Elapsed time

    info->tail->end_time = time; // Store time
    info->stopped = true;

    return time;
}

// Benchmark the now loading timing
void benchmark_back(const char *name, BenchmarkInfo benchmark)
{
    if (benchmark->dynamic) {
        printf("%s time: ...", name);
        fflush(stdout);
    }
    else
        fprintf(stderr, PROMPT_CLR "[%d] %s Δt: Loading...\33[m\n", benchmark->rank, name);

    Info info = benchmark->current;
    info->n++;

    // Add next node
    List node = emalloc(sizeof(struct node));
	info->tail = info->tail->next = node;

    node->end_time = 0.0; // nullify
	node->name = name;
	node->next = NULL;
	node->start_time = MPI_Wtime();
}

double benchmark_next(const char *name, BenchmarkInfo benchmark)
{
    double time = benchmark_stop(benchmark);
    benchmark_back(name, benchmark);

    return time;
}

static double _group_total_time(struct group_node *g, BenchmarkInfo benchmark)
{
    double total = 0.0f;
    for (struct node *n = g->list->list; n != NULL; n = n->next)
        total += n->end_time - n->start_time;

    if (benchmark->dynamic)
        printf("Group [%s] total time: %.3f\n", g->name, total);
    else
        printf("[%d]: Group [%s] total time: %.3f\n", benchmark->rank, g->name, total);

    return g->base_time = total;
}

void benchmark_group_next(const char *name, const char *next, BenchmarkInfo benchmark)
{
    if (!benchmark->current->stopped)
        benchmark_stop(benchmark);

    // Calculate group time:
    struct group_node *n = benchmark->tail;
    _group_total_time(n, benchmark);

    if (benchmark->dynamic)
        printf("\nGroup %s:\n", name);
    else
        printf("[%d] \nGroup %s:\n", benchmark->rank, name);

    benchmark->current = _create_info(next);

    // Add next group node
    struct group_node *node = emalloc(sizeof(struct group_node));
    benchmark->tail = benchmark->tail->next = node;
    node->list = benchmark->current;
    node->name = name;
    node->base_time = 0.0;
    node->next = NULL;

    n->next = node;
}

void print_efficiency(double t_serial, double t_parallel, unsigned threads_used, BenchmarkInfo benchmark)
{
	double speedup = t_serial / t_parallel;

    if (benchmark->dynamic) {
        printf("Speedup: %.4f\n", speedup);
        printf("Efficiency: %.4f\n", speedup / threads_used);
    }
    else {
        printf("[%d] Speedup: %.4f\n", benchmark->rank, speedup);
        printf("[%d] Efficiency: %.4f\n", benchmark->rank, speedup / threads_used);
    }
}

static void _benchmark_sequence(int wsize, bool clear, Info info, BenchmarkInfo benchmark)
{
    /* Fetch Serial Time */
    List head = info->list;
    double serial_time;

    if (benchmark->rank == ROOT) {
        double time = (!benchmark->has_serial) \
                ? benchmark->serial_time.start - benchmark->serial_time.end \
                : head->end_time - head->start_time;
        MPI_Bcast(&time, 1, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);
    }
    else
        MPI_Bcast(&serial_time, 1, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);

    /* Fetch fastest execution */
    char fast_name[NAME_BUFF_SIZE] = "Serial";
    double fast_time = serial_time;
    List current = info->list;

    for (int i = 0; i < wsize; i++) {
        if (benchmark->rank == i) {
            while (current != NULL) {
                if (benchmark->dynamic)
                    printf("\n* %s *", current->name);
                else
                    printf("\n* [%d] %s *\n", benchmark->rank, current->name);

                double time = current->start_time - current->end_time;
                if (time < fast_time) {
                    int len = min(strlen(current->name) + 1, NAME_BUFF_SIZE);
                    memccpy(fast_name, current->name, sizeof(char), len);
                    fast_name[len - 1] = '\0';

                    fast_time = time;
                }

                print_efficiency(serial_time, time, wsize, benchmark);

                List next = current->next;
                if (clear)
                    free(current);
                current = next;
            }

            if (clear)
                free(info);

            printf("\n[%d] Fastest: %s\n", benchmark->rank, fast_name);
            MPI_Barrier(MPI_COMM_WORLD);
        }
        else
            MPI_Barrier(MPI_COMM_WORLD);
    }
}

static void _benchmark_group(int wsize, bool clear, Info info)
{
    List next;
    for (List n = info->list; n != NULL; n = next) {
        double time = next->end_time - next->start_time;
        printf("\t%s %g\n", n->name, time);

        next = n->next;
        if (clear)
            free(n);
    }
}

static void _benchmark_groups(int wsize, bool clear, BenchmarkInfo benchmark)
{
    /* Fetch Serial Time */
    double serial_time;
    int serial_rank;

    for (int current = 0; current < wsize; current++) {
        short is_serial = benchmark->serial != NULL;

        MPI_Bcast(&is_serial, 1, MPI_INT, current, MPI_COMM_WORLD);

        if (is_serial)
            serial_rank = current;
    }

    if (serial_rank == benchmark->rank)
        MPI_Bcast(&benchmark->serial->base_time, 1, MPI_DOUBLE, serial_rank, MPI_COMM_WORLD);
    else
        MPI_Bcast(&serial_time, 1, MPI_DOUBLE, serial_rank, MPI_COMM_WORLD);

    /* Process groups */
    double fast_time = serial_time;
    const char *fast_name = benchmark->list->name;

    for (struct group_node *g = benchmark->list; g != NULL; g = g->next) {
        benchmark->current = g->list;
        double total = 0.0;
        double time = g->base_time;

        if (benchmark->dynamic) {
            printf("Group [%s]:\n", g->name);
            _benchmark_group(wsize, clear, benchmark->current);
            printf("\tTotal time: %g\n", time);
        }
        else {
            printf("[%d] Group [%s]:\n", benchmark->rank, g->name);
            _benchmark_group(wsize, clear, benchmark->current);
            printf("\t[%d] Total time: %g\n", benchmark->rank, time);
        }

        if (g != benchmark->list)
            print_efficiency(serial_time, time, wsize, benchmark);

        if (time <= fast_time) {
            fast_name = g->name;
            fast_time = time;
        }
    }

    printf("\n[%d] Fastest: %s\n", benchmark->rank, fast_name);

    /* Fetch fastest rank */
    double other_fast_time;
    char *other_fast_name;
    int other_fast_name_size;
    int fastest_rank = 0;

    for (int current = 0; current < wsize; current++) {
        if (current == ROOT)
            continue;
        if (current == benchmark->rank) {
            int fast_name_size = strlen(fast_name) + 1;
            MPI_Send(&fast_time, 1, MPI_DOUBLE, ROOT, benchmark->rank, MPI_COMM_WORLD);
            MPI_Send(&fast_name_size, 1, MPI_INT, ROOT, benchmark->rank, MPI_COMM_WORLD);
            MPI_Send(fast_name, fast_name_size, MPI_CHAR, ROOT, benchmark->rank, MPI_COMM_WORLD);
        }
        else if (benchmark->rank == ROOT) {
            MPI_Recv(&other_fast_time, 1, MPI_DOUBLE, current, current, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&other_fast_name_size, 1, MPI_INT, current, current, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            other_fast_name = emalloc(sizeof(char) * other_fast_name_size);
            MPI_Recv(other_fast_name, other_fast_name_size, MPI_CHAR, current, current, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (other_fast_time > fast_time) {
                fast_time = other_fast_time;
                fast_name = other_fast_name;
                fastest_rank = current;
            }
            free(other_fast_name);
        }
    }
    if (benchmark->rank == ROOT)
        printf("\nFastest: %s(rank %d)\n [%g]", fast_name, benchmark->rank, fast_time);
}

void benchmark_show(int wsize, bool clear, BenchmarkInfo benchmark)
{
#ifdef DEBUG
    if (benchmark == NULL) {
        fprintf(stderr, "\33[31m""Benchmark info is null""\33m\n");
        assert(benchmark != NULL);
    }
#endif

    if (!benchmark->current->stopped)
        benchmark_stop(benchmark);

    MPI_Barrier(MPI_COMM_WORLD);

    if (benchmark->rank == ROOT)
        printf("\n\nThreads used: %d\n", wsize);

    if (using_groups)
        _benchmark_groups(wsize, clear, benchmark);
    else
        _benchmark_sequence(wsize, clear, benchmark->list->list, benchmark);
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

static size_t _save_node(List node, int rank, char **out)
{
    // Create the JSON data for tracing
    char json_data[JSON_NODE_SIZE];
    sprintf(json_data, "{"
            "\"name\":\"%s\","
            "\"ph\":\"B\","
            "\"ts\":%f,"
            "\"pid\":%d,"
            "\"tid\":%d"
        "}," "\n{"
            "\"name\":\"%s\","
            "\"ph\":\"E\","
            "\"ts\":%f,"
            "\"pid\":%d,"
            "\"tid\":%d"
        "},\n", node->name, node->start_time, rank, rank,
             node->name, node->end_time, rank, rank);

    size_t size = strlen(json_data) + 1;

    *out = ecalloc(size, sizeof(char));
    strcpy(*out, json_data);

    return size;
}

void benchmark_save(int wsize, BenchmarkInfo benchmark)
{
    int rank = benchmark->rank;
    Info info = benchmark->list->list;

    char **data;

    int err;
    if (err = MPI_Alloc_mem(info->n, MPI_INFO_NULL, &data))
        MPI_Abort(MPI_COMM_WORLD, err);

    ecalloc(info->n, sizeof(char *));
    size_t total_size = 0;
    int c = 0;

    // TODO -> Benchmark save on groups
    for (List l = info->list; l != NULL; l = l->next) {
        // FIXME -> Memory error
        total_size += _save_node(l, rank, data + c) + 1; // Adds one space to the line-break
        c++;
    }

    // Gather JSON data from all ranks into a single buffer on ROOT rank
    char *file_buffer = NULL;
    size_t file_size = 0;

    MPI_Reduce(&total_size, &file_size, 1, MPI_UNSIGNED_LONG, MPI_SUM, ROOT, MPI_COMM_WORLD);

    if (rank == ROOT)
        file_buffer = (char *)ecalloc(file_size, sizeof(char));

    size_t offset = 0;
    for (int r = 0; r < wsize; r++) {
        if (r == ROOT && rank == ROOT)
            for (int i = 0; i < info->n; i++) {
                size_t size = strlen(data[i]);
                strcpy(file_buffer + offset, data[i]);
                offset += size;
                free(data[i]);
            }
        else if (r == rank) {
            MPI_Send(&info->n, 1, MPI_INT, ROOT, r, MPI_COMM_WORLD);

            for (int i = 0; i < info->n; i++) {
                size_t size = strlen(data[i]); // Removing terminator char
                MPI_Send(&size, 1, MPI_UNSIGNED_LONG, ROOT, i, MPI_COMM_WORLD);
                MPI_Send(data[i], size, MPI_CHAR, ROOT, i, MPI_COMM_WORLD);
                free(data[i]);
            }
        }
        else if (benchmark->rank == ROOT) {
            int n;
            MPI_Recv(&n, 1, MPI_INT, r, r, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int i = 0; i < n; i++) {
                size_t size;
                MPI_Recv(&size, 1, MPI_UNSIGNED_LONG, r, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(file_buffer + offset, size, MPI_CHAR, r, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                offset += size;
            }
        }
    }
    if (rank == ROOT)
        file_buffer[offset - 2] = '\0'; // Remove last comma
    free(data);

    // ROOT rank writes JSON data to file
    if (rank == ROOT) {
        // Create the JSON file for tracing
        FILE *fp = fopen("trace.json", "w");
        if (fp == NULL) {
            fprintf(stderr, "\33[31m""Error opening file.\33[m\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fprintf(fp, "{\"traceEvents\":[\n");
        fprintf(fp, "%s\n", file_buffer);
        fprintf(fp, "]}");

        fclose(fp);

        fprintf(stderr, "\33[32m""JSON file created successfully.\33[m\n");

        free(file_buffer); // Free the memory allocated for the buffer
    }
}
