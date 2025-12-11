#include "move.h"

Move Move_createMove(Face orientation, Direction direction) {
  Move move;
  move.orientation = orientation;
  move.direction = direction;
  return move;
}
