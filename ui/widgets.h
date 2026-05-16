#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

#include "raylib.h"

#include <stdbool.h>

/* Clickable text-only button (no background). Renders `label` inside
 * `bounds` at `font_size`. Hover state lightens the text. Writes the
 * hover flag to `*out_hover` (may be NULL) so the caller can aggregate
 * cursor state across multiple widgets in a frame. Returns true on a
 * left-button press while hovered. */
bool label_button_draw (Rectangle bounds, const char *label, int font_size, bool *out_hover);

/* Filled rounded-rectangle button with a centered label. Uses a small
 * brightness shift on hover. Writes the hover flag to `*out_hover` (may be NULL).
 * Returns true on a left-button press while hovered. */
bool button_draw (Rectangle bounds, const char *label, bool *out_hover);

/* Message box: title bar (with X close), centered message text,
 * and a row of equal-width action buttons at the bottom. `buttons` is
 * a semicolon-separated list of labels (e.g. "Cancel;OK"). Aggregates
 * hover across all interactive elements into `*out_hover` (may be
 * NULL). Returns:
 *   -1 if no click happened this frame (caller keeps the dialog open)
 *    0 if the X close button was clicked
 *    1..N if button i (1-indexed, left-to-right) was clicked */
int message_box_draw (Rectangle bounds, const char *title, const char *message, const char *buttons, bool *out_hover);

#endif  // UI_WIDGETS_H
