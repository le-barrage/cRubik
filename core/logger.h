#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE,
} log_level_t;

/* Configures minimum emission level and output stream. Pass NULL for `out`
 * to use stderr. Safe to call multiple times. */
void log_init (log_level_t level, FILE *out);

void log_set_level (log_level_t level);
log_level_t log_get_level (void);

/* Internal: use the LOG_* macros below. */
void log_logf (log_level_t level, const char *file, int line, const char *fmt, ...)
    __attribute__((format(printf, 4, 5)));
void log_perror (log_level_t level, const char *file, int line, const char *fmt, ...)
    __attribute__((format(printf, 4, 5)));

#define LOG_DEBUG(fmt, ...) log_logf(LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  log_logf(LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  log_logf(LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_logf(LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/* Like LOG_ERROR but appends ": <strerror(errno)>" to the message. */
#define LOG_PERROR(fmt, ...) log_perror(LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif  // LOGGER_H
