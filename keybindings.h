#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

/* Layout-position keycodes for AZERTY: raylib reports physical-position
 * scancodes. On AZERTY layouts the M / A / Q / Z / W positions have
 * different codes than on QWERTY. */
#define KEY_M_FR 59
#define KEY_A_FR 81
#define KEY_Q_FR 65
#define KEY_Z_FR 87
#define KEY_W_FR 90

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
const char *key_name (int key);

#endif // KEYBINDINGS_H
