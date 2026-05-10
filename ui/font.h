#ifndef UI_FONT_H
#define UI_FONT_H

#include "raylib.h"

/* Draws `text` at (x, y) using the JetBrainsMono atlas at `size`.
 * Atlases are loaded lazily on first use of each size and cached for
 * the lifetime of the process (up to a small fixed number of distinct
 * sizes). Falls back to raylib's default font on any load failure so
 * a missing TTF degrades gracefully rather than crashing. */
void font_draw (const char *text, int x, int y, int size, Color color);

/* Width in pixels of `text` rendered with font_draw at `size`.
 * Triggers a lazy atlas load on first use of each size. */
int  font_measure (const char *text, int size);

/* Returns the loaded Font handle for `size` (loading lazily on first
 * use). On load failure, returns raylib's default font so the caller
 * can still pass it to APIs like GuiSetFont without checking. */
Font font_get (int size);

/* Releases all cached atlases. Call before CloseWindow. */
void font_shutdown (void);

#endif // UI_FONT_H
