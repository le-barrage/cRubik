#ifndef MOVE_H
#define MOVE_H

#include "enums.h"

typedef struct Move {
  face_t orientation;
  Direction direction;
} Move;

Move Move_createMove(face_t orientation, Direction direction);

#endif // !MOVE_H
