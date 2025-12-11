#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

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

void
storeTime (char *time, int size)
{
  char filename[15];
  snprintf (filename, 15, "times/%d.time", size);

  FILE *file = fopen (filename, "a+");
  if (file == NULL)
    {
      perror ("fopen");
      exit (1);
    }

  fseek (file, 0, SEEK_SET);

  fputs (time, file);
  fputc ('\n', file);

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
