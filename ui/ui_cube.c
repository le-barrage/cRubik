#include "ui_cube.h"

#include "camera.h"
#include "options.h"
#include "raylib.h"
#include "raymath.h"

/* Each axis runs from the origin out to AXIS_LENGTH_FACTOR * cube_size.
 * Scaling with cube size keeps the axes looking proportional regardless
 * of N. */
#define AXIS_LENGTH_FACTOR 1.2f
#define BACKGROUND_COLOR   GRAY

void ui_cube_3d_draw (cube_t *cube, bool show_debug_axes)
{
    BeginMode3D(camera);
    ClearBackground(BACKGROUND_COLOR);

    if (show_debug_axes) {
        float axis_tip = (float)cube->size * AXIS_LENGTH_FACTOR;
        DrawLine3D(Vector3Zero(), (Vector3){ axis_tip, 0, 0 }, WHITE);
        DrawLine3D(Vector3Zero(), (Vector3){ 0, axis_tip, 0 }, WHITE);
        DrawLine3D(Vector3Zero(), (Vector3){ 0, 0, axis_tip }, WHITE);
    }
    cube_draw(cube, options_rotation_speed());
    EndMode3D();
}
