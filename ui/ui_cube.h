#ifndef UI_CUBE_H
#define UI_CUBE_H

#include "cube.h"

#include <stdbool.h>

/* Draws the cube in 3D mode using the global camera and the configured
 * rotation speed. When show_debug_axes is true, the world X/Y/Z axes are
 * drawn from the origin out past the cube. The caller is responsible for
 * the surrounding 2D HUD on the same frame. */
void ui_cube_3d_draw (cube_t *cube, bool show_debug_axes);

#endif  // UI_CUBE_H
