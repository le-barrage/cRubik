#include "utils.h"
#include "include/cJSON.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* GLFW is statically linked into libraylib.a; forward-declare the one symbol
 * we need rather than depending on the GLFW header. glfwGetKeyName returns
 * the 'layout-aware' character that a physical key produces under the current
 * OS keyboard layout (e.g., the W-position key returns "z" on AZERTY). 
*/
extern const char *glfwGetKeyName (int key, int scancode);

bool
colorsEqual (Color color1, Color color2)
{
  return color1.a == color2.a && color1.r == color2.r && color1.g == color2.g
         && color1.b == color2.b;
}

int
Cnk (int n, int k)
{
  int i, j, s;
  if (n < k)
    return 0;
  if (k > n / 2)
    k = n - k;
  for (s = 1, i = n, j = 1; i != n - k; i--, j++)
    {
      s *= i;
      s /= j;
    }
  return s;
}

static int
saveTimeWithScramble (cJSON *times, char *time, char *scramble, int isDNF,
                      int isPlusTwo)
{
  cJSON *solves = NULL;

  if (times == NULL)
    {
      fprintf (stderr, "Invalid JSON object\n");
      return 1;
    }

  solves = cJSON_GetObjectItemCaseSensitive (times, "solves");
  if (solves == NULL || !cJSON_IsArray (solves))
    {
      fprintf (stderr, "No 'solves' array found in JSON\n");
      return 1;
    }

  cJSON *solve = cJSON_CreateObject ();
  if (solve == NULL)
    {
      fprintf (stderr, "Failed to create solve object\n");
      return 1;
    }

  if (cJSON_AddStringToObject (solve, "time", time) == NULL)
    {
      cJSON_Delete (solve);
      fprintf (stderr, "Failed to add time\n");
      return 1;
    }

  if (cJSON_AddStringToObject (solve, "scramble", scramble) == NULL)
    {
      cJSON_Delete (solve);
      fprintf (stderr, "Failed to add scramble\n");
      return 1;
    }

  if (cJSON_AddBoolToObject (solve, "dnf", isDNF) == NULL)
    {
      cJSON_Delete (solve);
      fprintf (stderr, "Failed to add dnf\n");
      return 1;
    }

  if (cJSON_AddBoolToObject (solve, "plus_two", isPlusTwo) == NULL)
    {
      cJSON_Delete (solve);
      fprintf (stderr, "Failed to add plus_two\n");
      return 1;
    }

  cJSON_AddItemToArray (solves, solve);

  return 0;
}

void
storeTime (char *time, char *scramble, int size)
{
  char filename[64];
  getFileName (filename, size);

  FILE *file = fopen (filename, "a+");
  if (file == NULL)
    {
      perror ("fopen");
      exit (1);
    }

  char *buffer = NULL;
  long length;
  cJSON *json = NULL;

  fseek (file, 0, SEEK_END);
  length = ftell (file);
  fseek (file, 0, SEEK_SET);

  if (length == 0)
    {
      json = cJSON_CreateObject ();
      char puzzle_name[20];
      snprintf (puzzle_name, 20, "%dx%dx%d", size, size, size);
      cJSON_AddStringToObject (json, "puzzle", puzzle_name);
      cJSON_AddArrayToObject (json, "solves");
    }
  else
    {
      buffer = malloc (length + 1);
      if (!buffer)
        {
          fprintf (stderr, "Failed to alloc buffer\n");
          fclose (file);
          exit (1);
        }

      fread (buffer, 1, length, file);
      buffer[length] = '\0';

      json = cJSON_Parse (buffer);
      free (buffer);

      if (json == NULL)
        {
          const char *error_ptr = cJSON_GetErrorPtr ();
          if (error_ptr != NULL)
            {
              fprintf (stderr, "Error parsing JSON: %s\n", error_ptr);
            }
          fclose (file);
          exit (1);
        }
    }

  int status = saveTimeWithScramble (json, time, scramble, 0, 0);
  if (status != 0)
    {
      fprintf (stderr, "Failed to add solve\n");
      cJSON_Delete (json);
      fclose (file);
      exit (1);
    }

  fclose (file);
  file = fopen (filename, "w");
  if (file == NULL)
    {
      perror ("fopen for writing");
      cJSON_Delete (json);
      exit (1);
    }

  char *output = cJSON_Print (json);
  if (output)
    {
      fprintf (file, "%s", output);
      free (output);
    }

  cJSON_Delete (json);
  fclose (file);
}

int
timeToSeconds (char time[10])
{
  return 600 * (time[0] - '0') + 60 * (time[1] - '0') + 10 * (time[3] - '0')
         + time[4] - '0';
}

int
timeToMillis (char time[10])
{
  return 1000 * timeToSeconds (time) + 100 * (time[6] - '0')
         + 10 * (time[7] - '0') + (time[8] - '0');
}

int
getMinutesFromMillis (int millis)
{
  return millis / (1000 * 60);
}

int
getSecondsFromMillis (int millis)
{
  return (millis / 1000) % 60;
}

int
getMillisFromMillis (int millis)
{
  return millis % 1000;
}

int
countLines (FILE *fp)
{
  int count = 0;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline (&line, &len, fp)) != -1)
    count++;

  free (line);
  rewind (fp);
  return count;
}

void
getFileName (char filename[20], int cubeSize)
{
  snprintf (filename, 20, "times/%d.time", cubeSize);
}

KeyBindings keyBindings;

void
initDefaultKeyBindings (void)
{
  keyBindings.key_R = KEY_R;
  keyBindings.key_L = KEY_L;
  keyBindings.key_U = KEY_U;
  keyBindings.key_D = KEY_D;
  keyBindings.key_F = KEY_F;
  keyBindings.key_B = KEY_B;
  keyBindings.key_M = KEY_M_FR;
  keyBindings.key_S = KEY_S;
  keyBindings.key_E = KEY_E;
  keyBindings.key_X = KEY_X;
  keyBindings.key_Y = KEY_Y;
  keyBindings.key_Z = KEY_Z_FR;
  keyBindings.key_ALT = KEY_LEFT_ALT;
}

const char *
getKeyName (int key)
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

  /* Ask GLFW what character the current keyboard layout assigns to this
   * physical key. 
   * Returns NULL for keys with no printable representation. 
  */
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

  sprintf (str, "%d", key);
  return str;
}
