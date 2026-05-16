#include "amber.h"
#include "average.h"
#include "camera.h"
#include "cube.h"
#include "font.h"
#include "keybindings.h"
#include "logger.h"
#include "options.h"
#include "playback.h"
#include "queue.h"
#include "raylib.h"
#include "scramble.h"
#include "solver.h"
#include "time_consts.h"
#include "timer.h"
#include "ui_cube.h"
#include "ui_help.h"
#include "ui_loading.h"
#include "ui_moves.h"
#include "ui_patterns.h"
#include "utils.h"
#include "widgets.h"

#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

/* ----- App-wide constants ---------------------------------------------- */

#define CRUBIK_VERSION_MAJOR 0
#define CRUBIK_VERSION_MINOR 1
#define CRUBIK_VERSION_PATCH 0

#define CRUBIK_STRINGIFY_RAW(x) #x
#define CRUBIK_STRINGIFY(x)     CRUBIK_STRINGIFY_RAW(x)
#define CRUBIK_VERSION                     \
    CRUBIK_STRINGIFY(CRUBIK_VERSION_MAJOR) \
    "." CRUBIK_STRINGIFY(CRUBIK_VERSION_MINOR) "." CRUBIK_STRINGIFY(CRUBIK_VERSION_PATCH)

#define CUBIE_SIZE 0.98f

#define TIMER_STR_LEN            10
#define CURRENT_SCRAMBLE_TOK_LEN 6 /* upper bound on chars per scramble token plus delimiter */

#define KEEP_SPACE_DOWN_MS 300

#define WINDOW_INIT_W 1200
#define WINDOW_INIT_H 800
#define WINDOW_MIN_W  1080
#define WINDOW_MIN_H  600
#define TARGET_FPS    40

/* GUI text colors are 0xRRGGBBAA. */
#define GUI_TEXT_COLOR_NORMAL  0x000000FF
#define GUI_TEXT_COLOR_FOCUSED 0xBBBBBBFF
#define GUI_TEXT_COLOR_PRESSED 0xFFFFFFFF
#define GUI_TEXT_SPACING       2

/* ----- Cube / scramble / solution state -------------------------------- */

static cube_t cube;
static char *current_scramble;
static queue_t *queue;

static char times[LAST_N_SOLVES][TIME_STR_MAX];
static char avg[AVG_STR_LEN];
static char avg12[AVG_STR_LEN];
static char best[AVG_STR_LEN];

/* ----- UI mode --------------------------------------------------------- */

typedef enum {
    SCREEN_CUBE,
    SCREEN_HELP,
    SCREEN_OPTIONS,
    SCREEN_PATTERNS,
} screen_t;

static screen_t current_screen    = SCREEN_CUBE;
static bool show_exit_message_box = false;
static bool is_everything_loaded  = false;
static bool show_debug_axes       = false;

/* ----- Timer overlay --------------------------------------------------- */

#define TIMER_READY_COLOR ((Color){ 0, 204, 51, 255 })

static stopwatch_t timer;
static Color timer_color                    = BLACK;
static char timer_string[TIMER_STR_LEN]     = "00:00.000";
static bool is_timer_ready                  = false;
static struct timespec key_space_down_start = { .tv_nsec = -1 };

/* ----- Time-detail dialog state (clicked from the Ao5 history) -------- */

static bool is_time_detail_open = false;
static int time_detail_index    = -1;
static int time_detail_pos_y    = 0;

/* ----- Misc ----------------------------------------------------------- */

static bool exit_program = false;

/* ====================================================================== */

static void handle_rotation (rotation_t clockwise, rotation_t anti_clockwise)
{
    if (solver_is_running) return;
    solver_clear_solution();
    if (IsKeyDown(keybindings.key_ALT))
        queue_push(queue, move_face(anti_clockwise));
    else
        queue_push(queue, move_face(clockwise));
}

/* Consumes `moves[0..length-1]`: applies each to the cube, appends to
 * current_scramble for display, frees the token. The caller still owns
 * the outer array. */
static void apply_and_record_scramble (char **moves, int length)
{
    for (int i = 0; i < length; i++) {
        cube_apply_move(&cube, moves[i]);
        if (moves[i][0] == '1' && moves[i][1] == 'w')
            strcat(current_scramble, moves[i] + 2);
        else
            strcat(current_scramble, moves[i]);
        free(moves[i]);
        if (i != length - 1) strcat(current_scramble, " ");
    }
}

static void clear_scramble_and_solution (void)
{
    current_scramble[0] = '\0';
    solver_clear_solution();
}

static void reset_animation_and_solution (void)
{
    cube.is_animating = false;
    queue_clear(queue);
}

static void reset_cube_to_solved (void)
{
    int size = cube.size;
    cube_destroy(&cube);
    cube = cube_make(size, CUBIE_SIZE);
}

static void generate_new_scramble (void)
{
    solver_cancel();
    clear_scramble_and_solution();
    reset_animation_and_solution();
    reset_cube_to_solved();

    int length   = scramble_length(cube.size);
    char **moves = malloc(length * sizeof(char *));
    if (moves == NULL) {
        LOG_ERROR("out of memory generating scramble");
        return;
    }
    if (scramble_generate(moves, length, cube.size) != SCRAMBLE_OK) {
        LOG_ERROR("scramble_generate failed");
        free(moves);
        return;
    }
    apply_and_record_scramble(moves, length);
    free(moves);
    timer.is_disabled = false;
}

static void init_scramble_and_solution (int size)
{
    cube             = cube_make(size, CUBIE_SIZE);
    int length       = scramble_length(size);
    current_scramble = malloc((CURRENT_SCRAMBLE_TOK_LEN * length + 1) * sizeof(char));
    clear_scramble_and_solution();
    reset_animation_and_solution();
    avg[0]   = '\0';
    avg12[0] = '\0';
    best[0]  = '\0';
}

static void recompute_solve_stats (void)
{
    solves_average_of_5(cube.size, avg);
    solves_average_of_12(cube.size, avg12);
    solves_best_time(cube.size, best);
}

static void apply_queued_moves_instantly (void)
{
    move_t m;
    while (queue_pop(queue, &m) == QUEUE_OK) {
        if (m.single_layer)
            cube_rotate_single_layer(&cube, m.rotation, m.num_layers);
        else
            cube_rotate(&cube, m.rotation, m.num_layers);
    }
}

static void replay_scramble (const char *scramble_str)
{
    solver_cancel();
    clear_scramble_and_solution();
    reset_animation_and_solution();
    reset_cube_to_solved();

    char *copy = malloc(strlen(scramble_str) + 1);
    if (copy == NULL) {
        LOG_ERROR("out of memory replaying scramble");
        return;
    }
    strcpy(copy, scramble_str);
    for (char *t = strtok(copy, " "); t != NULL; t = strtok(NULL, " ")) {
        char move_buf[8];
        if (t[0] >= '0' && t[0] <= '9')
            snprintf(move_buf, sizeof move_buf, "%s", t);
        else
            snprintf(move_buf, sizeof move_buf, "1w%s", t);
        cube_apply_move(&cube, move_buf);
    }
    free(copy);
    strcpy(current_scramble, scramble_str);

    timer.is_disabled = false;
}

static void resize_cube (int increment)
{
    int new_size = cube.size;
    if (!((new_size == CUBE_MAX_SIZE && increment > 0) || (new_size == 1 && increment < 0))) new_size += increment;

    solver_cancel();
    free(current_scramble);
    cube_destroy(&cube);

    init_scramble_and_solution(new_size);
    camera_init(new_size);
    solves_load_last_5(times, new_size);
    recompute_solve_stats();
}

/* Space-bar arms then starts the timer (WCA-style). The user holds space
 * for KEEP_SPACE_DOWN_MS to "arm" (timer turns green), on release the
 * timer starts. Pressing any key while the timer runs stops it. */
static void handle_space_key_held (void)
{
    if (!is_timer_ready) {
        if (key_space_down_start.tv_nsec == -1) {
            clock_gettime(CLOCK_MONOTONIC, &key_space_down_start);
            return;
        }
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long long elapsed_ns = (now.tv_sec - key_space_down_start.tv_sec) * (long long)NS_PER_SEC
                               + (now.tv_nsec - key_space_down_start.tv_nsec);
        long long elapsed_ms = elapsed_ns / NS_PER_MS;
        is_timer_ready       = elapsed_ms > KEEP_SPACE_DOWN_MS;
        return;
    }
    if (!timer.is_running && !timer.just_stopped)
        timer_color = TIMER_READY_COLOR;
    else if (!timer.just_stopped)
        timer_stop(&timer);
}

static void handle_space_key_released (void)
{
    timer_color = BLACK;
    if (!timer.is_running && is_timer_ready) timer_start(&timer);
    key_space_down_start.tv_nsec = -1;
}

static void update_timer_string (void)
{
    snprintf(timer_string, sizeof timer_string, "%02d:%02d.%03d", timer.minutes, timer.seconds, timer.milliseconds);
}

static void handle_key_press (void)
{
    if (GetKeyPressed() && timer.is_running) {
        timer_stop(&timer);
        update_timer_string();
        solves_save(timer_string, current_scramble, cube.size);
        solves_load_last_5(times, cube.size);
        recompute_solve_stats();
        generate_new_scramble();
        timer.just_stopped = false;
        is_timer_ready     = false;
        return;
    }
    if (IsKeyPressed(keybindings.key_U))
        handle_rotation(ROT_U, ROT_U_PRIME);
    else if (IsKeyPressed(keybindings.key_D))
        handle_rotation(ROT_D, ROT_D_PRIME);
    else if (IsKeyPressed(keybindings.key_L))
        handle_rotation(ROT_L, ROT_L_PRIME);
    else if (IsKeyPressed(keybindings.key_R))
        handle_rotation(ROT_R, ROT_R_PRIME);
    else if (IsKeyPressed(keybindings.key_F))
        handle_rotation(ROT_F, ROT_F_PRIME);
    else if (IsKeyPressed(keybindings.key_B))
        handle_rotation(ROT_B, ROT_B_PRIME);
    else if (IsKeyPressed(keybindings.key_M))
        handle_rotation(ROT_M, ROT_M_PRIME);
    else if (IsKeyPressed(keybindings.key_E))
        handle_rotation(ROT_E, ROT_E_PRIME);
    else if (IsKeyPressed(keybindings.key_S))
        handle_rotation(ROT_S, ROT_S_PRIME);
    else if (IsKeyPressed(keybindings.key_X))
        handle_rotation(ROT_X, ROT_X_PRIME);
    else if (IsKeyPressed(keybindings.key_Y))
        handle_rotation(ROT_Y, ROT_Y_PRIME);
    else if (IsKeyPressed(keybindings.key_Z))
        handle_rotation(ROT_Z, ROT_Z_PRIME);
    else if (IsKeyPressed(KEY_ENTER))
        generate_new_scramble();
    else if (IsKeyPressed(KEY_K))
        solver_launch(&cube);
    else if (IsKeyDown(KEY_SPACE) && !timer.is_disabled)
        handle_space_key_held();
    else if (IsKeyReleased(KEY_SPACE) && !timer.is_disabled)
        handle_space_key_released();
    else if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_PAGE_UP))
        resize_cube(1);
    else if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_PAGE_DOWN))
        resize_cube(-1);
    else if (IsKeyPressed(KEY_ESCAPE))
        show_exit_message_box = true;
    else if (IsKeyPressed(KEY_END))
        show_debug_axes = !show_debug_axes;
}

static void handle_queue (void)
{
    if (cube.is_animating) return;
    if (queue_is_empty(queue)) {
        solver_finish(&timer);
        playback_clear();
        return;
    }
    move_t popped;
    if (queue_pop(queue, &popped) != QUEUE_OK) return;
    cube.is_animating = true;
    cube.current_move = popped;
    playback_advance();
}

static void handle_mouse_and_update_camera (void)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        solver_cancel();
        reset_cube_to_solved();
        clear_scramble_and_solution();
        reset_animation_and_solution();
    }
    camera_update(cube.size);
}

/* ----- Cube-screen layout shared with hints --------------------------- */

#define HINT_MARGIN 10

/* ----- Cube screen ---------------------------------------------------- */

#define SCRAMBLE_LABEL_FONT_SIZE 35
#define SCRAMBLE_MOVES_TOP_Y     75
#define TIMER_FONT_SIZE          45
#define TIMER_BOTTOM_Y           50
#define SOLUTION_FOUND_BOTTOM_Y  130
#define SOLUTION_MOVES_BOTTOM_Y  100
#define AO5_LEFT_X               10
#define AO5_TOP_OFFSET           100
#define AO5_LABEL_GAP            10
#define STATS_LINE_HEIGHT        30
#define TIME_LIST_LINE_HEIGHT    30
#define TIME_LIST_START_OFFSET   (-2)
#define TIME_DETAIL_W            450
#define TIME_DETAIL_H            100
#define APPLY_BUTTON_W           100
#define APPLY_BUTTON_H           30
#define APPLY_BUTTON_BOTTOM_Y    75

static void draw_hint_bar (void)
{
    font_draw("Press 'h' for help.", HINT_MARGIN, HINT_MARGIN, DEFAULT_FONT_SIZE, DARKGRAY);
    int hint_w = font_measure("Press 'o' for options. ", DEFAULT_FONT_SIZE);
    font_draw("Press 'o' for options.", GetScreenWidth() - hint_w - HINT_MARGIN, HINT_MARGIN, DEFAULT_FONT_SIZE,
              DARKGRAY);
    hint_w = font_measure("Press 'p' for patterns. ", DEFAULT_FONT_SIZE);
    font_draw("Press 'p' for patterns.", GetScreenWidth() - hint_w - HINT_MARGIN, HINT_MARGIN + DEFAULT_FONT_SIZE,
              DEFAULT_FONT_SIZE, DARKGRAY);
}

static void draw_scramble_strip (void)
{
    const char *scramble_label = "Current scramble:";
    font_draw(scramble_label, GetScreenWidth() / 2 - font_measure(scramble_label, SCRAMBLE_LABEL_FONT_SIZE) / 2,
              HINT_MARGIN, SCRAMBLE_LABEL_FONT_SIZE, BLACK);
    ui_moves_draw(current_scramble, DEFAULT_FONT_SIZE, SCRAMBLE_MOVES_TOP_Y, -1);
}

static void draw_timer_overlay (void)
{
    timer_update(&timer);
    update_timer_string();
    font_draw(timer_string, GetScreenWidth() / 2 - font_measure("00:00.00", TIMER_FONT_SIZE) / 2,
              GetScreenHeight() - TIMER_BOTTOM_Y, TIMER_FONT_SIZE, timer_color);
}

static void draw_solution_panel (bool *out_hover)
{
    bool hov = false;

    if (solver_current_solution_size != 0)
        font_draw(solver_solution_found_text,
                  GetScreenWidth() / 2 - font_measure(solver_solution_found_text, DEFAULT_FONT_SIZE) / 2,
                  GetScreenHeight() - SOLUTION_FOUND_BOTTOM_Y, DEFAULT_FONT_SIZE, BLACK);
    if (playback.active)
        ui_moves_draw(playback.text, DEFAULT_FONT_SIZE, GetScreenHeight() - SOLUTION_MOVES_BOTTOM_Y,
                      playback.current_move_index);
    else
        ui_moves_draw(solver_current_solution, DEFAULT_FONT_SIZE, GetScreenHeight() - SOLUTION_MOVES_BOTTOM_Y, -1);

    if (solver_current_solution_size > 0) {
        Rectangle rec = (Rectangle){
            .x      = GetScreenWidth() - 2 * APPLY_BUTTON_W,
            .y      = GetScreenHeight() - APPLY_BUTTON_BOTTOM_Y,
            .width  = APPLY_BUTTON_W,
            .height = APPLY_BUTTON_H,
        };
        if (button_draw(rec, "Apply", &hov)) solver_apply_current(queue, &timer);
    }

    if (out_hover) *out_hover = hov;
}

static void draw_stat_line (const char *label, const char *value, int y)
{
    font_draw(label, AO5_LEFT_X, y, DEFAULT_FONT_SIZE, BLACK);
    font_draw(value, AO5_LEFT_X + font_measure(label, DEFAULT_FONT_SIZE) + AO5_LABEL_GAP, y, DEFAULT_FONT_SIZE, BLACK);
}

static void draw_solves_history (bool *out_hover)
{
    int y0 = GetScreenHeight() / 2 - AO5_TOP_OFFSET;
    draw_stat_line("Best:", best, y0 - 2 * STATS_LINE_HEIGHT);
    draw_stat_line("Ao12:", avg12, y0 - STATS_LINE_HEIGHT);
    draw_stat_line("Ao5: ", avg, y0);

    bool any_hover = false;
    bool was_open  = is_time_detail_open;
    int pos_y      = TIME_LIST_START_OFFSET;
    for (int i = LAST_N_SOLVES - 1; i >= 0; i--) {
        if (times[i][0] == '-') continue;
        Rectangle bounds = (Rectangle){
            AO5_LEFT_X,
            (float)GetScreenHeight() / 2 + pos_y * TIME_LIST_LINE_HEIGHT,
            font_measure(times[i], DEFAULT_FONT_SIZE),
            DEFAULT_FONT_SIZE,
        };
        if (is_time_detail_open) {
            font_draw(times[i], (int)bounds.x, (int)bounds.y, DEFAULT_FONT_SIZE, BLACK);
        } else {
            bool hov;
            if (label_button_draw(bounds, times[i], DEFAULT_FONT_SIZE, &hov)) {
                is_time_detail_open = true;
                time_detail_index   = i;
                time_detail_pos_y   = pos_y;
            }
            any_hover |= hov;
        }
        pos_y++;
    }

    if (!is_time_detail_open) {
        if (out_hover) *out_hover = any_hover;
        return;
    }

    Rectangle dialog_bounds
        = { AO5_LEFT_X, (float)GetScreenHeight() / 2 + (time_detail_pos_y + 1) * TIME_LIST_LINE_HEIGHT, TIME_DETAIL_W,
            TIME_DETAIL_H };
    bool dialog_hov;
    int result = message_box_draw(dialog_bounds, "Time details", times[time_detail_index], "Cancel;+2;DNF;Replay",
                                  &dialog_hov);
    any_hover |= dialog_hov;
    if (!result || result == 1)
        is_time_detail_open = false;
    else if (result == 2) {
        solves_toggle_plus_two(time_detail_index, cube.size);
        solves_load_last_5(times, cube.size);
        recompute_solve_stats();
        is_time_detail_open = false;
    } else if (result == 3) {
        solves_toggle_dnf(time_detail_index, cube.size);
        solves_load_last_5(times, cube.size);
        recompute_solve_stats();
        is_time_detail_open = false;
    } else if (result == 4) {
        char scramble_buf[CURRENT_SCRAMBLE_TOK_LEN * scramble_length(cube.size) + 1];
        if (solves_get_scramble(time_detail_index, cube.size, scramble_buf, sizeof scramble_buf))
            replay_scramble(scramble_buf);
        is_time_detail_open = false;
    } else if (was_open && result < 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)
               && !CheckCollisionPointRec(GetMousePosition(), dialog_bounds)) {
        is_time_detail_open = false;
    }
    if (out_hover) *out_hover = any_hover;
}

static void draw_cube_screen (bool *out_hover)
{
    ui_cube_3d_draw(&cube, show_debug_axes);
    draw_hint_bar();
    draw_scramble_strip();
    draw_timer_overlay();
    bool sol_hov, hist_hov;
    draw_solution_panel(&sol_hov);
    draw_solves_history(&hist_hov);
    if (out_hover) *out_hover = sol_hov || hist_hov;
}

/* ----- Bootstrap ----------------------------------------------------- */

void *init_everything (void *arg)
{
    bool skip_kociemba = (arg != NULL) && *(bool *)arg;
    if (!skip_kociemba) solver_init_kociemba();

    keybindings_init();
    options_load();
    queue = queue_create();
    if (queue == NULL) {
        LOG_ERROR("queue_create failed");
        exit(1);
    }

    init_scramble_and_solution(CUBE_DEFAULT_SIZE);
    camera_init(cube.size);
    solves_load_last_5(times, cube.size);

    ui_help_init();

    is_everything_loaded = true;

    recompute_solve_stats();

    return NULL;
}

#define EXIT_DIALOG_W      400
#define EXIT_DIALOG_H      150
#define EXIT_DIALOG_HALF_W (EXIT_DIALOG_W / 2)
#define EXIT_DIALOG_HALF_H (EXIT_DIALOG_H / 2)

/* Pressing the hotkey for `target` from the cube screen opens it, pressing
 * the same hotkey from `target` returns to the cube. Other screens swallow
 * the keypress (no cross-screen jumps). */
static void toggle_screen (screen_t target)
{
    if (current_screen == SCREEN_CUBE)
        current_screen = target;
    else if (current_screen == target)
        current_screen = SCREEN_CUBE;
}

void update_draw_frame (void)
{
    if (IsKeyPressed(KEY_H))
        toggle_screen(SCREEN_HELP);
    else if (IsKeyPressed(KEY_O)) {
        bool was_on_options = (current_screen == SCREEN_OPTIONS);
        toggle_screen(SCREEN_OPTIONS);
        if (was_on_options) options_save();
    } else if (IsKeyPressed(KEY_P))
        toggle_screen(SCREEN_PATTERNS);

    if (current_screen == SCREEN_CUBE) {
        handle_mouse_and_update_camera();
        handle_key_press();
        handle_queue();
    }

    BeginDrawing();
    bool any_hover = false;
    switch (current_screen) {
        case SCREEN_HELP:
            ui_help_draw();
            break;
        case SCREEN_OPTIONS:
            options_draw_screen(&any_hover);
            break;
        case SCREEN_PATTERNS: {
            char text[PLAYBACK_TEXT_LEN];
            if (ui_patterns_draw(cube.size, queue, text, sizeof text, &any_hover)) {
                current_screen = SCREEN_CUBE;
                clear_scramble_and_solution();
                if (options_animate_patterns())
                    playback_init(text);
                else
                    apply_queued_moves_instantly();
            }
        } break;
        case SCREEN_CUBE:
            draw_cube_screen(&any_hover);
            break;
    }
    if (show_exit_message_box) {
        bool dialog_hov;
        int result = message_box_draw((Rectangle){ (float)GetScreenWidth() / 2 - EXIT_DIALOG_HALF_W,
                                                   (float)GetScreenHeight() / 2 - EXIT_DIALOG_HALF_H / 2, EXIT_DIALOG_W,
                                                   EXIT_DIALOG_H },
                                      "Exit", "Do you really want to quit ?", "Yes;No", &dialog_hov);
        any_hover |= dialog_hov;

        if (result == 1)
            exit_program = true;
        else if (result == 2 || result == 0)
            show_exit_message_box = false;
    }
    SetMouseCursor(any_hover ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);
    EndDrawing();
}

static void print_usage (const char *argv)
{
    printf("Usage: %s [options]\n", argv);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help               Show this help and exit\n");
    printf("  -v, --version            Show version and exit\n");
    printf("      --no-kociemba        Skip Kociemba pruning-table init\n");
    printf("      --log-level LEVEL    debug, info (default), warn, error, none\n");
    printf("      --log-file PATH      Redirect logs to a file (default: stderr)\n");
}

static log_level_t parse_log_level (const char *optarg)
{
    if (strcmp(optarg, "debug") == 0) return LOG_LEVEL_DEBUG;
    if (strcmp(optarg, "info") == 0) return LOG_LEVEL_INFO;
    if (strcmp(optarg, "warn") == 0) return LOG_LEVEL_WARN;
    if (strcmp(optarg, "error") == 0) return LOG_LEVEL_ERROR;
    if (strcmp(optarg, "none") == 0) return LOG_LEVEL_NONE;
    fprintf(stderr, "unknown log level: %s\n", optarg);
    exit(1);
}

int main (int argc, char **argv)
{
    const char *optstring = "hv";
    enum {
        OPT_LOG_LEVEL = 1000,
        OPT_LOG_FILE,
        OPT_NO_KOCIEMBA
    };
    const struct option long_opts[] = {
        { "help",        no_argument,       NULL, 'h'             },
        { "version",     no_argument,       NULL, 'v'             },
        { "no-kociemba", no_argument,       NULL, OPT_NO_KOCIEMBA },
        { "log-level",   required_argument, NULL, OPT_LOG_LEVEL   },
        { "log-file",    required_argument, NULL, OPT_LOG_FILE    },
        { 0,             0,                 0,    0               }
    };

    int c;
    bool skip_kociemba    = false;
    log_level_t log_level = LOG_LEVEL_INFO;
    FILE *log_out         = NULL;
    while ((c = getopt_long(argc, argv, optstring, long_opts, NULL)) != -1) {
        switch (c) {
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'v':
                printf("cRubik v%s\n", CRUBIK_VERSION);
                return 0;
            case OPT_NO_KOCIEMBA:
                skip_kociemba = true;
                break;
            case OPT_LOG_LEVEL:
                log_level = parse_log_level(optarg);
                break;
            case OPT_LOG_FILE:
                log_out = fopen(optarg, "w");
                break;
            case '?':
                return 1;
            default:
                abort();
        }
    }

    log_init(log_level, log_out);
    printf("cRubik v%s\n", CRUBIK_VERSION);
    SetTraceLogLevel(LOG_WARNING);

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_INIT_W, WINDOW_INIT_H, "cRubik");
    SetExitKey(-1);
    SetWindowMinSize(WINDOW_MIN_W, WINDOW_MIN_H);
    SetTargetFPS(TARGET_FPS);

    GuiLoadStyleAmber();
    GuiSetStyle(DEFAULT, TEXT_SIZE, DEFAULT_FONT_SIZE);
    GuiSetStyle(DEFAULT, TEXT_SPACING, GUI_TEXT_SPACING);
    // GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, GUI_TEXT_COLOR_NORMAL);
    // GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, GUI_TEXT_COLOR_FOCUSED);
    // GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, GUI_TEXT_COLOR_PRESSED);
    GuiSetFont(font_get(DEFAULT_FONT_SIZE));

    pthread_t thread;
    pthread_create(&thread, NULL, init_everything, &skip_kociemba);

    int frame_count = 0;
    while (!is_everything_loaded) {
        ui_loading_draw(frame_count);
        frame_count++;
    }

    pthread_join(thread, NULL);

    while (!WindowShouldClose()) {
        update_draw_frame();
        if (exit_program) break;
    }

    options_save();

    free(current_scramble);
    cube_destroy(&cube);
    queue_destroy(queue);

    font_shutdown();
    solves_shutdown();

    CloseWindow();
    return 0;
}
