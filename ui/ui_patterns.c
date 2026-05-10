#include "ui_patterns.h"

#include "font.h"
#include "raylib.h"
#include "patterns.h"
#include "utils.h"

#include <string.h>

#define PATTERN_BUTTON_W       350
#define PATTERN_BUTTON_H       50
#define PATTERN_SPACING        20
#define PATTERN_COLUMNS        2
#define PATTERN_START_Y        100
#define PATTERN_BUTTON_RADIUS  0.2f
#define HOVER_DARKEN           -0.1f
#define REST_LIGHTEN            0.1f
#define DISABLED_LIGHTEN        0.4f
#define HINT_MARGIN             10
#define BACKGROUND_COLOR        GRAY

static bool
pattern_applies (const pattern_t *p, int cube_size)
{
  if (cube_size < p->min_size)
    return false;
  if (p->max_size != 0 && cube_size > p->max_size)
    return false;
  return true;
}

bool
ui_patterns_draw (int cube_size, queue_t *queue, char *out_text,
                  size_t out_size)
{
  ClearBackground (BACKGROUND_COLOR);
  int hint_w = font_measure ("Press 'p' to exit.", DEFAULT_FONT_SIZE);
  font_draw ("Press 'p' to exit.", GetScreenWidth () - hint_w - HINT_MARGIN,
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
      bool applicable = pattern_applies (&PATTERNS[i], cube_size);
      bool hovering = applicable
                      && CheckCollisionPointRec (GetMousePosition (), button);
      any_hover |= hovering;

      Color fill;
      if (!applicable)
        fill = ColorBrightness (DARKGRAY, DISABLED_LIGHTEN);
      else if (hovering)
        fill = ColorBrightness (DARKGRAY, HOVER_DARKEN);
      else
        fill = ColorBrightness (DARKGRAY, REST_LIGHTEN);

      DrawRectangleRounded (button, PATTERN_BUTTON_RADIUS, 0, fill);

      if (hovering && IsMouseButtonPressed (MOUSE_LEFT_BUTTON))
        {
          PATTERNS[i].build (cube_size, queue, out_text, out_size);
          selected = true;
        }

      Color text_color = applicable ? BLACK : LIGHTGRAY;
      int text_w = font_measure (PATTERNS[i].name, DEFAULT_FONT_SIZE);
      font_draw (PATTERNS[i].name, x + (PATTERN_BUTTON_W - text_w) / 2,
                 y + (PATTERN_BUTTON_H - DEFAULT_FONT_SIZE) / 2,
                 DEFAULT_FONT_SIZE, text_color);
    }
  SetMouseCursor (any_hover ? MOUSE_CURSOR_POINTING_HAND
                            : MOUSE_CURSOR_DEFAULT);
  return selected;
}
