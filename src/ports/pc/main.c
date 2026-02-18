#include "../../platform.h"
#include "../../core/game.h"

#include <time.h>

int main(void)
{
    GameConfig_t config = {
        .board_width = 30,
        .board_height = 15,
        .initial_length = 3,
        .tick_ms = 150
    };

    Game_t game;
    game_init(&game, config, (uint32_t)time(NULL));

    if (platform_init(&config) != 0) {
        return 1;
    }

    while (game.status == STATE_PLAYING) {
        InputEvent_t input = platform_get_input();

        if (input == INPUT_QUIT) {
            break;
        }

        game_handle_input(&game, input);
        game_update(&game);
        platform_render(&game);
        platform_sleep_ms(config.tick_ms);
    }

    /* Show game over screen and wait for quit */
    if (game.status == STATE_GAME_OVER) {
        platform_render(&game);
        InputEvent_t input;
        do {
            input = platform_get_input();
            platform_sleep_ms(50);
        } while (input != INPUT_QUIT);
    }

    platform_shutdown();
    return 0;
}
