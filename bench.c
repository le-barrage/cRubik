/* Scrambles N 3x3 cubes, runs the kociemba two-phase solver on
 * each, and reports success rate, average move count, and average
 * search time. Built via `make bench`, output binary is ./bench. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cube.h"
#include "kociemba/coordCube.h"
#include "kociemba/twoPhase.h"
#include "logger.h"
#include "scramble.h"
#include "time_consts.h"

/* Replace raylib's GetRandomValue (which needs InitWindow) with a plain
 * libc-rand version. The bench Makefile target uses
 * `-Wl,--wrap=GetRandomValue` so all calls are routed here. */
int __wrap_GetRandomValue(int min, int max)
{
    return min + rand() % (max - min + 1);
}

#define BENCH_RUNS            1000
#define MAX_SOLUTION_MOVES    25
#define SOLUTION_DEPTH_LIMIT  22
#define SOLVER_TIMEOUT_MS     20000
#define PROGRESS_REPORT_EVERY 25

static long long elapsed_ns(struct timespec t0, struct timespec t1)
{
    return (t1.tv_sec - t0.tv_sec) * (long long)NS_PER_SEC
           + (t1.tv_nsec - t0.tv_nsec);
}

static int scramble_cube(cube_t *cube)
{
    int    len   = scramble_length(cube->size);
    char **moves = malloc(len * sizeof(char *));
    if (moves == NULL) return 1;
    if (scramble_generate(moves, len, cube->size) != SCRAMBLE_OK) {
        free(moves);
        return 1;
    }
    for (int i = 0; i < len; i++) {
        cube_apply_move(cube, moves[i]);
        free(moves[i]);
    }
    free(moves);
    return 0;
}

static int solve_cube(cube_t *cube, int *out_moves, long long *out_ns)
{
    cube_t canonical;
    cube_detect_orientation_and_normalize(cube, &canonical);
    char facelets[CUBE_FACELET_STR_LEN];
    cube_to_string(&canonical, facelets, sizeof facelets);
    cube_destroy(&canonical);

    Move            solution[MAX_SOLUTION_MOVES] = { 0 };
    int             depth                        = 0;
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int err = findSolutionBasic(facelets, SOLUTION_DEPTH_LIMIT,
                                SOLVER_TIMEOUT_MS, solution, &depth);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    *out_ns = elapsed_ns(t0, t1);
    if (err == 0) *out_moves = depth;
    return err;
}

int main(void)
{
    log_init(LOG_LEVEL_WARN, NULL);
    printf("Loading kociemba pruning tables...\n");
    init();
    printf("Running %d solves...\n", BENCH_RUNS);

    srand((unsigned)time(NULL));

    int       successes        = 0;
    long long total_moves      = 0;
    long long total_ns         = 0;
    long long success_total_ns = 0;
    int       min_moves        = MAX_SOLUTION_MOVES + 1;
    int       max_moves        = 0;

    for (int run = 0; run < BENCH_RUNS; run++) {
        cube_t cube = cube_make(3, 1.0f);
        if (scramble_cube(&cube) != 0) {
            LOG_WARN("run %d: scramble failed", run);
            cube_destroy(&cube);
            continue;
        }

        int       moves = 0;
        long long ns    = 0;
        int       err   = solve_cube(&cube, &moves, &ns);
        total_ns += ns;

        if (err == 0) {
            successes++;
            success_total_ns += ns;
            total_moves += moves;
            if (moves < min_moves) min_moves = moves;
            if (moves > max_moves) max_moves = moves;
        } else
            LOG_WARN("run %d: %s", run, printErrorMessage(err));

        cube_destroy(&cube);

        if ((run + 1) % PROGRESS_REPORT_EVERY == 0)
            fprintf(stderr, "\r%d/%d", run + 1, BENCH_RUNS);
    }
    fprintf(stderr, "\n\n");

    printf("Success rate:        %d/%d (%.1f%%)\n", successes, BENCH_RUNS,
           100.0 * successes / BENCH_RUNS);
    if (successes > 0) {
        printf("Avg move count:      %.2f (min %d, max %d)\n",
               (double)total_moves / successes, min_moves, max_moves);
        printf("Avg search time:     %.2f ms (successes only)\n",
               (double)success_total_ns / successes / NS_PER_MS);
    }
    printf("Avg search time:     %.2f ms (all runs)\n",
           (double)total_ns / BENCH_RUNS / NS_PER_MS);

    return 0;
}
