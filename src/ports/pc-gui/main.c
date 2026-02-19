#include "../../platform.h"
#include "../../core/game.h"
#include "theme.h"
#include "sprites.h"
#include <raylib.h>
#include <time.h>

/* Defined in platform_raylib.c */
extern Theme_t *platform_get_theme(void);
extern void platform_render_menu(int view, int selected, int first_visible,
                                 int scroll_px, const Theme_t *theme);
extern void platform_render_paused(const Game_t *game);

/* Menu views */
#define VIEW_MAIN     0
#define VIEW_COLORS   1
#define VIEW_VERSIONS 2
#define VIEW_CREDITS  3

/* Layout */
#define MAX_VISIBLE          4    /* items that fit on LCD */

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
    case VIEW_MAIN:     return 5;
    case VIEW_COLORS:   return PALETTE_COUNT;
    case VIEW_VERSIONS: return STYLE_COUNT;
    case VIEW_CREDITS:  return CREDITS_COUNT;
    }
    return 0;
}

static const char *item_label(int view, int idx)
{
    switch (view) {
    case VIEW_MAIN: {
        static const char *labels[] = {
            "New game", "Color", "Version", "Credits", "Exit"
        };
        return labels[idx];
    }
    case VIEW_COLORS:   return PALETTES[idx].name;
    case VIEW_VERSIONS: return STYLE_NAMES[idx];
    case VIEW_CREDITS:  return CREDITS_LINES[idx];
    }
    return "";
}

static int visible_text_w(int view)
{
    return (view == VIEW_COLORS || view == VIEW_VERSIONS)
        ? SUBMENU_TEXT_AREA_W : MENU_TEXT_AREA_W;
}

static void ensure_visible(int *first_visible, int selected, int count)
{
    int max_vis = count < MAX_VISIBLE ? count : MAX_VISIBLE;
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

        /* Scroll animation for highlighted item */
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

        /* Input */
        int key;
        while ((key = GetKeyPressed()) != 0) {
            int count = item_count(view);

            switch (key) {
            case KEY_UP: case KEY_W:
                selected = (selected - 1 + count) % count;
                ensure_visible(&first_visible, selected, count);
                scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                break;

            case KEY_DOWN: case KEY_S:
                selected = (selected + 1) % count;
                ensure_visible(&first_visible, selected, count);
                scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                break;

            case KEY_ENTER: case KEY_KP_ENTER:
                if (view == VIEW_MAIN) {
                    switch (selected) {
                    case 0: return true;   /* New game */
                    case 1:
                        view = VIEW_COLORS;
                        selected = theme->palette_idx;
                        first_visible = 0;
                        ensure_visible(&first_visible, selected,
                                       item_count(view));
                        scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                        break;
                    case 2:
                        view = VIEW_VERSIONS;
                        selected = theme->style_idx;
                        first_visible = 0;
                        ensure_visible(&first_visible, selected,
                                       item_count(view));
                        scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                        break;
                    case 3:
                        view = VIEW_CREDITS;
                        selected = 0;
                        first_visible = 0;
                        scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                        break;
                    case 4: return false;  /* Exit */
                    }
                } else if (view == VIEW_CREDITS) {
                    /* Back to main menu */
                    view = VIEW_MAIN;
                    selected = 3;
                    first_visible = 0;
                    ensure_visible(&first_visible, selected,
                                   item_count(view));
                    scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                } else {
                    /* Submenu: select option and go back */
                    int prev_view = view;
                    if (view == VIEW_COLORS)   theme->palette_idx = selected;
                    if (view == VIEW_VERSIONS) theme->style_idx = selected;
                    view = VIEW_MAIN;
                    selected = (prev_view == VIEW_COLORS) ? 1 : 2;
                    first_visible = 0;
                    ensure_visible(&first_visible, selected,
                                   item_count(view));
                    scroll_px = 0; scroll_timer = 0; scrolling_fwd = false;
                }
                break;

            case KEY_ESCAPE: case KEY_BACKSPACE:
                if (view != VIEW_MAIN) {
                    int prev_view = view;
                    view = VIEW_MAIN;
                    switch (prev_view) {
                    case VIEW_COLORS:   selected = 1; break;
                    case VIEW_VERSIONS: selected = 2; break;
                    case VIEW_CREDITS:  selected = 3; break;
                    }
                    first_visible = 0;
                    ensure_visible(&first_visible, selected,
                                   item_count(view));
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

            /* Pause modal */
            if (input == INPUT_PAUSE) {
                bool paused = true;
                while (paused && running) {
                    platform_render_paused(&game);
                    InputEvent_t pi = platform_get_input();
                    if (pi == INPUT_PAUSE) {
                        paused = false;
                    } else if (pi == INPUT_RESTART) {
                        paused = false;
                        game_init(&game, config, (uint32_t)time(NULL));
                        tick_accumulator = 0.0;
                    } else if (pi == INPUT_QUIT) {
                        running = false;
                    } else if (IsKeyPressed(KEY_M)) {
                        paused = false;
                        show_menu = true;
                        game.status = STATE_GAME_OVER; /* exit game loop */
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
            }

            platform_render(&game);
            platform_sleep_ms(config.tick_ms);
        }

        /* Game over: snake blinks, wait for restart, menu, or quit */
        while (running && !show_menu && game.status == STATE_GAME_OVER) {
            InputEvent_t input = platform_get_input();
            if (input == INPUT_QUIT) {
                running = false;
            } else if (input == INPUT_RESTART) {
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
