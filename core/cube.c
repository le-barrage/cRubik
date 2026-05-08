#include "cube.h"

#include "logger.h"
#include "raylib.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROTATION_DEGREES_FULL 90
#define ROTATION_PAIR_COUNT 12

/* The rotation_t enum is laid out as 12 (cw, prime) pairs: ROT_U/ROT_U_PRIME,
 * ROT_D/ROT_D_PRIME, ..., ROT_Z/ROT_Z_PRIME. This table maps each pair index
 * (0..11) to its single-character cubing notation letter. */
static const char ROTATION_LETTERS[] = "UDRLFBMESXYZ";

static bool
is_inner_cubie (int x, int y, int z, int size)
{
  return x != 0 && y != 0 && z != 0 && x != size - 1 && y != size - 1
         && z != size - 1;
}

cube_t
cube_make (int size, float cubie_size)
{
  cube_t cube;
  cube.size = size;
  cube.cube = malloc (size * sizeof (cubie_t **));
  for (int x = 0; x < size; x++)
    {
      cube.cube[x] = malloc (size * sizeof (cubie_t *));
      for (int y = 0; y < size; y++)
        {
          cube.cube[x][y] = malloc (size * sizeof (cubie_t));
          for (int z = 0; z < size; z++)
            {
              if (is_inner_cubie (x, y, z, size))
                continue;
              cube.cube[x][y][z] = cubie_make (x, y, z, cubie_size, size);
            }
        }
    }
  cube.is_animating = false;
  cube.rotation_degrees = 0;
  cube.current_rotation = (rotation_t)-1;
  return cube;
}

cube_t
cube_deep_copy (const cube_t *src)
{
  cube_t dst;
  dst.size = src->size;
  dst.cube = malloc (src->size * sizeof (cubie_t **));
  for (int x = 0; x < src->size; x++)
    {
      dst.cube[x] = malloc (src->size * sizeof (cubie_t *));
      for (int y = 0; y < src->size; y++)
        {
          dst.cube[x][y] = malloc (src->size * sizeof (cubie_t));
          memcpy (dst.cube[x][y], src->cube[x][y],
                  src->size * sizeof (cubie_t));
        }
    }
  dst.is_animating = src->is_animating;
  dst.rotation_degrees = src->rotation_degrees;
  dst.current_rotation = src->current_rotation;
  return dst;
}

void
cube_destroy (cube_t *cube)
{
  if (cube == NULL || cube->cube == NULL)
    return;
  for (int x = 0; x < cube->size; x++)
    {
      for (int y = 0; y < cube->size; y++)
        free (cube->cube[x][y]);
      free (cube->cube[x]);
    }
  free (cube->cube);
  cube->cube = NULL;
  cube->size = 0;
}

static void
step_animation (cube_t *cube, int rotation_speed)
{
  if (cube->is_animating)
    cube->rotation_degrees += rotation_speed;
  if (cube->rotation_degrees > ROTATION_DEGREES_FULL)
    {
      cube->rotation_degrees = 0;
      cube->is_animating = false;
      cube_rotate (cube, cube->current_rotation, 1);
      cube->current_rotation = (rotation_t)-1;
    }
}

/* Which layer along axis_dim is affected by a given rotation. ALL means the
 * whole cube spins, so every cubie participates. */
typedef enum
{
  LAYER_ALL,
  LAYER_INDEX_ZERO,
  LAYER_INDEX_LAST,
  LAYER_INDEX_MIDDLE,
} layer_pos_t;

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

typedef struct
{
  Vector3 axis;        /* axis direction: sign chosen for cw vs prime */
  int axis_dim;        /* AXIS_X / AXIS_Y / AXIS_Z, ignored for LAYER_ALL */
  layer_pos_t kind;
} rotation_axis_def_t;

static const rotation_axis_def_t ROTATION_AXES[24] = {
  [ROT_U]       = { {  0, -1,  0 }, AXIS_Y, LAYER_INDEX_LAST   },
  [ROT_U_PRIME] = { {  0,  1,  0 }, AXIS_Y, LAYER_INDEX_LAST   },
  [ROT_D]       = { {  0,  1,  0 }, AXIS_Y, LAYER_INDEX_ZERO   },
  [ROT_D_PRIME] = { {  0, -1,  0 }, AXIS_Y, LAYER_INDEX_ZERO   },
  [ROT_R]       = { { -1,  0,  0 }, AXIS_X, LAYER_INDEX_LAST   },
  [ROT_R_PRIME] = { {  1,  0,  0 }, AXIS_X, LAYER_INDEX_LAST   },
  [ROT_L]       = { {  1,  0,  0 }, AXIS_X, LAYER_INDEX_ZERO   },
  [ROT_L_PRIME] = { { -1,  0,  0 }, AXIS_X, LAYER_INDEX_ZERO   },
  [ROT_F]       = { {  0,  0, -1 }, AXIS_Z, LAYER_INDEX_LAST   },
  [ROT_F_PRIME] = { {  0,  0,  1 }, AXIS_Z, LAYER_INDEX_LAST   },
  [ROT_B]       = { {  0,  0,  1 }, AXIS_Z, LAYER_INDEX_ZERO   },
  [ROT_B_PRIME] = { {  0,  0, -1 }, AXIS_Z, LAYER_INDEX_ZERO   },
  [ROT_M]       = { {  1,  0,  0 }, AXIS_X, LAYER_INDEX_MIDDLE },
  [ROT_M_PRIME] = { { -1,  0,  0 }, AXIS_X, LAYER_INDEX_MIDDLE },
  [ROT_E]       = { {  0,  1,  0 }, AXIS_Y, LAYER_INDEX_MIDDLE },
  [ROT_E_PRIME] = { {  0, -1,  0 }, AXIS_Y, LAYER_INDEX_MIDDLE },
  [ROT_S]       = { {  0,  0, -1 }, AXIS_Z, LAYER_INDEX_MIDDLE },
  [ROT_S_PRIME] = { {  0,  0,  1 }, AXIS_Z, LAYER_INDEX_MIDDLE },
  [ROT_X]       = { { -1,  0,  0 }, AXIS_X, LAYER_ALL          },
  [ROT_X_PRIME] = { {  1,  0,  0 }, AXIS_X, LAYER_ALL          },
  [ROT_Y]       = { {  0, -1,  0 }, AXIS_Y, LAYER_ALL          },
  [ROT_Y_PRIME] = { {  0,  1,  0 }, AXIS_Y, LAYER_ALL          },
  [ROT_Z]       = { {  0,  0, -1 }, AXIS_Z, LAYER_ALL          },
  [ROT_Z_PRIME] = { {  0,  0,  1 }, AXIS_Z, LAYER_ALL          },
};

static bool
is_in_layer (int pos[3], int axis_dim, layer_pos_t kind, int size)
{
  switch (kind)
    {
    case LAYER_ALL:          return true;
    case LAYER_INDEX_ZERO:   return pos[axis_dim] == 0;
    case LAYER_INDEX_LAST:   return pos[axis_dim] == size - 1;
    case LAYER_INDEX_MIDDLE: return pos[axis_dim] > 0 && pos[axis_dim] < size - 1;
    }
  return false;
}

static Vector3
cubie_rotation_axis (rotation_t rotation, int pos_x, int pos_y, int pos_z,
                    int size)
{
  if (rotation < 0 || rotation >= (int)(sizeof (ROTATION_AXES) / sizeof (ROTATION_AXES[0])))
    return (Vector3){ 0, 0, 0 };
  const rotation_axis_def_t *def = &ROTATION_AXES[rotation];
  int pos[3] = { pos_x, pos_y, pos_z };
  if (!is_in_layer (pos, def->axis_dim, def->kind, size))
    return (Vector3){ 0, 0, 0 };
  return def->axis;
}

static void
draw_one_cubie (cube_t *cube, int pos_x, int pos_y, int pos_z)
{
  Vector3 position = (Vector3){
    pos_x - (float)cube->size / 2 + 0.5f,
    pos_y - (float)cube->size / 2 + 0.5f,
    pos_z - (float)cube->size / 2 + 0.5f,
  };

  Vector3 axis = cubie_rotation_axis (cube->current_rotation, pos_x, pos_y,
                                      pos_z, cube->size);
  bool axis_is_zero = (axis.x == 0 && axis.y == 0 && axis.z == 0);
  int rotation_degrees = axis_is_zero ? 0 : cube->rotation_degrees;
  cubie_draw (&cube->cube[pos_x][pos_y][pos_z], position, axis,
              rotation_degrees);
}

void
cube_draw (cube_t *cube, int rotation_speed)
{
  step_animation (cube, rotation_speed);
  for (int x = 0; x < cube->size; x++)
    for (int y = 0; y < cube->size; y++)
      for (int z = 0; z < cube->size; z++)
        {
          if (is_inner_cubie (x, y, z, cube->size))
            continue;
          draw_one_cubie (cube, x, y, z);
        }
}

bool
rotation_from_char (char c, rotation_t *out)
{
  bool prime = islower ((unsigned char)c);
  char upper = (char)toupper ((unsigned char)c);
  for (int i = 0; i < ROTATION_PAIR_COUNT; i++)
    if (ROTATION_LETTERS[i] == upper)
      {
        *out = (rotation_t)(2 * i + (prime ? 1 : 0));
        return true;
      }
  return false;
}

static char
face_letter_from_move (const char *move, size_t len)
{
  if (move[len - 1] == '\'')
    return tolower ((unsigned char)move[len - 2]);
  if (move[len - 1] == '2')
    return move[len - 2];
  return move[len - 1];
}

void
cube_apply_move (cube_t *cube, const char *move)
{
  size_t len = strlen (move);
  int num_layers;
  if (move[1] == 'w')
    num_layers = move[0] - '0';
  else
    num_layers = (move[0] - '0') * 10 + move[1] - '0';

  rotation_t r;
  if (!rotation_from_char (face_letter_from_move (move, len), &r))
    {
      LOG_WARN ("cube_apply_move: bad move '%s'", move);
      return;
    }

  if (move[len - 1] == '2')
    cube_rotate (cube, r, num_layers);
  cube_rotate (cube, r, num_layers);
}

/*----------------------------------------------------------------*/
/* Layer rotation                                                 */

static int
resolved_x (int dir_x, int i)
{
  return (dir_x == -1) ? i : dir_x;
}

static int
resolved_y (int dir_x, int dir_y, int i, int j)
{
  return (dir_y == -1) ? (dir_x == -1) ? j : i : dir_y;
}

static int
resolved_z (int dir_z, int j)
{
  return (dir_z == -1) ? j : dir_z;
}

static void
extract_face_and_rotate_cubies (cube_t *cube, Vector3 dir,
                                void (*cubie_rotation) (cubie_t *),
                                int size, cubie_t face[size][size])
{
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
      {
        int x = resolved_x (dir.x, i);
        int y = resolved_y (dir.x, dir.y, i, j);
        int z = resolved_z (dir.z, j);
        cubie_rotation (&cube->cube[x][y][z]);
        face[i][j] = cube->cube[x][y][z];
      }
}

static void
transpose_face (int size, cubie_t face[size][size])
{
  for (int i = 0; i < size; i++)
    for (int j = i + 1; j < size; j++)
      {
        cubie_t temp = face[i][j];
        face[i][j] = face[j][i];
        face[j][i] = temp;
      }
}

static void
reverse_face_rows (int size, cubie_t face[size][size])
{
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size / 2; j++)
      {
        cubie_t temp = face[i][j];
        face[i][j] = face[i][size - j - 1];
        face[i][size - j - 1] = temp;
      }
}

static void
reverse_face_columns (int size, cubie_t face[size][size])
{
  for (int j = 0; j < size; j++)
    for (int i = 0; i < size / 2; i++)
      {
        cubie_t temp = face[i][j];
        face[i][j] = face[size - i - 1][j];
        face[size - i - 1][j] = temp;
      }
}

static void
write_face_back (cube_t *cube, Vector3 dir, int size,
                 cubie_t face[size][size])
{
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
      {
        int x = resolved_x (dir.x, i);
        int y = resolved_y (dir.x, dir.y, i, j);
        int z = resolved_z (dir.z, j);
        cube->cube[x][y][z] = face[i][j];
      }
}

static void
rotate_layer (cube_t *cube, Vector3 dir,
              void (*cubie_rotation) (cubie_t *), bool anti_clockwise)
{
  int size = cube->size;
  cubie_t face[size][size];
  extract_face_and_rotate_cubies (cube, dir, cubie_rotation, size, face);
  transpose_face (size, face);
  if (anti_clockwise)
    reverse_face_rows (size, face);
  else
    reverse_face_columns (size, face);
  write_face_back (cube, dir, size, face);
}

/* How a rotation enumerates layers along its axis:
 *  - FROM_ZERO:  outer face anchored at index 0 (D, L, B). Layer i uses i.
 *  - FROM_LAST:  outer face anchored at index size-1 (U, R, F). Layer i uses
 *                size-1-i.
 *  - MIDDLE:     slice rotation (M, E, S). Iterates indices num_layers ..
 *                size-num_layers-1. */
typedef enum
{
  LAYER_ITER_FROM_ZERO,
  LAYER_ITER_FROM_LAST,
  LAYER_ITER_MIDDLE,
} layer_iter_t;

typedef struct
{
  int axis_dim;        /* AXIS_X / AXIS_Y / AXIS_Z */
  layer_iter_t iter;
  void (*cubie_rotation) (cubie_t *);
  bool anti_clockwise;
} layer_rotation_def_t;

static const layer_rotation_def_t LAYER_ROTATIONS[18] = {
  [ROT_U]       = { AXIS_Y, LAYER_ITER_FROM_LAST, cubie_rotate_left,          false },
  [ROT_U_PRIME] = { AXIS_Y, LAYER_ITER_FROM_LAST, cubie_rotate_right,         true  },
  [ROT_D]       = { AXIS_Y, LAYER_ITER_FROM_ZERO, cubie_rotate_right,         true  },
  [ROT_D_PRIME] = { AXIS_Y, LAYER_ITER_FROM_ZERO, cubie_rotate_left,          false },
  [ROT_R]       = { AXIS_X, LAYER_ITER_FROM_LAST, cubie_rotate_up,            true  },
  [ROT_R_PRIME] = { AXIS_X, LAYER_ITER_FROM_LAST, cubie_rotate_down,          false },
  [ROT_L]       = { AXIS_X, LAYER_ITER_FROM_ZERO, cubie_rotate_down,          false },
  [ROT_L_PRIME] = { AXIS_X, LAYER_ITER_FROM_ZERO, cubie_rotate_up,            true  },
  [ROT_F]       = { AXIS_Z, LAYER_ITER_FROM_LAST, cubie_rotate_clockwise,     true  },
  [ROT_F_PRIME] = { AXIS_Z, LAYER_ITER_FROM_LAST, cubie_rotate_anticlockwise, false },
  [ROT_B]       = { AXIS_Z, LAYER_ITER_FROM_ZERO, cubie_rotate_anticlockwise, false },
  [ROT_B_PRIME] = { AXIS_Z, LAYER_ITER_FROM_ZERO, cubie_rotate_clockwise,     true  },
  [ROT_M]       = { AXIS_X, LAYER_ITER_MIDDLE,    cubie_rotate_down,          false },
  [ROT_M_PRIME] = { AXIS_X, LAYER_ITER_MIDDLE,    cubie_rotate_up,            true  },
  [ROT_E]       = { AXIS_Y, LAYER_ITER_MIDDLE,    cubie_rotate_right,         true  },
  [ROT_E_PRIME] = { AXIS_Y, LAYER_ITER_MIDDLE,    cubie_rotate_left,          false },
  [ROT_S]       = { AXIS_Z, LAYER_ITER_MIDDLE,    cubie_rotate_clockwise,     true  },
  [ROT_S_PRIME] = { AXIS_Z, LAYER_ITER_MIDDLE,    cubie_rotate_anticlockwise, false },
};

/* Whole-cube rotations decompose into one face turn + one slice + one
 * opposite-face turn, all by num_layers = size/2. */
static const rotation_t WHOLE_CUBE_DECOMPOSITION[6][3] = {
  [ROT_X       - ROT_X] = { ROT_R,       ROT_M_PRIME, ROT_L_PRIME },
  [ROT_X_PRIME - ROT_X] = { ROT_R_PRIME, ROT_M,       ROT_L       },
  [ROT_Y       - ROT_X] = { ROT_U,       ROT_E_PRIME, ROT_D_PRIME },
  [ROT_Y_PRIME - ROT_X] = { ROT_U_PRIME, ROT_E,       ROT_D       },
  [ROT_Z       - ROT_X] = { ROT_F,       ROT_S,       ROT_B_PRIME },
  [ROT_Z_PRIME - ROT_X] = { ROT_F_PRIME, ROT_S_PRIME, ROT_B       },
};

static Vector3
layer_axis_vector (int axis_dim, int layer_idx)
{
  switch (axis_dim)
    {
    case AXIS_X: return (Vector3){ layer_idx, -1, -1 };
    case AXIS_Y: return (Vector3){ -1, layer_idx, -1 };
    case AXIS_Z: return (Vector3){ -1, -1, layer_idx };
    }
  return (Vector3){ -1, -1, -1 };
}

void
cube_rotate (cube_t *cube, rotation_t rotation, int num_layers)
{
  if (rotation >= ROT_X && rotation <= ROT_Z_PRIME)
    {
      const rotation_t *parts = WHOLE_CUBE_DECOMPOSITION[rotation - ROT_X];
      int half = cube->size / 2;
      for (int k = 0; k < 3; k++)
        cube_rotate (cube, parts[k], half);
      return;
    }

  if (rotation < 0 || rotation >= (int)(sizeof (LAYER_ROTATIONS) / sizeof (LAYER_ROTATIONS[0])))
    return;
  const layer_rotation_def_t *def = &LAYER_ROTATIONS[rotation];

  int size = cube->size;
  int start = (def->iter == LAYER_ITER_MIDDLE) ? num_layers : 0;
  int end = (def->iter == LAYER_ITER_MIDDLE) ? size - num_layers : num_layers;

  for (int i = start; i < end; i++)
    {
      int layer_idx = (def->iter == LAYER_ITER_FROM_LAST) ? size - i - 1 : i;
      Vector3 dir = layer_axis_vector (def->axis_dim, layer_idx);
      rotate_layer (cube, dir, def->cubie_rotation, def->anti_clockwise);
    }
}

/*----------------------------------------------------------------*/
/* Facelet string serialization                                   */

static char
face_letter_from_color (cubie_t cubie, face_t face)
{
  Color color = cubie.colors[face];
  if (colors_equal (color, WHITE))  return 'U';
  if (colors_equal (color, GREEN))  return 'F';
  if (colors_equal (color, RED))    return 'R';
  if (colors_equal (color, BLUE))   return 'B';
  if (colors_equal (color, ORANGE)) return 'L';
  if (colors_equal (color, YELLOW)) return 'D';
  return '?';
}

void
cube_to_string (const cube_t *cube, char *out, size_t out_size)
{
  if (out_size < CUBE_FACELET_STR_LEN)
    {
      if (out_size > 0)
        out[0] = '\0';
      return;
    }

  int size = cube->size;
  int idx = 0;
  for (int z = 0; z < size; z++)
    for (int x = 0; x < size; x++)
      out[idx++] = face_letter_from_color (cube->cube[x][size - 1][z],
                                           FACE_UP);
  for (int x = size - 1; x >= 0; x--)
    for (int z = size - 1; z >= 0; z--)
      out[idx++] = face_letter_from_color (cube->cube[size - 1][x][z],
                                           FACE_RIGHT);
  for (int z = size - 1; z >= 0; z--)
    for (int x = 0; x < size; x++)
      out[idx++] = face_letter_from_color (cube->cube[x][z][size - 1],
                                           FACE_FRONT);
  for (int z = size - 1; z >= 0; z--)
    for (int x = 0; x < size; x++)
      out[idx++] = face_letter_from_color (cube->cube[x][0][z], FACE_DOWN);
  for (int x = size - 1; x >= 0; x--)
    for (int z = 0; z < size; z++)
      out[idx++] = face_letter_from_color (cube->cube[0][x][z], FACE_LEFT);
  for (int z = size - 1; z >= 0; z--)
    for (int x = size - 1; x >= 0; x--)
      out[idx++] = face_letter_from_color (cube->cube[x][z][0], FACE_BACK);
  out[idx] = '\0';
}

/*----------------------------------------------------------------*/
/* Whole-cube orientation detection and normalization             */

static const Color CANONICAL_COLOR[FACE_COUNT] = {
  [FACE_UP]    = WHITE,
  [FACE_FRONT] = GREEN,
  [FACE_RIGHT] = RED,
  [FACE_BACK]  = BLUE,
  [FACE_LEFT]  = ORANGE,
  [FACE_DOWN]  = YELLOW,
};

static Color
world_center_color (const cube_t *cube, face_t face)
{
  int mid = cube->size / 2;
  int max = cube->size - 1;
  switch (face)
    {
    case FACE_UP:    return cube->cube[mid][max][mid].colors[FACE_UP];
    case FACE_DOWN:  return cube->cube[mid][0][mid].colors[FACE_DOWN];
    case FACE_FRONT: return cube->cube[mid][mid][max].colors[FACE_FRONT];
    case FACE_BACK:  return cube->cube[mid][mid][0].colors[FACE_BACK];
    case FACE_LEFT:  return cube->cube[0][mid][mid].colors[FACE_LEFT];
    case FACE_RIGHT: return cube->cube[max][mid][mid].colors[FACE_RIGHT];
    }
  return BLACK;
}

static face_t
find_world_face_showing (const cube_t *cube, Color target)
{
  static const face_t all[FACE_COUNT] = {
    FACE_UP, FACE_FRONT, FACE_RIGHT, FACE_BACK, FACE_LEFT, FACE_DOWN,
  };
  for (int i = 0; i < FACE_COUNT; i++)
    if (colors_equal (world_center_color (cube, all[i]), target))
      return all[i];
  return FACE_UP;
}

cube_orientation_t
cube_detect_orientation_and_normalize (const cube_t *src,
                                       cube_t *out_canonical)
{
  cube_orientation_t o = { .count = 0 };
  for (int f = 0; f < FACE_COUNT; f++)
    o.face_map[f] = find_world_face_showing (src, CANONICAL_COLOR[f]);

  *out_canonical = cube_deep_copy (src);

  face_t white_pos = find_world_face_showing (out_canonical, WHITE);
  switch (white_pos)
    {
    case FACE_UP:
      break;
    case FACE_FRONT:
      cube_rotate (out_canonical, ROT_X, out_canonical->size / 2);
      o.moves[o.count++] = ROT_X;
      break;
    case FACE_BACK:
      cube_rotate (out_canonical, ROT_X_PRIME, out_canonical->size / 2);
      o.moves[o.count++] = ROT_X_PRIME;
      break;
    case FACE_LEFT:
      cube_rotate (out_canonical, ROT_Z, out_canonical->size / 2);
      o.moves[o.count++] = ROT_Z;
      break;
    case FACE_RIGHT:
      cube_rotate (out_canonical, ROT_Z_PRIME, out_canonical->size / 2);
      o.moves[o.count++] = ROT_Z_PRIME;
      break;
    case FACE_DOWN:
      cube_rotate (out_canonical, ROT_X, out_canonical->size / 2);
      cube_rotate (out_canonical, ROT_X, out_canonical->size / 2);
      o.moves[o.count++] = ROT_X;
      o.moves[o.count++] = ROT_X;
      break;
    }

  face_t green_pos = find_world_face_showing (out_canonical, GREEN);
  switch (green_pos)
    {
    case FACE_FRONT:
      break;
    case FACE_RIGHT:
      cube_rotate (out_canonical, ROT_Y, out_canonical->size / 2);
      o.moves[o.count++] = ROT_Y;
      break;
    case FACE_LEFT:
      cube_rotate (out_canonical, ROT_Y_PRIME, out_canonical->size / 2);
      o.moves[o.count++] = ROT_Y_PRIME;
      break;
    case FACE_BACK:
      cube_rotate (out_canonical, ROT_Y, out_canonical->size / 2);
      cube_rotate (out_canonical, ROT_Y, out_canonical->size / 2);
      o.moves[o.count++] = ROT_Y;
      o.moves[o.count++] = ROT_Y;
      break;
    default:
      break;
    }

  return o;
}

static char
rotation_letter (rotation_t r)
{
  switch (r)
    {
    case ROT_X: case ROT_X_PRIME: return 'X';
    case ROT_Y: case ROT_Y_PRIME: return 'Y';
    case ROT_Z: case ROT_Z_PRIME: return 'Z';
    default:                       return '?';
    }
}

static bool
rotation_is_inverse (rotation_t r)
{
  return r == ROT_X_PRIME || r == ROT_Y_PRIME || r == ROT_Z_PRIME;
}

void
cube_append_normalization_tokens (char *buf, const cube_orientation_t *o)
{
  size_t len = strlen (buf);
  for (int i = 0; i < o->count;)
    {
      rotation_t r = o->moves[i];
      char letter = rotation_letter (r);
      if (i + 1 < o->count && o->moves[i + 1] == r)
        {
          buf[len++] = letter;
          buf[len++] = '2';
          buf[len++] = ' ';
          i += 2;
        }
      else
        {
          buf[len++] = letter;
          if (rotation_is_inverse (r))
            buf[len++] = '\'';
          buf[len++] = ' ';
          i++;
        }
    }
  buf[len] = '\0';
}

char
cube_face_letter (face_t face)
{
  switch (face)
    {
    case FACE_UP:    return 'U';
    case FACE_RIGHT: return 'R';
    case FACE_FRONT: return 'F';
    case FACE_DOWN:  return 'D';
    case FACE_LEFT:  return 'L';
    case FACE_BACK:  return 'B';
    }
  return '?';
}

void
cube_rotation_token (rotation_t r, char out[3])
{
  out[0] = ROTATION_LETTERS[r / 2];
  if (r % 2 == 1)
    {
      out[1] = '\'';
      out[2] = '\0';
    }
  else
    out[1] = '\0';
}
