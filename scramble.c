#include "scramble.h"
#include "include/raylib.h"
#include <stdio.h>
#include <string.h>

char *possibleMoves[] = {"R", "R'", "R2", "L", "L'", "L2", "U", "U'", "U2",
                         "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2"};
char *possibleMoves2x2x2[] = {"R",  "R'", "R2", "U", "U'",
                              "U2", "F",  "F'", "F2"};

bool areOppsitefaces(char c1, char c2) {
  return (c1 == 'R' && c2 == 'L') || (c1 == 'L' && c2 == 'R') ||
         (c1 == 'U' && c2 == 'D') || (c1 == 'D' && c2 == 'U') ||
         (c1 == 'F' && c2 == 'B') || (c1 == 'B' && c2 == 'F');
}

bool moveIsValid(const char *fullMove, const char *lastMove,
                 char *sequence[SCRAMBLE_SIZE],
                 unsigned short int sequenceLength) {
  if (strcmp(fullMove, lastMove) == 0 || fullMove[2] == lastMove[2])
    return false;
  if (sequenceLength > 1) {
    const char *secondtoLastMove = sequence[sequenceLength - 2];
    if (fullMove[2] == secondtoLastMove[2] &&
        areOppsitefaces(fullMove[2], lastMove[2]))
      return false;
  }
  return true;
}

char **generateScramble(char *sequence[SCRAMBLE_SIZE],
                        unsigned short int cubeSize) {
  unsigned short int sequenceLength = 0;
  char **moves = (cubeSize == 2) ? possibleMoves2x2x2 : possibleMoves;
  int possibleMovesSize = (cubeSize == 2) ? 9 : 18;

  while (sequenceLength < SCRAMBLE_SIZE) {
    unsigned short int rand = GetRandomValue(0, possibleMovesSize - 1);
    unsigned short int n =
        (cubeSize == 1) ? 1 : GetRandomValue(1, cubeSize / 2);
    const char *currentMove = moves[rand];

    const char layers[] = "%dw%s";
    unsigned short int size = snprintf(NULL, 0, layers, n, currentMove);
    char fullMove[size + 1];
    snprintf(fullMove, sizeof fullMove, layers, n, currentMove);

    if (sequenceLength < 1) {
      sequence[sequenceLength] = strdup(fullMove);
      sequenceLength++;
      continue;
    }

    const char *lastMove = sequence[sequenceLength - 1];
    if (!moveIsValid(fullMove, lastMove, sequence, sequenceLength))
      continue;

    sequence[sequenceLength] = strdup(fullMove);
    sequenceLength++;
  }
  return sequence;
}
