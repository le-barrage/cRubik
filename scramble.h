#ifndef SCRAMBLE_H
#define SCRAMBLE_H

#include "cube.h"

#define SCRAMBLE_SIZE ((SIZE <= 2) ? 10 : (SIZE > 12) ? 200 : 20 * (SIZE - 2))

char **generateScramble(char *sequence[SCRAMBLE_SIZE],
                        unsigned short int cubeSize);

#endif // !SCRAMBLE_H
