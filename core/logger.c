#include "logger.h"

#include "time_consts.h"

#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

static log_level_t current_level = LOG_LEVEL_INFO;
static FILE *output;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static const char *LEVEL_NAMES[] = {
    [LOG_LEVEL_DEBUG] = "DEBUG",
    [LOG_LEVEL_INFO]  = "INFO ",
    [LOG_LEVEL_WARN]  = "WARN ",
    [LOG_LEVEL_ERROR] = "ERROR",
};

void log_init (log_level_t level, FILE *out)
{
    pthread_mutex_lock(&lock);
    current_level = level;
    output        = (out == NULL) ? stderr : out;
    pthread_mutex_unlock(&lock);
}

void log_set_level (log_level_t level)
{
    pthread_mutex_lock(&lock);
    current_level = level;
    pthread_mutex_unlock(&lock);
}

log_level_t log_get_level (void) { return current_level; }

static void write_prefix (FILE *out, log_level_t level, const char *file, int line)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tm;
    localtime_r(&ts.tv_sec, &tm);
    int ms = (int)(ts.tv_nsec / NS_PER_MS);
    fprintf(out, "[%02d:%02d:%02d.%03d] %s %s:%d: ", tm.tm_hour, tm.tm_min, tm.tm_sec, ms, LEVEL_NAMES[level], file,
            line);
}

void log_logf (log_level_t level, const char *file, int line, const char *fmt, ...)
{
    if (level < current_level) return;
    pthread_mutex_lock(&lock);
    FILE *out = (output == NULL) ? stderr : output;
    write_prefix(out, level, file, line);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    va_end(ap);
    fputc('\n', out);
    pthread_mutex_unlock(&lock);
}

void log_perror (log_level_t level, const char *file, int line, const char *fmt, ...)
{
    if (level < current_level) return;
    int saved_errno = errno;
    pthread_mutex_lock(&lock);
    FILE *out = (output == NULL) ? stderr : output;
    write_prefix(out, level, file, line);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    va_end(ap);
    fprintf(out, ": %s\n", strerror(saved_errno));
    pthread_mutex_unlock(&lock);
}
