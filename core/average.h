#ifndef AVERAGE_H
#define AVERAGE_H

#include <stdbool.h>
#include <stddef.h>
#define LAST_N_SOLVES 5
#define TIME_STR_MAX 20
#define AVG_STR_LEN 10

/* Append a solve to the cube's persistent solves file (times/<size>.time).
 * `time` is in MM:SS.mmm format. `scramble` is the move sequence. */
void solves_save (const char *time, const char *scramble, int cube_size);

/* Load the most recent LAST_N_SOLVES solves' display strings from the cube's
 * file. Each slot holds either a time "12:34.567", "12:34.567+" for +2,
 * "DNF", or "-" for a missing slot. When all LAST_N_SOLVES slots are filled,
 * the best and worst entries are wrapped in parentheses, e.g. "(00:08.421)". */
void solves_load_last_5 (char times[LAST_N_SOLVES][TIME_STR_MAX],
                         int cube_size);

/* Compute the average of LAST_N_SOLVES solves: drop best and worst, average
 * the rest. Writes "MM:SS.mmm" to `avg`, "DNF" if 2+ solves are DNF, or "-"
 * if any slot is missing. */
void solves_average_of_5 (char times[LAST_N_SOLVES][TIME_STR_MAX],
                          char avg[AVG_STR_LEN]);

/* Toggle DNF or +2 status for the solve at `index` (0..LAST_N_SOLVES-1)
 * within the most recent solves. Updates the on-disk file. */
void solves_toggle_dnf (int index, int cube_size);
void solves_toggle_plus_two (int index, int cube_size);

/* Copy the scramble string of the solve at `last_n_index` to `out`.
 * `last_n_index` is 0..LAST_N_SOLVES-1 and matches the indexing of the
 * times[] array filled by solves_load_last_5. Returns true on success.
 * Returns false (and leaves `out` unmodified) on any failure: file missing
 * or unparseable, index out of range, scramble field missing, or
 * `out_size` too small to hold the scramble. */
bool solves_get_scramble (int last_n_index, int cube_size,
                          char *out, size_t out_size);

#endif // AVERAGE_H
