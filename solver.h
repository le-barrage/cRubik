#ifndef SOLVER_H
#define SOLVER_H

#include "cube.h"
#include "queue.h"
#include "timer.h"
#include <stdbool.h>

#define SOLVER_SOLUTION_MAX_LEN  100
#define SOLVER_FOUND_TEXT_LEN    80

/* The most recent solver result (or an error message) and the rendered
 * "N moves solution found in ~T ms:" header. The UI reads these directly
 * to display the solution and the Apply button. */
extern char solver_current_solution[SOLVER_SOLUTION_MAX_LEN];
extern char solver_solution_found_text[SOLVER_FOUND_TEXT_LEN];
extern int solver_current_solution_size;

/* True between solver_apply_current() and the move queue draining. The
 * input handler refuses face rotations while this is set. */
extern bool solver_is_running;

/* Loads the Kociemba pruning tables. Heavy (~1s).
 * Called once at startup. */
void solver_init_kociemba (void);

/* Kicks off the kociemba solver in a detached worker thread that reads
 * `cube`. The result (or an error string) lands in solver_current_solution
 * once the thread finishes. No-op if a worker is already running. */
void solver_launch (cube_t *cube);

/* Parses solver_current_solution and pushes the moves onto `queue`. Marks
 * solver_is_running and disables `timer` for the duration of the playback.
 * solver_finish() reverses both when the queue drains. */
void solver_apply_current (queue_t *queue, stopwatch_t *timer);

/* Clears solution buffer and size. */
void solver_clear_solution (void);

/* Called when the move queue drains. If a solver-driven playback was
 * running, clears the flag and re-enables `timer`. */
void solver_finish (stopwatch_t *timer);

#endif // SOLVER_H
