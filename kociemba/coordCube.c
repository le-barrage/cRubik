#include "coordCube.h"
#include "cubieCube.h"

short twistMove[N_TWIST][N_MOVE];
short flipMove[N_FLIP][N_MOVE];
short parityMove[2][18] = {
    {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
    {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0}};
short FRtoBR_Move[N_FRtoBR][N_MOVE];
short URFtoDLF_Move[N_URFtoDLF][N_MOVE];
short URtoDF_Move[N_URtoDF][N_MOVE];
short URtoUL_Move[N_URtoUL][N_MOVE];
short UBtoDF_Move[N_UBtoDF][N_MOVE];
short MergeURtoULandUBtoDF[336][336];

char Slice_URFtoDLF_Parity_Prun[N_SLICE2 * N_URFtoDLF * N_PARITY / 2];
char Slice_URtoDF_Parity_Prun[N_SLICE2 * N_URtoDF * N_PARITY / 2];
char Slice_Twist_Prun[N_SLICE1 * N_TWIST / 2 + 1];
char Slice_Flip_Prun[N_SLICE1 * N_FLIP / 2];

CoordCube CoordCube_make(CubieCube cubieCube) {
  return (CoordCube){
      .twist = getTwist(&cubieCube),
      .flip = getFlip(&cubieCube),
      .parity = cornerParity(&cubieCube),
      .FRtoBR = getFRtoBR(&cubieCube),
      .URFtoDLF = getURFtoDLF(&cubieCube),
      .URtoUL = getURtoUL(&cubieCube),
      .UBtoDF = getUBtoDF(&cubieCube),
      .URtoDF = getURtoDF(&cubieCube) // only needed in phase2
  };
}

void init() {
  initialize_moveCube();
  printf("\ninitialize_moveCube();\n");
  initTwistMove();
  printf("initTwistMove();\n");
  initFlipMove();
  printf("initFlipMove();\n");
  initFRtoBR_Move();
  printf("initFRtoBR_Move();\n");
  initURFtoDLF_Move();
  printf("initURFtoDLF_Move();\n");
  initURtoDF_Move();
  printf("initURtoDF_Move();\n");
  initURtoUL_Move();
  printf("initURtoUL_Move();\n");
  initUBtoDF_Move();
  printf("initUBtoDF_Move();\n");
  initMergeURtoULandUBtoDF();
  printf("initMergeURtoULandUBtoDF();\n");
  initSlice_URFtoDLF_Parity_Prun();
  printf("initSlice_URFtoDLF_Parity_Prun();\n");
  initSlice_URtoDF_Parity_Prun();
  printf("initSlice_URtoDF_Parity_Prun();\n");
  initSlice_Twist_Prun();
  printf("initSlice_Twist_Prun();\n");
  initSlice_Flip_Prun();
  printf("initSlice_Flip_Prun();\n\n");
}

// *****************Phase 1 move tables****************************

/** Move table for the twists of the corners.
 *  Keeps for each combination of corners orientation and move
 *  the new corner orientation after executing the move. <br>
 *  twist < 2187 in phase 1 <br>
 *  twist = 0 in phase 2 */

void initTwistMove() {
  CubieCube a = CubieCube_make();
  for (short twist = 0; twist < N_TWIST; twist++) {
    setTwist(&a, twist);
    // iterate through all possible moves (cube face X face turn)
    // iterate all cube faces
    for (short move = 0; move < 6; move++) {
      // iterate all possible number of face turns (90, 180 or 270 (-90)
      // degrees)
      for (short faceTurn = 0; faceTurn < 3; faceTurn++) {
        // apply face turn and set value
        CubieCube_cornerMultiply(&a, &moveCube[move]);
        twistMove[twist][3 * move + faceTurn] = getTwist(&a);
      }
      // apply face turn one more time to restore cube
      CubieCube_cornerMultiply(&a, &moveCube[move]);
    }
  }
}

/** Move table for the flips of the edges.
 * 	Keeps for each combination of corners orientation and move the new
 * corner orientation after executing the move. <br> flip < 2048 in phase 1 <br>
 *  flip = 0 in phase 2 */

void initFlipMove() {
  CubieCube a = CubieCube_make();
  // iterate through all possible edge orientations
  for (short flip = 0; flip < N_FLIP; flip++) {
    setFlip(&a, flip);
    // iterate through all possible moves (cube face X face turn)
    // iterate all cube faces
    for (short move = 0; move < 6; move++) {
      // iterate all possible number of face turns (90, 180 or 270 (-90)
      // degrees)
      for (short faceTurn = 0; faceTurn < 3; faceTurn++) {
        // apply face turn and set value
        CubieCube_edgeMultiply(&a, &moveCube[move]);
        flipMove[flip][3 * move + faceTurn] = getFlip(&a);
      }
      // apply face turn one more time to restore cube
      CubieCube_edgeMultiply(&a, &moveCube[move]);
    }
  }
}

/** Parity of the corner permutation. This is the same as the parity for the
 * edge permutation of a valid cube. parity has values 0 and 1 */

// *****************Phase 1 and 2 move tables****************************

/** Move table for the four UD-slice edges FR, FL, BL and BR <br>
 *  FRtoBRMove < 11880 in phase 1 <br>
 *  FRtoBRMove < 24 in phase 2 (since they are already in the slice) <br>
 *  FRtoBRMove = 0 for solved cube */

void initFRtoBR_Move() {
  CubieCube a = CubieCube_make();
  for (short i = 0; i < N_FRtoBR; i++) {
    setFRtoBR(&a, i);
    for (int j = 0; j < 6; j++) {
      for (int k = 0; k < 3; k++) {
        CubieCube_edgeMultiply(&a, &moveCube[j]);
        FRtoBR_Move[i][3 * j + k] = getFRtoBR(&a);
      }
      CubieCube_edgeMultiply(&a, &moveCube[j]);
    }
  }
}

/** Move table for permutation of six corners.
 *  The positions of the DBL and DRB corners are determined by the parity.<br>
 *  URFtoDLF < 20160 in both phases (1 and 2) <br>
 *  URFtoDLF = 0 for solved cube */

void initURFtoDLF_Move() {
  CubieCube a = CubieCube_make();
  for (short i = 0; i < N_URFtoDLF; i++) {
    setURFtoDLF(&a, i);
    for (int j = 0; j < 6; j++) {
      for (int k = 0; k < 3; k++) {
        CubieCube_cornerMultiply(&a, &moveCube[j]);
        URFtoDLF_Move[i][3 * j + k] = getURFtoDLF(&a);
      }
      CubieCube_cornerMultiply(&a, &moveCube[j]);
    }
  }
}

/** Move table for the permutation of six U-face and D-face edges in phase2.
 *  The positions of the DL and DB edges are determined by the parity.<br>
 *  URtoDF < 665280 in phase 1 <br>
 *  URtoDF < 20160 in phase 2 <br>
 *  URtoDF = 0 for solved cube */

void initURtoDF_Move() {
  CubieCube a = CubieCube_make();
  for (short i = 0; i < N_URtoDF; i++) {
    setURtoDF(&a, i);
    for (int j = 0; j < 6; j++) {
      for (int k = 0; k < 3; k++) {
        CubieCube_edgeMultiply(&a, &moveCube[j]);
        URtoDF_Move[i][3 * j + k] = getURtoDF(&a);
      }
      CubieCube_edgeMultiply(&a, &moveCube[j]);
    }
  }
}

// ******helper move tables to compute URtoDF for the beginning of phase2****

/** Move table for the three edges UR,UF and UL in phase1. */

void initURtoUL_Move() {
  CubieCube a = CubieCube_make();
  for (short i = 0; i < N_URtoUL; i++) {
    setURtoUL(&a, i);
    for (int j = 0; j < 6; j++) {
      for (int k = 0; k < 3; k++) {
        CubieCube_edgeMultiply(&a, &moveCube[j]);
        URtoUL_Move[i][3 * j + k] = getURtoUL(&a);
      }
      CubieCube_edgeMultiply(&a, &moveCube[j]);
    }
  }
}

/** Move table for the three edges UB,DR and DF in phase1. */

void initUBtoDF_Move() {
  CubieCube a = CubieCube_make();
  for (short i = 0; i < N_UBtoDF; i++) {
    setUBtoDF(&a, i);
    for (int j = 0; j < 6; j++) {
      for (int k = 0; k < 3; k++) {
        CubieCube_edgeMultiply(&a, &moveCube[j]);
        UBtoDF_Move[i][3 * j + k] = getUBtoDF(&a);
      }
      CubieCube_edgeMultiply(&a, &moveCube[j]);
    }
  }
}

/** Table to merge the coordinates of the UR,UF,UL and UB,DR,DF edges at the
 * beginning of phase2 */

void initMergeURtoULandUBtoDF() {
  // for i, j <336 the six edges UR,UF,UL,UB,DR,DF are not in the
  // UD-slice and the index is <20160
  for (short uRtoUL = 0; uRtoUL < 336; uRtoUL++) {
    for (short uBtoDF = 0; uBtoDF < 336; uBtoDF++) {
      MergeURtoULandUBtoDF[uRtoUL][uBtoDF] =
          (short)getURtoDFWithIndices(uRtoUL, uBtoDF);
    }
  }
}

// ****************Pruning tables for the search********************

/** Set pruning value in table. Two values are stored in one char. */
void setPruning(char table[], int index, char value) {
  if ((index & 1) == 0)
    table[index / 2] &= 0xf0 | value;
  else
    table[index / 2] &= 0x0f | (value << 4);
}

/** Extract pruning value */
char getPruning(char table[], int index) {
  if ((index & 1) == 0)
    return (char)(table[index / 2] & 0x0f);
  else
    return (char)((table[index / 2] & 0xf0) >> 4);
}

/** Pruning table for the permutation of the corners and the UD-slice edges in
 * phase2. The pruning table entries give a lower estimation for the number of
 * moves to reach the solved cube.*/

void initSlice_URFtoDLF_Parity_Prun() {
  for (int i = 0; i < N_SLICE2 * N_URFtoDLF * N_PARITY / 2; i++)
    Slice_URFtoDLF_Parity_Prun[i] = -1;
  int depth = 0;
  setPruning(Slice_URFtoDLF_Parity_Prun, 0, (char)0);
  int done = 1;
  while (done != N_SLICE2 * N_URFtoDLF * N_PARITY) {
    for (int i = 0; i < N_SLICE2 * N_URFtoDLF * N_PARITY; i++) {
      int parity = i % 2;
      int URFtoDLF = (i / 2) / N_SLICE2;
      int slice = (i / 2) % N_SLICE2;
      if (getPruning(Slice_URFtoDLF_Parity_Prun, i) == depth) {
        for (int j = 0; j < 18; j++) {
          int newSlice, newURFtoDLF, newParity;
          switch (j) {
          case 3:
          case 5:
          case 6:
          case 8:
          case 12:
          case 14:
          case 15:
          case 17:
            continue;
          default:
            newSlice = FRtoBR_Move[slice][j];
            newURFtoDLF = URFtoDLF_Move[URFtoDLF][j];
            newParity = parityMove[parity][j];
            if (getPruning(Slice_URFtoDLF_Parity_Prun,
                           (N_SLICE2 * newURFtoDLF + newSlice) * 2 +
                               newParity) == 0x0f) {
              setPruning(Slice_URFtoDLF_Parity_Prun,
                         (N_SLICE2 * newURFtoDLF + newSlice) * 2 + newParity,
                         (char)(depth + 1));
              done++;
            }
          }
        }
      }
    }
    depth++;
  }
}

/** Pruning table for the permutation of the edges in phase2.
 *  The pruning table entries give a lower estimation for the number of moves to
 * reach the solved cube. */
void initSlice_URtoDF_Parity_Prun() {
  for (int i = 0; i < N_SLICE2 * N_URtoDF * N_PARITY / 2; i++)
    Slice_URtoDF_Parity_Prun[i] = -1;
  int depth = 0;
  setPruning(Slice_URtoDF_Parity_Prun, 0, (char)0);
  int done = 1;
  while (done != N_SLICE2 * N_URtoDF * N_PARITY) {
    for (int i = 0; i < N_SLICE2 * N_URtoDF * N_PARITY; i++) {
      int newSlice, newURtoDF, newParity;
      int parity = i % 2;
      int URtoDF = (i / 2) / N_SLICE2;
      int slice = (i / 2) % N_SLICE2;
      if (getPruning(Slice_URtoDF_Parity_Prun, i) == depth) {
        for (int j = 0; j < 18; j++) {
          switch (j) {
          case 3:
          case 5:
          case 6:
          case 8:
          case 12:
          case 14:
          case 15:
          case 17:
            continue;
          default:
            newSlice = FRtoBR_Move[slice][j];
            newURtoDF = URtoDF_Move[URtoDF][j];
            newParity = parityMove[parity][j];
            if (getPruning(Slice_URtoDF_Parity_Prun,
                           (N_SLICE2 * newURtoDF + newSlice) * 2 + newParity) ==
                0x0f) {
              setPruning(Slice_URtoDF_Parity_Prun,
                         (N_SLICE2 * newURtoDF + newSlice) * 2 + newParity,
                         (char)(depth + 1));
              done++;
            }
          }
        }
      }
    }
    depth++;
  }
}

/** Pruning table for the twist of the corners and the position (not
 * permutation) of the UD-slice edges in phase1. The pruning table entries give
 * a lower estimation for the number of moves to reach the H-subgroup. */
void initSlice_Twist_Prun() {
  // initialize all entries to non valid value
  for (int i = 0; i < N_SLICE1 * N_TWIST / 2 + 1; i++)
    Slice_Twist_Prun[i] = -1;
  int depth = 0;
  // set pruning
  setPruning(Slice_Twist_Prun, 0, (char)0);
  int done = 1;
  while (done != N_SLICE1 * N_TWIST) {
    for (int i = 0; i < N_SLICE1 * N_TWIST; i++) {
      int twist = i / N_SLICE1, slice = i % N_SLICE1;
      if (getPruning(Slice_Twist_Prun, i) == depth) {
        for (int j = 0; j < 18; j++) {
          int newSlice = FRtoBR_Move[slice * 24][j] / 24;
          int newTwist = twistMove[twist][j];
          if (getPruning(Slice_Twist_Prun, N_SLICE1 * newTwist + newSlice) ==
              0x0f) {
            setPruning(Slice_Twist_Prun, N_SLICE1 * newTwist + newSlice,
                       (char)(depth + 1));
            done++;
          }
        }
      }
    }
    depth++;
  }
}

/** Pruning table for the flip of the edges and the position (not permutation)
 * of the UD-slice edges in phase1. The pruning table entries give a lower
 * estimation for the number of moves to reach the H-subgroup. */
void initSlice_Flip_Prun() {
  for (int i = 0; i < N_SLICE1 * N_FLIP / 2; i++)
    Slice_Flip_Prun[i] = -1;
  int depth = 0;
  setPruning(Slice_Flip_Prun, 0, (char)0);
  int done = 1;
  while (done != N_SLICE1 * N_FLIP) {
    for (int i = 0; i < N_SLICE1 * N_FLIP; i++) {
      int flip = i / N_SLICE1, slice = i % N_SLICE1;
      if (getPruning(Slice_Flip_Prun, i) == depth) {
        for (int j = 0; j < 18; j++) {
          int newSlice = FRtoBR_Move[slice * 24][j] / 24;
          int newFlip = flipMove[flip][j];
          if (getPruning(Slice_Flip_Prun, N_SLICE1 * newFlip + newSlice) ==
              0x0f) {
            setPruning(Slice_Flip_Prun, N_SLICE1 * newFlip + newSlice,
                       (char)(depth + 1));
            done++;
          }
        }
      }
    }
    depth++;
  }
}
