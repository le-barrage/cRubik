#ifndef UI_PATTERNS_H
#define UI_PATTERNS_H

#include "queue.h"
#include <stdbool.h>
#include <stddef.h>

/* Render the patterns screen. Patterns whose [min_size, max_size] range
 * doesn't include `cube_size` are drawn disabled and ignore clicks.
 * If the user clicks an applicable pattern, push its moves onto `queue`,
 * write the formatted move sequence (space-separated tokens) into
 * `out_text`, and return true. Otherwise return false and leave
 * `out_text` untouched. */
bool ui_patterns_draw (int cube_size, queue_t *queue,
                       char *out_text, size_t out_size);

#endif // UI_PATTERNS_H
