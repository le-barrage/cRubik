#include "options.h"
#include "cube.h"
#include "include/cJSON.h"
#include "include/raygui.h"
#include "include/raylib.h"
#include "keybindings.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPTIONS_FILE "options.json"
#define OPTIONS_VERSION 1

#define BACKGROUND_COLOR GRAY

#define DEFAULT_ROTATION_SPEED 25
#define DEFAULT_SOLVER_OUTPUT_MODE SOLVER_REORIENT

SolverOutputMode solverOutputMode = DEFAULT_SOLVER_OUTPUT_MODE;

static int editingKeyIndex = -1;

typedef struct
{
  const char *name;
  int *keyPtr;
} KeyBindingEntry;

static const KeyBindingEntry keyBindingEntries[] = {
  { "R",   &keybindings.key_R   },
  { "L",   &keybindings.key_L   },
  { "U",   &keybindings.key_U   },
  { "D",   &keybindings.key_D   },
  { "F",   &keybindings.key_F   },
  { "B",   &keybindings.key_B   },
  { "M",   &keybindings.key_M   },
  { "S",   &keybindings.key_S   },
  { "E",   &keybindings.key_E   },
  { "X",   &keybindings.key_X   },
  { "Y",   &keybindings.key_Y   },
  { "Z",   &keybindings.key_Z   },
  { "CCW", &keybindings.key_ALT },
};
#define KEY_BINDING_COUNT ARRAY_LEN (keyBindingEntries)

static bool
drawKeyBindingsUI (int startY)
{
  int buttonWidth = 120;
  int buttonHeight = 35;
  int labelWidth = 50;
  int spacing = 15;
  int columns = 3;
  int totalWidth = columns * (labelWidth + buttonWidth + spacing * 2);
  int startX = (GetScreenWidth () - totalWidth) / 2;

  const char *title = "Key Bindings (click to change):";
  int titleWidth = MeasureText (title, DEFAULT_FONT_SIZE);
  DrawText (title, (GetScreenWidth () - titleWidth) / 2, startY,
            DEFAULT_FONT_SIZE, BLACK);

  startY += 40;

  bool isHoveringButton = false;

  for (int i = 0; i < (int)KEY_BINDING_COUNT; i++)
    {
      int row = i / columns;
      int col = i % columns;
      int x = startX + col * (labelWidth + buttonWidth + spacing * 3);
      int y = startY + row * (buttonHeight + spacing);

      DrawText (TextFormat ("%s:", keyBindingEntries[i].name), x,
                y + (buttonHeight - DEFAULT_FONT_SIZE) / 2, DEFAULT_FONT_SIZE,
                BLACK);

      Rectangle button = (Rectangle){ .x = x + labelWidth,
                                      .y = y,
                                      .width = buttonWidth,
                                      .height = buttonHeight };

      bool isHovering = CheckCollisionPointRec (GetMousePosition (), button);
      bool isEditing = (editingKeyIndex == i);
      isHoveringButton |= isHovering;

      Color buttonColor;
      if (isEditing)
        buttonColor = GREEN;
      else if (isHovering)
        buttonColor = ColorBrightness (DARKGRAY, -0.1f);
      else
        buttonColor = ColorBrightness (DARKGRAY, 0.1f);

      DrawRectangleRounded (button, 0.2f, 0, buttonColor);

      const char *keyText
          = isEditing ? "Press key..."
                      : key_name (*keyBindingEntries[i].keyPtr);

      int textW = MeasureText (keyText, DEFAULT_FONT_SIZE);
      DrawText (keyText, button.x + (button.width - textW) / 2,
                button.y + (button.height - DEFAULT_FONT_SIZE) / 2,
                DEFAULT_FONT_SIZE, isEditing ? WHITE : BLACK);

      if (isHovering && IsMouseButtonPressed (MOUSE_LEFT_BUTTON))
        editingKeyIndex = i;
    }

  if (editingKeyIndex >= 0)
    {
      int key = GetKeyPressed ();
      if (key > 0 && key != KEY_O && key != KEY_ESCAPE && key != KEY_SPACE
          && key != KEY_ENTER)
        {
          *keyBindingEntries[editingKeyIndex].keyPtr = key;
          editingKeyIndex = -1;
        }
      if (IsKeyPressed (KEY_ESCAPE))
        editingKeyIndex = -1;
    }

  return isHoveringButton;
}

static void
rotationSpeedSlider (int startY)
{
  float r = (float)ROTATIONSPEED;
  int sliderWidth = 150, sliderHeight = 30;
  Rectangle sliderRectangle
      = (Rectangle){ .x = (float)(GetScreenWidth () - sliderWidth) / 2,
                     .y = (float)startY,
                     .width = sliderWidth,
                     .height = sliderHeight };

  if (GuiSlider (sliderRectangle, "0", "30", &r, 1.f, 30.f))
    ROTATIONSPEED = (int)r;

  const char *crs = "Cube Rotation Speed:";
  DrawText (
      crs,
      sliderRectangle.x
          + (sliderRectangle.width - MeasureText (crs, DEFAULT_FONT_SIZE)) / 2,
      sliderRectangle.y - 30, DEFAULT_FONT_SIZE, BLACK);
  const char *rs = TextFormat ("%d", ROTATIONSPEED);
  DrawText (rs,
            sliderRectangle.x
                + (sliderRectangle.width - MeasureText (rs, DEFAULT_FONT_SIZE))
                      / 2,
            sliderRectangle.y + sliderRectangle.height + 10,
            DEFAULT_FONT_SIZE, BLACK);
}

static void
solverOutputModeToggle (int startY)
{
  int toggleWidth = 280, toggleHeight = 30;
  Rectangle bounds
      = (Rectangle){ .x = (float)(GetScreenWidth () - toggleWidth) / 2,
                     .y = (float)startY,
                     .width = (float)toggleWidth / 2,
                     .height = (float)toggleHeight };

  int active = (int)solverOutputMode;
  GuiToggleGroup (bounds, "Re-orient cube;Preserve view", &active);
  solverOutputMode = (SolverOutputMode)active;

  const char *label = "Solver output:";
  DrawText (label,
            (GetScreenWidth () - MeasureText (label, DEFAULT_FONT_SIZE)) / 2,
            startY - 30, DEFAULT_FONT_SIZE, BLACK);
}

static bool
drawResetButton (int y)
{
  const char *resetText = "Reset to Defaults";
  int textW = MeasureText (resetText, DEFAULT_FONT_SIZE);
  int width = textW + 20;
  int height = 35;
  Rectangle button = (Rectangle){ .x = (GetScreenWidth () - width) / 2,
                                  .y = (float)y,
                                  .width = (float)width,
                                  .height = (float)height };

  bool isHovering = CheckCollisionPointRec (GetMousePosition (), button);
  DrawRectangleRounded (button, 0.2f, 0,
                        isHovering ? ColorBrightness (MAROON, -0.1f)
                                   : ColorBrightness (MAROON, 0.1f));
  DrawText (resetText, button.x + (button.width - textW) / 2,
            button.y + (button.height - DEFAULT_FONT_SIZE) / 2,
            DEFAULT_FONT_SIZE, WHITE);

  if (isHovering && IsMouseButtonPressed (MOUSE_LEFT_BUTTON))
    Options_resetToDefaults ();

  return isHovering;
}

void
Options_resetToDefaults (void)
{
  keybindings_init ();
  ROTATIONSPEED = DEFAULT_ROTATION_SPEED;
  solverOutputMode = DEFAULT_SOLVER_OUTPUT_MODE;
  editingKeyIndex = -1;
}

void
Options_drawScreen (void)
{
  ClearBackground (BACKGROUND_COLOR);
  int exitTextWidth = MeasureText ("Press 'o' to exit.", DEFAULT_FONT_SIZE);
  DrawText ("Press 'o' to exit.", GetScreenWidth () - exitTextWidth - 10, 10,
            DEFAULT_FONT_SIZE, DARKGRAY);

  bool hovering = false;
  hovering |= drawKeyBindingsUI (60);
  rotationSpeedSlider (450);
  solverOutputModeToggle (550);
  hovering |= drawResetButton (GetScreenHeight () - 60);

  SetMouseCursor (hovering ? MOUSE_CURSOR_POINTING_HAND
                           : MOUSE_CURSOR_DEFAULT);
}

void
Options_load (void)
{
  FILE *f = fopen (OPTIONS_FILE, "rb");
  if (!f)
    return;

  fseek (f, 0, SEEK_END);
  long len = ftell (f);
  fseek (f, 0, SEEK_SET);
  if (len <= 0)
    {
      fclose (f);
      return;
    }

  char *buf = malloc (len + 1);
  if (!buf)
    {
      fclose (f);
      return;
    }
  fread (buf, 1, len, f);
  buf[len] = '\0';
  fclose (f);

  cJSON *root = cJSON_Parse (buf);
  free (buf);
  if (!root)
    {
      fprintf (stderr, "%s: parse error, keeping defaults\n", OPTIONS_FILE);
      return;
    }

  cJSON *version = cJSON_GetObjectItemCaseSensitive (root, "version");
  if (!cJSON_IsNumber (version) || version->valueint != OPTIONS_VERSION)
    {
      fprintf (stderr, "%s: unsupported version, keeping defaults\n",
               OPTIONS_FILE);
      cJSON_Delete (root);
      return;
    }

  cJSON *rs = cJSON_GetObjectItemCaseSensitive (root, "rotationSpeed");
  if (cJSON_IsNumber (rs))
    ROTATIONSPEED = rs->valueint;

  cJSON *som = cJSON_GetObjectItemCaseSensitive (root, "solverOutputMode");
  if (cJSON_IsString (som) && som->valuestring)
    {
      if (strcmp (som->valuestring, "preserve") == 0)
        solverOutputMode = SOLVER_PRESERVE;
      else if (strcmp (som->valuestring, "reorient") == 0)
        solverOutputMode = SOLVER_REORIENT;
    }

  cJSON *kb = cJSON_GetObjectItemCaseSensitive (root, "keybindings");
  if (cJSON_IsObject (kb))
    {
      for (size_t i = 0; i < KEY_BINDING_COUNT; i++)
        {
          cJSON *item = cJSON_GetObjectItemCaseSensitive (
              kb, keyBindingEntries[i].name);
          if (cJSON_IsNumber (item))
            *keyBindingEntries[i].keyPtr = item->valueint;
        }
    }

  cJSON_Delete (root);
}

void
Options_save (void)
{
  cJSON *root = cJSON_CreateObject ();
  if (!root)
    return;

  cJSON_AddNumberToObject (root, "version", OPTIONS_VERSION);
  cJSON_AddNumberToObject (root, "rotationSpeed", ROTATIONSPEED);
  cJSON_AddStringToObject (root, "solverOutputMode",
                           solverOutputMode == SOLVER_PRESERVE ? "preserve"
                                                               : "reorient");

  cJSON *kb = cJSON_AddObjectToObject (root, "keybindings");
  if (kb)
    {
      for (size_t i = 0; i < KEY_BINDING_COUNT; i++)
        cJSON_AddNumberToObject (kb, keyBindingEntries[i].name,
                                 *keyBindingEntries[i].keyPtr);
    }

  char *out = cJSON_Print (root);
  cJSON_Delete (root);
  if (!out)
    return;

  const char *tmpPath = OPTIONS_FILE ".tmp";
  FILE *f = fopen (tmpPath, "wb");
  if (!f)
    {
      perror ("fopen options.json.tmp");
      free (out);
      return;
    }
  fputs (out, f);
  fclose (f);
  free (out);

  if (rename (tmpPath, OPTIONS_FILE) != 0)
    {
      perror ("rename options.json");
      remove (tmpPath);
    }
}
