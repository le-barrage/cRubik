#include "enums.h"

Face getCorrespondingColor(char color) {
  switch (color) {
  case 'U':
    return UP;
  case 'R':
    return RIGHT;
  case 'F':
    return FRONT;
  case 'D':
    return DOWN;
  case 'L':
    return LEFT;
  case 'B':
    return BACK;
  }
  return UP;
}
