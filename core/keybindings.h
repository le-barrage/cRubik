#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

typedef struct
{
  int key_R;
  int key_L;
  int key_U;
  int key_D;
  int key_F;
  int key_B;
  int key_M;
  int key_S;
  int key_E;
  int key_X;
  int key_Y;
  int key_Z;
  int key_ALT;
} keybindings_t;

extern keybindings_t keybindings;

void keybindings_init (void);

/* Returns a layout-aware display label for a physical keycode (e.g., "Z"
 * on AZERTY for the W-position key). The returned pointer points into a
 * shared static buffer: do not free, and the value may be overwritten. */
const char *keybindings_label (int key);

#endif // KEYBINDINGS_H
