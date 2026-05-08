#include "cublet.h"

#include "rlgl.h"

#define VERTICES_PER_FACE 4
#define BORDER_LINE_WIDTH 3

cubie_t
cubie_make (int x, int y, int z, float side_length, int size)
{
  return (cubie_t){
    .colors = {
      [FACE_UP]    = (y == size - 1) ? WHITE  : BLACK,
      [FACE_FRONT] = (z == size - 1) ? GREEN  : BLACK,
      [FACE_RIGHT] = (x == size - 1) ? RED    : BLACK,
      [FACE_BACK]  = (z == 0)        ? BLUE   : BLACK,
      [FACE_LEFT]  = (x == 0)        ? ORANGE : BLACK,
      [FACE_DOWN]  = (y == 0)        ? YELLOW : BLACK,
    },
    .side_length = side_length,
  };
}

static void
cycle_colors (Color colors[FACE_COUNT], face_t face_a, face_t face_b, face_t face_c,
              face_t face_d)
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
  cycle_colors (cubie->colors, FACE_FRONT, FACE_LEFT, FACE_BACK, FACE_RIGHT);
}

void
cubie_rotate_left (cubie_t *cubie)
{
  cycle_colors (cubie->colors, FACE_FRONT, FACE_RIGHT, FACE_BACK, FACE_LEFT);
}

void
cubie_rotate_up (cubie_t *cubie)
{
  cycle_colors (cubie->colors, FACE_FRONT, FACE_DOWN, FACE_BACK, FACE_UP);
}

void
cubie_rotate_down (cubie_t *cubie)
{
  cycle_colors (cubie->colors, FACE_FRONT, FACE_UP, FACE_BACK, FACE_DOWN);
}

void
cubie_rotate_clockwise (cubie_t *cubie)
{
  cycle_colors (cubie->colors, FACE_UP, FACE_LEFT, FACE_DOWN, FACE_RIGHT);
}

void
cubie_rotate_anticlockwise (cubie_t *cubie)
{
  cycle_colors (cubie->colors, FACE_UP, FACE_RIGHT, FACE_DOWN, FACE_LEFT);
}

static void
draw_faces (const cubie_t *cubie,
            const Vector3 verts[FACE_COUNT][VERTICES_PER_FACE])
{
  rlBegin (RL_TRIANGLES);
  for (face_t face = 0; face < FACE_COUNT; face++)
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
    [FACE_UP]    = { { -s,  s, -s }, { -s,  s,  s }, {  s,  s,  s }, {  s,  s, -s } },
    [FACE_FRONT] = { { -s, -s,  s }, {  s, -s,  s }, {  s,  s,  s }, { -s,  s,  s } },
    [FACE_RIGHT] = { {  s, -s, -s }, {  s,  s, -s }, {  s,  s,  s }, {  s, -s,  s } },
    [FACE_BACK]  = { { -s, -s, -s }, { -s,  s, -s }, {  s,  s, -s }, {  s, -s, -s } },
    [FACE_LEFT]  = { { -s, -s, -s }, { -s, -s,  s }, { -s,  s,  s }, { -s,  s, -s } },
    [FACE_DOWN]  = { { -s, -s, -s }, {  s, -s, -s }, {  s, -s,  s }, { -s, -s,  s } },
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
