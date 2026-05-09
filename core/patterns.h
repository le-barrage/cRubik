#ifndef PATTERNS_H
#define PATTERNS_H

#include "cube.h"
#include "queue.h"
#include <stddef.h>

/* Pushes the pattern's moves onto `q` and writes the human-readable
 * move string (e.g. "M2 E2 S2" or "2U2 2R2 2F2") to `out`. */
typedef void (*pattern_build_fn) (int cube_size, queue_t *q,
                                  char *out, size_t out_size);

typedef struct
{
  const char *name;
  int min_size;
  int max_size;          /* 0 = no upper bound */
  pattern_build_fn build;
} pattern_t;

extern const pattern_t PATTERNS[];
extern const size_t PATTERNS_COUNT;

/* Emits a static move_t array: pushes each move and writes the matching
 * display tokens, merging adjacent identical pairs into the "F2"
 * half-turn form. */
void pattern_emit_static (const move_t *arr, size_t n,
                          queue_t *q, char *out, size_t out_size);

#endif // PATTERNS_H
