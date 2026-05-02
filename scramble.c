#include "scramble.h"

#include "include/raylib.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_SCRAMBLE_LEN 10
#define MAX_SCRAMBLE_LEN 200
#define LARGE_CUBE_THRESHOLD 12
#define MOVES_PER_LAYER 20

#define MOVE_BUFFER_SIZE 8

/* WCA single-cube move sets. Layer thickness is prefixed at generation time. */
static const char *const MOVES_3X3[] = {
  "R", "R'", "R2", "L", "L'", "L2", "U", "U'", "U2",
  "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2",
};

static const char *const MOVES_2X2[] = {
  "R", "R'", "R2", "U", "U'", "U2", "F", "F'", "F2",
};

static bool
are_opposite_faces (char a, char b)
{
  return (a == 'R' && b == 'L') || (a == 'L' && b == 'R')
         || (a == 'U' && b == 'D') || (a == 'D' && b == 'U')
         || (a == 'F' && b == 'B') || (a == 'B' && b == 'F');
}

/* Layer-prefixed move format is "<digits>w<face>...". Returns the face
 * letter regardless of how many digits the layer count occupies. */
static char
move_face (const char *full_move)
{
  const char *p = full_move;
  while (*p >= '0' && *p <= '9')
    p++;
  if (*p == 'w')
    p++;
  return *p;
}

/* A move is invalid if it (a) repeats the last move's face, or (b) is the
 * meat of a same-face sandwich around an opposite face (e.g., "L R L": the
 * outer Ls commute with the R, so the second L is redundant with the first). */
static bool
move_is_valid (const char *full_move, char *const *sequence,
               int sequence_length)
{
  char fm_face = move_face (full_move);
  char last_face = move_face (sequence[sequence_length - 1]);

  if (fm_face == last_face)
    return false;

  if (sequence_length > 1)
    {
      char prev_face = move_face (sequence[sequence_length - 2]);
      if (fm_face == prev_face && are_opposite_faces (fm_face, last_face))
        return false;
    }
  return true;
}

static void
free_partial (char **sequence, int count)
{
  for (int i = 0; i < count; i++)
    {
      free (sequence[i]);
      sequence[i] = NULL;
    }
}

int
scramble_length (int cube_size)
{
  if (cube_size < 1)
    return 0;
  if (cube_size <= 2)
    return MIN_SCRAMBLE_LEN;
  if (cube_size > LARGE_CUBE_THRESHOLD)
    return MAX_SCRAMBLE_LEN;
  return MOVES_PER_LAYER * (cube_size - 2);
}

scramble_status_t
scramble_generate (char **sequence, int length, int cube_size)
{
  if (cube_size < 1 || length < 1)
    return SCRAMBLE_BAD_SIZE;

  for (int i = 0; i < length; i++)
    sequence[i] = NULL;

  const char *const *moves = (cube_size == 2) ? MOVES_2X2 : MOVES_3X3;
  int move_count = (cube_size == 2) ? (int)ARRAY_LEN (MOVES_2X2)
                                    : (int)ARRAY_LEN (MOVES_3X3);

  int seq_len = 0;
  while (seq_len < length)
    {
      int idx = GetRandomValue (0, move_count - 1);
      int n = (cube_size <= 3) ? 1 : GetRandomValue (1, cube_size / 2);

      char full_move[MOVE_BUFFER_SIZE];
      snprintf (full_move, sizeof full_move, "%dw%s", n, moves[idx]);

      if (seq_len > 0 && !move_is_valid (full_move, sequence, seq_len))
        continue;

      sequence[seq_len] = strdup (full_move);
      if (sequence[seq_len] == NULL)
        {
          free_partial (sequence, seq_len);
          return SCRAMBLE_OOM;
        }
      seq_len++;
    }
  return SCRAMBLE_OK;
}
