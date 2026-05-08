#include "ui_cube.h"

#include "camera.h"
#include "raylib.h"
#include "raymath.h"
#include "options.h"

#define AXIS_TIP_PADDING  2.0f
#define BACKGROUND_COLOR  GRAY

void
ui_cube_3d_draw (cube_t *cube)
{
  BeginMode3D (camera);
  ClearBackground (BACKGROUND_COLOR);

  float axis_tip = (float)cube->size / 2 + AXIS_TIP_PADDING;
  DrawLine3D (Vector3Zero (), (Vector3){ axis_tip, 0, 0 }, WHITE);
  DrawLine3D (Vector3Zero (), (Vector3){ 0, axis_tip, 0 }, WHITE);
  DrawLine3D (Vector3Zero (), (Vector3){ 0, 0, axis_tip }, WHITE);
  cube_draw (cube, options_rotation_speed ());
  EndMode3D ();
}
