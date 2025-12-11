#ifndef FACECUBE_H
#define FACECUBE_H

#include "cubieCube.h"
#include "enums.h"

typedef struct FaceCube {
  Face facelets[54];
} FaceCube;

FaceCube FaceCube_make(const char *cubeString);

CubieCube FaceCube_toCubieCube(FaceCube *faceCube);

#endif // !FACECUBE_H
