#include "cublet.h"
#include "include/raylib.h"
#include "include/rlgl.h"
#include "utils.h"

Cubie Cubie_make(int x, int y, int z, float sideLength, int size) {
  return (Cubie){.position = (Vector3){.x = x, .y = y, .z = z},
                 .colors = {(y == size - 1) ? WHITE : BLACK,
                            (z == size - 1) ? GREEN : BLACK,
                            (x == size - 1) ? RED : BLACK,
                            (z == 0) ? BLUE : BLACK, (x == 0) ? ORANGE : BLACK,
                            (y == 0) ? YELLOW : BLACK},
                 .sideLength = sideLength};
}

void Cubie_rotateRight(Cubie *cubie) {
  Color tmp = cubie->colors[FRONT];
  cubie->colors[FRONT] = cubie->colors[LEFT];
  cubie->colors[LEFT] = cubie->colors[BACK];
  cubie->colors[BACK] = cubie->colors[RIGHT];
  cubie->colors[RIGHT] = tmp;
}

void Cubie_rotateLeft(Cubie *cubie) {
  Color tmp = cubie->colors[FRONT];
  cubie->colors[FRONT] = cubie->colors[RIGHT];
  cubie->colors[RIGHT] = cubie->colors[BACK];
  cubie->colors[BACK] = cubie->colors[LEFT];
  cubie->colors[LEFT] = tmp;
}

void Cubie_rotateUp(Cubie *cubie) {
  Color tmp = cubie->colors[FRONT];
  cubie->colors[FRONT] = cubie->colors[DOWN];
  cubie->colors[DOWN] = cubie->colors[BACK];
  cubie->colors[BACK] = cubie->colors[UP];
  cubie->colors[UP] = tmp;
}

void Cubie_rotateDown(Cubie *cubie) {
  Color tmp = cubie->colors[FRONT];
  cubie->colors[FRONT] = cubie->colors[UP];
  cubie->colors[UP] = cubie->colors[BACK];
  cubie->colors[BACK] = cubie->colors[DOWN];
  cubie->colors[DOWN] = tmp;
}

void Cubie_rotateClockWise(Cubie *cubie) {
  Color tmp = cubie->colors[UP];
  cubie->colors[UP] = cubie->colors[LEFT];
  cubie->colors[LEFT] = cubie->colors[DOWN];
  cubie->colors[DOWN] = cubie->colors[RIGHT];
  cubie->colors[RIGHT] = tmp;
}

void Cubie_rotateAntiClockWise(Cubie *cubie) {
  Color tmp = cubie->colors[UP];
  cubie->colors[UP] = cubie->colors[RIGHT];
  cubie->colors[RIGHT] = cubie->colors[DOWN];
  cubie->colors[DOWN] = cubie->colors[LEFT];
  cubie->colors[LEFT] = tmp;
}

void Cubie_drawCubie(Cubie *cubie, Vector3 position, Vector3 rotationAxis,
                     float rotationAngle) {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;

  rlPushMatrix();
  rlTranslatef(rotationAxis.x, rotationAxis.y, rotationAxis.z);

  rlRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

  rlTranslatef(position.x - rotationAxis.x, position.y - rotationAxis.y,
               position.z - rotationAxis.z);

  rlBegin(RL_TRIANGLES);

  // Front face
  Color color = cubie->colors[FRONT];
  rlColor4ub(color.r, color.g, color.b, color.a);
  rlVertex3f(x - cubie->sideLength / 2, y - cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Left
  rlVertex3f(x + cubie->sideLength / 2, y - cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Right
  rlVertex3f(x - cubie->sideLength / 2, y + cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Top Left

  rlVertex3f(x + cubie->sideLength / 2, y + cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Top Right
  rlVertex3f(x - cubie->sideLength / 2, y + cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Top Left
  rlVertex3f(x + cubie->sideLength / 2, y - cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Right

  // Back face
  color = cubie->colors[BACK];
  rlColor4ub(color.r, color.g, color.b, color.a);
  rlVertex3f(x - cubie->sideLength / 2, y - cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Bottom Left
  rlVertex3f(x - cubie->sideLength / 2, y + cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Left
  rlVertex3f(x + cubie->sideLength / 2, y - cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Bottom Right

  rlVertex3f(x + cubie->sideLength / 2, y + cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Right
  rlVertex3f(x + cubie->sideLength / 2, y - cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Bottom Right
  rlVertex3f(x - cubie->sideLength / 2, y + cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Left

  // Top face
  color = cubie->colors[UP];
  rlColor4ub(color.r, color.g, color.b, color.a);
  rlVertex3f(x - cubie->sideLength / 2, y + cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Left
  rlVertex3f(x - cubie->sideLength / 2, y + cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Left
  rlVertex3f(x + cubie->sideLength / 2, y + cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Right

  rlVertex3f(x + cubie->sideLength / 2, y + cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Right
  rlVertex3f(x - cubie->sideLength / 2, y + cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Left
  rlVertex3f(x + cubie->sideLength / 2, y + cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Right

  // Bottom face
  color = cubie->colors[DOWN];
  rlColor4ub(color.r, color.g, color.b, color.a);
  rlVertex3f(x - cubie->sideLength / 2, y - cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Left
  rlVertex3f(x + cubie->sideLength / 2, y - cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Right
  rlVertex3f(x - cubie->sideLength / 2, y - cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Left

  rlVertex3f(x + cubie->sideLength / 2, y - cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Right
  rlVertex3f(x + cubie->sideLength / 2, y - cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Right
  rlVertex3f(x - cubie->sideLength / 2, y - cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Left

  // Right face
  color = cubie->colors[RIGHT];
  rlColor4ub(color.r, color.g, color.b, color.a);
  rlVertex3f(x + cubie->sideLength / 2, y - cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Bottom Right
  rlVertex3f(x + cubie->sideLength / 2, y + cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Right
  rlVertex3f(x + cubie->sideLength / 2, y + cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Top Left

  rlVertex3f(x + cubie->sideLength / 2, y - cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Left
  rlVertex3f(x + cubie->sideLength / 2, y - cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Bottom Right
  rlVertex3f(x + cubie->sideLength / 2, y + cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Top Left

  // Left face
  color = cubie->colors[LEFT];
  rlColor4ub(color.r, color.g, color.b, color.a);
  rlVertex3f(x - cubie->sideLength / 2, y - cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Bottom Right
  rlVertex3f(x - cubie->sideLength / 2, y + cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Top Left
  rlVertex3f(x - cubie->sideLength / 2, y + cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Top Right

  rlVertex3f(x - cubie->sideLength / 2, y - cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Bottom Left
  rlVertex3f(x - cubie->sideLength / 2, y + cubie->sideLength / 2,
             z + cubie->sideLength / 2); // Top Left
  rlVertex3f(x - cubie->sideLength / 2, y - cubie->sideLength / 2,
             z - cubie->sideLength / 2); // Bottom Right

  rlEnd();
  rlPopMatrix();
}

char Cubie_getColor(Cubie *cubie, Face face) {
  if (colorsEqual(cubie->colors[face], WHITE))
    return 'W';
  else if (colorsEqual(cubie->colors[face], GREEN))
    return 'G';
  else if (colorsEqual(cubie->colors[face], RED))
    return 'R';
  else if (colorsEqual(cubie->colors[face], BLUE))
    return 'B';
  else if (colorsEqual(cubie->colors[face], ORANGE))
    return 'O';
  else if (colorsEqual(cubie->colors[face], YELLOW))
    return 'Y';
  return 'X';
}
