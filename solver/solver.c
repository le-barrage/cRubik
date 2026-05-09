#include "solver.h"

#include "funny.h"
#include "kociemba/coordCube.h"
#include "kociemba/enums.h"
#include "kociemba/twoPhase.h"
#include "logger.h"
#include "options.h"
#include "playback.h"
#include "time_consts.h"
#include "utils.h"
#include <ctype.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SOLUTION_DEPTH      22
#define MAX_SOLUTION_MOVES  25
#define SOLVER_TIMEOUT_MS   20000

char solver_current_solution[SOLVER_SOLUTION_MAX_LEN];
char solver_solution_found_text[SOLVER_FOUND_TEXT_LEN];
int solver_current_solution_size;
bool solver_is_running = false;

/* Touched from both the main thread (solver_launch) and the worker thread
 * (clears on completion or error path), so atomic. */
static atomic_bool is_thread_launched = false;
static pthread_t solver_thread;

/* When true, solver_thread is joinable and we owe it a pthread_join.
 * Only the main thread reads/writes this. The worker thread doesn't touch
 * it. */
static bool thread_needs_join = false;

void
solver_init_kociemba (void)
{
  init ();
  srand ((unsigned)time (NULL));
}

static int
find_solution_and_update_moves (cube_t *cube, int depth_limit, int time_out)
{
  cube_t canonical;
  cube_orientation_t orientation
      = cube_detect_orientation_and_normalize (cube, &canonical);

  char cube_str[CUBE_FACELET_STR_LEN];
  cube_to_string (&canonical, cube_str, sizeof cube_str);
  cube_destroy (&canonical);

  Move moves[MAX_SOLUTION_MOVES] = { 0 };
  int depth;
  int error
      = findSolutionBasic (cube_str, depth_limit, time_out, moves, &depth);
  if (error != 0)
    return error;

  solver_current_solution[0] = '\0';

  if (options_solver_mode () == OPTIONS_SOLVER_REORIENT)
    cube_append_normalization_tokens (solver_current_solution, &orientation);

  int len = strlen (solver_current_solution);
  for (int i = 0; i < depth; i++)
    {
      Move cur = moves[i];
      face_t emit_face = (options_solver_mode () == OPTIONS_SOLVER_PRESERVE)
                             ? orientation.face_map[cur.orientation]
                             : cur.orientation;
      solver_current_solution[len++] = cube_face_letter (emit_face);
      if (cur.direction == ANTICW)
        solver_current_solution[len++] = '\'';
      else if (cur.direction == HALF)
        solver_current_solution[len++] = '2';
      if (i != depth - 1)
        solver_current_solution[len++] = ' ';
      solver_current_solution_size++;
    }
  solver_current_solution[len] = '\0';

  return 0;
}

static void *
solver_thread_main (void *arg)
{
  cube_t *cube = (cube_t *)arg;
  if (cube->size == 1)
    {
      const char *line = FUNNY_LINES[rand () % ARRAY_LEN (FUNNY_LINES)];
      snprintf (solver_current_solution, sizeof solver_current_solution,
                "%s", line);
      is_thread_launched = false;
      return NULL;
    }
  if (cube->size != 3)
    {
      snprintf (solver_current_solution, sizeof solver_current_solution,
                "The algorithm only works on 3x3x3 cubes.");
      is_thread_launched = false;
      return NULL;
    }

  struct timespec start, now;
  solver_current_solution_size = 0;

  clock_gettime (CLOCK_MONOTONIC, &start);
  int error
      = find_solution_and_update_moves (cube, SOLUTION_DEPTH, SOLVER_TIMEOUT_MS);
  clock_gettime (CLOCK_MONOTONIC, &now);
  if (error != 0)
    {
      if (error == -9)
        {
          LOG_INFO ("solver: cancelled");
          solver_current_solution[0] = '\0';
        }
      else
        snprintf (solver_current_solution, sizeof solver_current_solution,
                  "%s", printErrorMessage (error));
      is_thread_launched = false;
      return NULL;
    }

  long long elapsed_ns = (now.tv_sec - start.tv_sec) * (long long)NS_PER_SEC
                         + (now.tv_nsec - start.tv_nsec);
  int elapsed_ms = (int)(elapsed_ns / NS_PER_MS);
  if (elapsed_ms == 0)
    snprintf (solver_solution_found_text, sizeof solver_solution_found_text,
              "%d moves solution found in <1 millisecond:",
              solver_current_solution_size);
  else
    snprintf (solver_solution_found_text, sizeof solver_solution_found_text,
              "%d moves solution found in ~%d milliseconds:",
              solver_current_solution_size, elapsed_ms);

  if (elapsed_ms == 0)
    LOG_INFO ("solver: solution found in <1 ms");
  else
    LOG_INFO ("solver: solution found in ~%d ms", elapsed_ms);
  is_thread_launched = false;
  return NULL;
}

void
solver_launch (cube_t *cube)
{
  if (solver_is_searching ())
    return;
  if (thread_needs_join)
    {
      pthread_join (solver_thread, NULL);
      thread_needs_join = false;
    }
  kociemba_clear_cancel ();
  is_thread_launched = true;
  int error = pthread_create (&solver_thread, NULL, solver_thread_main, cube);
  if (error != 0)
    {
      LOG_ERROR ("pthread_create (solver): %s", strerror (error));
      is_thread_launched = false;
      return;
    }
  thread_needs_join = true;
}

bool
solver_is_searching (void)
{
  return atomic_load (&is_thread_launched);
}

void
solver_cancel (void)
{
  if (solver_is_searching ())
    {
      LOG_INFO ("solver: cancellation requested");
      kociemba_request_cancel ();
    }
  if (thread_needs_join)
    {
      pthread_join (solver_thread, NULL);
      thread_needs_join = false;
    }
}

void
solver_apply_current (queue_t *queue, stopwatch_t *timer)
{
  timer_disable (timer);
  solver_is_running = true;
  solver_current_solution_size = 0;

  playback_init (solver_current_solution);

  int i = 0;
  char rotation;
  while (solver_current_solution[i] != '\0')
    {
      char curr_move = solver_current_solution[i],
           next_move = solver_current_solution[i + 1];
      if (curr_move == ' ')
        {
          i++;
          continue;
        }
      if (next_move == '\'')
        {
          rotation = tolower (curr_move);
          i++;
        }
      else if (next_move == '2')
        {
          rotation_t r;
          if (rotation_from_char (curr_move, &r))
            queue_push (queue, move_face (r));
          rotation = curr_move;
          i++;
        }
      else
        {
          rotation = curr_move;
        }
      rotation_t r;
      if (rotation_from_char (rotation, &r))
        queue_push (queue, move_face (r));
      i++;
    }
}

void
solver_clear_solution (void)
{
  solver_current_solution[0] = '\0';
  solver_current_solution_size = 0;
}

void
solver_finish (stopwatch_t *timer)
{
  if (!solver_is_running)
    return;
  solver_is_running = false;
  timer->is_disabled = false;
}
