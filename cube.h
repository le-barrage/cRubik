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
  cubie_t ***cube;
  bool isAnimating;
  Rotation currentRotation;
  int rotationDegrees;
} Cube;

typedef struct {
  Rotation moves[4];
  int count;
  Face faceMap[6];
} CubeOrientation;

Cube Cube_make(float cubletSize);

Cube Cube_deepCopy(Cube *src);

void Cube_free(Cube cube);

void Cube_drawCube(Cube *cube);

Rotation getCorrespondingRotation(char c);

void Cube_applyMove(Cube *cube, char *move);

void Cube_rotate(Cube *cube, Rotation rotation, int numberOfLayers);

char *Cube_toString(Cube *cube, char cubeStr[55]);

CubeOrientation Cube_detectOrientationAndNormalize(Cube *src,
                                                   Cube *outCanonical);

void Cube_appendNormalizationTokens(char *buf, const CubeOrientation *o);

char Cube_faceLetter(Face f);

void Cube_rotationToken(Rotation r, char out[3]);

#endif // !CUBE_H
