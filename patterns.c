#include "patterns.h"

#include "utils.h"

static const Rotation SUPERFLIP[]
    = { m, u, m, u, m, u, m, u, X, Y, m, u, m, u, m,
        u, m, u, X, Y, m, u, m, u, m, u, m, u, X, Y };

static const Rotation CHECKERBOARD[] = { m, m, e, e, s, s };

static const Rotation CROSS[]
    = { R, R, l, D, F, F, r, d, r, L, u, D, R, D, B, B, r, U, D, D };

static const Rotation FOUR_CROSSES[]
    = { U, U, R, R, L, L, F, F, B, B, D, D, L, L, R, R, F, F, B, B };

static const Rotation CUBE_IN_CUBE[]
    = { F, L, F, u, R, U, F, F, L, L, u, l, B, d, b, L, L, U };

static const Rotation CUBE_IN_CUBE_IN_CUBE[]
    = { u, l, u, f, R, R, b, R, F, U, B, B, U, b, L, u, F, U, R, f };

static const Rotation FOUR_SPOTS[]
    = { F, F, B, B, U, d, R, R, L, L, U, d };

static const Rotation SIX_SPOTS[] = { U, d, R, l, F, b, U, d };

#define PATTERN_ENTRY(arr, n) \
  { .moves = (arr), .move_count = ARRAY_LEN (arr), .name = (n) }

const pattern_t PATTERNS[] = {
  PATTERN_ENTRY (SUPERFLIP,             "Superflip"),
  PATTERN_ENTRY (CHECKERBOARD,          "Checkerboard"),
  PATTERN_ENTRY (CROSS,                 "Cross"),
  PATTERN_ENTRY (FOUR_CROSSES,          "4 Crosses"),
  PATTERN_ENTRY (CUBE_IN_CUBE,          "Cube in a cube"),
  PATTERN_ENTRY (CUBE_IN_CUBE_IN_CUBE,  "Cube in a cube in a cube"),
  PATTERN_ENTRY (FOUR_SPOTS,            "Four spots"),
  PATTERN_ENTRY (SIX_SPOTS,             "Six spots"),
};

const size_t PATTERNS_COUNT = ARRAY_LEN (PATTERNS);
