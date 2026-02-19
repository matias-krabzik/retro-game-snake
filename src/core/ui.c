#include "ui.h"
#include "sprites.h"
#include "theme.h"

/* Layout constants (must match renderer.c) */
#define MAX_VISIBLE          4
#define MAX_VISIBLE_HEADER   3
#define PAUSE_MAX_VIS        3

/* Scroll animation constants */
#define SCROLL_PAUSE_FRAMES  60   /* 1 second at 60 fps */
#define SCROLL_SPEED_FRAMES   6   /* 1 px every 6 frames (~10px/sec) */
#define MENU_TEXT_AREA_W     72   /* content_w(76) - 2*pad(2) */
#define SUBMENU_TEXT_AREA_W  66   /* MENU_TEXT_AREA_W - check_w(6) */

/* --- Helpers --- */

static int item_count(int view)
{
    switch (view) {
    case VIEW_MAIN:     return MAIN_ITEM_COUNT;
    case VIEW_COLORS:   return PALETTE_COUNT;
    case VIEW_CREDITS:  return CREDITS_COUNT;
    default:            return 0;
    }
}

static const char *item_label(int view, int idx)
{
    switch (view) {
    case VIEW_MAIN:     return MAIN_ITEMS[idx];
    case VIEW_COLORS:   return PALETTES[idx].name;
    case VIEW_CREDITS:  return CREDITS_LINES[idx];
    }
    return "";
}

static int visible_text_w(int view)
{
    return (view == VIEW_COLORS) ? SUBMENU_TEXT_AREA_W : MENU_TEXT_AREA_W;
}

static int view_max_visible(int view)
{
    return (view == VIEW_MAIN) ? MAX_VISIBLE_HEADER : MAX_VISIBLE;
}

static void ensure_visible(int *first_visible, int selected, int count,
                           int cap)
{
    int max_vis = count < cap ? count : cap;
    if (selected < *first_visible)
        *first_visible = selected;
    else if (selected >= *first_visible + max_vis)
        *first_visible = selected - max_vis + 1;
}

static void reset_scroll(UiState_t *ui)
{
    ui->menu_scroll_px = 0;
    ui->_scroll_timer = 0;
    ui->_scrolling_fwd = false;
}

static UiResult_t make_result(UiAction_t action, SoundType_t sound)
{
    UiResult_t r;
    r.action = action;
    r.sound = sound;
    return r;
}

#define NO_RESULT  make_result(UI_ACTION_NONE, SND_COUNT)

/* --- Initialization --- */

void ui_init(UiState_t *ui, int palette_idx, int volume, int speed)
{
    ui->state = APP_MENU;
    ui->menu_view = VIEW_MAIN;
    ui->menu_selected = 0;
    ui->menu_first_visible = 0;
    ui->menu_scroll_px = 0;
    ui->_scroll_timer = 0;
    ui->_scrolling_fwd = false;
    ui->pause_selected = 0;
    ui->pause_first_visible = 0;
    ui->palette_idx = palette_idx;
    ui->volume = volume;
    ui->speed = speed;
}

/* --- Sound/Speed submenu handler --- */

static UiResult_t handle_menu_sound_speed(UiState_t *ui, UiInput_t input)
{
    switch (input) {
    case UI_INPUT_LEFT:
        if (ui->menu_view == VIEW_SOUND) {
            if (ui->volume > 0) ui->volume--;
        } else {
            if (ui->speed > 1) ui->speed--;
        }
        return make_result(UI_ACTION_NONE, SND_NAV);

    case UI_INPUT_RIGHT:
        if (ui->menu_view == VIEW_SOUND) {
            if (ui->volume < VOLUME_MAX) ui->volume++;
        } else {
            if (ui->speed < SPEED_MAX) ui->speed++;
        }
        return make_result(UI_ACTION_NONE, SND_NAV);

    case UI_INPUT_CONFIRM:
    case UI_INPUT_BACK: {
        int ret_idx = (ui->menu_view == VIEW_SPEED) ? 2 : 3;
        ui->menu_view = VIEW_MAIN;
        ui->menu_selected = ret_idx;
        ui->menu_first_visible = 0;
        ensure_visible(&ui->menu_first_visible, ui->menu_selected,
                       item_count(VIEW_MAIN), view_max_visible(VIEW_MAIN));
        reset_scroll(ui);
        return make_result(UI_ACTION_NONE, SND_SELECT);
    }

    case UI_INPUT_QUIT:
        ui->state = APP_QUIT;
        return make_result(UI_ACTION_QUIT, SND_COUNT);

    default:
        return NO_RESULT;
    }
}

/* --- List-based menu handler (main, colors, credits) --- */

static UiResult_t handle_menu_list(UiState_t *ui, UiInput_t input)
{
    int view = ui->menu_view;
    int count = item_count(view);

    switch (input) {
    case UI_INPUT_UP:
        ui->menu_selected = (ui->menu_selected - 1 + count) % count;
        ensure_visible(&ui->menu_first_visible, ui->menu_selected,
                       count, view_max_visible(view));
        reset_scroll(ui);
        return make_result(UI_ACTION_NONE, SND_NAV);

    case UI_INPUT_DOWN:
        ui->menu_selected = (ui->menu_selected + 1) % count;
        ensure_visible(&ui->menu_first_visible, ui->menu_selected,
                       count, view_max_visible(view));
        reset_scroll(ui);
        return make_result(UI_ACTION_NONE, SND_NAV);

    case UI_INPUT_CONFIRM:
        if (view == VIEW_MAIN) {
            switch (ui->menu_selected) {
            case 0: /* New game */
                ui->state = APP_PLAYING;
                return make_result(UI_ACTION_START_GAME, SND_SELECT);
            case 1: /* Color */
                ui->menu_view = VIEW_COLORS;
                ui->menu_selected = ui->palette_idx;
                ui->menu_first_visible = 0;
                ensure_visible(&ui->menu_first_visible, ui->menu_selected,
                               item_count(VIEW_COLORS),
                               view_max_visible(VIEW_COLORS));
                reset_scroll(ui);
                return make_result(UI_ACTION_NONE, SND_SELECT);
            case 2: /* Speed */
                ui->menu_view = VIEW_SPEED;
                ui->menu_selected = 0;
                ui->menu_first_visible = 0;
                reset_scroll(ui);
                return make_result(UI_ACTION_NONE, SND_SELECT);
            case 3: /* Sound */
                ui->menu_view = VIEW_SOUND;
                ui->menu_selected = 0;
                ui->menu_first_visible = 0;
                reset_scroll(ui);
                return make_result(UI_ACTION_NONE, SND_SELECT);
            case 4: /* Credits */
                ui->menu_view = VIEW_CREDITS;
                ui->menu_selected = 0;
                ui->menu_first_visible = 0;
                reset_scroll(ui);
                return make_result(UI_ACTION_NONE, SND_SELECT);
            case 5: /* Exit */
                ui->state = APP_QUIT;
                return make_result(UI_ACTION_QUIT, SND_SELECT);
            }
        } else if (view == VIEW_CREDITS) {
            ui->menu_view = VIEW_MAIN;
            ui->menu_selected = 4;
            ui->menu_first_visible = 0;
            ensure_visible(&ui->menu_first_visible, ui->menu_selected,
                           item_count(VIEW_MAIN),
                           view_max_visible(VIEW_MAIN));
            reset_scroll(ui);
            return make_result(UI_ACTION_NONE, SND_SELECT);
        } else if (view == VIEW_COLORS) {
            ui->palette_idx = ui->menu_selected;
            ui->menu_view = VIEW_MAIN;
            ui->menu_selected = 1;
            ui->menu_first_visible = 0;
            ensure_visible(&ui->menu_first_visible, ui->menu_selected,
                           item_count(VIEW_MAIN),
                           view_max_visible(VIEW_MAIN));
            reset_scroll(ui);
            return make_result(UI_ACTION_NONE, SND_SELECT);
        }
        break;

    case UI_INPUT_BACK:
        if (view != VIEW_MAIN) {
            int prev_view = view;
            ui->menu_view = VIEW_MAIN;
            switch (prev_view) {
            case VIEW_COLORS:   ui->menu_selected = 1; break;
            case VIEW_CREDITS:  ui->menu_selected = 4; break;
            default:            ui->menu_selected = 0; break;
            }
            ui->menu_first_visible = 0;
            ensure_visible(&ui->menu_first_visible, ui->menu_selected,
                           item_count(VIEW_MAIN),
                           view_max_visible(VIEW_MAIN));
            reset_scroll(ui);
            return make_result(UI_ACTION_NONE, SND_SELECT);
        } else {
            ui->state = APP_QUIT;
            return make_result(UI_ACTION_QUIT, SND_COUNT);
        }

    case UI_INPUT_QUIT:
        if (view == VIEW_MAIN) {
            ui->state = APP_QUIT;
            return make_result(UI_ACTION_QUIT, SND_COUNT);
        }
        break;

    default:
        break;
    }

    return NO_RESULT;
}

/* --- State dispatch --- */

static UiResult_t handle_menu(UiState_t *ui, UiInput_t input)
{
    if (ui->menu_view == VIEW_SOUND || ui->menu_view == VIEW_SPEED)
        return handle_menu_sound_speed(ui, input);
    return handle_menu_list(ui, input);
}

static UiResult_t handle_paused(UiState_t *ui, UiInput_t input)
{
    switch (input) {
    case UI_INPUT_UP:
        ui->pause_selected = (ui->pause_selected - 1 + PAUSE_ITEM_COUNT)
                             % PAUSE_ITEM_COUNT;
        if (ui->pause_selected < ui->pause_first_visible)
            ui->pause_first_visible = ui->pause_selected;
        else if (ui->pause_selected >= ui->pause_first_visible + PAUSE_MAX_VIS)
            ui->pause_first_visible = ui->pause_selected - PAUSE_MAX_VIS + 1;
        return make_result(UI_ACTION_NONE, SND_NAV);

    case UI_INPUT_DOWN:
        ui->pause_selected = (ui->pause_selected + 1) % PAUSE_ITEM_COUNT;
        if (ui->pause_selected < ui->pause_first_visible)
            ui->pause_first_visible = ui->pause_selected;
        else if (ui->pause_selected >= ui->pause_first_visible + PAUSE_MAX_VIS)
            ui->pause_first_visible = ui->pause_selected - PAUSE_MAX_VIS + 1;
        return make_result(UI_ACTION_NONE, SND_NAV);

    case UI_INPUT_CONFIRM:
        switch (ui->pause_selected) {
        case 0: /* Resume */
            ui->state = APP_PLAYING;
            return make_result(UI_ACTION_RESUME, SND_SELECT);
        case 1: /* Restart */
            ui->state = APP_PLAYING;
            return make_result(UI_ACTION_RESTART, SND_SELECT);
        case 2: /* Menu */
            ui->state = APP_MENU;
            ui->menu_view = VIEW_MAIN;
            ui->menu_selected = 0;
            ui->menu_first_visible = 0;
            reset_scroll(ui);
            return make_result(UI_ACTION_GOTO_MENU, SND_SELECT);
        case 3: /* Exit */
            ui->state = APP_QUIT;
            return make_result(UI_ACTION_QUIT, SND_SELECT);
        }
        break;

    case UI_INPUT_BACK:
        ui->state = APP_PLAYING;
        return make_result(UI_ACTION_RESUME, SND_COUNT);

    case UI_INPUT_QUIT:
        ui->state = APP_QUIT;
        return make_result(UI_ACTION_QUIT, SND_COUNT);

    default:
        break;
    }

    return NO_RESULT;
}

static UiResult_t handle_game_over(UiState_t *ui, UiInput_t input)
{
    switch (input) {
    case UI_INPUT_CONFIRM:
        ui->state = APP_PLAYING;
        return make_result(UI_ACTION_RESTART, SND_COUNT);

    case UI_INPUT_BACK:
        ui->state = APP_MENU;
        ui->menu_view = VIEW_MAIN;
        ui->menu_selected = 0;
        ui->menu_first_visible = 0;
        reset_scroll(ui);
        return make_result(UI_ACTION_GOTO_MENU, SND_COUNT);

    case UI_INPUT_QUIT:
        ui->state = APP_QUIT;
        return make_result(UI_ACTION_QUIT, SND_COUNT);

    default:
        return NO_RESULT;
    }
}

static UiResult_t handle_playing(UiState_t *ui, UiInput_t input)
{
    switch (input) {
    case UI_INPUT_CONFIRM:
        ui_enter_pause(ui);
        return NO_RESULT;

    case UI_INPUT_QUIT:
        ui->state = APP_QUIT;
        return make_result(UI_ACTION_QUIT, SND_COUNT);

    default:
        return NO_RESULT;
    }
}

/* --- Public API --- */

UiResult_t ui_handle_input(UiState_t *ui, UiInput_t input)
{
    if (input == UI_INPUT_NONE)
        return NO_RESULT;

    switch (ui->state) {
    case APP_MENU:      return handle_menu(ui, input);
    case APP_PLAYING:   return handle_playing(ui, input);
    case APP_PAUSED:    return handle_paused(ui, input);
    case APP_GAME_OVER: return handle_game_over(ui, input);
    case APP_QUIT:      return NO_RESULT;
    }
    return NO_RESULT;
}

void ui_tick_scroll(UiState_t *ui)
{
    if (ui->state != APP_MENU)
        return;
    if (ui->menu_view == VIEW_SOUND || ui->menu_view == VIEW_SPEED)
        return;

    int count = item_count(ui->menu_view);
    if (count == 0) return;

    int tw = lcd_text_width(item_label(ui->menu_view, ui->menu_selected));
    int max_w = visible_text_w(ui->menu_view);
    int max_scroll = tw - max_w;

    if (max_scroll <= 0)
        return;

    ui->_scroll_timer++;
    if (!ui->_scrolling_fwd) {
        if (ui->_scroll_timer >= SCROLL_PAUSE_FRAMES) {
            ui->_scrolling_fwd = true;
            ui->_scroll_timer = 0;
        }
    } else if (ui->menu_scroll_px < max_scroll) {
        if (ui->_scroll_timer >= SCROLL_SPEED_FRAMES) {
            ui->menu_scroll_px++;
            ui->_scroll_timer = 0;
        }
    } else {
        if (ui->_scroll_timer >= SCROLL_PAUSE_FRAMES) {
            ui->menu_scroll_px = 0;
            ui->_scroll_timer = 0;
            ui->_scrolling_fwd = false;
        }
    }
}

void ui_enter_pause(UiState_t *ui)
{
    ui->state = APP_PAUSED;
    ui->pause_selected = 0;
    ui->pause_first_visible = 0;
}

void ui_enter_game_over(UiState_t *ui)
{
    ui->state = APP_GAME_OVER;
}
