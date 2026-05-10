#include "patterns.h"

#include "utils.h"

#include <stdio.h>
#include <string.h>

/* Appends `tok` to `out` (with a leading space after the first token).
 * Bumps `*pos` and sets `*wrote_first` once a token has been written.
 * Silently skips writes that would overflow the buffer. */
static void append_token (char *out, size_t out_size, size_t *pos, bool *wrote_first, const char *tok)
{
    size_t tok_len = strlen(tok);
    size_t need    = (*wrote_first ? 1 : 0) + tok_len;
    if (*pos + need + 1 >= out_size) return;
    if (*wrote_first) out[(*pos)++] = ' ';
    memcpy(out + *pos, tok, tok_len);
    *pos += tok_len;
    out[*pos]    = '\0';
    *wrote_first = true;
}

void pattern_emit_static (const move_t *arr, size_t n, queue_t *q, char *out, size_t out_size)
{
    size_t pos             = 0;
    bool wrote_first_token = false;
    if (out_size > 0) out[0] = '\0';

    for (size_t j = 0; j < n; j++) {
        queue_push(q, arr[j]);

        bool is_pair_second = (j > 0 && arr[j].rotation == arr[j - 1].rotation);
        if (is_pair_second) continue;

        bool is_half = (j + 1 < n && arr[j + 1].rotation == arr[j].rotation);

        char tok[3];
        cube_rotation_token(arr[j].rotation, tok);
        size_t tok_len = strlen(tok);

        if (is_half) {
            if (tok_len > 0 && tok[tok_len - 1] == '\'')
                tok[tok_len - 1] = '2';
            else {
                tok[tok_len]     = '2';
                tok[tok_len + 1] = '\0';
                tok_len++;
            }
        }

        size_t needed = (wrote_first_token ? 1 : 0) + tok_len;
        if (pos + needed + 1 >= out_size) continue;
        if (wrote_first_token) out[pos++] = ' ';
        memcpy(out + pos, tok, tok_len);
        pos += tok_len;
        wrote_first_token = true;
    }
    if (out_size > 0) out[pos] = '\0';
}

static const move_t CROSS[] = {
    MOVE_FACE(ROT_R),       MOVE_FACE(ROT_R),       MOVE_FACE(ROT_L_PRIME), MOVE_FACE(ROT_D),       MOVE_FACE(ROT_F),
    MOVE_FACE(ROT_F),       MOVE_FACE(ROT_R_PRIME), MOVE_FACE(ROT_D_PRIME), MOVE_FACE(ROT_R_PRIME), MOVE_FACE(ROT_L),
    MOVE_FACE(ROT_U_PRIME), MOVE_FACE(ROT_D),       MOVE_FACE(ROT_R),       MOVE_FACE(ROT_D),       MOVE_FACE(ROT_B),
    MOVE_FACE(ROT_B),       MOVE_FACE(ROT_R_PRIME), MOVE_FACE(ROT_U),       MOVE_FACE(ROT_D),       MOVE_FACE(ROT_D),
};

static const move_t FOUR_CROSSES[] = {
    MOVE_FACE(ROT_U), MOVE_FACE(ROT_U), MOVE_FACE(ROT_R), MOVE_FACE(ROT_R), MOVE_FACE(ROT_L),
    MOVE_FACE(ROT_L), MOVE_FACE(ROT_F), MOVE_FACE(ROT_F), MOVE_FACE(ROT_B), MOVE_FACE(ROT_B),
    MOVE_FACE(ROT_D), MOVE_FACE(ROT_D), MOVE_FACE(ROT_L), MOVE_FACE(ROT_L), MOVE_FACE(ROT_R),
    MOVE_FACE(ROT_R), MOVE_FACE(ROT_F), MOVE_FACE(ROT_F), MOVE_FACE(ROT_B), MOVE_FACE(ROT_B),
};

static const move_t CUBE_IN_CUBE[] = {
    MOVE_FACE(ROT_F),       MOVE_FACE(ROT_L),       MOVE_FACE(ROT_F), MOVE_FACE(ROT_U_PRIME), MOVE_FACE(ROT_R),
    MOVE_FACE(ROT_U),       MOVE_FACE(ROT_F),       MOVE_FACE(ROT_F), MOVE_FACE(ROT_L),       MOVE_FACE(ROT_L),
    MOVE_FACE(ROT_U_PRIME), MOVE_FACE(ROT_L_PRIME), MOVE_FACE(ROT_B), MOVE_FACE(ROT_D_PRIME), MOVE_FACE(ROT_B_PRIME),
    MOVE_FACE(ROT_L),       MOVE_FACE(ROT_L),       MOVE_FACE(ROT_U),
};

static const move_t CUBE_IN_CUBE_IN_CUBE[] = {
    MOVE_FACE(ROT_U_PRIME), MOVE_FACE(ROT_L_PRIME), MOVE_FACE(ROT_U_PRIME), MOVE_FACE(ROT_F_PRIME),
    MOVE_FACE(ROT_R),       MOVE_FACE(ROT_R),       MOVE_FACE(ROT_B_PRIME), MOVE_FACE(ROT_R),
    MOVE_FACE(ROT_F),       MOVE_FACE(ROT_U),       MOVE_FACE(ROT_B),       MOVE_FACE(ROT_B),
    MOVE_FACE(ROT_U),       MOVE_FACE(ROT_B_PRIME), MOVE_FACE(ROT_L),       MOVE_FACE(ROT_U_PRIME),
    MOVE_FACE(ROT_F),       MOVE_FACE(ROT_U),       MOVE_FACE(ROT_R),       MOVE_FACE(ROT_F_PRIME),
};

static const move_t FOUR_SPOTS[] = {
    MOVE_FACE(ROT_F), MOVE_FACE(ROT_F), MOVE_FACE(ROT_B), MOVE_FACE(ROT_B), MOVE_FACE(ROT_U), MOVE_FACE(ROT_D_PRIME),
    MOVE_FACE(ROT_R), MOVE_FACE(ROT_R), MOVE_FACE(ROT_L), MOVE_FACE(ROT_L), MOVE_FACE(ROT_U), MOVE_FACE(ROT_D_PRIME),
};

static const move_t SIX_SPOTS[] = {
    MOVE_FACE(ROT_U), MOVE_FACE(ROT_D_PRIME), MOVE_FACE(ROT_R), MOVE_FACE(ROT_L_PRIME),
    MOVE_FACE(ROT_F), MOVE_FACE(ROT_B_PRIME), MOVE_FACE(ROT_U), MOVE_FACE(ROT_D_PRIME),
};

static void build_superflip (int size, queue_t *q, char *out, size_t out_size)
{
    /* For each k in [1, (size-1)/2], run 3 reps of ((kM' kwU')^4 X Y).
     * kM' is ROT_M_PRIME with num_layers=k: a "thick slice" of size-2k
     * layers. On 3x3 (k=1) this is the single central slice. On 5x5
     * k=1 is 3 layers (the whole inner band) and k=2 is just the
     * center. kwU' is a wide U' covering the top k layers (k=1 is
     * the outer face). 4x4 only has k=1 and runs the same shape as
     * 3x3 with M' interpreted as the inner 2-layer band. */
    size_t pos       = 0;
    bool wrote_first = false;
    if (out_size > 0) out[0] = '\0';

    int max_k = (size - 1) / 2;
    for (int k = 1; k <= max_k; k++)
        for (int rep = 0; rep < 3; rep++) {
            for (int i = 0; i < 4; i++) {
                queue_push(q, move_slice(ROT_M_PRIME, k));
                queue_push(q, move_face_wide(ROT_U_PRIME, k));

                char buf[16];
                if (k == 1)
                    snprintf(buf, sizeof buf, "M'");
                else
                    snprintf(buf, sizeof buf, "%dM'", k);
                append_token(out, out_size, &pos, &wrote_first, buf);

                if (k == 1)
                    snprintf(buf, sizeof buf, "U'");
                else
                    snprintf(buf, sizeof buf, "%dwU'", k);
                append_token(out, out_size, &pos, &wrote_first, buf);
            }
            queue_push(q, move_whole(ROT_X));
            append_token(out, out_size, &pos, &wrote_first, "X");
            queue_push(q, move_whole(ROT_Y));
            append_token(out, out_size, &pos, &wrote_first, "Y");
        }
}

static void build_checkerboard (int size, queue_t *q, char *out, size_t out_size)
{
    /* For each k in [1, max] where (size - 2k) >= 1, do (kwM2 kwE2 kwS2)
     * where kwM is ROT_M_PRIME with num_layers=k.
     * Iterations go from largest to smallest thickness:
     *   3x3 -> M2 E2 S2
     *   4x4 -> 2wM2 2wE2 2wS2
     *   5x5 -> 3wM2 3wE2 3wS2  M2 E2 S2
     *   6x6 -> 4wM2 4wE2 4wS2  2wM2 2wE2 2wS2
     *   7x7 -> 5wM2 5wE2 5wS2  3wM2 3wE2 3wS2  M2 E2 S2 */
    size_t pos       = 0;
    bool wrote_first = false;
    if (out_size > 0) out[0] = '\0';

    static const struct {
        rotation_t rot;
        char letter;
    } slices[] = {
        { ROT_M_PRIME, 'M' },
        { ROT_E_PRIME, 'E' },
        { ROT_S_PRIME, 'S' },
    };

    for (int k = 1; size - 2 * k >= 1; k++) {
        int thickness = size - 2 * k;
        for (size_t s = 0; s < ARRAY_LEN(slices); s++) {
            queue_push(q, move_slice(slices[s].rot, k));
            queue_push(q, move_slice(slices[s].rot, k));

            char buf[16];
            if (thickness == 1)
                snprintf(buf, sizeof buf, "%c2", slices[s].letter);
            else
                snprintf(buf, sizeof buf, "%dw%c2", thickness, slices[s].letter);
            append_token(out, out_size, &pos, &wrote_first, buf);
        }
    }
}

static void build_cross (int size, queue_t *q, char *out, size_t out_size)
{
    (void)size;
    pattern_emit_static(CROSS, ARRAY_LEN(CROSS), q, out, out_size);
}

static void build_four_crosses (int size, queue_t *q, char *out, size_t out_size)
{
    (void)size;
    pattern_emit_static(FOUR_CROSSES, ARRAY_LEN(FOUR_CROSSES), q, out, out_size);
}

static void build_cube_in_cube (int size, queue_t *q, char *out, size_t out_size)
{
    (void)size;
    pattern_emit_static(CUBE_IN_CUBE, ARRAY_LEN(CUBE_IN_CUBE), q, out, out_size);
}

static void build_cube_in_cube_in_cube (int size, queue_t *q, char *out, size_t out_size)
{
    (void)size;
    pattern_emit_static(CUBE_IN_CUBE_IN_CUBE, ARRAY_LEN(CUBE_IN_CUBE_IN_CUBE), q, out, out_size);
}

static void build_four_spots (int size, queue_t *q, char *out, size_t out_size)
{
    (void)size;
    pattern_emit_static(FOUR_SPOTS, ARRAY_LEN(FOUR_SPOTS), q, out, out_size);
}

static void build_six_spots (int size, queue_t *q, char *out, size_t out_size)
{
    (void)size;
    pattern_emit_static(SIX_SPOTS, ARRAY_LEN(SIX_SPOTS), q, out, out_size);
}

const pattern_t PATTERNS[] = {
    { "Superflip",                3, 0, build_superflip            },
    { "Checkerboard",             3, 0, build_checkerboard         },
    { "Cross",                    3, 3, build_cross                },
    { "4 Crosses",                3, 3, build_four_crosses         },
    { "Cube in a cube",           3, 3, build_cube_in_cube         },
    { "Cube in a cube in a cube", 3, 3, build_cube_in_cube_in_cube },
    { "Four spots",               3, 3, build_four_spots           },
    { "Six spots",                3, 3, build_six_spots            },
};

const size_t PATTERNS_COUNT = ARRAY_LEN(PATTERNS);
