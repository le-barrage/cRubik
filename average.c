#include "average.h"
#include "utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "include/cJSON.h"

#define MAX_TIME_LEN 20
#define MAX_FILENAME_LEN 64
#define MAX_LINE_LEN 256
#define NUM_TIMES 5
#define NUM_VALID_TIMES 3

typedef enum
{
  FIELD_DNF,
  FIELD_PLUS_TWO
} FieldType;

static bool
timeIsDNF (char time[20])
{
  return time[0] == 'D' || time[1] == 'D';
}

static cJSON *
readFsonFromFile (const char *filename)
{
  FILE *file = fopen (filename, "r");
  if (!file)
    {
      perror ("Error opening file for reading");
      return NULL;
    }

  fseek (file, 0, SEEK_END);
  long length = ftell (file);
  fseek (file, 0, SEEK_SET);

  if (length == 0)
    {
      fprintf (stderr, "File is empty\n");
      fclose (file);
      return NULL;
    }

  char *buffer = malloc (length + 1);
  if (!buffer)
    {
      fprintf (stderr, "Failed to alloc buffer\n");
      fclose (file);
      return NULL;
    }

  fread (buffer, 1, length, file);
  buffer[length] = '\0';
  fclose (file);

  cJSON *json = cJSON_Parse (buffer);
  free (buffer);

  if (json == NULL)
    {
      const char *error_ptr = cJSON_GetErrorPtr ();
      if (error_ptr != NULL)
        {
          fprintf (stderr, "Error parsing JSON: %s\n", error_ptr);
        }
      return NULL;
    }

  return json;
}

/* Helper function to write JSON to file */
static int
writeJsonToFile (const char *filename, cJSON *json)
{
  FILE *file = fopen (filename, "w");
  if (file == NULL)
    {
      perror ("Error opening file for writing");
      return 1;
    }

  char *output = cJSON_Print (json);
  if (output)
    {
      fprintf (file, "%s", output);
      free (output);
    }

  fclose (file);
  return 0;
}

/* Helper function to calculate target index */
static int
getTargetIndex (int index, int total_solves)
{
  if (total_solves < NUM_TIMES)
    {
      return index;
    }
  else
    {
      return total_solves - NUM_TIMES + index;
    }
}

/* Helper function to initialize times array */
static void
initializeTimesArray (char times[5][20])
{
  for (int i = 0; i < 5; i++)
    strcpy (times[i], "-");
}

void
getLast5Solves (char times[5][20], int cubeSize)
{
  char filename[MAX_FILENAME_LEN];
  getFileName (filename, cubeSize);
  FILE *f = fopen (filename, "a+");
  if (f == NULL)
    {
      perror ("fopen in average.c");
      exit (1);
    }

  char *buffer = NULL;
  long length;
  cJSON *json = NULL;

  fseek (f, 0, SEEK_END);
  length = ftell (f);
  fseek (f, 0, SEEK_SET);
  if (length == 0)
    {
      initializeTimesArray (times);
      fclose (f);
      return;
    }

  buffer = malloc (length + 1);
  if (!buffer)
    {
      fprintf (stderr, "Failed to alloc buffer\n");
      fclose (f);
      exit (1);
    }

  fread (buffer, 1, length, f);
  buffer[length] = '\0';
  fclose (f);

  json = cJSON_Parse (buffer);
  if (json == NULL)
    {
      const char *error_ptr = cJSON_GetErrorPtr ();
      if (error_ptr != NULL)
        {
          fprintf (stderr, "Error before: %s\n", error_ptr);
        }
      free (buffer);
      initializeTimesArray (times);
      return;
    }

  free (buffer);

  cJSON *solves = cJSON_GetObjectItemCaseSensitive (json, "solves");
  if (solves == NULL || !cJSON_IsArray (solves))
    {
      fprintf (stderr, "No 'solves' array found\n");
      cJSON_Delete (json);
      initializeTimesArray (times);
      return;
    }

  int total_solves = cJSON_GetArraySize (solves);
  int num_to_get = total_solves < 5 ? total_solves : 5;
  int start_index = total_solves - num_to_get;

  char raw_times[5][20];
  int best_index = -1;
  int worst_index = -1;
  double best_time = -1;
  double worst_time = -1;
  bool foundDNF = false;

  for (int i = 0; i < 5; i++)
    {
      strcpy (times[i], "-");
      strcpy (raw_times[i], "-");

      if (i >= num_to_get)
        continue;

      cJSON *solve = cJSON_GetArrayItem (solves, start_index + i);
      if (solve == NULL)
        continue;

      const cJSON *time = cJSON_GetObjectItemCaseSensitive (solve, "time");
      const cJSON *dnf = cJSON_GetObjectItemCaseSensitive (solve, "dnf");
      const cJSON *plus_two
          = cJSON_GetObjectItemCaseSensitive (solve, "plus_two");

      if (!cJSON_IsString (time) || time->valuestring == NULL)
        continue;

      strcpy (raw_times[i], time->valuestring);

      if (cJSON_IsTrue (dnf))
        {
          strcpy (times[i], "DNF");
          foundDNF = true;
          // DNF is always worst
          if (worst_time < 0)
            {
              worst_index = i;
              worst_time = -1;
            }
          else
            {
              worst_index = i; // Always update to later DNF
            }
        }
      else if (cJSON_IsTrue (plus_two))
        {
          snprintf (times[i], 20, "%s+", time->valuestring);
          double t = timeToMillis (time->valuestring);

          if (best_time < 0 || t < best_time)
            {
              best_time = t;
              best_index = i;
            }

          if (worst_time < 0)
            {
              worst_time = t;
              worst_index = i;
            }
          else if (!foundDNF && t >= worst_time)
            {
              worst_time = t;
              worst_index = i;
            }
        }
      else
        {
          strcpy (times[i], time->valuestring);
          double t = timeToMillis (time->valuestring);

          if (best_time < 0 || t < best_time)
            {
              best_time = t;
              best_index = i;
            }

          if (worst_time < 0)
            {
              worst_time = t;
              worst_index = i;
            }
          else if (!foundDNF && t >= worst_time)
            {
              worst_time = t;
              worst_index = i;
            }
        }
    }

  if (num_to_get >= 5 && best_index >= 0
      && strcmp (times[best_index], "-") != 0
      && strcmp (times[best_index], "DNF") != 0)
    {
      char temp[20];
      snprintf (temp, 20, "(%s)", times[best_index]);
      strcpy (times[best_index], temp);
    }

  if (num_to_get >= 5 && worst_index >= 0 && worst_index != best_index
      && strcmp (times[worst_index], "-") != 0)
    {
      char temp[20];
      snprintf (temp, 20, "(%s)", times[worst_index]);
      strcpy (times[worst_index], temp);
    }

  cJSON_Delete (json);
}

void
getAverageOf5 (char times[5][20], char avg[10])
{
  int dnfCount = 0;
  for (int i = 0; i < NUM_TIMES; i++)
    {
      if (timeIsDNF (times[i]))
        dnfCount++;
      if (dnfCount > 1)
        {
          strcpy (avg, "DNF");
          return;
        }
      if (times[i][0] == '-')
        {
          avg[0] = '-';
          avg[1] = '\0';
          return;
        }
    }

  char validTimes[NUM_VALID_TIMES][MAX_TIME_LEN];
  int x = 0;
  for (int i = 0; i < NUM_TIMES; i++)
    {
      if (times[i][0] == '(' || timeIsDNF (times[i]))
        continue;
      strcpy (validTimes[x++], times[i]);
    }
  int millisecondsTotal = 0;
  for (int i = 0; i < NUM_VALID_TIMES; i++)
    {
      millisecondsTotal += timeToMillis (validTimes[i]);
    }
  millisecondsTotal /= NUM_VALID_TIMES;
  int minutes = getMinutesFromMillis (millisecondsTotal);
  int seconds = getSecondsFromMillis (millisecondsTotal);
  int milliseconds = getMillisFromMillis (millisecondsTotal);
  snprintf (avg, 10, "%02d:%02d.%03d", minutes, seconds, milliseconds);
}

static int
set_dnf (cJSON *solve, int value)
{
  if (solve == NULL)
    {
      fprintf (stderr, "Invalid solve object\n");
      return 1;
    }

  cJSON *dnf = cJSON_GetObjectItemCaseSensitive (solve, "dnf");
  if (dnf == NULL)
    {
      fprintf (stderr, "DNF field not found\n");
      return 1;
    }

  if (value)
    cJSON_ReplaceItemInObject (solve, "dnf", cJSON_CreateTrue ());
  else
    cJSON_ReplaceItemInObject (solve, "dnf", cJSON_CreateFalse ());

  return 0;
}

static int
set_plus_two (cJSON *solve, int value)
{
  if (solve == NULL)
    {
      fprintf (stderr, "Invalid solve object\n");
      return 1;
    }

  cJSON *plus_two = cJSON_GetObjectItemCaseSensitive (solve, "plus_two");
  if (plus_two == NULL)
    {
      fprintf (stderr, "plus_two field not found\n");
      return 1;
    }

  cJSON *time_obj = cJSON_GetObjectItemCaseSensitive (solve, "time");
  if (!cJSON_IsString (time_obj) || time_obj->valuestring == NULL)
    {
      fprintf (stderr, "time field not found or invalid\n");
      return 1;
    }

  char *current_time = time_obj->valuestring;
  int current_plus_two = cJSON_IsTrue (plus_two) ? 1 : 0;

  int timeMs = timeToMillis (current_time);

  if (value && !current_plus_two)
    timeMs += 2000;
  else if (!value && current_plus_two)
    timeMs -= 2000;
  else
    return 0;

  int min = getMinutesFromMillis (timeMs);
  int sec = getSecondsFromMillis (timeMs);
  int millis = getMillisFromMillis (timeMs);

  char new_time[20] = { 0 };
  snprintf (new_time, 20, "%02d:%02d.%03d", min, sec, millis);

  cJSON_ReplaceItemInObject (solve, "time", cJSON_CreateString (new_time));

  if (value)
    cJSON_ReplaceItemInObject (solve, "plus_two", cJSON_CreateTrue ());
  else
    cJSON_ReplaceItemInObject (solve, "plus_two", cJSON_CreateFalse ());

  return 0;
}

/* Generic function to modify a field in a solve */
static void
modifySolveField (int index, int cubeSize, FieldType field_type)
{
  if (index < 0 || index >= NUM_TIMES)
    {
      fprintf (stderr, "Error: index %d out of range [0..%d]\n", index,
               NUM_TIMES - 1);
      return;
    }

  char filename[MAX_FILENAME_LEN];
  getFileName (filename, cubeSize);

  cJSON *json = readFsonFromFile (filename);
  if (json == NULL)
    {
      return;
    }

  cJSON *solves = cJSON_GetObjectItemCaseSensitive (json, "solves");
  if (solves == NULL || !cJSON_IsArray (solves))
    {
      fprintf (stderr, "No 'solves' array found\n");
      cJSON_Delete (json);
      return;
    }

  int total_solves = cJSON_GetArraySize (solves);
  int target_index = getTargetIndex (index, total_solves);

  if (target_index < 0 || target_index >= total_solves)
    {
      fprintf (stderr, "Target index %d out of range\n", target_index);
      cJSON_Delete (json);
      return;
    }

  cJSON *solve = cJSON_GetArrayItem (solves, target_index);
  if (solve == NULL)
    {
      fprintf (stderr, "Could not get solve at index %d\n", target_index);
      cJSON_Delete (json);
      return;
    }

  int status = 0;
  if (field_type == FIELD_DNF)
    {
      const cJSON *dnf = cJSON_GetObjectItemCaseSensitive (solve, "dnf");
      status = set_dnf (solve, !cJSON_IsTrue (dnf));
    }
  else if (field_type == FIELD_PLUS_TWO)
    {
      const cJSON *plus_two
          = cJSON_GetObjectItemCaseSensitive (solve, "plus_two");
      status = set_plus_two (solve, !cJSON_IsTrue (plus_two));
    }

  if (status != 0)
    {
      fprintf (stderr, "Failed to set field\n");
      cJSON_Delete (json);
      return;
    }

  writeJsonToFile (filename, json);
  cJSON_Delete (json);
}

void
setDNF (int index, int cubeSize)
{
  modifySolveField (index, cubeSize, FIELD_DNF);
}

void
setPlusTwo (int index, int cubeSize)
{
  modifySolveField (index, cubeSize, FIELD_PLUS_TWO);
}
