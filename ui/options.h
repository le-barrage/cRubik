#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

typedef enum {
    OPTIONS_SOLVER_REORIENT,
    OPTIONS_SOLVER_PRESERVE,
} options_solver_mode_t;

/* Renders the options screen. Writes hover state to `*out_hover` (may
 * be NULL) so the caller can aggregate cursor management across
 * overlays. */
void options_draw_screen (bool *out_hover);
void options_reset_to_defaults (void);
void options_load (void);
void options_save (void);

/* Current rotation animation speed (degrees per frame). Settable from the
 * Options screen slider. Read by the cube renderer. */
int options_rotation_speed (void);

/* How the solver should report its solution: as moves on the canonical
 * orientation (with prepended X/Y/Z normalization), or remapped to the
 * cube's current visible orientation. */
options_solver_mode_t options_solver_mode (void);

/* When true, selecting a pattern animates each move through the queue.
 * When false, the moves are applied to the cube state in one shot with
 * no animation. */
bool options_animate_patterns (void);

#endif  // OPTIONS_H
