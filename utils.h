#ifndef UTILS_H
#define UTILS_H

#include "include/raylib.h"
#include <stdio.h>

#define KEY_M_FR 59
#define KEY_A_FR 81
#define KEY_Q_FR 65
#define KEY_Z_FR 87
#define KEY_W_FR 90

bool colorsEqual(Color color1, Color color2);

int Cnk(int n, int k);

void storeTime(char *time, int size);

int timeToSeconds(char time[10]);

int timeToMillis(char time[10]);

int getMinutesFromMillis(int millis);

int getSecondsFromMillis(int millis);

int getMillisFromMillis(int millis);

int countLines(FILE *fp);

void getFileName(char filename[20], int cubeSize);

#endif // !UTILS_H
