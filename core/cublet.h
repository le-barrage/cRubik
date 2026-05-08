#ifndef CUBLET_H
#define CUBLET_H

#include "raylib.h"

#define FACE_COUNT 6

typedef enum
{
  FACE_UP,
  FACE_FRONT,
  FACE_RIGHT,
  FACE_BACK,
  FACE_LEFT,
  FACE_DOWN
} face_t;

/* A single small cube ("cubie") inside the larger Rubik's cube. Tracks the
 * color shown on each of its 6 faces: an inner cubie has BLACK on every
 * face, a corner has 3 non-black faces, an edge 2, a center 1. */
typedef struct
{
  Color colors[FACE_COUNT];
  float side_length;
} cubie_t;

/* Construct a cubie at grid position (x, y, z) within an N=size cube. Each
 * coordinate must satisfy 0 <= coord < size. The cubie's outer-facing colors
 * are derived from its position; inner faces get BLACK. */
cubie_t cubie_make (int x, int y, int z, float side_length, int size);

void cubie_rotate_right (cubie_t *cubie);
void cubie_rotate_left (cubie_t *cubie);
void cubie_rotate_up (cubie_t *cubie);
void cubie_rotate_down (cubie_t *cubie);
void cubie_rotate_clockwise (cubie_t *cubie);
void cubie_rotate_anticlockwise (cubie_t *cubie);

/* Render the cubie at world-space `position`. To animate a layer rotation,
 * pass the rotation axis as `rotation_axis` (also used as a pivot offset)
 * and the angle in degrees as `rotation_angle`. For static rendering (no
 * animation transform), pass {0,0,0} as `rotation_axis` and 0 as
 * `rotation_angle`. */
void cubie_draw (cubie_t *cubie, Vector3 position, Vector3 rotation_axis,
                 float rotation_angle);

#endif // CUBLET_H
