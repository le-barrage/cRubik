#include "font.h"

#include <stddef.h>

#define FONT_PATH       "JetBrainsMonoNerdFontMono-Regular.ttf"
#define FONT_CACHE_MAX  8

typedef struct
{
  int  size;
  Font font;
} cache_entry_t;

static cache_entry_t cache[FONT_CACHE_MAX];
static int           cache_count;

/* Mirrors raylib's DrawText default so MeasureTextEx widths match what
 * MeasureText returns at the same size. Keeps any existing positioning
 * math (e.g. ui_moves overlay placement) intact. */
static int
spacing_for (int size)
{
  int s = size / 10;
  return s < 1 ? 1 : s;
}

static const Font *
get_font (int size)
{
  for (int i = 0; i < cache_count; i++)
    if (cache[i].size == size)
      return &cache[i].font;

  if (cache_count >= FONT_CACHE_MAX)
    return NULL;

  Font f = LoadFontEx (FONT_PATH, size, NULL, 0);
  if (f.texture.id == 0)
    return NULL;

  cache[cache_count].size = size;
  cache[cache_count].font = f;
  return &cache[cache_count++].font;
}

void
font_draw (const char *text, int x, int y, int size, Color color)
{
  const Font *f = get_font (size);
  if (f)
    DrawTextEx (*f, text, (Vector2){ (float)x, (float)y }, (float)size,
                (float)spacing_for (size), color);
  else
    DrawText (text, x, y, size, color);
}

int
font_measure (const char *text, int size)
{
  const Font *f = get_font (size);
  if (f)
    return (int)MeasureTextEx (*f, text, (float)size,
                               (float)spacing_for (size)).x;
  return MeasureText (text, size);
}

Font
font_get (int size)
{
  const Font *f = get_font (size);
  return f ? *f : GetFontDefault ();
}

void
font_shutdown (void)
{
  for (int i = 0; i < cache_count; i++)
    UnloadFont (cache[i].font);
  cache_count = 0;
}
