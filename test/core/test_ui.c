#include <stdio.h>
#include <string.h>
#include "../../src/core/ui.h"
#include "../../src/core/theme.h"

/* ---- Minimal test harness ---- */

static int tests_run, tests_passed;

#define RUN(fn)  do { \
    printf("  %-46s", #fn); \
    fn(); \
    printf("OK\n"); \
    tests_passed++; tests_run++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL  %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        tests_run++; return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("FAIL  %s:%d: %d != %d\n", __FILE__, __LINE__, (int)(a), (int)(b)); \
        tests_run++; return; \
    } \
} while(0)

/* Helper: create default UI state */
static UiState_t make_ui(void)
{
    UiState_t ui;
    ui_init(&ui, PALETTE_GREEN, VOLUME_DEFAULT, SPEED_DEFAULT);
    return ui;
}

/* ---- Initialization ---- */

static void test_init_state(void)
{
    UiState_t ui = make_ui();
    ASSERT_EQ(ui.state, APP_MENU);
    ASSERT_EQ(ui.menu_view, VIEW_MAIN);
    ASSERT_EQ(ui.menu_selected, 0);
    ASSERT_EQ(ui.menu_first_visible, 0);
    ASSERT_EQ(ui.palette_idx, PALETTE_GREEN);
    ASSERT_EQ(ui.volume, VOLUME_DEFAULT);
    ASSERT_EQ(ui.speed, SPEED_DEFAULT);
}

/* ---- Menu: navigation ---- */

static void test_menu_nav_down(void)
{
    UiState_t ui = make_ui();
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_DOWN);
    ASSERT_EQ(ui.menu_selected, 1);
    ASSERT_EQ(r.sound, SND_NAV);
    ASSERT_EQ(r.action, UI_ACTION_NONE);
}

static void test_menu_nav_up_wraps(void)
{
    UiState_t ui = make_ui();
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_UP);
    /* Wraps to last item (MAIN_ITEM_COUNT - 1 = 5) */
    ASSERT_EQ(ui.menu_selected, MAIN_ITEM_COUNT - 1);
    ASSERT_EQ(r.sound, SND_NAV);
}

static void test_menu_nav_down_wraps(void)
{
    UiState_t ui = make_ui();
    /* Navigate to last item */
    for (int i = 0; i < MAIN_ITEM_COUNT - 1; i++)
        ui_handle_input(&ui, UI_INPUT_DOWN);
    ASSERT_EQ(ui.menu_selected, MAIN_ITEM_COUNT - 1);

    /* One more wraps to 0 */
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ASSERT_EQ(ui.menu_selected, 0);
}

static void test_menu_none_input_noop(void)
{
    UiState_t ui = make_ui();
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_NONE);
    ASSERT_EQ(ui.menu_selected, 0);
    ASSERT_EQ(r.action, UI_ACTION_NONE);
    ASSERT_EQ(r.sound, SND_COUNT);
}

/* ---- Menu: selections ---- */

static void test_menu_start_game(void)
{
    UiState_t ui = make_ui();
    /* Item 0 = "New game" */
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(r.action, UI_ACTION_START_GAME);
    ASSERT_EQ(r.sound, SND_SELECT);
    ASSERT_EQ(ui.state, APP_PLAYING);
}

static void test_menu_enter_colors(void)
{
    UiState_t ui = make_ui();
    /* Navigate to item 1 (Color) */
    ui_handle_input(&ui, UI_INPUT_DOWN);
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_COLORS);
    ASSERT_EQ(ui.menu_selected, ui.palette_idx);
    ASSERT_EQ(r.sound, SND_SELECT);
    ASSERT_EQ(ui.state, APP_MENU);
}

static void test_menu_select_color(void)
{
    UiState_t ui = make_ui();
    /* Enter colors submenu */
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_COLORS);

    /* Navigate to Grey (index 1) and select */
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ASSERT_EQ(ui.menu_selected, 1);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);

    /* Should return to main with palette updated */
    ASSERT_EQ(ui.palette_idx, PALETTE_GREY);
    ASSERT_EQ(ui.menu_view, VIEW_MAIN);
    ASSERT_EQ(ui.menu_selected, 1); /* returns to Color item */
}

static void test_menu_enter_speed(void)
{
    UiState_t ui = make_ui();
    /* Navigate to item 2 (Speed) */
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_SPEED);
}

static void test_menu_enter_sound(void)
{
    UiState_t ui = make_ui();
    /* Navigate to item 3 (Sound) */
    for (int i = 0; i < 3; i++) ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_SOUND);
}

static void test_menu_enter_credits(void)
{
    UiState_t ui = make_ui();
    /* Navigate to item 4 (Credits) */
    for (int i = 0; i < 4; i++) ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_CREDITS);
}

static void test_menu_exit(void)
{
    UiState_t ui = make_ui();
    /* Navigate to item 5 (Exit) */
    for (int i = 0; i < 5; i++) ui_handle_input(&ui, UI_INPUT_DOWN);
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(r.action, UI_ACTION_QUIT);
    ASSERT_EQ(ui.state, APP_QUIT);
}

static void test_menu_back_from_main_quits(void)
{
    UiState_t ui = make_ui();
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_BACK);
    ASSERT_EQ(r.action, UI_ACTION_QUIT);
    ASSERT_EQ(ui.state, APP_QUIT);
}

static void test_menu_back_from_submenu(void)
{
    UiState_t ui = make_ui();
    /* Enter colors */
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_COLORS);

    /* Back returns to main, selected on Color item */
    ui_handle_input(&ui, UI_INPUT_BACK);
    ASSERT_EQ(ui.menu_view, VIEW_MAIN);
    ASSERT_EQ(ui.menu_selected, 1);
}

static void test_menu_back_from_credits(void)
{
    UiState_t ui = make_ui();
    /* Enter credits */
    for (int i = 0; i < 4; i++) ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_CREDITS);

    /* Back returns to main, selected on Credits item */
    ui_handle_input(&ui, UI_INPUT_BACK);
    ASSERT_EQ(ui.menu_view, VIEW_MAIN);
    ASSERT_EQ(ui.menu_selected, 4);
}

/* ---- Sound/Speed views ---- */

static void test_sound_volume_up(void)
{
    UiState_t ui = make_ui();
    ui.volume = 3;
    /* Enter sound view */
    for (int i = 0; i < 3; i++) ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_SOUND);

    UiResult_t r = ui_handle_input(&ui, UI_INPUT_RIGHT);
    ASSERT_EQ(ui.volume, 4);
    ASSERT_EQ(r.sound, SND_NAV);
}

static void test_sound_volume_down(void)
{
    UiState_t ui = make_ui();
    ui.volume = 3;
    for (int i = 0; i < 3; i++) ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);

    ui_handle_input(&ui, UI_INPUT_LEFT);
    ASSERT_EQ(ui.volume, 2);
}

static void test_sound_volume_clamps_max(void)
{
    UiState_t ui = make_ui();
    ui.volume = VOLUME_MAX;
    for (int i = 0; i < 3; i++) ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);

    ui_handle_input(&ui, UI_INPUT_RIGHT);
    ASSERT_EQ(ui.volume, VOLUME_MAX);
}

static void test_sound_volume_clamps_min(void)
{
    UiState_t ui = make_ui();
    ui.volume = 0;
    for (int i = 0; i < 3; i++) ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);

    ui_handle_input(&ui, UI_INPUT_LEFT);
    ASSERT_EQ(ui.volume, 0);
}

static void test_speed_up(void)
{
    UiState_t ui = make_ui();
    ui.speed = 3;
    /* Enter speed view (item 2) */
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_SPEED);

    ui_handle_input(&ui, UI_INPUT_RIGHT);
    ASSERT_EQ(ui.speed, 4);
}

static void test_speed_down(void)
{
    UiState_t ui = make_ui();
    ui.speed = 3;
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);

    ui_handle_input(&ui, UI_INPUT_LEFT);
    ASSERT_EQ(ui.speed, 2);
}

static void test_speed_clamps(void)
{
    UiState_t ui = make_ui();
    ui.speed = 1;
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);

    ui_handle_input(&ui, UI_INPUT_LEFT);
    ASSERT_EQ(ui.speed, 1);
}

static void test_sound_back_returns_to_main(void)
{
    UiState_t ui = make_ui();
    for (int i = 0; i < 3; i++) ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_SOUND);

    UiResult_t r = ui_handle_input(&ui, UI_INPUT_BACK);
    ASSERT_EQ(ui.menu_view, VIEW_MAIN);
    ASSERT_EQ(ui.menu_selected, 3); /* Sound item */
    ASSERT_EQ(r.sound, SND_SELECT);
}

static void test_speed_confirm_returns_to_main(void)
{
    UiState_t ui = make_ui();
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_SPEED);

    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.menu_view, VIEW_MAIN);
    ASSERT_EQ(ui.menu_selected, 2); /* Speed item */
}

/* ---- Pause menu ---- */

static void test_pause_init(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    ui_enter_pause(&ui);
    ASSERT_EQ(ui.state, APP_PAUSED);
    ASSERT_EQ(ui.pause_selected, 0);
    ASSERT_EQ(ui.pause_first_visible, 0);
}

static void test_pause_nav(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    ui_enter_pause(&ui);

    UiResult_t r = ui_handle_input(&ui, UI_INPUT_DOWN);
    ASSERT_EQ(ui.pause_selected, 1);
    ASSERT_EQ(r.sound, SND_NAV);

    ui_handle_input(&ui, UI_INPUT_UP);
    ASSERT_EQ(ui.pause_selected, 0);
}

static void test_pause_nav_wraps(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    ui_enter_pause(&ui);

    ui_handle_input(&ui, UI_INPUT_UP);
    ASSERT_EQ(ui.pause_selected, PAUSE_ITEM_COUNT - 1);
}

static void test_pause_resume(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    ui_enter_pause(&ui);

    /* Item 0 = Resume */
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(r.action, UI_ACTION_RESUME);
    ASSERT_EQ(ui.state, APP_PLAYING);
}

static void test_pause_restart(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    ui_enter_pause(&ui);

    /* Item 1 = Restart */
    ui_handle_input(&ui, UI_INPUT_DOWN);
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(r.action, UI_ACTION_RESTART);
    ASSERT_EQ(ui.state, APP_PLAYING);
}

static void test_pause_goto_menu(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    ui_enter_pause(&ui);

    /* Item 2 = Menu */
    ui_handle_input(&ui, UI_INPUT_DOWN);
    ui_handle_input(&ui, UI_INPUT_DOWN);
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(r.action, UI_ACTION_GOTO_MENU);
    ASSERT_EQ(ui.state, APP_MENU);
    ASSERT_EQ(ui.menu_view, VIEW_MAIN);
}

static void test_pause_exit(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    ui_enter_pause(&ui);

    /* Item 3 = Exit */
    for (int i = 0; i < 3; i++) ui_handle_input(&ui, UI_INPUT_DOWN);
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(r.action, UI_ACTION_QUIT);
    ASSERT_EQ(ui.state, APP_QUIT);
}

static void test_pause_back_resumes(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    ui_enter_pause(&ui);

    UiResult_t r = ui_handle_input(&ui, UI_INPUT_BACK);
    ASSERT_EQ(r.action, UI_ACTION_RESUME);
    ASSERT_EQ(ui.state, APP_PLAYING);
}

/* ---- Playing state ---- */

static void test_playing_confirm_pauses(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(ui.state, APP_PAUSED);
}

static void test_playing_quit(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_QUIT);
    ASSERT_EQ(r.action, UI_ACTION_QUIT);
    ASSERT_EQ(ui.state, APP_QUIT);
}

static void test_playing_directional_noop(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    UiResult_t r = ui_handle_input(&ui, UI_INPUT_UP);
    ASSERT_EQ(r.action, UI_ACTION_NONE);
    ASSERT_EQ(ui.state, APP_PLAYING);
}

/* ---- Game over state ---- */

static void test_game_over_confirm_restarts(void)
{
    UiState_t ui = make_ui();
    ui_enter_game_over(&ui);
    ASSERT_EQ(ui.state, APP_GAME_OVER);

    UiResult_t r = ui_handle_input(&ui, UI_INPUT_CONFIRM);
    ASSERT_EQ(r.action, UI_ACTION_RESTART);
    ASSERT_EQ(ui.state, APP_PLAYING);
}

static void test_game_over_back_goes_to_menu(void)
{
    UiState_t ui = make_ui();
    ui_enter_game_over(&ui);

    UiResult_t r = ui_handle_input(&ui, UI_INPUT_BACK);
    ASSERT_EQ(r.action, UI_ACTION_GOTO_MENU);
    ASSERT_EQ(ui.state, APP_MENU);
}

static void test_game_over_quit(void)
{
    UiState_t ui = make_ui();
    ui_enter_game_over(&ui);

    UiResult_t r = ui_handle_input(&ui, UI_INPUT_QUIT);
    ASSERT_EQ(r.action, UI_ACTION_QUIT);
    ASSERT_EQ(ui.state, APP_QUIT);
}

/* ---- Scroll animation ---- */

static void test_scroll_noop_when_playing(void)
{
    UiState_t ui = make_ui();
    ui.state = APP_PLAYING;
    ui.menu_scroll_px = 0;
    ui_tick_scroll(&ui);
    ASSERT_EQ(ui.menu_scroll_px, 0);
}

static void test_scroll_noop_in_sound_view(void)
{
    UiState_t ui = make_ui();
    ui.menu_view = VIEW_SOUND;
    ui_tick_scroll(&ui);
    ASSERT_EQ(ui.menu_scroll_px, 0);
}

/* ---- main ---- */

int main(void)
{
    printf("[test_ui]\n");

    /* Initialization */
    RUN(test_init_state);

    /* Menu navigation */
    RUN(test_menu_nav_down);
    RUN(test_menu_nav_up_wraps);
    RUN(test_menu_nav_down_wraps);
    RUN(test_menu_none_input_noop);

    /* Menu selections */
    RUN(test_menu_start_game);
    RUN(test_menu_enter_colors);
    RUN(test_menu_select_color);
    RUN(test_menu_enter_speed);
    RUN(test_menu_enter_sound);
    RUN(test_menu_enter_credits);
    RUN(test_menu_exit);
    RUN(test_menu_back_from_main_quits);
    RUN(test_menu_back_from_submenu);
    RUN(test_menu_back_from_credits);

    /* Sound/Speed views */
    RUN(test_sound_volume_up);
    RUN(test_sound_volume_down);
    RUN(test_sound_volume_clamps_max);
    RUN(test_sound_volume_clamps_min);
    RUN(test_speed_up);
    RUN(test_speed_down);
    RUN(test_speed_clamps);
    RUN(test_sound_back_returns_to_main);
    RUN(test_speed_confirm_returns_to_main);

    /* Pause menu */
    RUN(test_pause_init);
    RUN(test_pause_nav);
    RUN(test_pause_nav_wraps);
    RUN(test_pause_resume);
    RUN(test_pause_restart);
    RUN(test_pause_goto_menu);
    RUN(test_pause_exit);
    RUN(test_pause_back_resumes);

    /* Playing state */
    RUN(test_playing_confirm_pauses);
    RUN(test_playing_quit);
    RUN(test_playing_directional_noop);

    /* Game over state */
    RUN(test_game_over_confirm_restarts);
    RUN(test_game_over_back_goes_to_menu);
    RUN(test_game_over_quit);

    /* Scroll animation */
    RUN(test_scroll_noop_when_playing);
    RUN(test_scroll_noop_in_sound_view);

    printf("  %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
