#include "ui_patterns.h"

#include "cube.h"
#include "raylib.h"
#include "patterns.h"
#include "utils.h"

#include <string.h>

#define PATTERN_BUTTON_W       300
#define PATTERN_BUTTON_H       50
#define PATTERN_SPACING        20
#define PATTERN_COLUMNS        2
#define PATTERN_START_Y        100
#define PATTERN_BUTTON_RADIUS  0.2f
#define HOVER_DARKEN           -0.1f
#define REST_LIGHTEN            0.1f
#define HINT_MARGIN             10
#define BACKGROUND_COLOR        GRAY

/* Pattern arrays encode 180° turns as two consecutive identical rotations
 * (e.g., R R or R' R'). When formatting display text we merge such pairs
 * into the conventional "R2" form. */
static void
format_pattern_moves (size_t pattern_idx, queue_t *queue, char *out, size_t out_size)
{
  const pattern_t *p = &PATTERNS[pattern_idx];
  size_t pos = 0;
  bool wrote_first_token = false;
  if (out_size > 0)
    out[0] = '\0';

  for (size_t j = 0; j < p->move_count; j++)
    {
      queue_push (queue, p->moves[j]);

      bool is_pair_second = (j > 0 && p->moves[j] == p->moves[j - 1]);
      if (is_pair_second)
        continue;

      bool is_half = (j + 1 < p->move_count && p->moves[j + 1] == p->moves[j]);

      char tok[3];
      cube_rotation_token (p->moves[j], tok);
      size_t tok_len = strlen (tok);

      if (is_half)
        {
          if (tok_len > 0 && tok[tok_len - 1] == '\'')
            tok[tok_len - 1] = '2';
          else
            {
              tok[tok_len] = '2';
              tok[tok_len + 1] = '\0';
              tok_len++;
            }
        }

      size_t needed = (wrote_first_token ? 1 : 0) + tok_len;
      if (pos + needed + 1 >= out_size)
        continue;
      if (wrote_first_token)
        out[pos++] = ' ';
      memcpy (out + pos, tok, tok_len);
      pos += tok_len;
      wrote_first_token = true;
    }
  if (out_size > 0)
    out[pos] = '\0';
}

bool
ui_patterns_draw (queue_t *queue, char *out_text, size_t out_size)
{
  ClearBackground (BACKGROUND_COLOR);
  int hint_w = MeasureText ("Press 'p' to exit.", DEFAULT_FONT_SIZE);
  DrawText ("Press 'p' to exit.", GetScreenWidth () - hint_w - HINT_MARGIN,
            HINT_MARGIN, DEFAULT_FONT_SIZE, DARKGRAY);

  int start_x
      = (GetScreenWidth ()
         - (PATTERN_COLUMNS * PATTERN_BUTTON_W
            + (PATTERN_COLUMNS - 1) * PATTERN_SPACING))
        / 2;
  bool any_hover = false;
  bool selected = false;

  for (int i = 0; i < (int)PATTERNS_COUNT; i++)
    {
      int row = i / PATTERN_COLUMNS;
      int col = i % PATTERN_COLUMNS;
      int x = start_x + col * (PATTERN_BUTTON_W + PATTERN_SPACING);
      int y = PATTERN_START_Y + row * (PATTERN_BUTTON_H + PATTERN_SPACING);

      Rectangle button = (Rectangle){
        .x = x, .y = y, .width = PATTERN_BUTTON_W, .height = PATTERN_BUTTON_H
      };
      bool hovering = CheckCollisionPointRec (GetMousePosition (), button);
      any_hover |= hovering;

      if (hovering)
        {
          DrawRectangleRounded (button, PATTERN_BUTTON_RADIUS, 0,
                                ColorBrightness (DARKGRAY, HOVER_DARKEN));
          if (IsMouseButtonPressed (MOUSE_LEFT_BUTTON))
            {
              format_pattern_moves (i, queue, out_text, out_size);
              selected = true;
            }
        }
      else
        DrawRectangleRounded (button, PATTERN_BUTTON_RADIUS, 0,
                              ColorBrightness (DARKGRAY, REST_LIGHTEN));

      int text_w = MeasureText (PATTERNS[i].name, DEFAULT_FONT_SIZE);
      DrawText (PATTERNS[i].name, x + (PATTERN_BUTTON_W - text_w) / 2,
                y + (PATTERN_BUTTON_H - DEFAULT_FONT_SIZE) / 2,
                DEFAULT_FONT_SIZE, BLACK);
    }
  SetMouseCursor (any_hover ? MOUSE_CURSOR_POINTING_HAND
                            : MOUSE_CURSOR_DEFAULT);
  return selected;
}
