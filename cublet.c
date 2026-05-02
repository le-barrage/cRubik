#include "cublet.h"

#include "include/rlgl.h"

#define VERTICES_PER_FACE 4
#define BORDER_LINE_WIDTH 3

cubie_t
cubie_make (int x, int y, int z, float side_length, int size)
{
  return (cubie_t){
    .colors = {
      [UP]    = (y == size - 1) ? WHITE  : BLACK,
      [FRONT] = (z == size - 1) ? GREEN  : BLACK,
      [RIGHT] = (x == size - 1) ? RED    : BLACK,
      [BACK]  = (z == 0)        ? BLUE   : BLACK,
      [LEFT]  = (x == 0)        ? ORANGE : BLACK,
      [DOWN]  = (y == 0)        ? YELLOW : BLACK,
    },
    .side_length = side_length,
  };
}

static void
cycle_colors (Color colors[FACE_COUNT], Face face_a, Face face_b, Face face_c,
              Face face_d)
{
  Color tmp = colors[face_a];
  colors[face_a] = colors[face_b];
  colors[face_b] = colors[face_c];
  colors[face_c] = colors[face_d];
  colors[face_d] = tmp;
}

void
cubie_rotate_right (cubie_t *cubie)
{
  cycle_colors (cubie->colors, FRONT, LEFT, BACK, RIGHT);
}

void
cubie_rotate_left (cubie_t *cubie)
{
  cycle_colors (cubie->colors, FRONT, RIGHT, BACK, LEFT);
}

void
cubie_rotate_up (cubie_t *cubie)
{
  cycle_colors (cubie->colors, FRONT, DOWN, BACK, UP);
}

void
cubie_rotate_down (cubie_t *cubie)
{
  cycle_colors (cubie->colors, FRONT, UP, BACK, DOWN);
}

void
cubie_rotate_clockwise (cubie_t *cubie)
{
  cycle_colors (cubie->colors, UP, LEFT, DOWN, RIGHT);
}

void
cubie_rotate_anticlockwise (cubie_t *cubie)
{
  cycle_colors (cubie->colors, UP, RIGHT, DOWN, LEFT);
}

static void
draw_faces (const cubie_t *cubie,
            const Vector3 verts[FACE_COUNT][VERTICES_PER_FACE])
{
  rlBegin (RL_TRIANGLES);
  for (Face face = 0; face < FACE_COUNT; face++)
    {
      Color color = cubie->colors[face];
      rlColor4ub (color.r, color.g, color.b, color.a);

      const Vector3 *v = verts[face];
      rlVertex3f (v[0].x, v[0].y, v[0].z);
      rlVertex3f (v[1].x, v[1].y, v[1].z);
      rlVertex3f (v[3].x, v[3].y, v[3].z);

      rlVertex3f (v[1].x, v[1].y, v[1].z);
      rlVertex3f (v[2].x, v[2].y, v[2].z);
      rlVertex3f (v[3].x, v[3].y, v[3].z);
    }
  rlEnd ();
}

static void
draw_borders (const Vector3 verts[FACE_COUNT][VERTICES_PER_FACE])
{
  rlBegin (RL_LINES);
  rlColor4ub (BLACK.r, BLACK.g, BLACK.b, BLACK.a);
  rlSetLineWidth (BORDER_LINE_WIDTH);

  for (int face = 0; face < FACE_COUNT; face++)
    {
      const Vector3 *v = verts[face];
      for (int i = 0; i < VERTICES_PER_FACE; i++)
        {
          const Vector3 a = v[i];
          const Vector3 b = v[(i + 1) % VERTICES_PER_FACE];
          rlVertex3f (a.x, a.y, a.z);
          rlVertex3f (b.x, b.y, b.z);
        }
    }
  rlEnd ();
}

void
cubie_draw (cubie_t *cubie, Vector3 position, Vector3 rotation_axis,
            float rotation_angle)
{
  const float s = cubie->side_length / 2.0f;
  const Vector3 verts[FACE_COUNT][VERTICES_PER_FACE] = {
    [UP]    = { { -s,  s, -s }, { -s,  s,  s }, {  s,  s,  s }, {  s,  s, -s } },
    [FRONT] = { { -s, -s,  s }, {  s, -s,  s }, {  s,  s,  s }, { -s,  s,  s } },
    [RIGHT] = { {  s, -s, -s }, {  s,  s, -s }, {  s,  s,  s }, {  s, -s,  s } },
    [BACK]  = { { -s, -s, -s }, { -s,  s, -s }, {  s,  s, -s }, {  s, -s, -s } },
    [LEFT]  = { { -s, -s, -s }, { -s, -s,  s }, { -s,  s,  s }, { -s,  s, -s } },
    [DOWN]  = { { -s, -s, -s }, {  s, -s, -s }, {  s, -s,  s }, { -s, -s,  s } },
  };

  rlPushMatrix ();
  rlTranslatef (rotation_axis.x, rotation_axis.y, rotation_axis.z);
  rlRotatef (rotation_angle, rotation_axis.x, rotation_axis.y, rotation_axis.z);
  rlTranslatef (position.x - rotation_axis.x, position.y - rotation_axis.y,
                position.z - rotation_axis.z);

  draw_faces (cubie, verts);
  draw_borders (verts);

  rlPopMatrix ();
}
