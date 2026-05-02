#ifndef PATTERNS_H
#define PATTERNS_H

#include "cube.h"
#include <stddef.h>

typedef struct
{
  const Rotation *moves;
  size_t move_count;
  const char *name;
} pattern_t;

extern const pattern_t PATTERNS[];
extern const size_t PATTERNS_COUNT;

#endif // PATTERNS_H
