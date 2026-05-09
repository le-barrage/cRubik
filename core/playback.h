#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <stdbool.h>

#define PLAYBACK_TEXT_LEN   1024
#define PLAYBACK_MAX_MOVES  256

/* Tracks an in-progress self-replay of a move sequence (e.g., a Kociemba
 * solution or a saved pattern). The cube renderer reads .text and
 * .current_move_index to highlight the move currently being animated. */
typedef struct
{
  bool active;
  char text[PLAYBACK_TEXT_LEN];
  int current_move_index;
  int pops_per_token[PLAYBACK_MAX_MOVES];
  int token_count;
  int pops_remaining;
} playback_t;

extern playback_t playback;

/* Begin playback of a space-separated move sequence. Each token contributes
 * one or two queue pops (two if the token ends in '2'). */
void playback_init (const char *text);

/* Step playback forward by one popped move from the move queue. No-op when
 * playback is inactive. */
void playback_advance (void);

/* Tear down playback state. Called when the move queue drains. */
void playback_clear (void);

#endif // PLAYBACK_H
