#include "platform.h"

#include <windows.h>

int atomic_replace (const char *src, const char *dst)
{
    return MoveFileExA(src, dst, MOVEFILE_REPLACE_EXISTING) ? 0 : -1;
}