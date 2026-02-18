#include "../../platform.h"
#include "../../core/game.h"
#include "theme.h"
#include <raylib.h>
#include <time.h>

/* Defined in platform_raylib.c */
extern Theme_t *platform_get_theme(void);
extern void platform_render_menu(const Theme_t *theme, int selected);

#define MENU_ITEMS 2

static bool run_menu(Theme_t *theme)
{
    int selected = 0;

    while (!WindowShouldClose()) {
        platform_render_menu(theme, selected);

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            selected = (selected - 1 + MENU_ITEMS) % MENU_ITEMS;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            selected = (selected + 1) % MENU_ITEMS;
        }
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            if (selected == 0) {
                theme->palette_idx = (theme->palette_idx - 1 + PALETTE_COUNT) % PALETTE_COUNT;
            } else {
                theme->style_idx = (theme->style_idx - 1 + STYLE_COUNT) % STYLE_COUNT;
            }
        }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            if (selected == 0) {
                theme->palette_idx = (theme->palette_idx + 1) % PALETTE_COUNT;
            } else {
                theme->style_idx = (theme->style_idx + 1) % STYLE_COUNT;
            }
        }
        if (IsKeyPressed(KEY_ENTER)) {
            return true;
        }
        if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
            return false;
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

        /* Game loop */
        while (game.status == STATE_PLAYING) {
            InputEvent_t input = platform_get_input();

            if (input == INPUT_QUIT) {
                running = false;
                break;
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

        /* Game over: wait for restart, menu, or quit */
        while (running && game.status == STATE_GAME_OVER) {
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
