#ifndef PLATFORM_H
#define PLATFORM_H

/* Atomically replace dst with src. Returns 0 on success, -1 on failure. */
int atomic_replace (const char *src, const char *dst);

#endif