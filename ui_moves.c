#include "ui_moves.h"

#include "include/raylib.h"
#include "utils.h"

#include <string.h>

#define MOVES_BUF_LEN     256
#define MOVES_MAX_TOKENS  64
#define MOVES_MAX_LINES   16
#define MOVES_LINE_HEIGHT 30

/* DrawText cursor advance after rendering `s` at x=0.
 *
 * raylib's DrawText advances its internal cursor by (advanceX*scale + spacing)
 * per character, so after N characters the cursor sits at
 *   sum(advanceX)*scale + N*spacing.
 * MeasureText returns the same minus one trailing spacing (it doesn't
 * account for the spacing past the last character) so adding `spacing`
 * once recovers the cursor position. Both formulas keep `spacing` unscaled.
 *
 * If we placed an overlay piece at lineX + MeasureText(prefix) directly,
 * the piece would land `spacing` pixels too far left.
 *
 * `spacing` follows raylib's DrawText default: fontSize/10, clamped >= 1. */
static int
draw_text_cursor_after (const char *s, int font_size)
{
  int spacing = font_size / 10;
  if (spacing < 1)
    spacing = 1;
  return MeasureText (s, font_size) + spacing;
}

void
ui_moves_draw (const char *text, float font_size, int y, int highlight_token)
{
  if (strlen (text) == 0)
    return;

  char work_copy[MOVES_BUF_LEN];
  strncpy (work_copy, text, sizeof (work_copy) - 1);
  work_copy[sizeof (work_copy) - 1] = '\0';

  char *tokens[MOVES_MAX_TOKENS];
  int token_count = 0;
  for (char *t = strtok (work_copy, " "); t && token_count < MOVES_MAX_TOKENS;
       t = strtok (NULL, " "))
    tokens[token_count++] = t;
  if (token_count == 0)
    return;

  int max_width = GetScreenWidth () - DEFAULT_FONT_SIZE;

  int line_start[MOVES_MAX_LINES] = { 0 };
  int line_count = 1;

  char line_buf[MOVES_BUF_LEN];
  int len0 = strlen (tokens[0]);
  memcpy (line_buf, tokens[0], len0);
  int line_len = len0;
  line_buf[line_len] = '\0';

  for (int i = 1; i < token_count; i++)
    {
      int tlen = strlen (tokens[i]);
      if (line_len + 1 + tlen + 1 > (int)sizeof (line_buf))
        break;
      line_buf[line_len] = ' ';
      memcpy (line_buf + line_len + 1, tokens[i], tlen);
      line_buf[line_len + 1 + tlen] = '\0';

      if (MeasureText (line_buf, font_size) > max_width
          && line_count < MOVES_MAX_LINES)
        {
          line_buf[line_len] = '\0';
          line_start[line_count++] = i;
          memcpy (line_buf, tokens[i], tlen);
          line_len = tlen;
          line_buf[line_len] = '\0';
        }
      else
        line_len += 1 + tlen;
    }
  line_start[line_count] = token_count;

  for (int line = 0; line < line_count; line++)
    {
      int start_idx = line_start[line];
      int end_idx = line_start[line + 1];

      char buf[MOVES_BUF_LEN];
      int len = 0;
      for (int i = start_idx; i < end_idx; i++)
        {
          if (i > start_idx && len + 1 < (int)sizeof (buf))
            buf[len++] = ' ';
          int tlen = strlen (tokens[i]);
          if (len + tlen >= (int)sizeof (buf))
            break;
          memcpy (buf + len, tokens[i], tlen);
          len += tlen;
        }
      buf[len] = '\0';

      int line_width = MeasureText (buf, font_size);
      int line_x = (GetScreenWidth () - line_width) / 2;
      int line_y = y + line * MOVES_LINE_HEIGHT;

      DrawText (buf, line_x, line_y, font_size, BLACK);

      if (highlight_token < start_idx || highlight_token >= end_idx)
        continue;

      int h_offset = 0;
      for (int i = start_idx; i < highlight_token; i++)
        h_offset += strlen (tokens[i]) + 1;
      int h_len = strlen (tokens[highlight_token]);

      int overlay_x = line_x;
      if (h_offset > 0)
        {
          char saved = buf[h_offset];
          buf[h_offset] = '\0';
          overlay_x += draw_text_cursor_after (buf, font_size);
          buf[h_offset] = saved;
        }

      char saved = buf[h_offset + h_len];
      buf[h_offset + h_len] = '\0';
      DrawText (buf + h_offset, overlay_x, line_y, font_size, GOLD);
      buf[h_offset + h_len] = saved;
    }
}
