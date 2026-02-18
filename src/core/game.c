#include "game.h"

/* Simple xorshift32 PRNG — no stdlib dependency */
static uint32_t rng_next(uint32_t *state)
{
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

void game_init(Game_t *game, GameConfig_t config, uint32_t seed)
{
    game->config = config;
    game->score = 0;
    game->status = STATE_PLAYING;
    game->rng_state = seed ? seed : 1;

    Point_t start = {
        .x = (int16_t)(config.board_width / 2),
        .y = (int16_t)(config.board_height / 2)
    };
    snake_init(&game->snake, start, config.initial_length, DIR_RIGHT);

    game_spawn_food(game);
}

void game_spawn_food(Game_t *game)
{
    uint16_t w = game->config.board_width;
    uint16_t h = game->config.board_height;
    Point_t candidate;

    do {
        candidate.x = (int16_t)(rng_next(&game->rng_state) % w);
        candidate.y = (int16_t)(rng_next(&game->rng_state) % h);
    } while (snake_occupies(&game->snake, candidate));

    game->food = candidate;
}

void game_update(Game_t *game)
{
    if (game->status != STATE_PLAYING) {
        return;
    }

    snake_move(&game->snake);

    Point_t head = snake_head(&game->snake);

    /* Wrap around walls */
    if (head.x < 0) head.x = game->config.board_width - 1;
    else if (head.x >= game->config.board_width) head.x = 0;
    if (head.y < 0) head.y = game->config.board_height - 1;
    else if (head.y >= game->config.board_height) head.y = 0;
    game->snake.body[0] = head;

    /* Self collision */
    if (snake_collides_self(&game->snake)) {
        game->status = STATE_GAME_OVER;
        return;
    }

    /* Food consumption */
    if (head.x == game->food.x && head.y == game->food.y) {
        game->snake.fat[0] = true;
        snake_grow(&game->snake);
        game->score += 10;
        game_spawn_food(game);
    }
}

void game_handle_input(Game_t *game, InputEvent_t input)
{
    switch (input) {
    case INPUT_UP:    snake_set_direction(&game->snake, DIR_UP);    break;
    case INPUT_DOWN:  snake_set_direction(&game->snake, DIR_DOWN);  break;
    case INPUT_LEFT:  snake_set_direction(&game->snake, DIR_LEFT);  break;
    case INPUT_RIGHT: snake_set_direction(&game->snake, DIR_RIGHT); break;
    default: break;
    }
}
