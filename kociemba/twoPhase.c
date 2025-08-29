#include "twoPhase.h"
#include "coordCube.h"
#include "enums.h"
#include "faceCube.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int axis[31];
int movePower[31];

// phase1 coordinates
int cornersOrientationCoord[31];
int edgesOrientationCoord[31];
/** UD slice coordinates (phase 1) */
int slice[31];

// phase2 coordinates
int parity[31];
int URFtoDLF[31];
int FRtoBR[31];
int URtoDF[31];
// helper coordinates to calculate URtoDF at the beginning of phase 2
int URtoUL[31];
int UBtoDF[31];

// IDA* distance do goal estimations
int minDistPhase1[31];
int minDistPhase2[31];

char *SOLVED_CUBE = "UUUUUUUUU"
                    "RRRRRRRRR"
                    "FFFFFFFFF"
                    "DDDDDDDDD"
                    "LLLLLLLLL"
                    "BBBBBBBBB";

char *printErrorMessage(int error) {
  if (error == -1)
    return "There are not exactly 9 facelets of each color";
  if (error == -2)
    return "Not all 12 edges exist exactly once";
  if (error == -3)
    return "Flip error: One edge has to be flipped";
  if (error == -4)
    return "Not all corners exist exactly once";
  if (error == -5)
    return "Twist error: One corner has to be twisted";
  if (error == -6)
    return "Parity error: Two corners or two edges have to be exchanged";
  if (error == -7)
    return "No solution exists for the given maxDepth";
  if (error == -8)
    return "Timeout, no solution within given time";
  if (error == 1)
    return "There are not exactly 9 facelets of each color in pattern";
  if (error == 2)
    return "Not all 12 edges exist exactly once in pattern";
  if (error == 3)
    return "Flip error: One edge has to be flipped in pattern";
  if (error == 4)
    return "Not all corners exist exactly once in pattern";
  if (error == 5)
    return "Twist error: One corner has to be twisted in pattern";
  if (error == 6)
    return "Parity error: Two corners or two edges have to be exchanged in "
           "pattern";
  if (error == 7)
    return "Cube is already solved";
  return NULL;
}

int findSolution(char *cube, int maxDepth, long timeOut, Move moves[maxDepth],
                 char *pattern, int *depth) {
  CubieCube cubieCube = CubieCube_make();
  // validate cube
  int errorCode = validateCubeStringAndInitCubieCube(cube, &cubieCube);
  if (errorCode != 0)
    return errorCode;
  // validate pattern
  CubieCube patternCubieCube = CubieCube_make();
  errorCode = validateCubeStringAndInitCubieCube(pattern, &patternCubieCube);
  if (errorCode != 0)
    return abs(errorCode);
  // check if cube is already solved
  if (strcmp(cube, pattern) == 0) {
    // cube is already solved, no moves required to solve it
    moves = NULL;
    return 7;
  }
  // create new cubieCube to be solved to achieve pattern
  CubieCube cubieCubeToSolve = getInvCubieCube(&patternCubieCube);
  CubieCube_multiply(&cubieCubeToSolve, &cubieCube);

  CoordCube coordCube = CoordCube_make(cubieCubeToSolve);
  axis[0] = 0;
  movePower[0] = 0;
  cornersOrientationCoord[0] = coordCube.flip;
  edgesOrientationCoord[0] = coordCube.twist;
  parity[0] = coordCube.parity;
  slice[0] = coordCube.FRtoBR / 24;
  URFtoDLF[0] = coordCube.URFtoDLF;
  FRtoBR[0] = coordCube.FRtoBR;
  URtoUL[0] = coordCube.URtoUL;
  UBtoDF[0] = coordCube.UBtoDF;

  // set up search
  int move;             // the move to make - axis + power
  int n = 0;            // current depth
  int depthPhase1 = 1;  // current depth of the search
  minDistPhase1[1] = 1; // to avoid failure for depth=1, n=0
  bool busy = false;    // if true, indicate that we are backtracking and still
                        // looking for the next move
  int depthTotal;

  struct timespec tStart;
  clock_gettime(CLOCK_MONOTONIC, &tStart);

  // +++++++++++++++++++ Main loop ++++++++++++++++++++++++++++++++++++++++++
  // run until solution found
  do {
    // compute next move (IDA*)
    do {
      /*if not all branches that continue from current position are "dead ends",
        i.e. current position can be solved within depthPhase1-n or fewer moves,
        go deeper*/
      if ((depthPhase1 - n > minDistPhase1[n + 1]) && !busy) {
        if (axis[n] == 0 || axis[n] == 3)
          // if previous move rotated U or D faces, start from R axis
          // we don't want to twist the same faces or the parallel faces move
          // after move
          axis[++n] = 1;
        else
          axis[++n] = 0;  // start from U axis
        movePower[n] = 1; // start from 90 degrees rotation
      }
      // otherwise, try the next branch
      // increase power and check if we tried all powers
      else if (++movePower[n] > 3) {
        // we tried all possible powers for current axis, so move to the next
        // axis
        do {
          // increase axis and check if we tried all axes
          if (++axis[n] > 5) {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);

            long long elapsed_time_ns =
                (now.tv_sec - tStart.tv_sec) * 1000000000LL +
                (now.tv_nsec - tStart.tv_nsec);
            double elapsed_time_ms = (double)elapsed_time_ns / 1000000.0;
            // check for timeout
            if (elapsed_time_ms > timeOut) {
              return -8;
            }
            // we tried all 18 moves, so we need to backtrack or increase search
            // depth if we finished the DFS for current search depth, increase
            // search depth
            if (n == 0) {
              // increase search depth (if possible)
              if (depthPhase1 >= maxDepth)
                return -7;
              else {
                depthPhase1++;
                axis[n] = 0;      // start from U axis
                movePower[n] = 1; // start from 90 degrees rotation
                busy = false;
                break;
              }
            }
            // else, we need to backtrack
            else {
              n--;
              busy = true;
              // busy since we still need to calculate our next move
              // we need to backtrack and move to another branch
              break;
            }
          } else {
            // just move to next axis (increasing done in the if condition)
            movePower[n] = 1; // start from 90 degrees
            busy = false;
          }
        } while (n != 0 &&
                 (axis[n - 1] == axis[n] || axis[n - 1] == axis[n] + 3));
        // we don't want to twist the same faces or the parallel faces move
        // after move
      } else
        busy = false; // we found our next move
    } while (busy);

    // compute new coordinates and new minDistPhase1
    move = 3 * axis[n] + movePower[n] - 1;
    cornersOrientationCoord[n + 1] = flipMove[cornersOrientationCoord[n]][move];
    edgesOrientationCoord[n + 1] = twistMove[edgesOrientationCoord[n]][move];
    slice[n + 1] = FRtoBR_Move[slice[n] * 24][move] / 24;
    // get a lower bound on the number of moves needed to solve current position
    //(in phase 1, solve means reaching H subgroup)
    minDistPhase1[n + 1] = fmax(
        // according to edges orientations and UD slice
        getPruning(Slice_Flip_Prun,
                   N_SLICE1 * cornersOrientationCoord[n + 1] + slice[n + 1]),
        // according to corners orientations and UD slice
        getPruning(Slice_Twist_Prun,
                   N_SLICE1 * edgesOrientationCoord[n + 1] + slice[n + 1]));

    // if minDistPhase1 = 0, the H subgroup is reached
    if (minDistPhase1[n + 1] == 0 && n >= depthPhase1 - 5) {
      minDistPhase1[n + 1] = 10; // we don't want to go deeper in this branch,
                                 // so any value >5 is possible
      // we look for phase2 solution only if this is a "new maneuver", meaning n
      // == depthPhase1 - 1
      if (n == depthPhase1 - 1 &&
          (depthTotal = totalDepth(depthPhase1, maxDepth)) >= 0) {
        // if depthPhase2 = 0, this is an optimal solution
        // we also check that we don't twist the same faces or the parallel
        // faces move after move (in the phase1 phase2 connection)
        if (depthTotal == depthPhase1 ||
            (axis[depthPhase1 - 1] != axis[depthPhase1] &&
             axis[depthPhase1 - 1] != axis[depthPhase1] + 3)) {
          createMovesList(moves, depthTotal);
          *depth = depthTotal;
          return 0;
        }
      }
    }
  } while (true);
}

int findSolutionBasic(char *cube, int maxDepth, long timeOut,
                      Move moves[maxDepth], int *depth) {
  return findSolution(cube, maxDepth, timeOut, moves, SOLVED_CUBE, depth);
}

int validateCubeStringAndInitCubieCube(char *cube, CubieCube *cubieCube) {
  int count[6] = {0};
  for (int i = 0; i < 54; i++) {
    count[getCorrespondingColor(cube[i])]++;
  }
  for (int i = 0; i < 6; i++) {
    if (count[i] != 9) {
      return -1;
    }
  }
  // create face and cubie level representations
  FaceCube fc = FaceCube_make(cube);
  CubieCube cc = FaceCube_toCubieCube(&fc);
  // verify cubie level
  int errorCode = verify(&cc);
  if (errorCode == 0) {
    // copy cubieCube representation to entered cube
    for (int i = 0; i < 8; i++) {
      cubieCube->cornerPermutation[i] = cc.cornerPermutation[i];
      cubieCube->cornerOrientation[i] = cc.cornerOrientation[i];
    }
    for (int i = 0; i < 12; i++) {
      cubieCube->edgePermutation[i] = cc.edgePermutation[i];
      cubieCube->edgeOrientation[i] = cc.edgeOrientation[i];
    }
  }
  return errorCode;
}

void createMovesList(Move moves[], int depth) {
  for (int i = 0; i < depth; i++) {
    moves[i] = Move_createMove(axis[i], movePower[i]);
  }
}

int totalDepth(int depthPhase1, int maxDepth) {
  int mv; // the move to make - axis + power (face + rotation degrees)
  maxDepth =
      fmin(10, maxDepth - depthPhase1); // allow only max 10 moves in phase2
  // initialize phase2 coordinates for all the moves in phase1
  for (int i = 0; i < depthPhase1; i++) {
    mv = 3 * axis[i] + movePower[i] - 1;
    URFtoDLF[i + 1] = URFtoDLF_Move[URFtoDLF[i]][mv];
    FRtoBR[i + 1] = FRtoBR_Move[FRtoBR[i]][mv];
    parity[i + 1] = parityMove[parity[i]][mv];
  }
  // get lower bound on the number of moves needed to solve current position
  int d1 =
      getPruning(Slice_URFtoDLF_Parity_Prun,
                 (N_SLICE2 * URFtoDLF[depthPhase1] + FRtoBR[depthPhase1]) * 2 +
                     parity[depthPhase1]);
  if (d1 > maxDepth)
    return -1;
  // initialize helping coordinates for all the moves in phase1
  // these coordinates are used for initializing the URtoDF coordinate
  for (int i = 0; i < depthPhase1; i++) {
    mv = 3 * axis[i] + movePower[i] - 1;
    URtoUL[i + 1] = URtoUL_Move[URtoUL[i]][mv];
    UBtoDF[i + 1] = UBtoDF_Move[UBtoDF[i]][mv];
  }
  URtoDF[depthPhase1] =
      MergeURtoULandUBtoDF[URtoUL[depthPhase1]][UBtoDF[depthPhase1]];
  // get lower bound on the number of moves needed to solve current position
  int d2 =
      getPruning(Slice_URtoDF_Parity_Prun,
                 (N_SLICE2 * URtoDF[depthPhase1] + FRtoBR[depthPhase1]) * 2 +
                     parity[depthPhase1]);
  if (d2 > maxDepth)
    return -1;

  if ((minDistPhase2[depthPhase1] = fmax(d1, d2)) == 0) // already solved
    return depthPhase1;

  // set up search
  int n = depthPhase1;      // current depth
  int depthPhase2 = 1;      // current depth of the search
  minDistPhase2[n + 1] = 1; // to avoid failure for depthPhase2=1, n=depthPhase1
  bool busy = false; // if true, indicate that we are backtracking and still
                     // looking for the next move
  movePower[depthPhase1] = 0;
  axis[depthPhase1] = 0;

  // similar to the way it was done in phase1
  do {
    do {
      if ((depthPhase1 + depthPhase2 - n > minDistPhase2[n + 1]) && !busy) {
        if (axis[n] == 0 || axis[n] == 3) {
          // if the previous move twisted U or D, start move iteration from R
          // face and power is set to 180 degrees since it is the only move
          // allowed
          axis[++n] = 1;
          movePower[n] = 2;
        } else {
          // otherwise, start from U, and power starts from 90 degrees
          axis[++n] = 0;
          movePower[n] = 1;
        }
      } else if ((axis[n] == 0 || axis[n] == 3)
                     ? (++movePower[n] > 3)
                     : ((movePower[n] = movePower[n] + 2) > 3)) {
        do {
          if (++axis[n] > 5) {
            if (n == depthPhase1) {
              if (depthPhase2 >= maxDepth)
                return -1;
              else {
                depthPhase2++;
                axis[n] = 0;      // start from U axis
                movePower[n] = 1; // start from 90 degrees
                busy = false;
                break;
              }
            } else {
              n--;
              busy = true;
              break;
            }
          } else {
            // just move to next axis (increasing done in the if condition)
            if (axis[n] == 0 || axis[n] == 3)
              movePower[n] =
                  1; // if we move the U or D faces, start from 90 degrees
            else
              movePower[n] =
                  2; // otherwise, only 180 degrees rotation are allowed
            busy = false;
          }
        } while (n != depthPhase1 &&
                 (axis[n - 1] == axis[n] || axis[n - 1] == axis[n] + 3));
      } else
        busy = false;
    } while (busy);

    mv = 3 * axis[n] + movePower[n] - 1;
    URFtoDLF[n + 1] = URFtoDLF_Move[URFtoDLF[n]][mv];
    FRtoBR[n + 1] = FRtoBR_Move[FRtoBR[n]][mv];
    parity[n + 1] = parityMove[parity[n]][mv];
    URtoDF[n + 1] = URtoDF_Move[URtoDF[n]][mv];
    // get lower bound on the number of moves needed to solve current position
    minDistPhase2[n + 1] =
        fmax(getPruning(Slice_URtoDF_Parity_Prun,
                        (N_SLICE2 * URtoDF[n + 1] + FRtoBR[n + 1]) * 2 +
                            parity[n + 1]),
             getPruning(Slice_URFtoDLF_Parity_Prun,
                        (N_SLICE2 * URFtoDLF[n + 1] + FRtoBR[n + 1]) * 2 +
                            parity[n + 1]));

  } while (minDistPhase2[n + 1] != 0);
  return depthPhase1 + depthPhase2;
}
