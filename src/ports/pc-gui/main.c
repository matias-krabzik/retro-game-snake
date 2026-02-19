#include "../../platform.h"
#include "../../core/game.h"
#include "../../core/renderer.h"
#include "../../core/ui.h"
#include "../../core/lcd.h"
#include "../../core/theme.h"
#include <time.h>

int main(void)
{
    if (platform_init() != 0)
        return 1;

    GameConfig_t config = {
        .board_width = 20,
        .board_height = 9,
        .initial_length = 5,
        .tick_ms = 150
    };

    Game_t game;
    UiState_t ui;
    ui_init(&ui, PALETTE_GREEN, VOLUME_DEFAULT, SPEED_DEFAULT);

    double last_time = platform_get_time();
    double tick_acc = 0.0;

    while (ui.state != APP_QUIT && !platform_should_close()) {
        UiInput_t input = platform_get_input();
        UiResult_t result = ui_handle_input(&ui, input);

        /* Play UI sound */
        if (result.sound != SND_COUNT)
            platform_play_sound(result.sound);

        /* Handle actions */
        switch (result.action) {
        case UI_ACTION_START_GAME:
        case UI_ACTION_RESTART:
            config.tick_ms = SPEED_TICK_MS[ui.speed - 1];
            game_init(&game, config, (uint32_t)time(NULL));
            tick_acc = 0.0;
            last_time = platform_get_time();
            break;
        default:
            break;
        }

        /* Apply volume setting */
        platform_set_volume(ui.volume);

        /* Game logic */
        if (ui.state == APP_PLAYING) {
            /* Route directional input to game engine */
            InputEvent_t gi = ui_input_to_game(input);
            if (gi != INPUT_NONE)
                game_handle_input(&game, gi);

            /* Accumulate time and tick */
            double now = platform_get_time();
            tick_acc += now - last_time;
            last_time = now;

            double tick_s = config.tick_ms / 1000.0;
            if (tick_acc >= tick_s) {
                game_update(&game);
                tick_acc -= tick_s;

                /* Game audio events */
                if (game.evt_ate_bonus)
                    platform_play_sound(SND_BONUS);
                else if (game.evt_ate_food)
                    platform_play_sound(SND_EAT);
                if (game.status == STATE_GAME_OVER) {
                    platform_play_sound(SND_GAME_OVER);
                    ui_enter_game_over(&ui);
                }
            }
        } else {
            /* Reset timing when not playing */
            last_time = platform_get_time();
        }

        /* Render */
        switch (ui.state) {
        case APP_MENU: {
            ui_tick_scroll(&ui);
            int level = 0;
            if (ui.menu_view == VIEW_SOUND)
                level = ui.volume;
            else if (ui.menu_view == VIEW_SPEED)
                level = ui.speed;
            render_menu(ui.menu_view, ui.menu_selected,
                        ui.menu_first_visible, ui.menu_scroll_px,
                        ui.palette_idx, level);
            break;
        }
        case APP_PLAYING:
            lcd_clear();
            render_game_frame(&game, true);
            break;
        case APP_GAME_OVER: {
            lcd_clear();
            bool show = ((int)(platform_get_time() * 2) % 2) == 0;
            render_game_frame(&game, show);
            break;
        }
        case APP_PAUSED:
            render_pause_menu(ui.pause_selected, ui.pause_first_visible);
            break;
        case APP_QUIT:
            break;
        }

        /* Present framebuffer */
        if (ui.state != APP_QUIT) {
            platform_present(lcd_get_framebuffer(),
                             &PALETTES[ui.palette_idx].colors);
        }

        platform_sleep_ms(16);
    }

    platform_shutdown();
    return 0;
}
