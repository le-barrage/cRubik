#ifndef UI_MOVES_H
#define UI_MOVES_H

/* Renders a space-separated token sequence (e.g., a scramble or solution)
 * centered horizontally at `y`, with greedy line wrapping at the screen
 * width. Pass highlight_token = -1 for no highlight, otherwise the
 * matching token is overlaid in GOLD on top of the BLACK base line. */
void ui_moves_draw (const char *text, float font_size, int y,
                    int highlight_token);

#endif // UI_MOVES_H
