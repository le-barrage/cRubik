#include "widgets.h"

#include "font.h"
#include "raylib.h"
#include "utils.h"

#include <string.h>

#define LABEL_BUTTON_HOVER_COLOR DARKGRAY
#define LABEL_BUTTON_REST_COLOR  BLACK

#define BUTTON_BASE_COLOR   DARKGRAY
#define BUTTON_TEXT_COLOR   BLACK
#define BUTTON_RADIUS       0.2f
#define BUTTON_HOVER_DARKEN -0.1f
#define BUTTON_REST_LIGHTEN 0.1f

#define MSGBOX_TITLE_H      30
#define MSGBOX_BUTTON_H     30
#define MSGBOX_PADDING      10
#define MSGBOX_BUTTON_GAP   5
#define MSGBOX_CLOSE_SIZE   20
#define MSGBOX_BORDER_COLOR BLACK
#define MSGBOX_BG_COLOR     RAYWHITE
#define MSGBOX_TITLE_BG     DARKGRAY
#define MSGBOX_TITLE_FG     RAYWHITE
#define MSGBOX_MSG_FG       BLACK
#define MSGBOX_MAX_LABEL    32

#define TOGGLE_BUTTON_GAP   4
#define TOGGLE_BORDER_THICK 2.f
#define TOGGLE_MAX_LABEL    32

/* Amber palette (mirrors vendor/amber.h DEFAULT control colors). */
static const Color AMBER_BORDER_NORMAL  = { 0x89, 0x89, 0x88, 0xff };
static const Color AMBER_BASE_NORMAL    = { 0x29, 0x29, 0x29, 0xff };
static const Color AMBER_TEXT_NORMAL    = { 0xd4, 0xd4, 0xd4, 0xff };
static const Color AMBER_BORDER_FOCUSED = { 0xeb, 0x89, 0x1d, 0xff };
static const Color AMBER_BASE_FOCUSED   = { 0x29, 0x29, 0x29, 0xff };
static const Color AMBER_TEXT_FOCUSED   = { 0xff, 0xff, 0xff, 0xff };
static const Color AMBER_BORDER_PRESSED = { 0xf1, 0xcf, 0x9d, 0xff };
static const Color AMBER_BASE_PRESSED   = { 0xf3, 0x93, 0x33, 0xff };
static const Color AMBER_TEXT_PRESSED   = { 0x19, 0x14, 0x10, 0xff };

static bool label_button_draw_colored (Rectangle bounds, const char *label, int font_size, Color rest, Color hover,
                                       bool *out_hover)
{
    bool hovering = CheckCollisionPointRec(GetMousePosition(), bounds);

    font_draw(label, (int)bounds.x, (int)bounds.y, font_size, hovering ? hover : rest);

    if (out_hover) *out_hover = hovering;
    return hovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

bool label_button_draw (Rectangle bounds, const char *label, int font_size, bool *out_hover)
{
    return label_button_draw_colored(bounds, label, font_size, LABEL_BUTTON_REST_COLOR, LABEL_BUTTON_HOVER_COLOR,
                                     out_hover);
}

bool button_draw (Rectangle bounds, const char *label, bool *out_hover)
{
    bool hovering = CheckCollisionPointRec(GetMousePosition(), bounds);

    Color fill = ColorBrightness(BUTTON_BASE_COLOR, hovering ? BUTTON_HOVER_DARKEN : BUTTON_REST_LIGHTEN);
    DrawRectangleRounded(bounds, BUTTON_RADIUS, 0, fill);

    int text_w = font_measure(label, DEFAULT_FONT_SIZE);
    font_draw(label, (int)(bounds.x + (bounds.width - text_w) / 2),
              (int)(bounds.y + (bounds.height - DEFAULT_FONT_SIZE) / 2), DEFAULT_FONT_SIZE, BUTTON_TEXT_COLOR);

    if (out_hover) *out_hover = hovering;
    return hovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

int message_box_draw (Rectangle bounds, const char *title, const char *message, const char *buttons, bool *out_hover)
{
    bool any_hover = false;
    int result     = -1;

    DrawRectangleRec(bounds, MSGBOX_BG_COLOR);
    DrawRectangleLinesEx(bounds, 1, MSGBOX_BORDER_COLOR);

    Rectangle title_bar = { bounds.x, bounds.y, bounds.width, MSGBOX_TITLE_H };
    DrawRectangleRec(title_bar, MSGBOX_TITLE_BG);
    font_draw(title, (int)bounds.x + MSGBOX_PADDING, (int)bounds.y + (MSGBOX_TITLE_H - DEFAULT_FONT_SIZE) / 2,
              DEFAULT_FONT_SIZE, MSGBOX_TITLE_FG);

    Rectangle close_btn = {
        bounds.x + bounds.width - MSGBOX_CLOSE_SIZE - MSGBOX_PADDING / 2,
        bounds.y + (MSGBOX_TITLE_H - MSGBOX_CLOSE_SIZE) / 2,
        MSGBOX_CLOSE_SIZE,
        MSGBOX_CLOSE_SIZE,
    };
    bool close_hov;
    if (label_button_draw_colored(close_btn, "X", DEFAULT_FONT_SIZE, MSGBOX_TITLE_FG, GRAY, &close_hov)) result = 0;
    any_hover |= close_hov;

    int btn_y      = (int)bounds.y + (int)bounds.height - MSGBOX_BUTTON_H - MSGBOX_PADDING;
    int msg_area_y = (int)bounds.y + MSGBOX_TITLE_H;
    int msg_area_h = btn_y - msg_area_y;
    int msg_w      = font_measure(message, DEFAULT_FONT_SIZE);
    font_draw(message, (int)bounds.x + ((int)bounds.width - msg_w) / 2,
              msg_area_y + (msg_area_h - DEFAULT_FONT_SIZE) / 2, DEFAULT_FONT_SIZE, MSGBOX_MSG_FG);

    int btn_count = 1;
    for (const char *p = buttons; *p; p++)
        if (*p == ';') btn_count++;

    int total_btn_w = (int)bounds.width - 2 * MSGBOX_PADDING - MSGBOX_BUTTON_GAP * (btn_count - 1);
    int btn_w       = total_btn_w / btn_count;
    int btn_x       = (int)bounds.x + MSGBOX_PADDING;

    const char *start = buttons;
    for (int i = 0; i < btn_count; i++) {
        const char *end = strchr(start, ';');
        if (!end) end = start + strlen(start);
        int len = (int)(end - start);
        if (len >= MSGBOX_MAX_LABEL) len = MSGBOX_MAX_LABEL - 1;

        char label[MSGBOX_MAX_LABEL];
        memcpy(label, start, len);
        label[len] = '\0';

        Rectangle r = { btn_x, btn_y, btn_w, MSGBOX_BUTTON_H };
        bool hov;
        if (button_draw(r, label, &hov) && result < 0) result = i + 1;
        any_hover |= hov;

        btn_x += btn_w + MSGBOX_BUTTON_GAP;
        start = end + 1;
    }

    if (out_hover) *out_hover = any_hover;
    return result;
}

bool toggle_group_draw (Rectangle bounds, const char *labels, int *active, bool *out_hover)
{
    bool any_hover = false;
    bool changed   = false;

    int btn_count = 1;
    for (const char *p = labels; *p; p++)
        if (*p == ';') btn_count++;

    int btn_w = ((int)bounds.width - TOGGLE_BUTTON_GAP * (btn_count - 1)) / btn_count;
    int btn_x = (int)bounds.x;

    const char *start = labels;
    for (int i = 0; i < btn_count; i++) {
        const char *end = strchr(start, ';');
        if (!end) end = start + strlen(start);
        int len = (int)(end - start);
        if (len >= TOGGLE_MAX_LABEL) len = TOGGLE_MAX_LABEL - 1;

        char label[TOGGLE_MAX_LABEL];
        memcpy(label, start, len);
        label[len] = '\0';

        Rectangle r    = { btn_x, bounds.y, btn_w, bounds.height };
        bool hov       = CheckCollisionPointRec(GetMousePosition(), r);
        bool is_active = (i == *active);

        Color base, border, text;
        if (is_active) {
            base   = AMBER_BASE_PRESSED;
            border = AMBER_BORDER_PRESSED;
            text   = AMBER_TEXT_PRESSED;
        } else if (hov) {
            base   = AMBER_BASE_FOCUSED;
            border = AMBER_BORDER_FOCUSED;
            text   = AMBER_TEXT_FOCUSED;
        } else {
            base   = AMBER_BASE_NORMAL;
            border = AMBER_BORDER_NORMAL;
            text   = AMBER_TEXT_NORMAL;
        }
        DrawRectangleRounded(r, BUTTON_RADIUS, 0, base);
        DrawRectangleRoundedLinesEx(r, BUTTON_RADIUS, 0, TOGGLE_BORDER_THICK, border);

        int text_w = font_measure(label, DEFAULT_FONT_SIZE);
        font_draw(label, (int)(r.x + (r.width - text_w) / 2), (int)(r.y + (r.height - DEFAULT_FONT_SIZE) / 2),
                  DEFAULT_FONT_SIZE, text);

        if (hov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !is_active) {
            *active = i;
            changed = true;
        }

        any_hover |= hov;
        btn_x += btn_w + TOGGLE_BUTTON_GAP;
        start = end + 1;
    }

    if (out_hover) *out_hover = any_hover;
    return changed;
}
