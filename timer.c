#include "timer.h"
#include "time_consts.h"

void
timer_start (stopwatch_t *timer)
{
  clock_gettime (CLOCK_MONOTONIC, &timer->start_time);
  timer->is_running = true;
}

void
timer_update (stopwatch_t *timer)
{
  if (!timer->is_running)
    return;

  struct timespec now;
  clock_gettime (CLOCK_MONOTONIC, &now);

  long long elapsed_ms
      = (now.tv_sec - timer->start_time.tv_sec) * (long long)MS_PER_SEC
        + (now.tv_nsec - timer->start_time.tv_nsec) / NS_PER_MS;

  timer->minutes = (int)(elapsed_ms / MS_PER_MIN);
  timer->seconds = (int)((elapsed_ms / MS_PER_SEC) % SECONDS_PER_MIN);
  timer->milliseconds = (int)(elapsed_ms % MS_PER_SEC);
}

void
timer_stop (stopwatch_t *timer)
{
  timer_update (timer);
  timer->is_running = false;
  timer->just_stopped = true;
}

void
timer_disable (stopwatch_t *timer)
{
  timer->is_disabled = true;
}

void
timer_enable (stopwatch_t *timer)
{
  timer->is_disabled = false;
}
