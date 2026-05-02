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

#endif // !OPTIONS_H
