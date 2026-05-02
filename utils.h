#ifndef UTILS_H
#define UTILS_H

#include "include/raylib.h"

#define ARRAY_LEN(array) (sizeof (array) / sizeof (array[0]))

#define DEFAULT_FONT_SIZE 20

bool colors_equal (Color a, Color b);

int Cnk (int n, int k);

#endif // UTILS_H
