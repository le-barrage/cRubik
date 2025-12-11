#include "cubieCube.h"
#include "../utils.h"

const Corner cornerPermutationU[] = {UBR, URF, UFL, ULB, DFR, DLF, DBL, DRB};
const char cornerOrientationU[] = {0, 0, 0, 0, 0, 0, 0, 0};
const Edge edgePermutationU[] = {UB, UR, UF, UL, DR, DF,
                                 DL, DB, FR, FL, BL, BR};
const char edgeOrientationU[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const Corner cornerPermutationR[] = {DFR, UFL, ULB, URF, DRB, DLF, DBL, UBR};
const char cornerOrientationR[] = {2, 0, 0, 1, 1, 0, 0, 2};
const Edge edgePermutationR[] = {FR, UF, UL, UB, BR, DF,
                                 DL, DB, DR, FL, BL, UR};
const char edgeOrientationR[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const Corner cornerPermutationF[] = {UFL, DLF, ULB, UBR, URF, DFR, DBL, DRB};
const char cornerOrientationF[] = {1, 2, 0, 0, 2, 1, 0, 0};
const Edge edgePermutationF[] = {UR, FL, UL, UB, DR, FR,
                                 DL, DB, UF, DF, BL, BR};
const char edgeOrientationF[] = {0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0};

const Corner cornerPermutationD[] = {URF, UFL, ULB, UBR, DLF, DBL, DRB, DFR};
const char cornerOrientationD[] = {0, 0, 0, 0, 0, 0, 0, 0};
const Edge edgePermutationD[] = {UR, UF, UL, UB, DF, DL,
                                 DB, DR, FR, FL, BL, BR};
const char edgeOrientationD[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const Corner cornerPermutationL[] = {URF, ULB, DBL, UBR, DFR, UFL, DLF, DRB};
const char cornerOrientationL[] = {0, 1, 2, 0, 0, 2, 1, 0};
const Edge edgePermutationL[] = {UR, UF, BL, UB, DR, DF,
                                 FL, DB, FR, UL, DL, BR};
const char edgeOrientationL[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const Corner cornerPermutationB[] = {URF, UFL, UBR, DRB, DFR, DLF, ULB, DBL};
const char cornerOrientationB[] = {0, 0, 1, 2, 0, 0, 2, 1};
const Edge edgePermutationB[] = {UR, UF, UL, BR, DR, DF,
                                 DL, BL, FR, FL, UB, DB};
const char edgeOrientationB[] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1};

/*-------------------------------------------------------------------------------------*/

CubieCube moveCube[6];

CubieCube CubieCube_make() {
  return (CubieCube){
      .cornerPermutation = {URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB},
      .cornerOrientation = {0, 0, 0, 0, 0, 0, 0, 0},
      .edgePermutation = {UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR},
      .edgeOrientation = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
}

void initialize_moveCube() {
  for (int i = 0; i < 8; i++) {
    moveCube[0].cornerPermutation[i] = cornerPermutationU[i];
    moveCube[0].cornerOrientation[i] = cornerOrientationU[i];

    moveCube[1].cornerPermutation[i] = cornerPermutationR[i];
    moveCube[1].cornerOrientation[i] = cornerOrientationR[i];

    moveCube[2].cornerPermutation[i] = cornerPermutationF[i];
    moveCube[2].cornerOrientation[i] = cornerOrientationF[i];

    moveCube[3].cornerPermutation[i] = cornerPermutationD[i];
    moveCube[3].cornerOrientation[i] = cornerOrientationD[i];

    moveCube[4].cornerPermutation[i] = cornerPermutationL[i];
    moveCube[4].cornerOrientation[i] = cornerOrientationL[i];

    moveCube[5].cornerPermutation[i] = cornerPermutationB[i];
    moveCube[5].cornerOrientation[i] = cornerOrientationB[i];
  }
  for (int i = 0; i < 12; i++) {
    moveCube[0].edgePermutation[i] = edgePermutationU[i];
    moveCube[0].edgeOrientation[i] = edgeOrientationU[i];

    moveCube[1].edgePermutation[i] = edgePermutationR[i];
    moveCube[1].edgeOrientation[i] = edgeOrientationR[i];

    moveCube[2].edgePermutation[i] = edgePermutationF[i];
    moveCube[2].edgeOrientation[i] = edgeOrientationF[i];

    moveCube[3].edgePermutation[i] = edgePermutationD[i];
    moveCube[3].edgeOrientation[i] = edgeOrientationD[i];

    moveCube[4].edgePermutation[i] = edgePermutationL[i];
    moveCube[4].edgeOrientation[i] = edgeOrientationL[i];

    moveCube[5].edgePermutation[i] = edgePermutationB[i];
    moveCube[5].edgeOrientation[i] = edgeOrientationB[i];
  }
}

void rotateLeftCorners(Corner *arr, int l, int r) {
  Corner temp = arr[l];
  for (int i = l; i < r; i++)
    arr[i] = arr[i + 1];
  arr[r] = temp;
}

void rotateRightCorners(Corner *arr, int l, int r) {
  Corner temp = arr[r];
  for (int i = r; i > l; i--)
    arr[i] = arr[i - 1];
  arr[l] = temp;
}

void rotateLeftEdges(Edge *arr, int l, int r) {
  Edge temp = arr[l];
  for (int i = l; i < r; i++)
    arr[i] = arr[i + 1];
  arr[r] = temp;
}

void rotateRightEdges(Edge *arr, int l, int r) {
  Edge temp = arr[r];
  for (int i = r; i > l; i--)
    arr[i] = arr[i - 1];
  arr[l] = temp;
}

void CubieCube_cornerMultiply(CubieCube *a, CubieCube *b) {
  Corner cPerm[8] = {0};
  char cOri[8] = {0};
  for (int corner = 0; corner < 8; corner++) {
    cPerm[corner] = a->cornerPermutation[b->cornerPermutation[corner]];

    char oriA = a->cornerOrientation[b->cornerPermutation[corner]];
    char oriB = b->cornerOrientation[corner];
    char ori = 0;
    if (oriA < 3 && oriB < 3) {
      ori = (char)(oriA + oriB);
      if (ori >= 3)
        ori -= 3;
    }
    cOri[corner] = ori;
  }
  for (int c = 0; c < 8; c++) {
    a->cornerPermutation[c] = cPerm[c];
    a->cornerOrientation[c] = cOri[c];
  }
}

void CubieCube_edgeMultiply(CubieCube *a, CubieCube *b) {
  Edge ePerm[12];
  char eOri[12];
  for (int edge = 0; edge < 12; edge++) {
    ePerm[edge] = a->edgePermutation[b->edgePermutation[edge]];
    eOri[edge] = (char)((b->edgeOrientation[edge] +
                         a->edgeOrientation[b->edgePermutation[edge]]) %
                        2);
  }
  for (int e = 0; e < 12; e++) {
    a->edgePermutation[e] = ePerm[e];
    a->edgeOrientation[e] = eOri[e];
  }
}

void CubieCube_multiply(CubieCube *a, CubieCube *b) {
  CubieCube_cornerMultiply(a, b);
  CubieCube_edgeMultiply(a, b);
}

CubieCube getInvCubieCube(CubieCube *cubieCube) {
  CubieCube c;
  for (int edge = 0; edge < 12; edge++)
    c.edgePermutation[cubieCube->edgePermutation[edge]] = edge;
  for (int edge = 0; edge < 12; edge++)
    c.edgeOrientation[edge] =
        cubieCube->edgeOrientation[c.edgePermutation[edge]];

  for (int corner = 0; corner < 8; corner++)
    c.cornerPermutation[cubieCube->cornerPermutation[corner]] = corner;
  for (int corner = 0; corner < 8; corner++) {
    char ori = cubieCube->cornerOrientation[c.cornerPermutation[corner]];
    if (ori >= 3)
      c.cornerOrientation[corner] = ori;
    else {
      c.cornerOrientation[corner] = (char)-ori;
      if (c.cornerOrientation[corner] < 0)
        c.cornerOrientation[corner] += 3;
    }
  }
  return c;
}

/*-------------------------------------------------------------------------------------*/

short getTwist(CubieCube *cubieCube) {
  short ret = 0;
  for (int i = 0; i < 7; i++)
    ret = (short)(3 * ret + cubieCube->cornerOrientation[i]);
  return ret;
}

void setTwist(CubieCube *cubieCube, short twist) {
  int twistParity = 0;
  for (int i = 7 - 1; i >= 0; i--) {
    twistParity += cubieCube->cornerOrientation[i] = (char)(twist % 3);
    twist /= 3;
  }
  cubieCube->cornerOrientation[7] = (char)((3 - twistParity % 3) % 3);
}

short getFlip(CubieCube *cubieCube) {
  short ret = 0;
  for (int i = 0; i < 11; i++)
    ret = (short)(2 * ret + cubieCube->edgeOrientation[i]);
  return ret;
}

void setFlip(CubieCube *cubieCube, short flip) {
  int flipParity = 0;
  for (int i = 11 - 1; i >= 0; i--) {
    flipParity += cubieCube->edgeOrientation[i] = (char)(flip % 2);
    flip /= 2;
  }
  cubieCube->edgeOrientation[11] = (char)((2 - flipParity % 2) % 2);
}

short cornerParity(CubieCube *cubieCube) {
  int s = 0;
  for (int i = 7; i >= 0 + 1; i--)
    for (int j = i - 1; j >= URF; j--)
      if (cubieCube->cornerPermutation[j] > cubieCube->cornerPermutation[i])
        s++;
  return (short)(s % 2);
}

short edgeParity(CubieCube *cubieCube) {
  int s = 0;
  for (int i = 11; i >= 0 + 1; i--)
    for (int j = i - 1; j >= UR; j--)
      if (cubieCube->edgePermutation[j] > cubieCube->edgePermutation[i])
        s++;
  return (short)(s % 2);
}

short getFRtoBR(CubieCube *cubieCube) {
  int a = 0, x = 0;
  Edge edge4[4] = {0};
  for (int j = BR; j >= UR; j--) {
    if (FR <= cubieCube->edgePermutation[j] &&
        cubieCube->edgePermutation[j] <= BR) {
      a += Cnk(11 - j, x + 1);
      edge4[3 - x++] = cubieCube->edgePermutation[j];
    }
  }
  int b = 0;
  for (Edge j = 3; j > 0; j--) {
    int k = 0;
    while (edge4[j] != j + 8) {
      // printf("%d\n", edge4[j]);
      rotateLeftEdges(edge4, 0, j);
      k++;
    }
    b = (j + 1) * b + k;
  }
  return (short)(24 * a + b);
}

void setFRtoBR(CubieCube *cubieCube, short idx) {
  int x;
  Edge sliceEdge[] = {FR, FL, BL, BR};
  Edge otherEdge[] = {UR, UF, UL, UB, DR, DF, DL, DB};
  int b = idx % 24;
  int a = idx / 24;
  for (int e = UR; e <= BR; e++)
    cubieCube->edgePermutation[e] = DB;
  for (int j = 1, k; j < 4; j++) {
    k = b % (j + 1);
    b /= j + 1;
    while (k-- > 0)
      rotateRightEdges(sliceEdge, 0, j);
  }
  x = 3;
  for (int j = UR; j <= BR; j++)
    if (a - Cnk(11 - j, x + 1) >= 0) {
      cubieCube->edgePermutation[j] = sliceEdge[3 - x];
      a -= Cnk(11 - j, x-- + 1);
    }
  x = 0;
  for (int j = UR; j <= BR; j++)
    if (cubieCube->edgePermutation[j] == DB)
      cubieCube->edgePermutation[j] = otherEdge[x++];
}

short getURFtoDLF(CubieCube *cubieCube) {
  int a = 0, x = 0;
  Corner corner6[6] = {0};
  for (int j = URF; j <= DRB; j++)
    if (cubieCube->cornerPermutation[j] <= DLF) {
      a += Cnk(j, x + 1);
      corner6[x++] = cubieCube->cornerPermutation[j];
    }
  int b = 0;
  for (Corner j = 5; j > 0; j--) {
    int k = 0;
    while (corner6[j] != j) {
      rotateLeftCorners(corner6, 0, j);
      k++;
    }
    b = (j + 1) * b + k;
  }
  return (short)(720 * a + b);
}

void setURFtoDLF(CubieCube *cubieCube, short idx) {
  int x;
  Corner corner6[] = {URF, UFL, ULB, UBR, DFR, DLF};
  Corner otherCorner[] = {DBL, DRB};
  int b = idx % 720;
  int a = idx / 720;
  for (int c = URF; c <= DRB; c++)
    cubieCube->cornerPermutation[c] = DRB;
  for (int j = 1, k; j < 6; j++) {
    k = b % (j + 1);
    b /= j + 1;
    while (k-- > 0)
      rotateRightCorners(corner6, 0, j);
  }
  x = 5;
  for (int j = DRB; j >= 0; j--)
    if (a - Cnk(j, x + 1) >= 0) {
      cubieCube->cornerPermutation[j] = corner6[x];
      a -= Cnk(j, x-- + 1);
    }
  x = 0;
  for (int j = URF; j <= DRB; j++)
    if (cubieCube->cornerPermutation[j] == DRB)
      cubieCube->cornerPermutation[j] = otherCorner[x++];
}

int getURtoDF(CubieCube *cubieCube) {
  int a = 0, x = 0;
  Edge edge6[6] = {0};
  for (int j = UR; j <= BR; j++)
    if (cubieCube->edgePermutation[j] <= DF) {
      a += Cnk(j, x + 1);
      edge6[x++] = cubieCube->edgePermutation[j];
    }
  int b = 0;
  for (Edge j = 5; j > 0; j--) {
    int k = 0;
    while (edge6[j] != j) {
      rotateLeftEdges(edge6, 0, j);
      k++;
    }
    b = (j + 1) * b + k;
  }
  return 720 * a + b;
}

void setURtoDF(CubieCube *cubieCube, int idx) {
  int x;
  Edge edge6[] = {UR, UF, UL, UB, DR, DF};
  Edge otherEdge[] = {DL, DB, FR, FL, BL, BR};
  int b = idx % 720;
  int a = idx / 720;
  for (int e = UR; e <= BR; e++)
    cubieCube->edgePermutation[e] = BR;
  for (int j = 1, k; j < 6; j++) {
    k = b % (j + 1);
    b /= j + 1;
    while (k-- > 0)
      rotateRightEdges(edge6, 0, j);
  }
  x = 5;
  for (int j = BR; j >= 0; j--)
    if (a - Cnk(j, x + 1) >= 0) {
      cubieCube->edgePermutation[j] = edge6[x];
      a -= Cnk(j, x-- + 1);
    }
  x = 0;
  for (int j = UR; j <= BR; j++)
    if (cubieCube->edgePermutation[j] == BR)
      cubieCube->edgePermutation[j] = otherEdge[x++];
}

short getURtoUL(CubieCube *cubieCube) {
  int a = 0, x = 0;
  Edge edge3[3] = {0};
  for (int j = UR; j <= BR; j++)
    if (cubieCube->edgePermutation[j] <= UL) {
      a += Cnk(j, x + 1);
      edge3[x++] = cubieCube->edgePermutation[j];
    }

  int b = 0;
  for (Edge j = 2; j > 0; j--) {
    int k = 0;
    while (edge3[j] != j) {
      rotateLeftEdges(edge3, 0, j);
      k++;
    }
    b = (j + 1) * b + k;
  }
  return (short)(6 * a + b);
}

void setURtoUL(CubieCube *cubieCube, short idx) {
  int x;
  Edge edge3[] = {UR, UF, UL};
  int b = idx % 6;
  int a = idx / 6;
  for (int e = UR; e <= BR; e++)
    cubieCube->edgePermutation[e] = BR;
  for (int j = 1, k; j < 3; j++) {
    k = b % (j + 1);
    b /= j + 1;
    while (k-- > 0)
      rotateRightEdges(edge3, 0, j);
  }
  x = 2;
  for (int j = BR; j >= 0; j--)
    if (a - Cnk(j, x + 1) >= 0) {
      cubieCube->edgePermutation[j] = edge3[x];
      a -= Cnk(j, x-- + 1);
    }
}

short getUBtoDF(CubieCube *cubieCube) {
  int a = 0, x = 0;
  Edge edge3[3] = {0};
  for (int j = UR; j <= BR; j++)
    if (UB <= cubieCube->edgePermutation[j] &&
        cubieCube->edgePermutation[j] <= DF) {
      a += Cnk(j, x + 1);
      edge3[x++] = cubieCube->edgePermutation[j];
    }

  int b = 0;
  for (Edge j = 2; j > 0; j--) {
    int k = 0;
    while (edge3[j] != UB + j) {
      rotateLeftEdges(edge3, 0, j);
      k++;
    }
    b = (j + 1) * b + k;
  }
  return (short)(6 * a + b);
}

void setUBtoDF(CubieCube *cubieCube, short idx) {
  int x;
  Edge edge3[] = {UB, DR, DF};
  int b = idx % 6;
  int a = idx / 6;
  for (int e = UR; e <= BR; e++)
    cubieCube->edgePermutation[e] = BR;
  for (int j = 1, k; j < 3; j++) {
    k = b % (j + 1);
    b /= j + 1;
    while (k-- > 0)
      rotateRightEdges(edge3, 0, j);
  }
  x = 2;
  for (int j = BR; j >= 0; j--)
    if (a - Cnk(j, x + 1) >= 0) {
      cubieCube->edgePermutation[j] = edge3[x];
      a -= Cnk(j, x-- + 1);
    }
}

int getURtoDFWithIndices(short idx1, short idx2) {
  CubieCube a = CubieCube_make();
  CubieCube b = CubieCube_make();
  setURtoUL(&a, idx1);
  setUBtoDF(&b, idx2);
  for (int i = 0; i < 8; i++) {
    if (a.edgePermutation[i] != BR) {
      if (b.edgePermutation[i] != BR) // collision
        return -1;
      else
        b.edgePermutation[i] = a.edgePermutation[i];
    }
  }
  return getURtoDF(&b);
}

int verify(CubieCube *cubieCube) {
  int sum = 0;
  int edgeCount[12] = {0};
  for (int e = UR; e <= BR; e++)
    edgeCount[cubieCube->edgePermutation[e]]++;
  for (int i = 0; i < 12; i++)
    if (edgeCount[i] != 1)
      return -2;
  for (int i = 0; i < 12; i++)
    sum += cubieCube->edgeOrientation[i];
  if (sum % 2 != 0)
    return -3;
  int cornerCount[8] = {0};
  for (int c = URF; c <= DRB; c++)
    cornerCount[cubieCube->cornerPermutation[c]]++;
  for (int i = 0; i < 8; i++)
    if (cornerCount[i] != 1)
      return -4;
  sum = 0;
  for (int i = 0; i < 8; i++)
    sum += cubieCube->cornerOrientation[i];
  if (sum % 3 != 0)
    return -5;
  if ((edgeParity(cubieCube) ^ cornerParity(cubieCube)) != 0)
    return -6;

  return 0;
}
