#include "cublet.h"
#include "include/raylib.h"
#include "include/raymath.h"
#include "include/rlgl.h"
#include "utils.h"

Cubie
Cubie_make (int x, int y, int z, float sideLength, int size)
{
  return (Cubie){ .position = (Vector3){ .x = x, .y = y, .z = z },
                  .colors
                  = { (y == size - 1) ? WHITE : BLACK,
                      (z == size - 1) ? GREEN : BLACK,
                      (x == size - 1) ? RED : BLACK, (z == 0) ? BLUE : BLACK,
                      (x == 0) ? ORANGE : BLACK, (y == 0) ? YELLOW : BLACK },
                  .sideLength = sideLength };
}

void
Cubie_rotateRight (Cubie *cubie)
{
  Color tmp = cubie->colors[FRONT];
  cubie->colors[FRONT] = cubie->colors[LEFT];
  cubie->colors[LEFT] = cubie->colors[BACK];
  cubie->colors[BACK] = cubie->colors[RIGHT];
  cubie->colors[RIGHT] = tmp;
}

void
Cubie_rotateLeft (Cubie *cubie)
{
  Color tmp = cubie->colors[FRONT];
  cubie->colors[FRONT] = cubie->colors[RIGHT];
  cubie->colors[RIGHT] = cubie->colors[BACK];
  cubie->colors[BACK] = cubie->colors[LEFT];
  cubie->colors[LEFT] = tmp;
}

void
Cubie_rotateUp (Cubie *cubie)
{
  Color tmp = cubie->colors[FRONT];
  cubie->colors[FRONT] = cubie->colors[DOWN];
  cubie->colors[DOWN] = cubie->colors[BACK];
  cubie->colors[BACK] = cubie->colors[UP];
  cubie->colors[UP] = tmp;
}

void
Cubie_rotateDown (Cubie *cubie)
{
  Color tmp = cubie->colors[FRONT];
  cubie->colors[FRONT] = cubie->colors[UP];
  cubie->colors[UP] = cubie->colors[BACK];
  cubie->colors[BACK] = cubie->colors[DOWN];
  cubie->colors[DOWN] = tmp;
}

void
Cubie_rotateClockWise (Cubie *cubie)
{
  Color tmp = cubie->colors[UP];
  cubie->colors[UP] = cubie->colors[LEFT];
  cubie->colors[LEFT] = cubie->colors[DOWN];
  cubie->colors[DOWN] = cubie->colors[RIGHT];
  cubie->colors[RIGHT] = tmp;
}

void
Cubie_rotateAntiClockWise (Cubie *cubie)
{
  Color tmp = cubie->colors[UP];
  cubie->colors[UP] = cubie->colors[RIGHT];
  cubie->colors[RIGHT] = cubie->colors[DOWN];
  cubie->colors[DOWN] = cubie->colors[LEFT];
  cubie->colors[LEFT] = tmp;
}

void
Cubie_drawCubie (Cubie *cubie, Vector3 position, Vector3 rotationAxis,
                 float rotationAngle)
{
  const float s = cubie->sideLength / 2.0f;

  const Vector3 faceVertices[6][4]
      = { // UP
          { { -s, s, -s }, { -s, s, s }, { s, s, s }, { s, s, -s } },
          // FRONT
          { { -s, -s, s }, { s, -s, s }, { s, s, s }, { -s, s, s } },
          // RIGHT
          { { s, -s, -s }, { s, s, -s }, { s, s, s }, { s, -s, s } },
          // BACK
          { { -s, -s, -s }, { -s, s, -s }, { s, s, -s }, { s, -s, -s } },
          // LEFT
          { { -s, -s, -s }, { -s, -s, s }, { -s, s, s }, { -s, s, -s } },
          // DOWN
          { { -s, -s, -s }, { s, -s, -s }, { s, -s, s }, { -s, -s, s } }
        };

  rlPushMatrix ();
  rlTranslatef (rotationAxis.x, rotationAxis.y, rotationAxis.z);
  rlRotatef (rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
  rlTranslatef (position.x - rotationAxis.x, position.y - rotationAxis.y,
                position.z - rotationAxis.z);

  // === Draw faces ===
  rlBegin (RL_TRIANGLES);
  for (Face face = 0; face < 6; face++)
    {
      Color color = cubie->colors[face];
      rlColor4ub (color.r, color.g, color.b, color.a);

      const Vector3 *v = faceVertices[face];
      rlVertex3f (v[0].x, v[0].y, v[0].z);
      rlVertex3f (v[1].x, v[1].y, v[1].z);
      rlVertex3f (v[3].x, v[3].y, v[3].z);

      rlVertex3f (v[1].x, v[1].y, v[1].z);
      rlVertex3f (v[2].x, v[2].y, v[2].z);
      rlVertex3f (v[3].x, v[3].y, v[3].z);
    }
  rlEnd ();

  // === Draw borders ===
  rlBegin (RL_LINES);
  rlColor4ub (BLACK.r, BLACK.g, BLACK.b, BLACK.a);
  rlSetLineWidth (3);

  for (int face = 0; face < 6; face++)
    {
      const Vector3 *v = faceVertices[face];
      for (int i = 0; i < 4; i++)
        {
          const Vector3 a = v[i];
          const Vector3 b = v[(i + 1) % 4];
          rlVertex3f (a.x, a.y, a.z);
          rlVertex3f (b.x, b.y, b.z);
        }
    }
  rlEnd ();

  rlPopMatrix ();
}

char
Cubie_getColor (Cubie *cubie, Face face)
{
  if (colorsEqual (cubie->colors[face], WHITE))
    return 'W';
  else if (colorsEqual (cubie->colors[face], GREEN))
    return 'G';
  else if (colorsEqual (cubie->colors[face], RED))
    return 'R';
  else if (colorsEqual (cubie->colors[face], BLUE))
    return 'B';
  else if (colorsEqual (cubie->colors[face], ORANGE))
    return 'O';
  else if (colorsEqual (cubie->colors[face], YELLOW))
    return 'Y';
  return 'X';
}
