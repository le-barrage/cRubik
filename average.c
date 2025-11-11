#include "average.h"
#include "utils.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TIME_LEN 20
#define MAX_FILENAME_LEN 64
#define MAX_LINE_LEN 256
#define NUM_TIMES 5
#define NUM_VALID_TIMES 3

static int
compare (const void *a, const void *b)
{
  char *time1 = *(char **)a;
  char *time2 = *(char **)b;
  return strcmp (time1, time2);
}

static bool
timeIsDNF (char time[20])
{
  return time[0] == 'D';
}

void
getTimes (char times[5][20], int cubeSize)
{
  char filename[MAX_FILENAME_LEN];
  getFileName (filename, cubeSize);
  FILE *fp = fopen (filename, "a+");
  if (fp == NULL)
    {
      perror ("fopen in average.c");
      exit (1);
    }

  int count = countLines (fp);
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  char **sortedTimes = malloc (fmax (count, NUM_TIMES) * sizeof (char *));
  if (sortedTimes == NULL)
    {
      perror ("malloc");
      free (line);
      fclose (fp);
      exit (1);
    }

  rewind (fp);

  int i = 0, x = 0;
  while ((read = getline (&line, &len, fp)) != -1)
    {
      if (i < count - NUM_TIMES)
        {
          i++;
          continue;
        }

      line[strcspn (line, "\n")] = '\0';

      sortedTimes[x] = malloc ((strlen (line) + 1) * sizeof (char));
      if (sortedTimes[x] == NULL)
        {
          perror ("malloc");
          for (int j = 0; j < x; j++)
            free (sortedTimes[j]);
          free (sortedTimes);
          free (line);
          fclose (fp);
          exit (1);
        }
      snprintf (times[x], MAX_TIME_LEN, "%s", line);
      strcpy (sortedTimes[x], line);
      x++;
    }

  if (count < NUM_TIMES)
    {
      for (int i = count; i < NUM_TIMES; i++)
        strcpy (times[i], "-");
    }
  else
    {

      qsort (sortedTimes, x, sizeof (char *), compare);

      bool found = false;
      for (int i = 0; i < x; i++)
        if (!timeIsDNF (times[i]) && !found
            && strcmp (times[i], sortedTimes[0]) == 0)
          {
            snprintf (times[i], MAX_TIME_LEN, "(%s)", sortedTimes[0]);
            found = true;
          }
      found = false;
      for (int i = 0; i < x; i++)
        if (!timeIsDNF (times[i]) && !found
            && strcmp (times[i], sortedTimes[4]) == 0)
          {
            snprintf (times[i], MAX_TIME_LEN, "(%s)", sortedTimes[4]);
            found = true;
          }
    }

  for (int i = 0; i < x; i++)
    {
      free (sortedTimes[i]);
    }
  free (sortedTimes);

  free (line);
  fclose (fp);
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

void
setDNF (int index, int cubeSize)
{
  if (index < 0 || index >= NUM_TIMES)
    {
      fprintf (stderr, "Error: index %d out of range [0..%d]\n", index,
               NUM_TIMES - 1);
      return;
    }

  char filename[MAX_FILENAME_LEN];
  getFileName (filename, cubeSize);

  FILE *fp = fopen (filename, "r");
  if (!fp)
    {
      perror ("Error opening file for reading");
      return;
    }

  int lineNumber = countLines (fp);
  int targetLine = lineNumber < NUM_TIMES ? index + 1 : lineNumber - 4 + index;

  rewind (fp);
  char **lines = malloc (lineNumber * sizeof (char *));
  if (!lines)
    {
      perror ("malloc");
      fclose (fp);
      return;
    }

  char buffer[MAX_LINE_LEN];
  int i = 0;
  while (fgets (buffer, sizeof (buffer), fp))
    {
      lines[i] = strdup (buffer);
      if (!lines[i])
        {
          perror ("strdup");
          for (int j = 0; j < i; j++)
            free (lines[j]);
          free (lines);
          fclose (fp);
          return;
        }
      i++;
    }
  fclose (fp);

  if (targetLine >= 1 && targetLine <= lineNumber)
    {
      char *line = lines[targetLine - 1];
      if (line[0] != 'D')
        {
          size_t len = strlen (line);
          if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            {
              line[len - 1] = '\0';
            }

          char newLine[MAX_LINE_LEN];
          snprintf (newLine, sizeof (newLine), "DNF(%s)\n", line);

          free (line);
          lines[targetLine - 1] = strdup (newLine);
          if (!lines[targetLine - 1])
            {
              perror ("strdup");
              for (int j = 0; j < lineNumber; j++)
                if (lines[j])
                  free (lines[j]);
              free (lines);
              return;
            }
        }
    }

  fp = fopen (filename, "w");
  if (!fp)
    {
      perror ("Error opening file for writing");
      for (i = 0; i < lineNumber; i++)
        free (lines[i]);
      free (lines);
      return;
    }
  for (i = 0; i < lineNumber; i++)
    {
      fputs (lines[i], fp);
      free (lines[i]);
    }
  free (lines);
  fclose (fp);
}

void
setPlusTwo (int index, int cubeSize)
{
  if (index < 0 || index >= NUM_TIMES)
    {
      fprintf (stderr, "Error: index %d out of range [0..%d]\n", index,
               NUM_TIMES - 1);
      return;
    }

  char filename[MAX_FILENAME_LEN];
  getFileName (filename, cubeSize);

  FILE *fp = fopen (filename, "r");
  if (!fp)
    {
      perror ("Error opening file for reading");
      return;
    }

  int lineNumber = countLines (fp);
  int targetLine = lineNumber < NUM_TIMES ? index + 1 : lineNumber - 4 + index;

  rewind (fp);
  char **lines = malloc (lineNumber * sizeof (char *));
  if (!lines)
    {
      perror ("malloc");
      fclose (fp);
      return;
    }

  char buffer[MAX_LINE_LEN];
  int i = 0;
  while (fgets (buffer, sizeof (buffer), fp))
    {
      lines[i] = strdup (buffer);
      if (!lines[i])
        {
          perror ("strdup");
          for (int j = 0; j < i; j++)
            free (lines[j]);
          free (lines);
          fclose (fp);
          return;
        }
      i++;
    }
  fclose (fp);

  bool needsUpdate = false;
  if (targetLine >= 1 && targetLine <= lineNumber)
    {
      char *line = lines[targetLine - 1];

      size_t len = strlen (line);
      if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        {
          line[len - 1] = '\0';
          len--;
        }

      if (len > 0 && line[len - 1] == '+')
        needsUpdate = false;
      else
        {
          int timeMs = timeToMillis (line) + 2000;
          int min = getMinutesFromMillis (timeMs);
          int sec = getSecondsFromMillis (timeMs);
          int millis = getMillisFromMillis (timeMs);

          char newLine[MAX_LINE_LEN];
          snprintf (newLine, sizeof (newLine), "%02d:%02d.%03d+\n", min, sec,
                    millis);

          free (line);
          lines[targetLine - 1] = strdup (newLine);
          if (!lines[targetLine - 1])
            {
              perror ("strdup");
              for (int j = 0; j < lineNumber; j++)
                if (lines[j])
                  free (lines[j]);
              free (lines);
              return;
            }
          needsUpdate = true;
        }
    }

  if (needsUpdate)
    {
      fp = fopen (filename, "w");
      if (!fp)
        {
          perror ("Error opening file for writing");
          for (i = 0; i < lineNumber; i++)
            free (lines[i]);
          free (lines);
          return;
        }
      for (i = 0; i < lineNumber; i++)
        {
          fputs (lines[i], fp);
          free (lines[i]);
        }
      free (lines);
      fclose (fp);
    }
  else
    {
      for (i = 0; i < lineNumber; i++)
        free (lines[i]);
      free (lines);
    }
}
