#include "keybindings.h"

#include "raylib.h"
#include <ctype.h>
#include <stdio.h>

/* GLFW is statically linked into libraylib.a: forward-declare the one symbol
 * we need rather than depending on the GLFW header. glfwGetKeyName returns
 * the layout-aware character that a physical key produces under the current
 * OS keyboard layout (e.g., the W-position key returns "z" on AZERTY). */
extern const char *glfwGetKeyName (int key, int scancode);

keybindings_t keybindings;

void
keybindings_init (void)
{
  keybindings.key_R = KEY_R;
  keybindings.key_L = KEY_L;
  keybindings.key_U = KEY_U;
  keybindings.key_D = KEY_D;
  keybindings.key_F = KEY_F;
  keybindings.key_B = KEY_B;
  keybindings.key_M = KEY_M;
  keybindings.key_S = KEY_S;
  keybindings.key_E = KEY_E;
  keybindings.key_X = KEY_X;
  keybindings.key_Y = KEY_Y;
  keybindings.key_Z = KEY_Z;
  keybindings.key_ALT = KEY_LEFT_ALT;
}

const char *
keybindings_label (int key)
{
  switch (key)
    {
    case KEY_LEFT_ALT:      return "L-ALT";
    case KEY_RIGHT_ALT:     return "R-ALT";
    case KEY_LEFT_SHIFT:    return "L-SHIFT";
    case KEY_RIGHT_SHIFT:   return "R-SHIFT";
    case KEY_LEFT_CONTROL:  return "L-CTRL";
    case KEY_RIGHT_CONTROL: return "R-CTRL";
    case KEY_SPACE:         return "SPACE";
    case KEY_ENTER:         return "ENTER";
    case KEY_TAB:           return "TAB";
    case KEY_BACKSPACE:     return "BACKSPACE";
    case KEY_ESCAPE:        return "ESCAPE";
    }

  static char str[16];

  const char *name = glfwGetKeyName (key, 0);
  if (name != NULL && name[0] != '\0')
    {
      size_t i = 0;
      while (name[i] != '\0' && i < sizeof (str) - 1)
        {
          str[i] = (i == 0) ? (char)toupper ((unsigned char)name[i]) : name[i];
          i++;
        }
      str[i] = '\0';
      return str;
    }

  snprintf (str, sizeof str, "%d", key);
  return str;
}
