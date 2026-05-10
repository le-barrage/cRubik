#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <time.h>

/* Stopwatch-style timer.
 *
 * `is_running` reflects whether the timer is currently counting (between
 * timer_start and timer_stop). The display fields (minutes/seconds/
 * milliseconds) are updated by timer_update against the elapsed time since
 * the last timer_start. The caller must call timer_update each frame.
 *
 * `is_disabled` is an external input gate independent of is_running. It
 * exists so input-handling code can suppress the arming behavior without
 * affecting whether the timer is counting.
 *
 * `just_stopped` is a one-shot handshake flag set by timer_stop. The caller
 * is responsible for resetting it once it has acknowledged the stop. */
typedef struct {
    int minutes;
    int seconds;
    int milliseconds;
    struct timespec start_time;
    bool is_running;
    bool just_stopped;
    bool is_disabled;
} stopwatch_t;

void timer_start (stopwatch_t *timer);
void timer_update (stopwatch_t *timer);
void timer_stop (stopwatch_t *timer);
void timer_disable (stopwatch_t *timer);
void timer_enable (stopwatch_t *timer);

#endif  // TIMER_H
