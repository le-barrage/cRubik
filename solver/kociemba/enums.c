#include "enums.h"

face_t getCorrespondingColor(char color) {
  switch (color) {
  case 'U':
    return FACE_UP;
  case 'R':
    return FACE_RIGHT;
  case 'F':
    return FACE_FRONT;
  case 'D':
    return FACE_DOWN;
  case 'L':
    return FACE_LEFT;
  case 'B':
    return FACE_BACK;
  }
  return FACE_UP;
}
