#include "ui_help.h"

#include "raylib.h"
#include "utils.h"

#include <math.h>

#define HELP_MARGIN          100
#define HELP_MIN_FONT_SIZE   18
#define HELP_MAX_FONT_SIZE   40
#define HELP_LINE_HEIGHT     50
#define HINT_MARGIN          10
#define BACKGROUND_COLOR     GRAY

static const char *HELP_TEXTS[] = {
  "Press 'Enter' to scramble the cube.",
  "Press the corresponding key to move each face clockwise (Hold 'alt' down "
  "for counter-clockwise):",
  "R (right), L (left), U (up), D (down), F (front), B (back).",
  "Press 'K' to find an optimal solution to the cube (only 3x3x3).",
  "Press right mouse button to reset the cube to its original, solved state.",
  "Press middle mouse button to reset camera settings.",
  "Hold left mouse button down to move the camera around.",
  "Press the space bar to start (or stop) the timer.",
  "Press '-' or 'page down' to reduce the cube size and '+' or 'page up' to "
  "increase it.",
  "Press 'End' to toggle the debug axes overlay.",
};
#define HELP_TEXTS_COUNT ((int)ARRAY_LEN (HELP_TEXTS))

static int help_texts_max_length;

void
ui_help_init (void)
{
  int max = 0;
  for (int i = 0; i < HELP_TEXTS_COUNT; i++)
    {
      int t = MeasureText (HELP_TEXTS[i], DEFAULT_FONT_SIZE);
      max = t > max ? t : max;
    }
  help_texts_max_length = max;
}

void
ui_help_draw (void)
{
  int available_width = GetScreenWidth () - HELP_MARGIN;
  int chunk_size = help_texts_max_length / 2;
  int font_size
      = fmax (fmin (floor ((float)available_width / chunk_size) * 10,
                    HELP_MAX_FONT_SIZE),
              HELP_MIN_FONT_SIZE);

  ClearBackground (BACKGROUND_COLOR);
  DrawText ("Press 'h' to exit.", HINT_MARGIN, HINT_MARGIN, DEFAULT_FONT_SIZE,
            DARKGRAY);
  int start_y
      = GetScreenHeight () / 2 - HELP_TEXTS_COUNT / 2 * HELP_LINE_HEIGHT;
  for (int i = 0; i < HELP_TEXTS_COUNT; i++)
    DrawText (HELP_TEXTS[i],
              GetScreenWidth () / 2 - MeasureText (HELP_TEXTS[i], font_size) / 2,
              start_y + i * HELP_LINE_HEIGHT, font_size, BLACK);
}
