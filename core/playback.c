#include "playback.h"

#include <string.h>

playback_t playback = { .active = false, .current_move_index = -1 };

static int count_pops_and_tokens (const char *text, int pops_per_token[PLAYBACK_MAX_MOVES])
{
    int token_count = 0;
    int i           = 0;
    while (text[i] != '\0' && token_count < PLAYBACK_MAX_MOVES) {
        if (text[i] == ' ') {
            i++;
            continue;
        }
        int j = i;
        while (text[j] != '\0' && text[j] != ' ') j++;
        int pops                      = (j - i >= 2 && text[j - 1] == '2') ? 2 : 1;
        pops_per_token[token_count++] = pops;
        i                             = j;
    }
    return token_count;
}

void playback_init (const char *text)
{
    strncpy(playback.text, text, sizeof(playback.text) - 1);
    playback.text[sizeof(playback.text) - 1] = '\0';
    playback.token_count                     = count_pops_and_tokens(playback.text, playback.pops_per_token);
    playback.current_move_index              = -1;
    playback.pops_remaining                  = 0;
    playback.active                          = true;
}

void playback_advance (void)
{
    if (!playback.active) return;
    if (playback.pops_remaining <= 0) {
        playback.current_move_index++;
        if (playback.current_move_index < playback.token_count)
            playback.pops_remaining = playback.pops_per_token[playback.current_move_index];
    }
    playback.pops_remaining--;
}

void playback_clear (void)
{
    playback.active             = false;
    playback.current_move_index = -1;
    playback.pops_remaining     = 0;
}
