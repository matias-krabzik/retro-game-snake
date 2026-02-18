#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "snake.h"

typedef enum {
    STATE_PLAYING,
    STATE_GAME_OVER
} GameStatus_t;

typedef struct {
    Snake_t snake;
    Point_t food;
    uint32_t score;
    GameStatus_t status;
    GameConfig_t config;
    uint32_t rng_state;
} Game_t;

void game_init(Game_t *game, GameConfig_t config, uint32_t seed);
void game_update(Game_t *game);
void game_handle_input(Game_t *game, InputEvent_t input);
void game_spawn_food(Game_t *game);

#endif /* GAME_H */
