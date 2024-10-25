#include "average.h"
#include "cube.h"
#include "include/raylib.h"
#include "kociemba/coordCube.h"
#include "kociemba/enums.h"
#include "kociemba/twoPhase.h"
#include "queue.h"
#include "scramble.h"
#include "timer.h"
#include "utils.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include "include/raygui.h"

#define CUBIE_SIZE 0.9
#define BACKGROUND_COLOR GRAY

float camera_mag;
float camera_mag_vel;
float camera_theta;
float camera_phi;

Camera camera = {{0}, {0, 0, 0}, {0, 1, 0}, 90, CAMERA_PERSPECTIVE};

Cube cube;
char **scramble, *currentScramble, currentSolution[76], solutionFoundText[45],
    times[5][20], avg[10];
int currentSolutionSize;
Queue queue;

bool showHelp = false, showExitMessageBox = false, isEverythingLoaded = false;

char *enter = "Press 'Enter' to scramble the cube.";
char *rotateFace =
    "Press the corresponding key to move each face (Hold 'alt' down for "
    "prime moves):";
char *facesKey = "R (right), L (left), U (up), D (down), F (front), B (back).";
char *solveKey =
    "Press 'K' to find an optimal solution to the cube (only 3x3x3).";
char *mouseRight =
    "Press right mouse button to reset the cube to its original, solved state.";
char *mouseMiddle = "Press middle mouse button to reset camera settings.";
char *mouseLeft = "Hold left mouse button down to move the camera around.";
char *spaceBar = "Press the space bar to start (or stop) the timer";
char *cubeSize = "Press '-' or 'page down' to reduce the cube size and '+' or "
                 "'page up' to increase it.";

Timer timer;
Color timerColor = BLACK;
char timerString[10] = "00:00.000";

bool show = false, isSolutionRunning = false;
int timeToShow = -1, posYToShow = 0;

void handleRotation(Rotation clockwise, Rotation antiClockwise) {
  if (isSolutionRunning)
    return;
  if (IsKeyDown(KEY_LEFT_ALT))
    Queue_add(&queue, antiClockwise);
  else
    Queue_add(&queue, clockwise);
}

void applyMovesAndUpdateCurrentScramble() {
  for (int i = 0; i < SCRAMBLE_SIZE; i++) {
    Cube_applyMove(&cube, scramble[i]);
    if (scramble[i][0] == '1' && scramble[i][1] == 'w')
      strcat(currentScramble, scramble[i] + 2);
    else
      strcat(currentScramble, scramble[i]);
    free(scramble[i]);
    if (i != SCRAMBLE_SIZE - 1)
      strcat(currentScramble, " ");
  }
}

int findSolutionAndUpdateMoves(Cube *cube, int depthLimit, int timeOut) {
  char cubeStr[55];
  Cube_toString(cube, cubeStr);
  Move moves[25] = {0};
  int depth;
  int error = findSolutionBasic(cubeStr, depthLimit, timeOut, moves, &depth);
  if (error != 0) {
    return error;
  }

  int currentMovesIndex = 0;
  for (int i = 0; i < 25; i++) {
    if (currentSolutionSize == depth)
      break;
    Move cur = moves[i];
    if (cur.orientation == 0)
      currentSolution[currentMovesIndex++] = 'U';
    else if (cur.orientation == 1)
      currentSolution[currentMovesIndex++] = 'R';
    else if (cur.orientation == 2)
      currentSolution[currentMovesIndex++] = 'F';
    else if (cur.orientation == 3)
      currentSolution[currentMovesIndex++] = 'D';
    else if (cur.orientation == 4)
      currentSolution[currentMovesIndex++] = 'L';
    else if (cur.orientation == 5)
      currentSolution[currentMovesIndex++] = 'B';
    if (cur.direction == ANTICW)
      currentSolution[currentMovesIndex++] = '\'';
    else if (cur.direction == HALF)
      currentSolution[currentMovesIndex++] = '2';
    if (i != 24)
      currentSolution[currentMovesIndex++] = ' ';
    currentSolutionSize++;
  }
  currentSolution[currentMovesIndex] = '\0';

  return 0;
}

void findSolutionAndUpdateCurrentSolution() {
  if (SIZE != 3) {
    snprintf(currentSolution, 41, "The algorithm only works on 3x3x3 cubes.");
    return;
  }

  struct timespec start, now;

  currentSolutionSize = 0;

  clock_gettime(CLOCK_MONOTONIC, &start);
  int error = findSolutionAndUpdateMoves(&cube, 22, 20000);
  clock_gettime(CLOCK_MONOTONIC, &now);
  if (error != 0) {
    snprintf(currentSolution, 75, "%s", printErrorMessage(error));
    return;
  }

  long long elapsed_time_ns = (now.tv_sec - start.tv_sec) * 1000000000LL +
                              (now.tv_nsec - start.tv_nsec);
  double elapsed_time_ms = (double)elapsed_time_ns / 1000000.0;
  snprintf(solutionFoundText, 45,
           "%d moves solution found in ~%d milliseconds:", currentSolutionSize,
           (int)elapsed_time_ms);

  printf("Solution found in ~%d milliseconds\n", (int)elapsed_time_ms);
}

void clearCurrentScrambleAndSolution() {
  currentScramble[0] = '\0';
  currentSolution[0] = '\0';
  currentSolutionSize = 0;
}

void resetAnimationAndSolution() {
  cube.isAnimating = false;
  Queue_clear(&queue);
}

void generateNewScramble() {
  clearCurrentScrambleAndSolution();
  resetAnimationAndSolution();
  Cube_free(cube);
  cube = Cube_make(CUBIE_SIZE);
  generateScramble(scramble, SIZE);
  applyMovesAndUpdateCurrentScramble();
  timer.isDisabled = false;
}

void initCameraSettings() {
  camera_mag = 2 * SIZE;
  camera_mag_vel = 0.0f;
  camera_theta = PI / 5;
  camera_phi = PI / 3;
}

void initCurrentScrambleAndSolution() {
  cube = Cube_make(CUBIE_SIZE);
  scramble = malloc(SCRAMBLE_SIZE * sizeof(char *));
  currentScramble = malloc((6 * SCRAMBLE_SIZE + 1) * sizeof(char));
  clearCurrentScrambleAndSolution();
  resetAnimationAndSolution();
  avg[0] = '\0';
}

void resizeCube(int increment) {
  free(currentScramble);
  free(scramble);
  Cube_free(cube);

  SIZE += (SIZE == 11 && increment > 0) || (SIZE == 1 && increment < 0)
              ? 0
              : increment;

  initCurrentScrambleAndSolution();

  initCameraSettings();

  getTimes(times, SIZE);
}

// TODO: Make this function more readable (change if)
void applyCurrentSolution() {
  Timer_disable(&timer);
  isSolutionRunning = true;
  currentSolutionSize = 0;
  int i = 0;
  char rotation;
  while (currentSolution[i] != '\0') {
    char currMove = currentSolution[i], nextMove = currentSolution[i + 1];
    if (currMove == ' ') {
      i++;
      continue;
    }
    if (nextMove == '\'') {
      rotation = tolower(currMove);
      i++;
    } else if (nextMove == '2') {
      Queue_add(&queue, getCorrespondingRotation(currMove));
      rotation = currMove;
      i++;
    } else {
      rotation = currMove;
    }
    Queue_add(&queue, getCorrespondingRotation(rotation));
    i++;
  }
}

void handleKeyPress() {
  if (IsKeyPressed(KEY_U))
    handleRotation(U, u);
  else if (IsKeyPressed(KEY_D))
    handleRotation(D, d);
  else if (IsKeyPressed(KEY_L))
    handleRotation(L, l);
  else if (IsKeyPressed(KEY_R))
    handleRotation(R, r);
  else if (IsKeyPressed(KEY_F))
    handleRotation(F, f);
  else if (IsKeyPressed(KEY_B))
    handleRotation(B, b);
  else if (IsKeyPressed(KEY_M_FR))
    handleRotation(M, m);
  else if (IsKeyPressed(KEY_E))
    handleRotation(E, e);
  else if (IsKeyPressed(KEY_S))
    handleRotation(S, s);
  else if (IsKeyPressed(KEY_X))
    handleRotation(X, x);
  else if (IsKeyPressed(KEY_Y))
    handleRotation(Y, y);
  else if (IsKeyPressed(KEY_Z_FR))
    handleRotation(Z, z);
  else if (IsKeyPressed(KEY_ENTER))
    generateNewScramble();
  else if (IsKeyPressed(KEY_K))
    findSolutionAndUpdateCurrentSolution();
  else if (IsKeyDown(KEY_SPACE) && !timer.isDisabled) {
    if (!timer.isRunning && !timer.justStopped)
      timerColor = (Color){0, 204, 51, 255};
    else {
      Timer_stop(&timer);
    }
  } else if (IsKeyReleased(KEY_SPACE) && !timer.isDisabled) {
    if (timer.justStopped) {
      storeTime(timerString, SIZE);
      getTimes(times, SIZE);
      getAverageOf5(times, avg);
      generateNewScramble();
      timer.justStopped = false;
      return;
    }
    timerColor = BLACK;
    if (!timer.isRunning)
      Timer_start(&timer);
  } else if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_PAGE_UP))
    resizeCube(1);
  else if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_PAGE_DOWN))
    resizeCube(-1);
  else if (IsKeyPressed(KEY_ESCAPE))
    showExitMessageBox = true;
}

void handleQueue() {
  if (cube.isAnimating)
    return;
  if (Queue_isEmpty(&queue)) {
    if (isSolutionRunning) {
      isSolutionRunning = false;
      timer.isDisabled = false;
    }
    return;
  }
  cube.isAnimating = true;
  cube.currentRotation = Queue_pop(&queue);
}

void handleMouseMovementAndUpdateCamera() {
  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    Cube_free(cube);
    cube = Cube_make(CUBIE_SIZE);
    clearCurrentScrambleAndSolution();
    resetAnimationAndSolution();
  } else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
    initCameraSettings();
  }

  float dt = GetFrameTime();

  camera_mag += camera_mag_vel * dt;
  if (camera_mag < 1.25f * SIZE)
    camera_mag = 1.25f * SIZE;
  if (camera_mag > 2.5f * SIZE)
    camera_mag = 2.5f * SIZE;
  camera_mag_vel -= GetMouseWheelMove() * 10;
  camera_mag_vel *= 0.9;

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    Vector2 delta = GetMouseDelta();
    camera_theta -= delta.x * 0.005;
    camera_phi -= delta.y * 0.005;
  }
  if (camera_phi >= PI)
    camera_phi = PI - 0.01;
  if (camera_phi <= 0)
    camera_phi = 0.01;

  camera.position.z = sinf(camera_phi) * cosf(camera_theta) * camera_mag;
  camera.position.x = sinf(camera_phi) * sinf(camera_theta) * camera_mag;
  camera.position.y = cosf(camera_phi) * camera_mag;
}

void drawHelpScreen() {
  int fontSize =
      fmax(fmin(floor((float)(GetScreenWidth() - 100) / 400) * 10, 40), 18);

  ClearBackground(BACKGROUND_COLOR);
  DrawText("Press 'h' to exit.", 10, 10, 20, DARKGRAY);
  DrawText(enter, GetScreenWidth() / 2 - MeasureText(enter, fontSize) / 2,
           GetScreenHeight() / 2 - 200, fontSize, BLACK);
  DrawText(rotateFace,
           GetScreenWidth() / 2 - MeasureText(rotateFace, fontSize) / 2,
           GetScreenHeight() / 2 - 150, fontSize, BLACK);
  DrawText(facesKey, GetScreenWidth() / 2 - MeasureText(facesKey, fontSize) / 2,
           GetScreenHeight() / 2 - 100, fontSize, BLACK);
  DrawText(solveKey, GetScreenWidth() / 2 - MeasureText(solveKey, fontSize) / 2,
           GetScreenHeight() / 2 - 50, fontSize, BLACK);
  DrawText(mouseRight,
           GetScreenWidth() / 2 - MeasureText(mouseRight, fontSize) / 2,
           GetScreenHeight() / 2, fontSize, BLACK);
  DrawText(mouseMiddle,
           GetScreenWidth() / 2 - MeasureText(mouseMiddle, fontSize) / 2,
           GetScreenHeight() / 2 + 50, fontSize, BLACK);
  DrawText(mouseLeft,
           GetScreenWidth() / 2 - MeasureText(mouseLeft, fontSize) / 2,
           GetScreenHeight() / 2 + 100, fontSize, BLACK);
  DrawText(spaceBar, GetScreenWidth() / 2 - MeasureText(spaceBar, fontSize) / 2,
           GetScreenHeight() / 2 + 150, fontSize, BLACK);
  DrawText(cubeSize, GetScreenWidth() / 2 - MeasureText(cubeSize, fontSize) / 2,
           GetScreenHeight() / 2 + 200, fontSize, BLACK);
}

void DrawTextBoxed(const char *text, float fontSize, int y) {
  if (strlen(text) == 0)
    return;

  int lastSpace = 0;
  char *dup = strdup(text);
  char *lastSpacePtr = strrchr(dup, ' ');
  lastSpace = (lastSpacePtr != NULL) ? lastSpacePtr - dup : -1;
  while (MeasureText(dup, fontSize) > GetScreenWidth() - 20) {
    if (lastSpace == -1)
      break;
    dup[lastSpace] = '\0';
    lastSpacePtr = strrchr(dup, ' ');
    lastSpace = (lastSpacePtr != NULL) ? lastSpacePtr - dup : -1;
  }
  DrawText(dup, GetScreenWidth() / 2 - MeasureText(dup, fontSize) / 2, y,
           fontSize, BLACK);
  if (strlen(text) > strlen(dup))
    DrawTextBoxed(text + strlen(dup) + 1, fontSize, y + 30);
  free(dup);
}

void updateTimerString() {
  snprintf(timerString, 10, "%02d:%02d.%03d", timer.minutes, timer.seconds,
           timer.milliseconds);
}

void drawCubeScreen() {
  BeginMode3D(camera);
  ClearBackground(BACKGROUND_COLOR);

  DrawLine3D(Vector3Zero(), (Vector3){(float)SIZE / 2 + 2, 0, 0}, WHITE);
  DrawLine3D(Vector3Zero(), (Vector3){0, (float)SIZE / 2 + 2, 0}, WHITE);
  DrawLine3D(Vector3Zero(), (Vector3){0, 0, (float)SIZE / 2 + 2}, WHITE);
  // DrawCube((Vector3){0}, SIZE - (1 - CUBIE_SIZE) - 0.05,
  //          SIZE - (1 - CUBIE_SIZE) - 0.05, SIZE - (1 - CUBIE_SIZE) - 0.05,
  //          BLACK);
  Cube_drawCube(&cube);
  EndMode3D();

  DrawText("Press 'h' for help.", 10, 10, 20, DARKGRAY);

  DrawText("Current scramble:",
           GetScreenWidth() / 2 - MeasureText("Current scramble:", 30) / 2, 10,
           30, BLACK);
  DrawTextBoxed(currentScramble, 20, 50);

  Timer_update(&timer);
  updateTimerString();
  DrawText(timerString, GetScreenWidth() / 2 - MeasureText("00:00.00", 40) / 2,
           GetScreenHeight() - 50, 40, timerColor);

  if (currentSolutionSize != 0)
    DrawText(solutionFoundText,
             GetScreenWidth() / 2 - MeasureText(solutionFoundText, 20) / 2,
             GetScreenHeight() - 130, 20, BLACK);
  DrawTextBoxed(currentSolution, 20, GetScreenHeight() - 100);

  DrawText("Ao5:", 10, GetScreenHeight() / 2 - 100, 20, BLACK);
  DrawText(avg, 20 + MeasureText("Ao5:", 20), GetScreenHeight() / 2 - 100, 20,
           BLACK);
  int posY = -2;
  for (int i = 4; i >= 0; i--) {
    if (times[i][0] == '-')
      continue;
    if (GuiLabelButton((Rectangle){10, (float)GetScreenHeight() / 2 + posY * 30,
                                   MeasureText(times[i], 20), 20},
                       times[i])) {
      show = !show;
      timeToShow = i;
      posYToShow = posY;
    }

    posY++;
  }
  if (show) {
    int result = GuiMessageBox(
        (Rectangle){10, (float)GetScreenHeight() / 2 + (posYToShow + 1) * 30,
                    350, 100},
        "Time details", times[timeToShow], "Cancel;+2;DNF");
    if (!result || result == 1)
      show = !show;
    else if (result == 2) {
      setPlusTwo(timeToShow, SIZE);
      getTimes(times, SIZE);
      getAverageOf5(times, avg);
      show = !show;
    } else if (result == 3) {
      setDNF(timeToShow, SIZE);
      getTimes(times, SIZE);
      getAverageOf5(times, avg);
      show = !show;
    }
  }

  if (currentSolutionSize > 0) {
    int recW = 100, recH = 30;
    Rectangle rec = (Rectangle){.x = GetScreenWidth() - 2 * recW,
                                .y = GetScreenHeight() - 100,
                                .width = recW,
                                .height = recH};
    if (CheckCollisionPointRec(GetMousePosition(), rec)) {
      SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
      DrawRectangleRounded(rec, 0.5, 0, ColorBrightness(DARKGRAY, -.1f));
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        applyCurrentSolution();
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
      }
    } else {
      SetMouseCursor(MOUSE_CURSOR_DEFAULT);
      DrawRectangleRounded(rec, 0.5, 0, ColorBrightness(DARKGRAY, .1f));
    }
    float fontSize = 20;
    float textWidth = MeasureText("Apply", fontSize);
    DrawText("Apply", rec.x + rec.width / 2 - textWidth / 2,
             rec.y + rec.height / 2 - fontSize / 2, fontSize, BLACK);
  }
}

void drawLoadingScreen(int frameCount) {
  int x = frameCount % 40;
  BeginDrawing();
  ClearBackground(BACKGROUND_COLOR);
  if (0 <= x && x < 10)
    DrawText("LOADING",
             GetScreenWidth() / 2 - MeasureText("LOADING...", 40) / 2,
             GetScreenHeight() / 2 - 20, 40, BLACK);
  else if (10 <= x && x < 20)
    DrawText("LOADING.",
             GetScreenWidth() / 2 - MeasureText("LOADING...", 40) / 2,
             GetScreenHeight() / 2 - 20, 40, BLACK);
  else if (20 <= x && x < 30)
    DrawText("LOADING..",
             GetScreenWidth() / 2 - MeasureText("LOADING...", 40) / 2,
             GetScreenHeight() / 2 - 20, 40, BLACK);
  else
    DrawText("LOADING...",
             GetScreenWidth() / 2 - MeasureText("LOADING...", 40) / 2,
             GetScreenHeight() / 2 - 20, 40, BLACK);
  EndDrawing();
}

void *initEverything() {
  init();

  initCameraSettings();
  queue = Queue_make();

  timer = Timer_make();
  getTimes(times, SIZE);

  initCurrentScrambleAndSolution();

  isEverythingLoaded = true;

  getAverageOf5(times, avg);

  return NULL;
}

int main(int argc, char **argv) {
  SetTraceLogLevel(LOG_WARNING);

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1200, 800, "Rubik's Cube");
  SetExitKey(-1);
  SetWindowMinSize(800, 600);
  SetTargetFPS(40);

  GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
  GuiSetStyle(DEFAULT, TEXT_SPACING, 2);
  GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x000000FF);
  GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0xBBBBBBFF);
  GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, 0xFFFFFFFF);

  pthread_t thread;
  pthread_create(&thread, NULL, initEverything, NULL);

  int frameCount = 0;
  while (!isEverythingLoaded) {
    drawLoadingScreen(frameCount);
    frameCount++;
  }

  pthread_join(thread, NULL);

  if (argc >= 2)
    for (int i = 1; i < argc; i++)
      Cube_applyMove(&cube, argv[i]);

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_H))
      showHelp = !showHelp;

    if (!showHelp) {
      handleMouseMovementAndUpdateCamera();
      handleKeyPress();
      handleQueue();
    }

    BeginDrawing();
    if (showHelp)
      drawHelpScreen();
    else
      drawCubeScreen();
    if (showExitMessageBox) {
      int result = GuiMessageBox(
          (Rectangle){(float)GetScreenWidth() / 2 - 200,
                      (float)GetScreenHeight() / 2 - 75, 400, 150},
          "#191#Exit", "Do you really want to quit ?", "Yes;No");

      if (result == 1)
        break;
      else if (result == 2)
        showExitMessageBox = false;
    }
    EndDrawing();
  }

  free(currentScramble);
  free(scramble);
  Cube_free(cube);

  CloseWindow();
  return 0;
}
