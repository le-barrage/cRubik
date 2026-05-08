#ifndef CUBIECUBE_H
#define CUBIECUBE_H

#include "enums.h"

typedef struct {
  Corner cornerPermutation[8];
  char cornerOrientation[8];
  Edge edgePermutation[12];
  char edgeOrientation[12];
} CubieCube;

extern CubieCube moveCube[6];

CubieCube CubieCube_make();
void initialize_moveCube();
void rotateLeftCorners(Corner *arr, int l, int r);
void rotateRightCorners(Corner *arr, int l, int r);
void rotateLeftEdges(Edge *arr, int l, int r);
void rotateRightEdges(Edge *arr, int l, int r);
void CubieCube_cornerMultiply(CubieCube *a, CubieCube *b);
void CubieCube_edgeMultiply(CubieCube *a, CubieCube *b);
void CubieCube_multiply(CubieCube *a, CubieCube *b);
CubieCube getInvCubieCube(CubieCube *cubieCube);
short getTwist(CubieCube *cubieCube);
void setTwist(CubieCube *cubieCube, short twist);
short getFlip(CubieCube *cubieCube);
void setFlip(CubieCube *cubieCube, short flip);
short cornerParity(CubieCube *cubieCube);
short edgeParity(CubieCube *cubieCube);
short getFRtoBR(CubieCube *cubieCube);
void setFRtoBR(CubieCube *cubieCube, short idx);
short getURFtoDLF(CubieCube *cubieCube);
void setURFtoDLF(CubieCube *cubieCube, short idx);
int getURtoDF(CubieCube *cubieCube);
void setURtoDF(CubieCube *cubieCube, int idx);
short getURtoUL(CubieCube *cubieCube);
void setURtoUL(CubieCube *cubieCube, short idx);
short getUBtoDF(CubieCube *cubieCube);
void setUBtoDF(CubieCube *cubieCube, short idx);
int getURtoDFWithIndices(short idx1, short idx2);
int verify(CubieCube *cubieCube);

#endif // !CUBIECUBE_H
