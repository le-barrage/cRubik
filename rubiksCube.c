#include "average.h"
#include "cube.h"
#include "include/raylib.h"
#include "include/raymath.h"
#include "kociemba/coordCube.h"
#include "kociemba/enums.h"
#include "kociemba/twoPhase.h"
#include "keybindings.h"
#include "options.h"
#include "patterns.h"
#include "queue.h"
#include "scramble.h"
#include "timer.h"
#include "utils.h"
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define RAYGUI_IMPLEMENTATION
#include "include/raygui.h"

#define CUBIE_SIZE 0.98
#define BACKGROUND_COLOR GRAY

#define KEEP_SPACE_DOWN_MS 300

short SOLUTION_DEPTH = 22;

float camera_mag;
float camera_mag_vel;
float camera_theta;
float camera_phi;

Camera camera = { { 0 }, { 0, 0, 0 }, { 0, 1, 0 }, 90, CAMERA_PERSPECTIVE };

cube_t cube;
char **scramble, *currentScramble, currentSolution[100], solutionFoundText[45],
    times[LAST_N_SOLVES][TIME_STR_MAX], avg[AVG_STR_LEN];
int currentSolutionSize;
queue_t *queue;
bool isSolutionRunning = false, isThreadLaunched = false;
pthread_t solutionThread;

typedef struct
{
  bool active;
  char text[256];
  int currentMoveIndex;
  int popsPerToken[64];
  int tokenCount;
  int popsRemaining;
} Playback;

Playback playback = { .active = false, .currentMoveIndex = -1 };

static int
countPopsAndTokens (const char *text, int popsPerToken[64])
{
  int tokenCount = 0;
  int i = 0;
  while (text[i] != '\0' && tokenCount < 64)
    {
      if (text[i] == ' ')
        {
          i++;
          continue;
        }
      int j = i;
      while (text[j] != '\0' && text[j] != ' ')
        j++;
      int pops = (j - i >= 2 && text[j - 1] == '2') ? 2 : 1;
      popsPerToken[tokenCount++] = pops;
      i = j;
    }
  return tokenCount;
}

static void
initPlaybackFromText (const char *text)
{
  strncpy (playback.text, text, sizeof (playback.text) - 1);
  playback.text[sizeof (playback.text) - 1] = '\0';
  playback.tokenCount
      = countPopsAndTokens (playback.text, playback.popsPerToken);
  playback.currentMoveIndex = -1;
  playback.popsRemaining = 0;
  playback.active = true;
}

bool showHelp = false, showExitMessageBox = false, showOptions = false,
     isEverythingLoaded = false, showPatterns = false;

char *helpTexts[] = {
  "Press 'Enter' to scramble the cube.",
  "Press the corresponding key to move each face clockwise (Hold 'alt' down "
  "for counter-clockwise):",
  "R (right), L (left), U (up), D (down), F (front), B (back).",
  "Press 'K' to find an optimal solution to the cube (only 3x3x3).",
  "Press right mouse button to reset the cube to its original, solved state.",
  "Press middle mouse button to reset camera settings.",
  "Hold left mouse button down to move the camera around.",
  "Press the space bar to start (or stop) the timer.",
  "Press '-' or 'page down' to reduce the cube size and '+' or 'page up' to "
  "increase it.",
};
int helpTextsSize = ARRAY_LEN (helpTexts);
int helpTextsMaxLength;

stopwatch_t timer;
Color timerColor = BLACK;
char timerString[10] = "00:00.000";
bool isTimerReady = false;
struct timespec keySpaceDownStart = { .tv_nsec = -1 }, now;

bool show = false;
int timeToShow = -1, posYToShow = 0;

bool exitProgram = false;

void
handleRotation (rotation_t clockwise, rotation_t antiClockwise)
{
  if (isSolutionRunning)
    return;
  currentSolution[0] = '\0';
  currentSolutionSize = 0;
  if (IsKeyDown (keybindings.key_ALT))
    queue_push (queue, antiClockwise);
  else
    queue_push (queue, clockwise);
}

void
applyMovesAndUpdateCurrentScramble ()
{
  int length = scramble_length (cube.size);
  for (int i = 0; i < length; i++)
    {
      cube_apply_move (&cube, scramble[i]);
      if (scramble[i][0] == '1' && scramble[i][1] == 'w')
        strcat (currentScramble, scramble[i] + 2);
      else
        strcat (currentScramble, scramble[i]);
      free (scramble[i]);
      if (i != length - 1)
        strcat (currentScramble, " ");
    }
}

int
findSolutionAndUpdateMoves (cube_t *cube, int depthLimit, int timeOut)
{
  cube_t canonical;
  cube_orientation_t orientation
      = cube_detect_orientation_and_normalize (cube, &canonical);

  char cubeStr[CUBE_FACELET_STR_LEN];
  cube_to_string (&canonical, cubeStr, sizeof cubeStr);
  cube_destroy (&canonical);

  Move moves[25] = { 0 };
  int depth;
  int error = findSolutionBasic (cubeStr, depthLimit, timeOut, moves, &depth);
  if (error != 0)
    return error;

  currentSolution[0] = '\0';

  if (solverOutputMode == SOLVER_REORIENT)
    cube_append_normalization_tokens (currentSolution, &orientation);

  int len = strlen (currentSolution);
  for (int i = 0; i < depth; i++)
    {
      Move cur = moves[i];
      face_t emitFace = (solverOutputMode == SOLVER_PRESERVE)
                          ? orientation.face_map[cur.orientation]
                          : cur.orientation;
      currentSolution[len++] = cube_face_letter (emitFace);
      if (cur.direction == ANTICW)
        currentSolution[len++] = '\'';
      else if (cur.direction == HALF)
        currentSolution[len++] = '2';
      if (i != depth - 1)
        currentSolution[len++] = ' ';
      currentSolutionSize++;
    }
  currentSolution[len] = '\0';

  return 0;
}

void *
findSolutionAndUpdateCurrentSolution (void *arg)
{
  (void)arg;
  if (cube.size != 3)
    {
      snprintf (currentSolution, 41,
                "The algorithm only works on 3x3x3 cubes.");
      isThreadLaunched = false;
      return NULL;
    }

  struct timespec start, now;

  currentSolutionSize = 0;

  clock_gettime (CLOCK_MONOTONIC, &start);
  int error = findSolutionAndUpdateMoves (&cube, SOLUTION_DEPTH, 20000);
  clock_gettime (CLOCK_MONOTONIC, &now);
  if (error != 0)
    {
      snprintf (currentSolution, 75, "%s", printErrorMessage (error));
      isThreadLaunched = false;
      return NULL;
    }

  long long elapsed_time_ns = (now.tv_sec - start.tv_sec) * 1000000000LL
                              + (now.tv_nsec - start.tv_nsec);
  double elapsed_time_ms = (double)elapsed_time_ns / 1000000.0;
  snprintf (solutionFoundText, 45,
            "%d moves solution found in ~%d milliseconds:",
            currentSolutionSize, (int)elapsed_time_ms);

  printf ("Solution found in ~%d milliseconds\n", (int)elapsed_time_ms);
  isThreadLaunched = false;
  return NULL;
}

void
clearCurrentScrambleAndSolution ()
{
  currentScramble[0] = '\0';
  currentSolution[0] = '\0';
  currentSolutionSize = 0;
}

void
resetAnimationAndSolution ()
{
  cube.is_animating = false;
  queue_clear (queue);
}

void
generateNewScramble ()
{
  clearCurrentScrambleAndSolution ();
  resetAnimationAndSolution ();
  int size = cube.size;
  cube_destroy (&cube);
  cube = cube_make (size, CUBIE_SIZE);
  if (scramble_generate (scramble, scramble_length (size), size)
      != SCRAMBLE_OK)
    {
      fprintf (stderr, "scramble_generate failed\n");
      return;
    }
  applyMovesAndUpdateCurrentScramble ();
  timer.is_disabled = false;
}

void
initCameraSettings ()
{
  camera_mag = 2 * cube.size;
  camera_mag_vel = 0.0f;
  camera_theta = PI / 5;
  camera_phi = PI / 3;
}

void
initCurrentScrambleAndSolution (int size)
{
  cube = cube_make (size, CUBIE_SIZE);
  int length = scramble_length (size);
  scramble = malloc (length * sizeof (char *));
  currentScramble = malloc ((6 * length + 1) * sizeof (char));
  clearCurrentScrambleAndSolution ();
  resetAnimationAndSolution ();
  avg[0] = '\0';
}

void
resizeCube (int increment)
{
  int new_size = cube.size;
  if (!((new_size == CUBE_MAX_SIZE && increment > 0)
        || (new_size == 1 && increment < 0)))
    new_size += increment;

  free (currentScramble);
  free (scramble);
  cube_destroy (&cube);

  initCurrentScrambleAndSolution (new_size);
  initCameraSettings ();
  solves_load_last_5 (times, new_size);
}

// TODO: Make this function more readable (change if)
void
applyCurrentSolution ()
{
  timer_disable (&timer);
  isSolutionRunning = true;
  currentSolutionSize = 0;

  initPlaybackFromText (currentSolution);

  int i = 0;
  char rotation;
  while (currentSolution[i] != '\0')
    {
      char currMove = currentSolution[i], nextMove = currentSolution[i + 1];
      if (currMove == ' ')
        {
          i++;
          continue;
        }
      if (nextMove == '\'')
        {
          rotation = tolower (currMove);
          i++;
        }
      else if (nextMove == '2')
        {
          rotation_t r;
          if (rotation_from_char (currMove, &r))
            queue_push (queue, r);
          rotation = currMove;
          i++;
        }
      else
        {
          rotation = currMove;
        }
      rotation_t r;
      if (rotation_from_char (rotation, &r))
        queue_push (queue, r);
      i++;
    }
}

void
updateTimerString ()
{
  snprintf (timerString, 10, "%02d:%02d.%03d", timer.minutes, timer.seconds,
            timer.milliseconds);
}

void
handleKeyPress ()
{
  if (GetKeyPressed () && timer.is_running)
    {
      timer_stop (&timer);
      updateTimerString ();
      solves_save (timerString, currentScramble, cube.size);
      solves_load_last_5 (times, cube.size);
      solves_average_of_5 (times, avg);
      generateNewScramble ();
      timer.just_stopped = false;
      isTimerReady = false;
      return;
    }
  if (IsKeyPressed (keybindings.key_U))
    handleRotation (ROT_U, ROT_U_PRIME);
  else if (IsKeyPressed (keybindings.key_D))
    handleRotation (ROT_D, ROT_D_PRIME);
  else if (IsKeyPressed (keybindings.key_L))
    handleRotation (ROT_L, ROT_L_PRIME);
  else if (IsKeyPressed (keybindings.key_R))
    handleRotation (ROT_R, ROT_R_PRIME);
  else if (IsKeyPressed (keybindings.key_F))
    handleRotation (ROT_F, ROT_F_PRIME);
  else if (IsKeyPressed (keybindings.key_B))
    handleRotation (ROT_B, ROT_B_PRIME);
  else if (IsKeyPressed (keybindings.key_M))
    handleRotation (ROT_M, ROT_M_PRIME);
  else if (IsKeyPressed (keybindings.key_E))
    handleRotation (ROT_E, ROT_E_PRIME);
  else if (IsKeyPressed (keybindings.key_S))
    handleRotation (ROT_S, ROT_S_PRIME);
  else if (IsKeyPressed (keybindings.key_X))
    handleRotation (ROT_X, ROT_X_PRIME);
  else if (IsKeyPressed (keybindings.key_Y))
    handleRotation (ROT_Y, ROT_Y_PRIME);
  else if (IsKeyPressed (keybindings.key_Z))
    handleRotation (ROT_Z, ROT_Z_PRIME);
  else if (IsKeyPressed (KEY_ENTER))
    generateNewScramble ();
  else if (IsKeyPressed (KEY_K))
    {
      if (isThreadLaunched)
        return;
      isThreadLaunched = true;
      int error = pthread_create (&solutionThread, NULL,
                                  findSolutionAndUpdateCurrentSolution, NULL);
      if (error != 0)
        {
          printf ("Error creating thread: %s\n", strerror (error));
          return;
        }
      pthread_detach (solutionThread);
    }
  else if (IsKeyDown (KEY_SPACE) && !timer.is_disabled)
    {
      if (!isTimerReady)
        {
          if (keySpaceDownStart.tv_nsec == -1)
            {
              clock_gettime (CLOCK_MONOTONIC, &keySpaceDownStart);
              printf ("%lu\n", keySpaceDownStart.tv_sec);
            }
          else
            {
              clock_gettime (CLOCK_MONOTONIC, &now);
              long long elapsed_time_ns
                  = (now.tv_sec - keySpaceDownStart.tv_sec) * 1000000000LL
                    + (now.tv_nsec - keySpaceDownStart.tv_nsec);
              double elapsed_time_ms = (double)elapsed_time_ns / 1000000.0;
              isTimerReady = elapsed_time_ms > KEEP_SPACE_DOWN_MS;
            }
        }
      else if (!timer.is_running && !timer.just_stopped)
        timerColor = (Color){ 0, 204, 51, 255 };
      else if (!timer.just_stopped)
        {
          timer_stop (&timer);
        }
    }
  else if (IsKeyReleased (KEY_SPACE) && !timer.is_disabled)
    {
      timerColor = BLACK;
      if (!timer.is_running && isTimerReady)
        timer_start (&timer);
      keySpaceDownStart.tv_nsec = -1;
    }
  else if (IsKeyPressed (KEY_KP_ADD) || IsKeyPressed (KEY_PAGE_UP))
    resizeCube (1);
  else if (IsKeyPressed (KEY_KP_SUBTRACT) || IsKeyPressed (KEY_PAGE_DOWN))
    resizeCube (-1);
  else if (IsKeyPressed (KEY_ESCAPE))
    showExitMessageBox = true;
}

void
handleQueue ()
{
  if (cube.is_animating)
    return;
  if (queue_is_empty (queue))
    {
      if (isSolutionRunning)
        {
          isSolutionRunning = false;
          timer.is_disabled = false;
        }
      if (playback.active)
        {
          playback.active = false;
          playback.currentMoveIndex = -1;
          playback.popsRemaining = 0;
        }
      return;
    }
  rotation_t popped;
  if (queue_pop (queue, &popped) != QUEUE_OK)
    return;
  cube.is_animating = true;
  cube.current_rotation = popped;
  if (playback.active)
    {
      if (playback.popsRemaining <= 0)
        {
          playback.currentMoveIndex++;
          if (playback.currentMoveIndex < playback.tokenCount)
            playback.popsRemaining
                = playback.popsPerToken[playback.currentMoveIndex];
        }
      playback.popsRemaining--;
    }
}

void
handleMouseMovementAndUpdateCamera ()
{
  if (IsMouseButtonPressed (MOUSE_BUTTON_RIGHT))
    {
      int size = cube.size;
      cube_destroy (&cube);
      cube = cube_make (size, CUBIE_SIZE);
      clearCurrentScrambleAndSolution ();
      resetAnimationAndSolution ();
    }
  else if (IsMouseButtonPressed (MOUSE_BUTTON_MIDDLE))
    {
      initCameraSettings ();
    }

  float dt = GetFrameTime ();

  camera_mag += camera_mag_vel * dt;
  if (camera_mag < 1.25f * cube.size)
    camera_mag = 1.25f * cube.size;
  if (camera_mag > 2.5f * cube.size)
    camera_mag = 2.5f * cube.size;
  camera_mag_vel -= GetMouseWheelMove () * 10;
  camera_mag_vel *= 0.9;

  if (IsMouseButtonDown (MOUSE_BUTTON_LEFT))
    {
      Vector2 delta = GetMouseDelta ();
      camera_theta -= delta.x * 0.005;
      camera_phi -= delta.y * 0.005;
    }
  if (camera_phi >= PI)
    camera_phi = PI - 0.01;
  if (camera_phi <= 0)
    camera_phi = 0.01;

  camera.position.z = sinf (camera_phi) * cosf (camera_theta) * camera_mag;
  camera.position.x = sinf (camera_phi) * sinf (camera_theta) * camera_mag;
  camera.position.y = cosf (camera_phi) * camera_mag;
}

void
drawHelpScreen ()
{
  int margin = 100;
  int availableWidth = GetScreenWidth () - margin;
  int minFontSize = 18;
  int maxFontSize = 40;
  int chunkSize = helpTextsMaxLength / 2;
  int fontSize = fmax (
      fmin (floor ((float)availableWidth / chunkSize) * 10, maxFontSize),
      minFontSize);

  ClearBackground (BACKGROUND_COLOR);
  DrawText ("Press 'h' to exit.", 10, 10, DEFAULT_FONT_SIZE, DARKGRAY);
  int heightRoomForText = 50;
  int startY = GetScreenHeight () / 2 - helpTextsSize / 2 * 50;
  for (int i = 0; i < helpTextsSize; i++)
    {
      DrawText (helpTexts[i],
                GetScreenWidth () / 2
                    - MeasureText (helpTexts[i], fontSize) / 2,
                startY + i * heightRoomForText, fontSize, BLACK);
    }
}

void
drawPatternsScreen ()
{
  ClearBackground (BACKGROUND_COLOR);
  int textWidth = MeasureText ("Press 'p' to exit.", DEFAULT_FONT_SIZE);
  DrawText ("Press 'p' to exit.", GetScreenWidth () - textWidth - 10, 10,
            DEFAULT_FONT_SIZE, DARKGRAY);

  int buttonWidth = 300;
  int buttonHeight = 50;
  int spacing = 20;
  int columns = 2;
  int startX
      = (GetScreenWidth () - (columns * buttonWidth + (columns - 1) * spacing))
        / 2;
  int startY = 100;
  bool isHoveringButton = false;

  for (int i = 0; i < (int)PATTERNS_COUNT; i++)
    {
      int row = i / columns;
      int col = i % columns;
      int x = startX + col * (buttonWidth + spacing);
      int y = startY + row * (buttonHeight + spacing);

      Rectangle button = (Rectangle){
        .x = x, .y = y, .width = buttonWidth, .height = buttonHeight
      };
      bool isHovering = CheckCollisionPointRec (GetMousePosition (), button);
      isHoveringButton |= isHovering;

      if (isHovering)
        {
          DrawRectangleRounded (button, 0.2, 0,
                                ColorBrightness (DARKGRAY, -.1f));
          if (IsMouseButtonPressed (MOUSE_LEFT_BUTTON))
            {
              char patternText[256];
              size_t pos = 0;
              patternText[0] = '\0';
              for (size_t j = 0; j < PATTERNS[i].move_count; j++)
                {
                  queue_push (queue, PATTERNS[i].moves[j]);
                  char tok[3];
                  cube_rotation_token (PATTERNS[i].moves[j], tok);
                  size_t tokLen = strlen (tok);
                  size_t needed = (j > 0 ? 1 : 0) + tokLen;
                  if (pos + needed + 1 >= sizeof (patternText))
                    continue;
                  if (j > 0)
                    patternText[pos++] = ' ';
                  memcpy (patternText + pos, tok, tokLen);
                  pos += tokLen;
                }
              patternText[pos] = '\0';
              showPatterns = false;
              clearCurrentScrambleAndSolution ();
              initPlaybackFromText (patternText);
            }
        }
      else
        DrawRectangleRounded (button, 0.2, 0, ColorBrightness (DARKGRAY, .1f));

      int textW = MeasureText (PATTERNS[i].name, DEFAULT_FONT_SIZE);
      DrawText (PATTERNS[i].name, x + (buttonWidth - textW) / 2,
                y + (buttonHeight - DEFAULT_FONT_SIZE) / 2, DEFAULT_FONT_SIZE,
                BLACK);
    }
  if (isHoveringButton)
    SetMouseCursor (MOUSE_CURSOR_POINTING_HAND);
  else
    SetMouseCursor (MOUSE_CURSOR_DEFAULT);
}

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
drawTextCursorAfter (const char *s, int fontSize)
{
  int spacing = fontSize / 10;
  if (spacing < 1)
    spacing = 1;
  return MeasureText (s, fontSize) + spacing;
}

/* Renders a space-separated token sequence with greedy line wrapping at the
 * screen width. Pass highlightToken = -1 for no highlight. The whole line is
 * always drawn once in BLACK; if a token in this line is highlighted, that
 * token is overlaid in GOLD at the cursor-advance position after the prefix.
 * raylib's default font is a non-AA bitmap, so the overlay replaces pixels
 * cleanly. */
void
DrawMoves (const char *text, float fontSize, int y, int highlightToken)
{
  if (strlen (text) == 0)
    return;

  // tokenize
  char workCopy[256];
  strncpy (workCopy, text, sizeof (workCopy) - 1);
  workCopy[sizeof (workCopy) - 1] = '\0';

  char *tokens[64];
  int tokenCount = 0;
  for (char *t = strtok (workCopy, " "); t && tokenCount < 64;
       t = strtok (NULL, " "))
    tokens[tokenCount++] = t;
  if (tokenCount == 0)
    return;

  int maxWidth = GetScreenWidth () - DEFAULT_FONT_SIZE;
  int lineHeight = 30;

  // figure out line breaks
  int lineStart[16] = { 0 };
  int lineCount = 1;

  char lineBuf[256];
  int lineLen = 0;
  int len0 = strlen (tokens[0]);
  memcpy (lineBuf, tokens[0], len0);
  lineLen = len0;
  lineBuf[lineLen] = '\0';

  for (int i = 1; i < tokenCount; i++)
    {
      int tlen = strlen (tokens[i]);
      if (lineLen + 1 + tlen + 1 > (int)sizeof (lineBuf))
        break;
      lineBuf[lineLen] = ' ';
      memcpy (lineBuf + lineLen + 1, tokens[i], tlen);
      lineBuf[lineLen + 1 + tlen] = '\0';

      if (MeasureText (lineBuf, fontSize) > maxWidth && lineCount < 16)
        {
          lineBuf[lineLen] = '\0';
          lineStart[lineCount++] = i;
          memcpy (lineBuf, tokens[i], tlen);
          lineLen = tlen;
          lineBuf[lineLen] = '\0';
        }
      else
        lineLen += 1 + tlen;
    }
  lineStart[lineCount] = tokenCount;

  // build line strings and draw
  for (int line = 0; line < lineCount; line++)
    {
      int startIdx = lineStart[line];
      int endIdx = lineStart[line + 1];

      // build
      char buf[256];
      int len = 0;
      for (int i = startIdx; i < endIdx; i++)
        {
          if (i > startIdx && len + 1 < (int)sizeof (buf))
            buf[len++] = ' ';
          int tlen = strlen (tokens[i]);
          if (len + tlen >= (int)sizeof (buf))
            break;
          memcpy (buf + len, tokens[i], tlen);
          len += tlen;
        }
      buf[len] = '\0';

      // draw line in center in BLACK
      int lineWidth = MeasureText (buf, fontSize);
      int lineX = (GetScreenWidth () - lineWidth) / 2;
      int yLine = y + line * lineHeight;

      DrawText (buf, lineX, yLine, fontSize, BLACK);

      if (highlightToken < startIdx || highlightToken >= endIdx)
        continue;

      // compute highlight overlay position
      int hOffset = 0;
      for (int i = startIdx; i < highlightToken; i++)
        hOffset += strlen (tokens[i]) + 1;
      int hLen = strlen (tokens[highlightToken]);

      int overlayX = lineX;
      if (hOffset > 0)
        {
          char saved = buf[hOffset];
          buf[hOffset] = '\0';
          overlayX += drawTextCursorAfter (buf, fontSize);
          buf[hOffset] = saved;
        }

      // draw highlight in GOLD
      char saved = buf[hOffset + hLen];
      buf[hOffset + hLen] = '\0';
      DrawText (buf + hOffset, overlayX, yLine, fontSize, GOLD);
      buf[hOffset + hLen] = saved;
    }
}

void
drawCubeScreen ()
{
  BeginMode3D (camera);
  ClearBackground (BACKGROUND_COLOR);

  DrawLine3D (Vector3Zero (), (Vector3){ (float)cube.size / 2 + 2, 0, 0 }, WHITE);
  DrawLine3D (Vector3Zero (), (Vector3){ 0, (float)cube.size / 2 + 2, 0 }, WHITE);
  DrawLine3D (Vector3Zero (), (Vector3){ 0, 0, (float)cube.size / 2 + 2 }, WHITE);
  // DrawCube ((Vector3){ 0 }, cube.size - (1 - CUBIE_SIZE) - 0.05,
  //           cube.size - (1 - CUBIE_SIZE) - 0.05, cube.size - (1 - CUBIE_SIZE) - 0.05,
  //           BLACK);
  cube_draw (&cube, options_rotation_speed ());
  EndMode3D ();

  DrawText ("Press 'h' for help.", 10, 10, DEFAULT_FONT_SIZE, DARKGRAY);
  int textWidth = MeasureText ("Press 'o' for options. ", DEFAULT_FONT_SIZE);
  DrawText ("Press 'o' for options.", GetScreenWidth () - textWidth - 10, 10,
            DEFAULT_FONT_SIZE, DARKGRAY);
  textWidth = MeasureText ("Press 'p' for patterns. ", DEFAULT_FONT_SIZE);
  DrawText ("Press 'p' for patterns.", GetScreenWidth () - textWidth - 10,
            10 + DEFAULT_FONT_SIZE, DEFAULT_FONT_SIZE, DARKGRAY);

  DrawText ("Current scramble:",
            GetScreenWidth () / 2 - MeasureText ("Current scramble:", 30) / 2,
            10, 30, BLACK);
  DrawMoves (currentScramble, DEFAULT_FONT_SIZE, 50, -1);

  timer_update (&timer);
  updateTimerString ();
  DrawText (timerString,
            GetScreenWidth () / 2 - MeasureText ("00:00.00", 40) / 2,
            GetScreenHeight () - 50, 40, timerColor);

  if (currentSolutionSize != 0)
    DrawText (solutionFoundText,
              GetScreenWidth () / 2
                  - MeasureText (solutionFoundText, DEFAULT_FONT_SIZE) / 2,
              GetScreenHeight () - 130, DEFAULT_FONT_SIZE, BLACK);
  if (playback.active)
    DrawMoves (playback.text, DEFAULT_FONT_SIZE, GetScreenHeight () - 100,
               playback.currentMoveIndex);
  else
    DrawMoves (currentSolution, DEFAULT_FONT_SIZE, GetScreenHeight () - 100,
               -1);

  DrawText ("Ao5:", 10, GetScreenHeight () / 2 - 100, DEFAULT_FONT_SIZE,
            BLACK);
  DrawText (avg, DEFAULT_FONT_SIZE + MeasureText ("Ao5:", DEFAULT_FONT_SIZE),
            GetScreenHeight () / 2 - 100, DEFAULT_FONT_SIZE, BLACK);
  int posY = -2;
  for (int i = 4; i >= 0; i--)
    {
      if (times[i][0] == '-')
        continue;
      if (GuiLabelButton (
              (Rectangle){ 10, (float)GetScreenHeight () / 2 + posY * 30,
                           MeasureText (times[i], DEFAULT_FONT_SIZE),
                           DEFAULT_FONT_SIZE },
              times[i]))
        {
          show = !show;
          timeToShow = i;
          posYToShow = posY;
        }

      posY++;
    }
  if (show)
    {
      int result = GuiMessageBox (
          (Rectangle){ 10,
                       (float)GetScreenHeight () / 2 + (posYToShow + 1) * 30,
                       350, 100 },
          "Time details", times[timeToShow], "Cancel;+2;DNF");
      if (!result || result == 1)
        show = !show;
      else if (result == 2)
        {
          solves_toggle_plus_two (timeToShow, cube.size);
          solves_load_last_5 (times, cube.size);
          solves_average_of_5 (times, avg);
          show = !show;
        }
      else if (result == 3)
        {
          solves_toggle_dnf (timeToShow, cube.size);
          solves_load_last_5 (times, cube.size);
          solves_average_of_5 (times, avg);
          show = !show;
        }
    }

  bool isHoveringButton = false;
  int buttonW = 100, buttonH = 30;

  if (currentSolutionSize > 0)
    {
      Rectangle rec = (Rectangle){ .x = GetScreenWidth () - 2 * buttonW,
                                   .y = GetScreenHeight () - 75,
                                   .width = buttonW,
                                   .height = buttonH };
      bool isHovering = CheckCollisionPointRec (GetMousePosition (), rec);
      isHoveringButton = isHoveringButton || isHovering;

      if (isHovering)
        {
          DrawRectangleRounded (rec, 0.5, 0, ColorBrightness (DARKGRAY, -.1f));
          if (IsMouseButtonPressed (MOUSE_LEFT_BUTTON))
            applyCurrentSolution ();
        }
      else
        DrawRectangleRounded (rec, 0.5, 0, ColorBrightness (DARKGRAY, .1f));
      float fontSize = DEFAULT_FONT_SIZE;
      float textWidth = MeasureText ("Apply", fontSize);
      DrawText ("Apply", rec.x + rec.width / 2 - textWidth / 2,
                rec.y + rec.height / 2 - fontSize / 2, fontSize, BLACK);
    }

  if (isHoveringButton)
    SetMouseCursor (MOUSE_CURSOR_POINTING_HAND);
  else
    SetMouseCursor (MOUSE_CURSOR_DEFAULT);
}

void
drawLoadingScreen (int frameCount)
{
  int x = frameCount % 40;
  BeginDrawing ();
  ClearBackground (BACKGROUND_COLOR);
  if (0 <= x && x < 10)
    DrawText ("LOADING",
              GetScreenWidth () / 2 - MeasureText ("LOADING...", 40) / 2,
              GetScreenHeight () / 2 - DEFAULT_FONT_SIZE, 40, BLACK);
  else if (10 <= x && x < DEFAULT_FONT_SIZE)
    DrawText ("LOADING.",
              GetScreenWidth () / 2 - MeasureText ("LOADING...", 40) / 2,
              GetScreenHeight () / 2 - DEFAULT_FONT_SIZE, 40, BLACK);
  else if (DEFAULT_FONT_SIZE <= x && x < 30)
    DrawText ("LOADING..",
              GetScreenWidth () / 2 - MeasureText ("LOADING...", 40) / 2,
              GetScreenHeight () / 2 - DEFAULT_FONT_SIZE, 40, BLACK);
  else
    DrawText ("LOADING...",
              GetScreenWidth () / 2 - MeasureText ("LOADING...", 40) / 2,
              GetScreenHeight () / 2 - DEFAULT_FONT_SIZE, 40, BLACK);
  EndDrawing ();
}

bool initK = true;

void *
initEverything (void *arg)
{
  (void)arg;
  if (initK)
    init ();

  keybindings_init ();
  Options_load ();
  queue = queue_create ();
  if (queue == NULL)
    {
      fprintf (stderr, "queue_create failed\n");
      exit (1);
    }

  initCurrentScrambleAndSolution (CUBE_DEFAULT_SIZE);
  initCameraSettings ();
  solves_load_last_5 (times, cube.size);

  int max = 0;
  for (int i = 0; i < helpTextsSize; i++)
    {
      int t = MeasureText (helpTexts[i], DEFAULT_FONT_SIZE);
      max = t > max ? t : max;
    }
  helpTextsMaxLength = max;

  isEverythingLoaded = true;

  solves_average_of_5 (times, avg);

  return NULL;
}

void
UpdateDrawFrame ()
{
  if (IsKeyPressed (KEY_H) && !showOptions && !showPatterns)
    showHelp = !showHelp;
  else if (IsKeyPressed (KEY_O) && !showHelp && !showPatterns)
    {
      showOptions = !showOptions;
      if (!showOptions)
        Options_save ();
    }
  else if (IsKeyPressed (KEY_P) && !showOptions && !showHelp)
    showPatterns = !showPatterns;

  if (!showHelp && !showOptions && !showPatterns)
    {
      handleMouseMovementAndUpdateCamera ();
      handleKeyPress ();
      handleQueue ();
    }

  BeginDrawing ();
  if (showHelp)
    drawHelpScreen ();
  else if (showOptions)
    Options_drawScreen ();
  else if (showPatterns)
    drawPatternsScreen ();
  else
    drawCubeScreen ();
  if (showExitMessageBox)
    {
      int result = GuiMessageBox (
          (Rectangle){ (float)GetScreenWidth () / 2 - 200,
                       (float)GetScreenHeight () / 2 - 75, 400, 150 },
          "#191#Exit", "Do you really want to quit ?", "Yes;No");

      if (result == 1)
        exitProgram = true;
      else if (result == 2 || result == 0)
        showExitMessageBox = false;
    }
  EndDrawing ();
}

int
main (int argc, char **argv)
{
  printf ("cRubik v0.1\n");
  SetTraceLogLevel (LOG_WARNING);

  SetConfigFlags (FLAG_MSAA_4X_HINT); // Anti-Aliasing

  SetConfigFlags (FLAG_WINDOW_RESIZABLE);
  InitWindow (1200, 800, "cRubik");
  SetExitKey (-1);
  SetWindowMinSize (800, 600);
  SetTargetFPS (40);

  GuiSetStyle (DEFAULT, TEXT_SIZE, DEFAULT_FONT_SIZE);
  GuiSetStyle (DEFAULT, TEXT_SPACING, 2);
  GuiSetStyle (DEFAULT, TEXT_COLOR_NORMAL, 0x000000FF);
  GuiSetStyle (DEFAULT, TEXT_COLOR_FOCUSED, 0xBBBBBBFF);
  GuiSetStyle (DEFAULT, TEXT_COLOR_PRESSED, 0xFFFFFFFF);

  if (argc >= 2 && strncmp (argv[1], "-nk", 3) == 0)
    initK = false;

  pthread_t thread;
  pthread_create (&thread, NULL, initEverything, NULL);

  int frameCount = 0;
  while (!isEverythingLoaded)
    {
      drawLoadingScreen (frameCount);
      frameCount++;
    }

  pthread_join (thread, NULL);

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop (UpdateDrawFrame, 0, 1);
#else

  while (!WindowShouldClose ())
    {
      UpdateDrawFrame ();
      if (exitProgram)
        break;
    }
#endif

  Options_save ();

  free (currentScramble);
  free (scramble);
  cube_destroy (&cube);
  queue_destroy (queue);

  CloseWindow ();
  return 0;
}
