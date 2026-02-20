#ifndef CORE_UI_H
#define CORE_UI_H

#include "renderer.h"
#include "audio_types.h"
#include "types.h"
#include <stdbool.h>

/* Abstract input events (ports map physical keys to these) */
typedef enum {
    UI_INPUT_NONE,
    UI_INPUT_UP,
    UI_INPUT_DOWN,
    UI_INPUT_LEFT,
    UI_INPUT_RIGHT,
    UI_INPUT_CONFIRM,
    UI_INPUT_BACK,
    UI_INPUT_QUIT,
} UiInput_t;

/* Top-level application states */
typedef enum {
    APP_MENU,
    APP_PLAYING,
    APP_PAUSED,
    APP_GAME_OVER,
    APP_QUIT_CONFIRM,
    APP_QUIT,
} AppState_t;

/* Actions returned to the caller (things the UI can't do itself) */
typedef enum {
    UI_ACTION_NONE,
    UI_ACTION_START_GAME,
    UI_ACTION_RESUME,
    UI_ACTION_RESTART,
    UI_ACTION_GOTO_MENU,
    UI_ACTION_QUIT,
} UiAction_t;

/* Result of processing one input */
typedef struct {
    UiAction_t action;
    SoundType_t sound;      /* SND_COUNT = no sound to play */
} UiResult_t;

/* Full UI state (menu + pause + settings) */
typedef struct {
    AppState_t state;

    /* Menu navigation */
    int menu_view;
    int menu_selected;
    int menu_first_visible;
    int menu_scroll_px;

    /* Pause menu navigation */
    int pause_selected;
    int pause_first_visible;

    /* Quit confirmation */
    int quit_confirm_selected;       /* 0 = No, 1 = Yes */
    AppState_t quit_confirm_return;  /* state to restore on cancel */

    /* Settings (owned by UI so menus can display/modify them) */
    int palette_idx;
    int volume;
    int speed;

    /* Scroll animation internals */
    int _scroll_timer;
    bool _scrolling_fwd;
} UiState_t;

/* Initialize the UI state machine */
void ui_init(UiState_t *ui, int palette_idx, int volume, int speed);

/* Process one input event. Returns action + optional sound. */
UiResult_t ui_handle_input(UiState_t *ui, UiInput_t input);

/* Advance scroll animation by one frame (call once per render frame) */
void ui_tick_scroll(UiState_t *ui);

/* Transition helpers (called by the main loop) */
void ui_enter_pause(UiState_t *ui);
void ui_enter_game_over(UiState_t *ui);

/* Convert UI directional input to game input event */
static inline InputEvent_t ui_input_to_game(UiInput_t input)
{
    switch (input) {
    case UI_INPUT_UP:    return INPUT_UP;
    case UI_INPUT_DOWN:  return INPUT_DOWN;
    case UI_INPUT_LEFT:  return INPUT_LEFT;
    case UI_INPUT_RIGHT: return INPUT_RIGHT;
    default:             return INPUT_NONE;
    }
}

#endif /* CORE_UI_H */
