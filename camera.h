#ifndef CAMERA_H
#define CAMERA_H

#include "include/raylib.h"

/* The active 3D camera. Writable so the renderer can pass it to
 * BeginMode3D directly. */
extern Camera camera;

/* Initialize / reset to default angles and magnification scaled to
 * `cube_size`. Larger cubes need the camera further away. */
void camera_init (int cube_size);

/* Per-frame update: handles middle-mouse reset, left-mouse drag rotation,
 * mouse-wheel zoom (with inertia). `cube_size` is read to clamp the zoom
 * magnitude proportionally. */
void camera_update (int cube_size);

#endif // CAMERA_H
