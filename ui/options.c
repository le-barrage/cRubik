#include "options.h"

#include "cJSON.h"
#include "font.h"
#include "keybindings.h"
#include "logger.h"
#include "raygui.h"
#include "raylib.h"
#include "widgets.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPTIONS_FILE    "options.json"
#define OPTIONS_VERSION 1

#define OPTIONS_BG_COLOR GRAY

#define DEFAULT_ROTATION_SPEED   25
#define MIN_ROTATION_SPEED       1
#define MAX_ROTATION_SPEED       30
#define DEFAULT_SOLVER_MODE      OPTIONS_SOLVER_REORIENT
#define DEFAULT_ANIMATE_PATTERNS true

#define KEYBIND_SECTION_Y     60
#define KEYBIND_TITLE_GAP     40
#define KEYBIND_BUTTON_WIDTH  120
#define KEYBIND_BUTTON_HEIGHT 35
#define KEYBIND_LABEL_WIDTH   50
#define KEYBIND_SPACING       15
#define KEYBIND_COLUMNS       3

#define SLIDER_Y         450
#define SLIDER_WIDTH     150
#define SLIDER_HEIGHT    30
#define SLIDER_LABEL_GAP 30
#define SLIDER_VALUE_GAP 10

#define TOGGLE_Y           550
#define TOGGLE_GROUP_WIDTH 400
#define TOGGLE_HEIGHT      30
#define TOGGLE_LABEL_GAP   30

#define CHECKBOX_Y         620
#define CHECKBOX_SIZE      20
#define CHECKBOX_LABEL_GAP 10

#define RESET_BUTTON_HEIGHT    35
#define RESET_BUTTON_PADDING_X 20
#define RESET_BUTTON_BOTTOM_Y  60

#define EXIT_TEXT_MARGIN 10

/* Empirical color tweaks: lighten for the resting state, darken on hover. */
#define HOVER_DARKEN -0.1f
#define REST_LIGHTEN 0.1f

/* JSON keys are storage format. Do not rename without a migration. */
#define JSON_KEY_VERSION          "version"
#define JSON_KEY_ROTATION         "rotationSpeed"
#define JSON_KEY_SOLVER_MODE      "solverOutputMode"
#define JSON_KEY_KEYBINDINGS      "keybindings"
#define JSON_KEY_ANIMATE_PATTERNS "animatePatterns"
#define JSON_VAL_PRESERVE         "preserve"
#define JSON_VAL_REORIENT         "reorient"

static int rotation_speed                = DEFAULT_ROTATION_SPEED;
static options_solver_mode_t solver_mode = DEFAULT_SOLVER_MODE;
static bool animate_patterns             = DEFAULT_ANIMATE_PATTERNS;
static int editing_key_index             = -1;

int options_rotation_speed (void) { return rotation_speed; }

options_solver_mode_t options_solver_mode (void) { return solver_mode; }

bool options_animate_patterns (void) { return animate_patterns; }

typedef struct {
    const char *name;
    int *key_ptr;
} keybinding_entry_t;

static const keybinding_entry_t KEYBINDING_ENTRIES[] = {
    { "R",   &keybindings.key_R   },
    { "L",   &keybindings.key_L   },
    { "U",   &keybindings.key_U   },
    { "D",   &keybindings.key_D   },
    { "F",   &keybindings.key_F   },
    { "B",   &keybindings.key_B   },
    { "M",   &keybindings.key_M   },
    { "S",   &keybindings.key_S   },
    { "E",   &keybindings.key_E   },
    { "X",   &keybindings.key_X   },
    { "Y",   &keybindings.key_Y   },
    { "Z",   &keybindings.key_Z   },
    { "CCW", &keybindings.key_ALT },
};
#define KEYBINDING_COUNT ARRAY_LEN(KEYBINDING_ENTRIES)

static void draw_keybindings_ui (int start_y, bool *out_hover)
{
    int total_width = KEYBIND_COLUMNS * (KEYBIND_LABEL_WIDTH + KEYBIND_BUTTON_WIDTH + KEYBIND_SPACING * 2);
    int start_x     = (GetScreenWidth() - total_width) / 2;

    const char *title = "Key Bindings (click to change):";
    int title_width   = font_measure(title, DEFAULT_FONT_SIZE);
    font_draw(title, (GetScreenWidth() - title_width) / 2, start_y, DEFAULT_FONT_SIZE, BLACK);

    int row_y0     = start_y + KEYBIND_TITLE_GAP;
    bool any_hover = false;

    for (int i = 0; i < (int)KEYBINDING_COUNT; i++) {
        int row = i / KEYBIND_COLUMNS;
        int col = i % KEYBIND_COLUMNS;
        int x   = start_x + col * (KEYBIND_LABEL_WIDTH + KEYBIND_BUTTON_WIDTH + KEYBIND_SPACING * 3);
        int y   = row_y0 + row * (KEYBIND_BUTTON_HEIGHT + KEYBIND_SPACING);

        font_draw(TextFormat("%s:", KEYBINDING_ENTRIES[i].name), x, y + (KEYBIND_BUTTON_HEIGHT - DEFAULT_FONT_SIZE) / 2,
                  DEFAULT_FONT_SIZE, BLACK);

        Rectangle button = (Rectangle){
            .x      = x + KEYBIND_LABEL_WIDTH,
            .y      = y,
            .width  = KEYBIND_BUTTON_WIDTH,
            .height = KEYBIND_BUTTON_HEIGHT,
        };

        bool hovering = CheckCollisionPointRec(GetMousePosition(), button);
        bool editing  = (editing_key_index == i);
        any_hover |= hovering;

        Color color;
        if (editing)
            color = GREEN;
        else if (hovering)
            color = ColorBrightness(DARKGRAY, HOVER_DARKEN);
        else
            color = ColorBrightness(DARKGRAY, REST_LIGHTEN);

        DrawRectangleRounded(button, 0.2f, 0, color);

        const char *key_text = editing ? "Press key" : keybindings_label(*KEYBINDING_ENTRIES[i].key_ptr);

        int text_w = font_measure(key_text, DEFAULT_FONT_SIZE);
        font_draw(key_text, button.x + (button.width - text_w) / 2, button.y + (button.height - DEFAULT_FONT_SIZE) / 2,
                  DEFAULT_FONT_SIZE, editing ? WHITE : BLACK);

        if (hovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) editing_key_index = i;
    }

    if (editing_key_index >= 0) {
        int key = GetKeyPressed();
        if (key > 0 && key != KEY_O && key != KEY_ESCAPE && key != KEY_SPACE && key != KEY_ENTER) {
            *KEYBINDING_ENTRIES[editing_key_index].key_ptr = key;
            editing_key_index                              = -1;
        }
        if (IsKeyPressed(KEY_ESCAPE)) editing_key_index = -1;
    }

    if (out_hover) *out_hover = any_hover;
}

static void draw_rotation_speed_slider (int start_y)
{
    float r          = (float)rotation_speed;
    Rectangle slider = (Rectangle){
        .x      = (float)(GetScreenWidth() - SLIDER_WIDTH) / 2,
        .y      = (float)start_y,
        .width  = SLIDER_WIDTH,
        .height = SLIDER_HEIGHT,
    };

    if (GuiSlider(slider, TextFormat("%d", MIN_ROTATION_SPEED), TextFormat("%d", MAX_ROTATION_SPEED), &r,
                  (float)MIN_ROTATION_SPEED, (float)MAX_ROTATION_SPEED))
        rotation_speed = (int)r;

    const char *label = "Cube Rotation Speed:";
    font_draw(label, slider.x + (slider.width - font_measure(label, DEFAULT_FONT_SIZE)) / 2,
              slider.y - SLIDER_LABEL_GAP, DEFAULT_FONT_SIZE, BLACK);

    const char *value = TextFormat("%d", rotation_speed);
    font_draw(value, slider.x + (slider.width - font_measure(value, DEFAULT_FONT_SIZE)) / 2,
              slider.y + slider.height + SLIDER_VALUE_GAP, DEFAULT_FONT_SIZE, BLACK);
}

static void draw_solver_mode_toggle (int start_y, bool *out_hover)
{
    Rectangle bounds = (Rectangle){
        .x      = (float)(GetScreenWidth() - TOGGLE_GROUP_WIDTH) / 2,
        .y      = (float)start_y,
        .width  = TOGGLE_GROUP_WIDTH,
        .height = TOGGLE_HEIGHT,
    };

    int active = (int)solver_mode;
    if (toggle_group_draw(bounds, "Re-orient cube;Preserve view", &active, out_hover))
        solver_mode = (options_solver_mode_t)active;

    const char *label = "Solver output:";
    font_draw(label, (GetScreenWidth() - font_measure(label, DEFAULT_FONT_SIZE)) / 2, start_y - TOGGLE_LABEL_GAP,
              DEFAULT_FONT_SIZE, BLACK);
}

static void draw_animate_patterns_checkbox (int y)
{
    const char *label = "Animate patterns";
    int label_w       = font_measure(label, DEFAULT_FONT_SIZE);
    int total_w       = CHECKBOX_SIZE + CHECKBOX_LABEL_GAP + label_w;
    Rectangle box     = (Rectangle){
            .x      = (float)(GetScreenWidth() - total_w) / 2,
            .y      = (float)y,
            .width  = CHECKBOX_SIZE,
            .height = CHECKBOX_SIZE,
    };
    GuiCheckBox(box, label, &animate_patterns);
}

static void draw_reset_button (int y, bool *out_hover)
{
    const char *text = "Reset to Defaults";
    int text_w       = font_measure(text, DEFAULT_FONT_SIZE);
    int width        = text_w + RESET_BUTTON_PADDING_X;
    Rectangle button = (Rectangle){
        .x      = (GetScreenWidth() - width) / 2,
        .y      = (float)y,
        .width  = (float)width,
        .height = RESET_BUTTON_HEIGHT,
    };

    bool hovering = CheckCollisionPointRec(GetMousePosition(), button);
    DrawRectangleRounded(button, 0.2f, 0,
                         hovering ? ColorBrightness(MAROON, HOVER_DARKEN) : ColorBrightness(MAROON, REST_LIGHTEN));
    font_draw(text, button.x + (button.width - text_w) / 2, button.y + (button.height - DEFAULT_FONT_SIZE) / 2,
              DEFAULT_FONT_SIZE, WHITE);

    if (hovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) options_reset_to_defaults();
    if (out_hover) *out_hover = hovering;
}

void options_reset_to_defaults (void)
{
    keybindings_init();
    rotation_speed    = DEFAULT_ROTATION_SPEED;
    solver_mode       = DEFAULT_SOLVER_MODE;
    animate_patterns  = DEFAULT_ANIMATE_PATTERNS;
    editing_key_index = -1;
}

void options_draw_screen (bool *out_hover)
{
    ClearBackground(OPTIONS_BG_COLOR);
    const char *exit_hint = "Press 'o' to exit.";
    int exit_w            = font_measure(exit_hint, DEFAULT_FONT_SIZE);
    font_draw(exit_hint, GetScreenWidth() - exit_w - EXIT_TEXT_MARGIN, EXIT_TEXT_MARGIN, DEFAULT_FONT_SIZE, DARKGRAY);

    bool hovering = false, sub_hov;
    draw_keybindings_ui(KEYBIND_SECTION_Y, &sub_hov);
    hovering |= sub_hov;
    draw_rotation_speed_slider(SLIDER_Y);
    draw_solver_mode_toggle(TOGGLE_Y, &sub_hov);
    hovering |= sub_hov;
    draw_animate_patterns_checkbox(CHECKBOX_Y);
    draw_reset_button(GetScreenHeight() - RESET_BUTTON_BOTTOM_Y, &sub_hov);
    hovering |= sub_hov;

    if (out_hover) *out_hover = hovering;
}

static char *read_file_to_string (const char *path)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) return NULL;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long len = ftell(f);
    if (len <= 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buf = malloc((size_t)len + 1);
    if (buf == NULL) {
        LOG_ERROR("%s: out of memory reading %ld bytes", path, len);
        fclose(f);
        return NULL;
    }

    size_t got = fread(buf, 1, (size_t)len, f);
    fclose(f);
    if (got != (size_t)len) {
        LOG_ERROR("%s: short read (%zu of %ld)", path, got, len);
        free(buf);
        return NULL;
    }
    buf[len] = '\0';
    return buf;
}

static void load_solver_mode (const cJSON *root)
{
    cJSON *node = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_SOLVER_MODE);
    if (!cJSON_IsString(node) || node->valuestring == NULL) return;
    if (strcmp(node->valuestring, JSON_VAL_PRESERVE) == 0)
        solver_mode = OPTIONS_SOLVER_PRESERVE;
    else if (strcmp(node->valuestring, JSON_VAL_REORIENT) == 0)
        solver_mode = OPTIONS_SOLVER_REORIENT;
}

static void load_keybindings (const cJSON *root)
{
    cJSON *kb = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_KEYBINDINGS);
    if (!cJSON_IsObject(kb)) return;
    for (size_t i = 0; i < KEYBINDING_COUNT; i++) {
        cJSON *item = cJSON_GetObjectItemCaseSensitive(kb, KEYBINDING_ENTRIES[i].name);
        if (cJSON_IsNumber(item)) *KEYBINDING_ENTRIES[i].key_ptr = item->valueint;
    }
}

void options_load (void)
{
    char *buf = read_file_to_string(OPTIONS_FILE);
    if (buf == NULL) return;

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (root == NULL) {
        LOG_ERROR("%s: parse error, keeping defaults", OPTIONS_FILE);
        return;
    }

    cJSON *version = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_VERSION);
    if (!cJSON_IsNumber(version) || version->valueint != OPTIONS_VERSION) {
        LOG_ERROR("%s: unsupported version, keeping defaults", OPTIONS_FILE);
        cJSON_Delete(root);
        return;
    }

    cJSON *rs = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_ROTATION);
    if (cJSON_IsNumber(rs)) rotation_speed = rs->valueint;

    load_solver_mode(root);
    load_keybindings(root);

    cJSON *ap = cJSON_GetObjectItemCaseSensitive(root, JSON_KEY_ANIMATE_PATTERNS);
    if (cJSON_IsBool(ap)) animate_patterns = cJSON_IsTrue(ap);

    cJSON_Delete(root);
}

void options_save (void)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        LOG_ERROR("%s: out of memory building JSON", OPTIONS_FILE);
        return;
    }

    cJSON_AddNumberToObject(root, JSON_KEY_VERSION, OPTIONS_VERSION);
    cJSON_AddNumberToObject(root, JSON_KEY_ROTATION, rotation_speed);
    cJSON_AddStringToObject(root, JSON_KEY_SOLVER_MODE,
                            solver_mode == OPTIONS_SOLVER_PRESERVE ? JSON_VAL_PRESERVE : JSON_VAL_REORIENT);
    cJSON_AddBoolToObject(root, JSON_KEY_ANIMATE_PATTERNS, animate_patterns);

    cJSON *kb = cJSON_AddObjectToObject(root, JSON_KEY_KEYBINDINGS);
    if (kb != NULL)
        for (size_t i = 0; i < KEYBINDING_COUNT; i++)
            cJSON_AddNumberToObject(kb, KEYBINDING_ENTRIES[i].name, *KEYBINDING_ENTRIES[i].key_ptr);

    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (out == NULL) {
        LOG_ERROR("%s: cJSON_Print failed", OPTIONS_FILE);
        return;
    }

    const char *tmp_path = OPTIONS_FILE ".tmp";
    FILE *f              = fopen(tmp_path, "wb");
    if (f == NULL) {
        LOG_PERROR("fopen " OPTIONS_FILE ".tmp");
        free(out);
        return;
    }
    fputs(out, f);
    fclose(f);
    free(out);

    if (rename(tmp_path, OPTIONS_FILE) != 0) {
        LOG_PERROR("rename " OPTIONS_FILE);
        remove(tmp_path);
    }
}
