#ifndef TWOPHASE_H
#define TWOPHASE_H

#include "cubieCube.h"
#include "move.h"

/*
  -1 There are not exactly 9 facelets of each color
  -2 Not all 12 edges exist exactly once
  -3 Flip error: One edge has to be flipped
  -4 Not all corners exist exactly once
  -5 Twist error: One corner has to be twisted
  -6 Parity error: Two corners or two edges have to be exchanged
  -7 No solution exists for the given maxDepth
  -8 Timeout, no solution within given time
  1 There are not exactly 9 facelets of each color in pattern
  2 Not all 12 edges exist exactly once in pattern
  3 Flip error: One edge has to be flipped in pattern
  4 Not all corners exist exactly once in pattern
  5 Twist error: One corner has to be twisted in pattern
  6 Parity error: Two corners or two edges have to be exchanged in pattern
 */
char *printErrorMessage(int error);

/**
 * Finds list of moves for solving received cube with no more than maxDepth
 * moves.
 * param: cube - the cube definition string.
 * 			The format is a 54 long string containing the characters
 * U, R, F, D, L and B where each char corresponds to the color of a facelet.
 * 			Faceletes colors should appear in the following order:
 * 			U1 ... U9 R1 ... R9 F1 ... F9 D1 ... D9 L1 ... L9 B1 ... B9 
 * param: maxDepth -
 * defines the maximal allowed maneuver length. For random cubes, a maxDepth of
 * 21 usually will returns: a solution in less than 0.5 seconds. With a maxDepth
 * of 20 it takes a few seconds on average to find a solution, but it may take
 * much longer for specific cubes. 
 * param: timeOut - 
 * defines the maximum computing time of the method in seconds. 
 * If it does not returns: with a solution, it returns with an error code. 
 * param: moves - 
 * a list to contain suggested solution moves. If a solution was found, 
 * the list is cleared before adding the required moves to it. Otherwise, the list remains the same. 
 * param: pattern - 
 * the desired pattern definition string. The returned list of moves
 * can bring entered cube to this pattern (and not to the standard solved cube).
 * param: depth -
 * stores the total depth of the algorithm.
 * returns: the corresponding error code
 * */
int findSolution(char *cube, int maxDepth, long timeOut, Move moves[maxDepth],
                 char *pattern, int *depth);

int findSolutionBasic(char *cube, int maxDepth, long timeOut,
                      Move moves[maxDepth], int *depth);

/**
 * Validates input cube string and returns a corresponding error code.
 */
int validateCubeStringAndInitCubieCube(char *cube, CubieCube *cubieCube);

/**
 * Creates the list of moves that was found for solving the cube
 * param: moves - the list to contain the moves for solving the cube
 * param: depth - the depth of the solution that was found (number of moves)
 */
void createMovesList(Move moves[], int depth);

/**
 * Apply phase2 of algorithm and returns: the combined phase1 and phase2 depth.
 * In phase2, only the moves U,D,R2,F2,L2 and B2 are allowed.
 * param: depthPhase1 is the maneuver length phase1 that led to this phase2
 * search
 * param: maxDepth defines the maximal allowed maneuver length
 * returns: total maneuver length
 */
int totalDepth(int depthPhase1, int maxDepth);

#endif // !TWOPHASE_H
