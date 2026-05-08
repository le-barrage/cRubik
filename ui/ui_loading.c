#include "ui_loading.h"

#include "raylib.h"
#include "utils.h"

#define LOADING_FONT_SIZE       40
#define LOADING_PHASE_LEN       10
#define LOADING_PERIOD_FRAMES   (4 * LOADING_PHASE_LEN)
#define BACKGROUND_COLOR        GRAY

void
ui_loading_draw (int frame_count)
{
  int phase = (frame_count % LOADING_PERIOD_FRAMES) / LOADING_PHASE_LEN;
  static const char *DOTS[4] = { "LOADING", "LOADING.", "LOADING..",
                                 "LOADING..." };
  BeginDrawing ();
  ClearBackground (BACKGROUND_COLOR);
  int text_w = MeasureText ("LOADING...", LOADING_FONT_SIZE);
  DrawText (DOTS[phase], GetScreenWidth () / 2 - text_w / 2,
            GetScreenHeight () / 2 - DEFAULT_FONT_SIZE, LOADING_FONT_SIZE,
            BLACK);
  EndDrawing ();
}
