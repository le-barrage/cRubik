#include "faceCube.h"
#include <assert.h>
#include <string.h>

Facelet cornerFacelet[8][3] = {{U9, R1, F3}, {U7, F1, L3}, {U1, L1, B3},
                               {U3, B1, R3}, {D3, F9, R7}, {D1, L9, F7},
                               {D7, B9, L7}, {D9, R9, B7}};
Facelet edgeFacelet[12][2] = {{U6, R2}, {U8, F2}, {U4, L2}, {U2, B2},
                              {D6, R8}, {D2, F8}, {D4, L8}, {D8, B8},
                              {F6, R4}, {F4, L6}, {B6, L4}, {B4, R6}};

Face cornerColor[8][3] = {{UP, RIGHT, FRONT},   {UP, FRONT, LEFT},
                          {UP, LEFT, BACK},     {UP, BACK, RIGHT},
                          {DOWN, FRONT, RIGHT}, {DOWN, LEFT, FRONT},
                          {DOWN, BACK, LEFT},   {DOWN, RIGHT, BACK}};

Face edgeColor[12][3] = {{UP, RIGHT},   {UP, FRONT},   {UP, LEFT},
                         {UP, BACK},    {DOWN, RIGHT}, {DOWN, FRONT},
                         {DOWN, LEFT},  {DOWN, BACK},  {FRONT, RIGHT},
                         {FRONT, LEFT}, {BACK, LEFT},  {BACK, RIGHT}};

FaceCube FaceCube_make(const char *cubeString) {
  assert(strlen(cubeString) == 54);
  FaceCube faceCube;
  for (int i = 0; i < 54; i++)
    faceCube.facelets[i] = getCorrespondingColor(cubeString[i]);
  return faceCube;
}

CubieCube FaceCube_toCubieCube(FaceCube *faceCube) {
  CubieCube cubieCubeToReturn = CubieCube_make();
  for (int i = 0; i < 8; i++)
    cubieCubeToReturn.cornerPermutation[i] = URF;
  for (int i = 0; i < 12; i++)
    cubieCubeToReturn.edgePermutation[i] = UR;
  Face color1, color2;
  short ori;
  for (Corner corner = URF; corner <= DRB; corner++) {
    for (ori = 0; ori < 3; ori++)
      if (faceCube->facelets[cornerFacelet[corner][ori]] == UP ||
          faceCube->facelets[cornerFacelet[corner][ori]] == DOWN)
        break;
    color1 = faceCube->facelets[cornerFacelet[corner][(ori + 1) % 3]];
    color2 = faceCube->facelets[cornerFacelet[corner][(ori + 2) % 3]];
    for (Corner j = URF; corner <= DRB; j++) {
      if (color1 == cornerColor[j][1] && color2 == cornerColor[j][2]) {
        cubieCubeToReturn.cornerPermutation[corner] = j;
        cubieCubeToReturn.cornerOrientation[corner] = (char)(ori % 3);
        break;
      }
    }
  }
  for (Edge edge = UR; edge <= BR; edge++)
    for (Edge j = UR; edge <= BR; j++) {
      if (faceCube->facelets[edgeFacelet[edge][0]] == edgeColor[j][0] &&
          faceCube->facelets[edgeFacelet[edge][1]] == edgeColor[j][1]) {
        cubieCubeToReturn.edgePermutation[edge] = j;
        cubieCubeToReturn.edgeOrientation[edge] = 0;
        break;
      }
      if (faceCube->facelets[edgeFacelet[edge][0]] == edgeColor[j][1] &&
          faceCube->facelets[edgeFacelet[edge][1]] == edgeColor[j][0]) {
        cubieCubeToReturn.edgePermutation[edge] = j;
        cubieCubeToReturn.edgeOrientation[edge] = 1;
        break;
      }
    }
  return cubieCubeToReturn;
}
