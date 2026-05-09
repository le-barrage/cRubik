#include "average.h"

#include "cJSON.h"
#include "logger.h"
#include "time_consts.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VALID_TIMES_FOR_AVG 3
#define FILENAME_MAX_LEN 64
#define PLUS_TWO_PENALTY_MS 2000

typedef enum
{
  FIELD_DNF,
  FIELD_PLUS_TWO,
} field_type_t;

typedef struct
{
  bool valid;
  bool dnf;
  int ms;
  char display[TIME_STR_MAX];
} solve_entry_t;

static void
get_solves_filename (char *out, int cube_size)
{
  snprintf (out, FILENAME_MAX_LEN, "times/%d.time", cube_size);
}

/* Parses "MM:SS.mmm" into total milliseconds. Caller is responsible for
 * passing a 9-character (or longer) string in that format; behavior is
 * undefined otherwise. */
static int
time_to_ms (const char *time)
{
  int seconds = 600 * (time[0] - '0') + 60 * (time[1] - '0')
                + 10 * (time[3] - '0') + (time[4] - '0');
  return MS_PER_SEC * seconds + 100 * (time[6] - '0')
         + 10 * (time[7] - '0') + (time[8] - '0');
}

/* Writes total_ms as "MM:SS.mmm" + null into out. out_size should be at
 * least AVG_STR_LEN. Values are clamped (minutes <= 99) so a stopwatch
 * never overflows the format width. */
static void
time_format (int total_ms, char *out, size_t out_size)
{
  if (total_ms < 0)
    total_ms = 0;
  int minutes = total_ms / MS_PER_MIN;
  if (minutes > 99)
    minutes = 99;
  int seconds = (total_ms / MS_PER_SEC) % SECONDS_PER_MIN;
  int ms = total_ms % MS_PER_SEC;
  snprintf (out, out_size, "%02d:%02d.%03d", minutes, seconds, ms);
}

static bool
time_is_dnf (const char *time)
{
  if (time[0] == '(')
    time++;
  return strncmp (time, "DNF", 3) == 0;
}

static bool
is_excluded_from_average (const char *time)
{
  return time[0] == '(' || time_is_dnf (time);
}

static cJSON *
read_json_file (const char *filename)
{
  FILE *f = fopen (filename, "rb");
  if (!f)
    return NULL;

  fseek (f, 0, SEEK_END);
  long length = ftell (f);
  fseek (f, 0, SEEK_SET);
  if (length <= 0)
    {
      fclose (f);
      return NULL;
    }

  char *buf = malloc (length + 1);
  if (!buf)
    {
      fclose (f);
      return NULL;
    }
  fread (buf, 1, length, f);
  buf[length] = '\0';
  fclose (f);

  cJSON *json = cJSON_Parse (buf);
  free (buf);
  if (!json)
    {
      const char *err = cJSON_GetErrorPtr ();
      if (err)
        LOG_ERROR ("%s: parse error: %s", filename, err);
    }
  return json;
}

static int
write_json_file (const char *filename, cJSON *json)
{
  FILE *f = fopen (filename, "wb");
  if (!f)
    {
      LOG_PERROR ("%s", filename);
      return 1;
    }

  char *out = cJSON_Print (json);
  if (out)
    {
      fputs (out, f);
      free (out);
    }
  fclose (f);
  return 0;
}

static void
parse_solve_json (const cJSON *solve, solve_entry_t *out)
{
  *out = (solve_entry_t){ .valid = false, .dnf = false, .ms = -1 };

  const cJSON *time = cJSON_GetObjectItemCaseSensitive (solve, "time");
  const cJSON *dnf = cJSON_GetObjectItemCaseSensitive (solve, "dnf");
  const cJSON *plus_two = cJSON_GetObjectItemCaseSensitive (solve, "plus_two");

  if (!cJSON_IsString (time) || !time->valuestring)
    return;
  out->valid = true;

  if (cJSON_IsTrue (dnf))
    {
      strcpy (out->display, "DNF");
      out->dnf = true;
      return;
    }

  out->ms = time_to_ms (time->valuestring);
  if (cJSON_IsTrue (plus_two))
    snprintf (out->display, TIME_STR_MAX, "%s+", time->valuestring);
  else
    {
      strncpy (out->display, time->valuestring, TIME_STR_MAX - 1);
      out->display[TIME_STR_MAX - 1] = '\0';
    }
}

static void
wrap_in_parens (char *str)
{
  size_t len = strlen (str);
  if (len + 3 > TIME_STR_MAX)
    {
      LOG_WARN ("wrap_in_parens: display too long: '%s'", str);
      return;
    }
  memmove (str + 1, str, len + 1);
  str[0] = '(';
  str[len + 1] = ')';
  str[len + 2] = '\0';
}

static void
mark_best_worst (solve_entry_t entries[LAST_N_SOLVES])
{
  int best_idx = -1;
  int worst_idx = -1;
  int best_ms = -1;
  int worst_ms = -1;
  bool found_dnf = false;

  for (int i = 0; i < LAST_N_SOLVES; i++)
    {
      if (!entries[i].valid)
        return;

      if (entries[i].dnf)
        {
          worst_idx = i;
          found_dnf = true;
          continue;
        }

      if (best_idx < 0 || entries[i].ms < best_ms)
        {
          best_idx = i;
          best_ms = entries[i].ms;
        }
      if (!found_dnf && (worst_idx < 0 || entries[i].ms >= worst_ms))
        {
          worst_idx = i;
          worst_ms = entries[i].ms;
        }
    }

  if (best_idx >= 0)
    wrap_in_parens (entries[best_idx].display);
  if (worst_idx >= 0 && worst_idx != best_idx)
    wrap_in_parens (entries[worst_idx].display);
}

static int
add_solve_entry (cJSON *root, const char *time, const char *scramble)
{
  cJSON *solves = cJSON_GetObjectItemCaseSensitive (root, "solves");
  if (!cJSON_IsArray (solves))
    return 1;

  cJSON *solve = cJSON_CreateObject ();
  if (!solve)
    return 1;

  if (!cJSON_AddStringToObject (solve, "time", time)
      || !cJSON_AddStringToObject (solve, "scramble", scramble)
      || !cJSON_AddBoolToObject (solve, "dnf", 0)
      || !cJSON_AddBoolToObject (solve, "plus_two", 0))
    {
      cJSON_Delete (solve);
      return 1;
    }

  cJSON_AddItemToArray (solves, solve);
  return 0;
}

static cJSON *
new_solves_root (int cube_size)
{
  cJSON *root = cJSON_CreateObject ();
  if (!root)
    return NULL;
  char puzzle_name[20];
  snprintf (puzzle_name, sizeof puzzle_name, "%dx%dx%d", cube_size, cube_size,
            cube_size);
  cJSON_AddStringToObject (root, "puzzle", puzzle_name);
  cJSON_AddArrayToObject (root, "solves");
  return root;
}

void
solves_save (const char *time, const char *scramble, int cube_size)
{
  char filename[FILENAME_MAX_LEN];
  get_solves_filename (filename, cube_size);

  cJSON *root = read_json_file (filename);
  if (!root)
    {
      FILE *probe = fopen (filename, "rb");
      if (probe)
        {
          fseek (probe, 0, SEEK_END);
          long size = ftell (probe);
          fclose (probe);
          if (size > 0)
            {
              LOG_ERROR ("%s: refusing to overwrite unparseable file",
                         filename);
              return;
            }
        }

      root = new_solves_root (cube_size);
      if (!root)
        {
          LOG_ERROR ("%s: out of memory creating solves root", filename);
          return;
        }
    }

  if (add_solve_entry (root, time, scramble) != 0)
    {
      LOG_ERROR ("%s: failed to append solve", filename);
      cJSON_Delete (root);
      return;
    }

  write_json_file (filename, root);
  cJSON_Delete (root);
}

void
solves_load_last_5 (char times[LAST_N_SOLVES][TIME_STR_MAX], int cube_size)
{
  for (int i = 0; i < LAST_N_SOLVES; i++)
    strcpy (times[i], "-");

  char filename[FILENAME_MAX_LEN];
  get_solves_filename (filename, cube_size);
  cJSON *root = read_json_file (filename);
  if (!root)
    return;

  cJSON *solves = cJSON_GetObjectItemCaseSensitive (root, "solves");
  if (!cJSON_IsArray (solves))
    {
      LOG_ERROR ("%s: 'solves' array missing or wrong type", filename);
      cJSON_Delete (root);
      return;
    }

  int total = cJSON_GetArraySize (solves);
  int n = total < LAST_N_SOLVES ? total : LAST_N_SOLVES;
  int start = total - n;

  solve_entry_t entries[LAST_N_SOLVES] = { 0 };
  for (int i = 0; i < n; i++)
    parse_solve_json (cJSON_GetArrayItem (solves, start + i), &entries[i]);

  if (n == LAST_N_SOLVES)
    mark_best_worst (entries);

  for (int i = 0; i < LAST_N_SOLVES; i++)
    if (entries[i].valid)
      strcpy (times[i], entries[i].display);

  cJSON_Delete (root);
}

void
solves_average_of_5 (char times[LAST_N_SOLVES][TIME_STR_MAX],
                     char avg[AVG_STR_LEN])
{
  int dnf_count = 0;
  for (int i = 0; i < LAST_N_SOLVES; i++)
    {
      if (times[i][0] == '-')
        {
          avg[0] = '-';
          avg[1] = '\0';
          return;
        }
      if (time_is_dnf (times[i]))
        dnf_count++;
    }
  if (dnf_count > 1)
    {
      strcpy (avg, "DNF");
      return;
    }

  int total_ms = 0;
  int summed = 0;
  for (int i = 0; i < LAST_N_SOLVES && summed < VALID_TIMES_FOR_AVG; i++)
    {
      if (is_excluded_from_average (times[i]))
        continue;
      total_ms += time_to_ms (times[i]);
      summed++;
    }

  time_format (total_ms / VALID_TIMES_FOR_AVG, avg, AVG_STR_LEN);
}

static int
target_index (int index, int total_solves)
{
  if (total_solves < LAST_N_SOLVES)
    return index;
  return total_solves - LAST_N_SOLVES + index;
}

static int
set_dnf (cJSON *solve, bool value)
{
  cJSON_ReplaceItemInObject (solve, "dnf",
                             value ? cJSON_CreateTrue () : cJSON_CreateFalse ());
  return 0;
}

static int
set_plus_two (cJSON *solve, bool value)
{
  cJSON *plus_two = cJSON_GetObjectItemCaseSensitive (solve, "plus_two");
  cJSON *time_obj = cJSON_GetObjectItemCaseSensitive (solve, "time");
  if (!cJSON_IsString (time_obj) || !time_obj->valuestring)
    return 1;

  bool current = cJSON_IsTrue (plus_two);
  if (value == current)
    return 0;

  int ms = time_to_ms (time_obj->valuestring);
  ms += value ? PLUS_TWO_PENALTY_MS : -PLUS_TWO_PENALTY_MS;

  char new_time[AVG_STR_LEN];
  time_format (ms, new_time, AVG_STR_LEN);

  cJSON_ReplaceItemInObject (solve, "time", cJSON_CreateString (new_time));
  cJSON_ReplaceItemInObject (solve, "plus_two",
                             value ? cJSON_CreateTrue () : cJSON_CreateFalse ());
  return 0;
}

static void
modify_solve_field (int index, int cube_size, field_type_t field)
{
  if (index < 0 || index >= LAST_N_SOLVES)
    {
      LOG_ERROR ("modify_solve_field: index %d out of range", index);
      return;
    }

  char filename[FILENAME_MAX_LEN];
  get_solves_filename (filename, cube_size);
  cJSON *root = read_json_file (filename);
  if (!root)
    return;

  cJSON *solves = cJSON_GetObjectItemCaseSensitive (root, "solves");
  if (!cJSON_IsArray (solves))
    {
      LOG_ERROR ("%s: 'solves' array missing or wrong type", filename);
      cJSON_Delete (root);
      return;
    }

  int total = cJSON_GetArraySize (solves);
  int idx = target_index (index, total);
  if (idx < 0 || idx >= total)
    {
      LOG_ERROR ("%s: solve index %d out of range (total %d)", filename, idx,
                 total);
      cJSON_Delete (root);
      return;
    }

  cJSON *solve = cJSON_GetArrayItem (solves, idx);
  if (!solve)
    {
      LOG_ERROR ("%s: solve at index %d is null", filename, idx);
      cJSON_Delete (root);
      return;
    }

  int status = 0;
  if (field == FIELD_DNF)
    {
      const cJSON *dnf = cJSON_GetObjectItemCaseSensitive (solve, "dnf");
      status = set_dnf (solve, !cJSON_IsTrue (dnf));
    }
  else
    {
      const cJSON *plus_two
          = cJSON_GetObjectItemCaseSensitive (solve, "plus_two");
      status = set_plus_two (solve, !cJSON_IsTrue (plus_two));
    }

  if (status == 0)
    write_json_file (filename, root);
  cJSON_Delete (root);
}

void
solves_toggle_dnf (int index, int cube_size)
{
  modify_solve_field (index, cube_size, FIELD_DNF);
}

void
solves_toggle_plus_two (int index, int cube_size)
{
  modify_solve_field (index, cube_size, FIELD_PLUS_TWO);
}

bool
solves_get_scramble (int last_n_index, int cube_size, char *out,
                     size_t out_size)
{
  char filename[FILENAME_MAX_LEN];
  get_solves_filename (filename, cube_size);
  cJSON *root = read_json_file (filename);
  if (!root)
    return false;

  cJSON *solves = cJSON_GetObjectItemCaseSensitive (root, "solves");
  if (!cJSON_IsArray (solves))
    {
      LOG_ERROR ("%s: 'solves' array missing or wrong type", filename);
      cJSON_Delete (root);
      return false;
    }

  int total = cJSON_GetArraySize (solves);
  int idx = target_index (last_n_index, total);
  if (idx < 0 || idx >= total)
    {
      LOG_ERROR ("%s: solve index %d out of range (total %d)", filename, idx,
                 total);
      cJSON_Delete (root);
      return false;
    }

  cJSON *solve = cJSON_GetArrayItem (solves, idx);
  if (!solve)
    {
      LOG_ERROR ("%s: solve at index %d is null", filename, idx);
      cJSON_Delete (root);
      return false;
    }

  const cJSON *scramble = cJSON_GetObjectItemCaseSensitive (solve, "scramble");
  if (!cJSON_IsString (scramble) || !scramble->valuestring)
    {
      LOG_ERROR ("%s: solve at index %d missing 'scramble' field", filename,
                 idx);
      cJSON_Delete (root);
      return false;
    }
  if (strlen (scramble->valuestring) + 1 > out_size)
    {
      LOG_WARN ("solves_get_scramble: out_size %zu too small for scramble (%zu)",
                out_size, strlen (scramble->valuestring) + 1);
      cJSON_Delete (root);
      return false;
    }
  snprintf (out, out_size, "%s", scramble->valuestring);

  cJSON_Delete (root);
  return true;
}
