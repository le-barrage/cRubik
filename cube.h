#ifndef CUBE_H
#define CUBE_H

#include "cublet.h"

extern int SIZE, MAXSIZE, ROTATIONSPEED;

typedef enum Rotations {
  U,
  u,
  D,
  d,
  R,
  r,
  L,
  l,
  F,
  f,
  B,
  b,
  M,
  m,
  E,
  e,
  S,
  s,
  X,
  x,
  Y,
  y,
  Z,
  z
} Rotation;

typedef struct Cube {
  Cubie ***cube;
  bool isAnimating;
  Rotation currentRotation;
  int rotationDegrees;
} Cube;

Cube Cube_make(float cubletSize);

void Cube_free(Cube cube);

void Cube_drawCube(Cube *cube);

Rotation getCorrespondingRotation(char c);

void Cube_applyMove(Cube *cube, char *move);

void Cube_rotate(Cube *cube, Rotation rotation, int numberOfLayers);

char *Cube_toString(Cube *cube, char cubeStr[55]);

#endif // !CUBE_H
