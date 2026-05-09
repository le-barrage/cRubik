#ifndef CUBE_H
#define CUBE_H

#include "cublet.h"
#include <stdbool.h>
#include <stddef.h>

#define CUBE_MAX_SIZE 9
#define CUBE_DEFAULT_SIZE 3

/* "MM:SS.mmm" form not relevant here. This is the facelet string Kociemba
 * consumes: 6 faces * 9 facelets + null. Only valid for 3x3x3. */
#define CUBE_FACELET_STR_LEN 55

typedef enum
{
  ROT_U, ROT_U_PRIME,
  ROT_D, ROT_D_PRIME,
  ROT_R, ROT_R_PRIME,
  ROT_L, ROT_L_PRIME,
  ROT_F, ROT_F_PRIME,
  ROT_B, ROT_B_PRIME,
  ROT_M, ROT_M_PRIME,
  ROT_E, ROT_E_PRIME,
  ROT_S, ROT_S_PRIME,
  ROT_X, ROT_X_PRIME,
  ROT_Y, ROT_Y_PRIME,
  ROT_Z, ROT_Z_PRIME,
} rotation_t;

/* A rotation with context. Expresses an animatable cube move with enough
 * information for the animator and the cube_rotate machinery to dispatch
 * correctly:
 *
 *  - For face rotations (ROT_U..ROT_B_PRIME):
 *      single_layer = false: turn the outer `num_layers` layers as one
 *        wide chunk. num_layers=1 is the plain outer face turn.
 *      single_layer = true: turn ONLY the layer at depth=num_layers from
 *        the face. num_layers=1 is equivalent to single_layer=false.
 *
 *  - For slice rotations (ROT_M, ROT_E, ROT_S):
 *      num_layers has the same meaning as cube_rotate's argument
 *      (controls how far from the outer the slice reaches). single_layer
 *      is ignored.
 *
 *  - For whole-cube rotations (ROT_X, ROT_Y, ROT_Z):
 *      both num_layers and single_layer are ignored. */
typedef struct
{
  rotation_t rotation;
  int        num_layers;
  bool       single_layer;
} move_t;

/* Convenience constructors. Use these instead of brace-init at call sites. */
static inline move_t move_face (rotation_t r) {
  return (move_t){ .rotation = r, .num_layers = 1, .single_layer = false };
}
static inline move_t move_face_wide (rotation_t r, int n) {
  return (move_t){ .rotation = r, .num_layers = n, .single_layer = false };
}
static inline move_t move_face_single (rotation_t r, int depth) {
  return (move_t){ .rotation = r, .num_layers = depth, .single_layer = true };
}
static inline move_t move_slice (rotation_t r, int n) {
  return (move_t){ .rotation = r, .num_layers = n, .single_layer = false };
}
static inline move_t move_whole (rotation_t r) {
  return (move_t){ .rotation = r, .num_layers = 0, .single_layer = false };
}

/* Macro forms of the above for use in static const initializers, where
 * inline functions are not constant expressions. */
#define MOVE_FACE(r)            { (r), 1, false }
#define MOVE_FACE_WIDE(r, n)    { (r), (n), false }
#define MOVE_FACE_SINGLE(r, d)  { (r), (d), true }
#define MOVE_SLICE(r, n)        { (r), (n), false }
#define MOVE_WHOLE(r)           { (r), 0, false }

/* NxNxN Rubik's cube. Storage is a heap-allocated 3D array of cubies; the
 * struct itself is caller-managed (return by value, pass by pointer). Use
 * cube_make to construct, cube_destroy to release the heap arrays. */
typedef struct
{
  cubie_t ***cube;
  int size;
  bool is_animating;
  move_t current_move;
  int rotation_degrees;
} cube_t;

typedef struct
{
  rotation_t moves[4];
  int count;
  face_t face_map[FACE_COUNT];
} cube_orientation_t;

/* Construct a `size`x`size`x`size` cube. `cubie_size` is the per-cubie side
 * length used for rendering. Must call cube_destroy on the returned cube. */
cube_t cube_make (int size, float cubie_size);

cube_t cube_deep_copy (const cube_t *src);

/* Frees the cube's internal heap arrays. The struct shell itself is not
 * freed, cube_t is a value type. */
void cube_destroy (cube_t *cube);

/* Renders all cubies. `rotation_speed` advances the in-progress animation
 * (degrees per frame); pass 0 for no animation. */
void cube_draw (cube_t *cube, int rotation_speed);

/* Parses a single rotation letter (e.g. 'U', 'u', 'X') into a rotation_t.
 * Returns true on success and writes to `out`; false for any other char. */
bool rotation_from_char (char c, rotation_t *out);

/* Applies a WCA-style move string like "1wR2" or "3wU'" by parsing it and
 * delegating to cube_rotate. */
void cube_apply_move (cube_t *cube, const char *move);

void cube_rotate (cube_t *cube, rotation_t rotation, int num_layers);

/* Rotates a single layer at `depth` from the side identified by
 * `face_rotation`. depth=1 is the outer face layer (equivalent to
 * cube_rotate(face_rotation, 1)), depth=2 is one layer in, ... up to
 * depth=cube->size/2 which is the deepest layer reachable from this
 * face (anything deeper is closer to the opposite face and should be
 * addressed from there instead).
 *
 * `face_rotation` must be a face turn (ROT_U..ROT_B_PRIME). Slice and
 * whole-cube rotations are no-ops here. */
void cube_rotate_single_layer (cube_t *cube, rotation_t rotation,
                               int depth);

/* Writes the 54-character facelet string to `out` (must be at least
 * CUBE_FACELET_STR_LEN bytes). Only meaningful for 3x3x3. */
void cube_to_string (const cube_t *cube, char *out, size_t out_size);

cube_orientation_t cube_detect_orientation_and_normalize (const cube_t *src,
                                                          cube_t *out_canonical);

void cube_append_normalization_tokens (char *buf,
                                       const cube_orientation_t *orientation);

char cube_face_letter (face_t face);

void cube_rotation_token (rotation_t r, char out[3]);

#endif // CUBE_H
