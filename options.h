#ifndef OPTIONS_H
#define OPTIONS_H

typedef enum
{
  OPTIONS_SOLVER_REORIENT,
  OPTIONS_SOLVER_PRESERVE,
} options_solver_mode_t;

void options_draw_screen (void);
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

#endif // OPTIONS_H
