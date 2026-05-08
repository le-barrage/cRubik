#ifndef UI_PATTERNS_H
#define UI_PATTERNS_H

#include "queue.h"
#include <stdbool.h>
#include <stddef.h>

/* Render the patterns screen. If the user clicks a pattern, push its moves
 * onto `queue`, write the formatted move sequence (space-separated tokens)
 * into `out_text`, and return true. Otherwise return false and leave
 * `out_text` untouched. */
bool ui_patterns_draw (queue_t *queue, char *out_text, size_t out_size);

#endif // UI_PATTERNS_H
