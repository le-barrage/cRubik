#ifndef OPTIONS_H
#define OPTIONS_H

typedef enum
{
  SOLVER_REORIENT,
  SOLVER_PRESERVE
} SolverOutputMode;

extern SolverOutputMode solverOutputMode;

void Options_drawScreen (void);
void Options_resetToDefaults (void);
void Options_load (void);
void Options_save (void);

/* Current rotation animation speed (degrees per frame). Settable from the
 * Options screen slider. Read by the cube renderer. */
int options_rotation_speed (void);

#endif // !OPTIONS_H
