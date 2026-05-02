#ifndef UTILS_H
#define UTILS_H

#include "include/raylib.h"

#define ARRAY_LEN(array) (sizeof (array) / sizeof (array[0]))

#define DEFAULT_FONT_SIZE 20

bool colors_equal (Color a, Color b);

int Cnk (int n, int k);

/* Solve-time helpers. Planned to migrate to average.{c,h} in a follow-up
 * batch, they all operate on the same JSON solve files that average.c
 * already manipulates. */
void storeTime (char *time, char *scramble, int size);
int timeToSeconds (char time[10]);
int timeToMillis (char time[10]);
int getMinutesFromMillis (int millis);
int getSecondsFromMillis (int millis);
int getMillisFromMillis (int millis);
void getFileName (char filename[20], int cubeSize);

#endif // UTILS_H
