#ifndef UI_LOADING_H
#define UI_LOADING_H

/* Renders a centered "LOADING..." spinner cycling through 0..3 dots. Owns
 * its own BeginDrawing/EndDrawing pair. `frame_count` drives the dot phase. */
void ui_loading_draw (int frame_count);

#endif // UI_LOADING_H
