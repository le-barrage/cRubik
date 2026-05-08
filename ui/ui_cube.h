#ifndef UI_CUBE_H
#define UI_CUBE_H

#include "cube.h"

/* Draws the cube in 3D mode (axis lines + cubies) using the global camera
 * and the configured rotation speed. The caller is responsible for the
 * surrounding 2D HUD on the same frame. */
void ui_cube_3d_draw (cube_t *cube);

#endif // UI_CUBE_H
