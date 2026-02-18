#include "render.h"

#include <stdio.h>
#include <string.h>

#define ANSI_RESET   "\x1b[0m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_CYAN    "\x1b[36m"

void render_clear_screen(void)
{
    /* Move cursor to home and clear screen */
    printf("\x1b[H\x1b[2J");
}

void render_frame(const Game_t *game)
{
    uint16_t w = game->config.board_width;
    uint16_t h = game->config.board_height;

    /* Move cursor to home position (avoid full clear to reduce flicker) */
    printf("\x1b[H");

    /* Top border */
    printf(ANSI_CYAN "+");
    for (uint16_t x = 0; x < w; x++) printf("-");
    printf("+" ANSI_RESET "\n");

    for (uint16_t y = 0; y < h; y++) {
        printf(ANSI_CYAN "|" ANSI_RESET);
        for (uint16_t x = 0; x < w; x++) {
            Point_t p = { .x = (int16_t)x, .y = (int16_t)y };

            if (p.x == snake_head(&game->snake).x &&
                p.y == snake_head(&game->snake).y) {
                printf(ANSI_GREEN "@" ANSI_RESET);
            } else if (snake_occupies(&game->snake, p)) {
                printf(ANSI_GREEN "o" ANSI_RESET);
            } else if (p.x == game->food.x && p.y == game->food.y) {
                printf(ANSI_RED "*" ANSI_RESET);
            } else {
                printf(" ");
            }
        }
        printf(ANSI_CYAN "|" ANSI_RESET "\n");
    }

    /* Bottom border */
    printf(ANSI_CYAN "+");
    for (uint16_t x = 0; x < w; x++) printf("-");
    printf("+" ANSI_RESET "\n");

    /* Score */
    printf(ANSI_YELLOW " Score: %u" ANSI_RESET "\n", game->score);

    if (game->status == STATE_GAME_OVER) {
        printf("\n GAME OVER! Press 'q' to quit.\n");
    } else {
        printf("\n WASD/Arrows: move | Q: quit\n");
    }
}
