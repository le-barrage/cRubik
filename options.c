#include "options.h"
#include "cube.h"
#include "include/raygui.h"
#include "include/raylib.h"
#include "utils.h"
#include <stdio.h>

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

static KeyBindingEntry keyBindingEntries[13];

static void
initKeyBindingEntries (void)
{
  keyBindingEntries[0]  = (KeyBindingEntry){ "R",   &keyBindings.key_R };
  keyBindingEntries[1]  = (KeyBindingEntry){ "L",   &keyBindings.key_L };
  keyBindingEntries[2]  = (KeyBindingEntry){ "U",   &keyBindings.key_U };
  keyBindingEntries[3]  = (KeyBindingEntry){ "D",   &keyBindings.key_D };
  keyBindingEntries[4]  = (KeyBindingEntry){ "F",   &keyBindings.key_F };
  keyBindingEntries[5]  = (KeyBindingEntry){ "B",   &keyBindings.key_B };
  keyBindingEntries[6]  = (KeyBindingEntry){ "M",   &keyBindings.key_M };
  keyBindingEntries[7]  = (KeyBindingEntry){ "S",   &keyBindings.key_S };
  keyBindingEntries[8]  = (KeyBindingEntry){ "E",   &keyBindings.key_E };
  keyBindingEntries[9]  = (KeyBindingEntry){ "X",   &keyBindings.key_X };
  keyBindingEntries[10] = (KeyBindingEntry){ "Y",   &keyBindings.key_Y };
  keyBindingEntries[11] = (KeyBindingEntry){ "Z",   &keyBindings.key_Z };
  keyBindingEntries[12] = (KeyBindingEntry){ "CCW", &keyBindings.key_ALT };
}

static bool
drawKeyBindingsUI (int startY)
{
  static bool initialized = false;
  if (!initialized)
    {
      initKeyBindingEntries ();
      initialized = true;
    }

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

  for (int i = 0; i < 13; i++)
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
                      : getKeyName (*keyBindingEntries[i].keyPtr);

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
  initDefaultKeyBindings ();
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
