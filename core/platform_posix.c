#include "platform.h"
#include <stdio.h>

int atomic_replace(const char *src, const char *dst) {
    return rename(src, dst);
}