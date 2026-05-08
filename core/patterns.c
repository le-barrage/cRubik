#include "patterns.h"

#include "utils.h"

static const rotation_t SUPERFLIP[]
    = { ROT_M_PRIME, ROT_U_PRIME, ROT_M_PRIME, ROT_U_PRIME, ROT_M_PRIME, ROT_U_PRIME, ROT_M_PRIME, ROT_U_PRIME, ROT_X, ROT_Y, ROT_M_PRIME, ROT_U_PRIME, ROT_M_PRIME, ROT_U_PRIME, ROT_M_PRIME,
        ROT_U_PRIME, ROT_M_PRIME, ROT_U_PRIME, ROT_X, ROT_Y, ROT_M_PRIME, ROT_U_PRIME, ROT_M_PRIME, ROT_U_PRIME, ROT_M_PRIME, ROT_U_PRIME, ROT_M_PRIME, ROT_U_PRIME, ROT_X, ROT_Y };

static const rotation_t CHECKERBOARD[] = { ROT_M_PRIME, ROT_M_PRIME, ROT_E_PRIME, ROT_E_PRIME, ROT_S_PRIME, ROT_S_PRIME };

static const rotation_t CROSS[]
    = { ROT_R, ROT_R, ROT_L_PRIME, ROT_D, ROT_F, ROT_F, ROT_R_PRIME, ROT_D_PRIME, ROT_R_PRIME, ROT_L, ROT_U_PRIME, ROT_D, ROT_R, ROT_D, ROT_B, ROT_B, ROT_R_PRIME, ROT_U, ROT_D, ROT_D };

static const rotation_t FOUR_CROSSES[]
    = { ROT_U, ROT_U, ROT_R, ROT_R, ROT_L, ROT_L, ROT_F, ROT_F, ROT_B, ROT_B, ROT_D, ROT_D, ROT_L, ROT_L, ROT_R, ROT_R, ROT_F, ROT_F, ROT_B, ROT_B };

static const rotation_t CUBE_IN_CUBE[]
    = { ROT_F, ROT_L, ROT_F, ROT_U_PRIME, ROT_R, ROT_U, ROT_F, ROT_F, ROT_L, ROT_L, ROT_U_PRIME, ROT_L_PRIME, ROT_B, ROT_D_PRIME, ROT_B_PRIME, ROT_L, ROT_L, ROT_U };

static const rotation_t CUBE_IN_CUBE_IN_CUBE[]
    = { ROT_U_PRIME, ROT_L_PRIME, ROT_U_PRIME, ROT_F_PRIME, ROT_R, ROT_R, ROT_B_PRIME, ROT_R, ROT_F, ROT_U, ROT_B, ROT_B, ROT_U, ROT_B_PRIME, ROT_L, ROT_U_PRIME, ROT_F, ROT_U, ROT_R, ROT_F_PRIME };

static const rotation_t FOUR_SPOTS[]
    = { ROT_F, ROT_F, ROT_B, ROT_B, ROT_U, ROT_D_PRIME, ROT_R, ROT_R, ROT_L, ROT_L, ROT_U, ROT_D_PRIME };

static const rotation_t SIX_SPOTS[] = { ROT_U, ROT_D_PRIME, ROT_R, ROT_L_PRIME, ROT_F, ROT_B_PRIME, ROT_U, ROT_D_PRIME };

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
