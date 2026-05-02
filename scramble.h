#ifndef SCRAMBLE_H
#define SCRAMBLE_H

typedef enum
{
  SCRAMBLE_OK = 0,
  SCRAMBLE_OOM,
  SCRAMBLE_BAD_SIZE,
} scramble_status_t;

/* Default scramble length for an NxNxN cube.
 * Returns 0 if cube_size < 1. */
int scramble_length (int cube_size);

/* Generate a `length`-long random-move scramble into `sequence`. Each entry
 * is set to a heap-allocated string like "1wR", "2wF'", "3wU2"; the caller
 * takes ownership and must free each.
 *
 * This is a random-move generator with anti-redundancy filtering, NOT a
 * WCA-compliant random-state generator.
 *
 * `cube_size` must be >= 1. On SCRAMBLE_OOM, any partially-allocated
 * strings are freed and `sequence` is left NULL-filled. */
scramble_status_t scramble_generate (char **sequence, int length,
                                     int cube_size);

#endif // SCRAMBLE_H
