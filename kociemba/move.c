#include "move.h"

Move Move_createMove(face_t orientation, Direction direction) {
  Move move;
  move.orientation = orientation;
  move.direction = direction;
  return move;
}
