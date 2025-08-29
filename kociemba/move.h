#ifndef MOVE_H
#define MOVE_H

#include "enums.h"

typedef struct Move {
  Face orientation;
  Direction direction;
} Move;

Move Move_createMove(Face orientation, Direction direction);

#endif // !MOVE_H
