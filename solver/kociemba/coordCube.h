#ifndef COORDCUBE_H
#define COORDCUBE_H

#include "cubieCube.h"
#include <stdio.h>

/** 3^7 possible corner twists (orientations) */
#define N_TWIST 2187
/** 2^11 possible edge flips (orientations) */
#define N_FLIP 2048
/** 12 choose 4 possible positions of FR,FL,BL,BR edges */
#define N_SLICE1 495
/** 4! permutations of FR,FL,BL,BR edges in phase2 */
#define N_SLICE2 24
/** 2 possible corner parities */
#define N_PARITY 2
/** 8!/(8-6)! permutation of URF,UFL,ULB,UBR,DFR,DLF corners */
#define N_URFtoDLF 20160
/** 12!/(12-4)! permutation of FR,FL,BL,BR edges */
#define N_FRtoBR 11880
/** 12!/(12-3)! permutation of UR,UF,UL edges */
#define N_URtoUL 1320
/** 12!/(12-3)! permutation of UB,DR,DF edges */
#define N_UBtoDF 1320
/** 8!/(8-6)! permutation of UR,UF,UL,UB,DR,DF edges in phase2 */
#define N_URtoDF 20160
/** 6 faces, 3 rotation per face */
#define N_MOVE 18

typedef struct CoordCube {
  short twist;
  short flip;
  short parity;
  /* UD slice coordinates */
  short FRtoBR;
  short URFtoDLF;
  short URtoUL;
  short UBtoDF;
  int URtoDF;
} CoordCube;

extern short twistMove[N_TWIST][N_MOVE];
extern short flipMove[N_FLIP][N_MOVE];
extern short parityMove[2][18];
extern short FRtoBR_Move[N_FRtoBR][N_MOVE];
extern short URFtoDLF_Move[N_URFtoDLF][N_MOVE];
extern short URtoDF_Move[N_URtoDF][N_MOVE];
extern short URtoUL_Move[N_URtoUL][N_MOVE];
extern short UBtoDF_Move[N_UBtoDF][N_MOVE];
extern short MergeURtoULandUBtoDF[336][336];

extern char Slice_URFtoDLF_Parity_Prun[N_SLICE2 * N_URFtoDLF * N_PARITY / 2];
extern char Slice_URtoDF_Parity_Prun[N_SLICE2 * N_URtoDF * N_PARITY / 2];
extern char Slice_Twist_Prun[N_SLICE1 * N_TWIST / 2 + 1];
extern char Slice_Flip_Prun[N_SLICE1 * N_FLIP / 2];

CoordCube CoordCube_make(CubieCube cubieCube);

void init();
void initTwistMove();
void initFlipMove();
void initFRtoBR_Move();
void initURFtoDLF_Move();
void initURtoDF_Move();
void initURtoUL_Move();
void initUBtoDF_Move();
void initMergeURtoULandUBtoDF();
void initSlice_URFtoDLF_Parity_Prun();
void initSlice_URtoDF_Parity_Prun();
void initSlice_Twist_Prun();
void initSlice_Flip_Prun();
void setPruning(char table[], int index, char value);
char getPruning(char table[], int index);

#endif // !COORDCUBE_H
