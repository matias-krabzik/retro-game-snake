#include "../../platform.h"
#include "../../core/game.h"
#include "theme.h"
#include "sprites.h"
#include "audio.h"
#include <raylib.h>
#include <time.h>

/* Defined in platform_raylib.c */
extern Theme_t *platform_get_theme(void);
extern void platform_render_menu(int view, int selected, int first_visible,
                                 int scroll_px, const Theme_t *theme);
extern void platform_render_pause_menu(const Game_t *game, int selected,
                                       int first_visible);
extern int platform_speed_up(void);
extern int platform_speed_down(void);
extern uint32_t platform_get_tick_ms(void);

/* Menu views */
#define VIEW_MAIN     0
#define VIEW_COLORS   1
#define VIEW_CREDITS  2
#define VIEW_SOUND    3
#define VIEW_SPEED    4

/* Layout */
#define MAX_VISIBLE          4    /* items that fit on LCD (no header) */
#define MAX_VISIBLE_HEADER   3    /* items that fit with header bar */

/* Scroll constants */
#define SCROLL_PAUSE_FRAMES  60   /* 1 second at 60 fps */
#define SCROLL_SPEED_FRAMES   6   /* 1 px every 6 frames (~10px/sec) */
#define MENU_TEXT_AREA_W     72   /* content_w(76) - 2*pad(2) */
#define SUBMENU_TEXT_AREA_W  66   /* MENU_TEXT_AREA_W - check_w(6) */

static const char *CREDITS_LINES[] = {
    "Retro Snake",
    "by Matias Krabzik",
    "A love letter to",
    "the Nokia 3310",
    "C99 + Raylib",
};
#define CREDITS_COUNT  5

static int item_count(int view)
{
    switch (view) {
    case VIEW_MAIN:     return 6;
    case VIEW_COLORS:   return PALETTE_COUNT;
    case VIEW_CREDITS:  return CREDITS_COUNT;
    case VIEW_SOUND:    return 0;
    case VIEW_SPEED:    return 0;
    }
    return 0;
}

static const char *item_label(int view, int idx)
{
    switch (view) {
    case VIEW_MAIN: {
        static const char *labels[] = {
            "New game", "Color", "Speed", "Sound", "Credits", "Exit"
        };
        return labels[idx];
    }
    case VIEW_COLORS:   return PALETTES[idx].name;
    case VIEW_CREDITS:  return CREDITS_LINES[idx];
    }
    return "";
}

static int visible_text_w(int view)
{
    return (view == VIEW_COLORS)
        ? SUBMENU_TEXT_AREA_W : MENU_TEXT_AREA_W;
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

static bool run_menu(Theme_t *theme)
{
    int view = VIEW_MAIN;
    int selected = 0;
    int first_visible = 0;
    int scroll_px = 0;
    int scroll_timer = 0;
    bool scrolling_fwd = false;

    while (!WindowShouldClose()) {
        /* Render */
        platform_render_menu(view, selected, first_visible, scroll_px, theme);

        /* Scroll animation for highlighted item (not used in sound view) */
        if (view != VIEW_SOUND && view != VIEW_SPEED) {
            int tw = lcd_text_width(item_label(view, selected));
            int max_w = visible_text_w(view);
            int max_scroll = tw - max_w;

            if (max_scroll > 0) {
                scroll_timer++;
                if (!scrolling_fwd) {
                    /* Pausing at start */
                    if (scroll_timer >= SCROLL_PAUSE_FRAMES) {
                        scrolling_fwd = true;
                        scroll_timer = 0;
                    }
                } else if (scroll_px < max_scroll) {
                    /* Scrolling forward */
                    if (scroll_timer >= SCROLL_SPEED_FRAMES) {
                        scroll_px++;
                        scroll_timer = 0;
                    }
                } else {
                    /* Pausing at end, then reset */
                    if (scroll_timer >= SCROLL_PAUSE_FRAMES) {
                        scroll_px = 0;
                        scroll_timer = 0;
                        scrolling_fwd = false;
                    }
                }
            }
        }

        /* Input */
        int key;
        while ((key = GetKeyPressed()) != 0) {
            /* Sound/Speed view: LEFT/RIGHT adjusts level */
            if (view == VIEW_SOUND || view == VIEW_SPEED) {
                switch (key) {
                case KEY_LEFT: case KEY_A:
                    if (view == VIEW_SOUND) audio_volume_down();
                    else platform_speed_down();
                    audio_play(SND_NAV);
                    break;
                case KEY_RIGHT: case KEY_D:
                    if (view == VIEW_SOUND) audio_volume_up();
                    else platform_speed_up();
                    audio_play(SND_NAV);
                    break;
                case KEY_ENTER: case KEY_KP_ENTER:
                case KEY_ESCAPE: case KEY_BACKSPACE: {
                    int ret_idx = (view == VIEW_SPEED) ? 2 : 3;
                    audio_play(SND_SELECT);
                    view = VIEW_MAIN;
                    selected = ret_idx;
                    first_visible = 0;
                    ensure_visible(&first_visible, selected,
                                   item_count(view),
                                   view_max_visible(view));
                    scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                    break;
                }
                }
                continue;
            }

            int count = item_count(view);

            switch (key) {
            case KEY_UP: case KEY_W:
                selected = (selected - 1 + count) % count;
                ensure_visible(&first_visible, selected, count,
                                view_max_visible(view));
                scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                audio_play(SND_NAV);
                break;

            case KEY_DOWN: case KEY_S:
                selected = (selected + 1) % count;
                ensure_visible(&first_visible, selected, count,
                                view_max_visible(view));
                scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                audio_play(SND_NAV);
                break;

            case KEY_ENTER: case KEY_KP_ENTER:
                audio_play(SND_SELECT);
                if (view == VIEW_MAIN) {
                    switch (selected) {
                    case 0: return true;   /* New game */
                    case 1:
                        view = VIEW_COLORS;
                        selected = theme->palette_idx;
                        first_visible = 0;
                        ensure_visible(&first_visible, selected,
                                       item_count(view),
                                       view_max_visible(view));
                        scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                        break;
                    case 2:
                        view = VIEW_SPEED;
                        selected = 0;
                        first_visible = 0;
                        scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                        break;
                    case 3:
                        view = VIEW_SOUND;
                        selected = 0;
                        first_visible = 0;
                        scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                        break;
                    case 4:
                        view = VIEW_CREDITS;
                        selected = 0;
                        first_visible = 0;
                        scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                        break;
                    case 5: return false;  /* Exit */
                    }
                } else if (view == VIEW_CREDITS) {
                    /* Back to main menu */
                    view = VIEW_MAIN;
                    selected = 4;
                    first_visible = 0;
                    ensure_visible(&first_visible, selected,
                                   item_count(view),
                                   view_max_visible(view));
                    scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                } else {
                    /* Submenu: select option and go back */
                    if (view == VIEW_COLORS) theme->palette_idx = selected;
                    view = VIEW_MAIN;
                    selected = 1;
                    first_visible = 0;
                    ensure_visible(&first_visible, selected,
                                   item_count(view),
                                   view_max_visible(view));
                    scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                }
                break;

            case KEY_ESCAPE: case KEY_BACKSPACE:
                if (view != VIEW_MAIN) {
                    int prev_view = view;
                    view = VIEW_MAIN;
                    switch (prev_view) {
                    case VIEW_COLORS:   selected = 1; break;
                    case VIEW_CREDITS:  selected = 4; break;
                    }
                    first_visible = 0;
                    ensure_visible(&first_visible, selected,
                                   item_count(view),
                                   view_max_visible(view));
                    scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                } else {
                    return false;  /* quit */
                }
                break;

            case KEY_Q:
                if (view == VIEW_MAIN) return false;
                break;
            }
        }
    }
    return false;
}

int main(void)
{
    GameConfig_t config = {
        .board_width = 20,
        .board_height = 9,
        .initial_length = 5,
        .tick_ms = 150
    };

    if (platform_init(&config) != 0) {
        return 1;
    }

    Theme_t *theme = platform_get_theme();
    Game_t game;
    bool running = true;
    bool show_menu = true;

    while (running) {
        if (show_menu) {
            if (!run_menu(theme)) {
                break;
            }
            show_menu = false;
        }

        config.tick_ms = platform_get_tick_ms();
        game_init(&game, config, (uint32_t)time(NULL));
        double tick_accumulator = 0.0;
        double tick_interval = config.tick_ms / 1000.0;

        /* Wait for ENTER release so it doesn't leak into the game loop */
        do {
            platform_render(&game);
        } while (IsKeyDown(KEY_ENTER) || IsKeyDown(KEY_KP_ENTER));

        /* Game loop */
        while (game.status == STATE_PLAYING) {
            InputEvent_t input = platform_get_input();

            if (input == INPUT_QUIT) {
                running = false;
                break;
            }

            /* Pause menu: Resume(0), Restart(1), Menu(2), Exit(3) */
            if (input == INPUT_PAUSE) {
                int psel = 0, pfv = 0;
                bool paused = true;

                while (paused && running) {
                    platform_render_pause_menu(&game, psel, pfv);

                    int key;
                    while ((key = GetKeyPressed()) != 0) {
                        switch (key) {
                        case KEY_UP: case KEY_W:
                            psel = (psel - 1 + 4) % 4;
                            if (psel < pfv) pfv = psel;
                            else if (psel >= pfv + 3) pfv = psel - 2;
                            audio_play(SND_NAV);
                            break;
                        case KEY_DOWN: case KEY_S:
                            psel = (psel + 1) % 4;
                            if (psel < pfv) pfv = psel;
                            else if (psel >= pfv + 3) pfv = psel - 2;
                            audio_play(SND_NAV);
                            break;
                        case KEY_ENTER: case KEY_KP_ENTER:
                            audio_play(SND_SELECT);
                            switch (psel) {
                            case 0: /* Resume */
                                paused = false;
                                break;
                            case 1: /* Restart */
                                paused = false;
                                game_init(&game, config, (uint32_t)time(NULL));
                                tick_accumulator = 0.0;
                                break;
                            case 2: /* Menu */
                                paused = false;
                                show_menu = true;
                                game.status = STATE_GAME_OVER;
                                break;
                            case 3: /* Exit */
                                running = false;
                                break;
                            }
                            break;
                        case KEY_ESCAPE:
                            paused = false; /* same as Resume */
                            break;
                        }
                    }
                }
                if (!running || show_menu) break;
                /* Flush ENTER so it doesn't re-trigger pause */
                do {
                    platform_render(&game);
                } while (IsKeyDown(KEY_ENTER) || IsKeyDown(KEY_KP_ENTER));
                continue;
            }

            game_handle_input(&game, input);

            tick_accumulator += 1.0 / 60.0;
            if (tick_accumulator >= tick_interval) {
                game_update(&game);
                tick_accumulator -= tick_interval;

                if (game.evt_ate_bonus)
                    audio_play(SND_BONUS);
                else if (game.evt_ate_food)
                    audio_play(SND_EAT);
                if (game.status == STATE_GAME_OVER)
                    audio_play(SND_GAME_OVER);
            }

            platform_render(&game);
            platform_sleep_ms(config.tick_ms);
        }

        /* Game over: snake blinks, wait for restart, menu, or quit */
        while (running && !show_menu && game.status == STATE_GAME_OVER) {
            InputEvent_t input = platform_get_input();
            if (input == INPUT_QUIT) {
                running = false;
            } else if (input == INPUT_RESTART || input == INPUT_PAUSE) {
                break;
            } else if (IsKeyPressed(KEY_M)) {
                show_menu = true;
                break;
            }
            platform_render(&game);
            platform_sleep_ms(0);
        }
    }

    platform_shutdown();
    return 0;
}
