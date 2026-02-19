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
    Point_t bonus;           /* bonus position (left cell) — 2 cells wide */
    uint32_t score;
    GameStatus_t status;
    GameConfig_t config;
    uint32_t rng_state;
    uint16_t food_count;     /* normal food eaten (triggers bonus every 5) */
    uint8_t bonus_steps;     /* ticks remaining before bonus disappears */
    uint8_t bonus_sprite;    /* bonus sprite variant (0-5) */
    bool bonus_active;       /* bonus currently on the board */
    bool evt_ate_food;       /* set by game_update when food eaten */
    bool evt_ate_bonus;      /* set by game_update when bonus eaten */
} Game_t;

void game_init(Game_t *game, GameConfig_t config, uint32_t seed);
void game_update(Game_t *game);
void game_handle_input(Game_t *game, InputEvent_t input);
void game_spawn_food(Game_t *game);
void game_spawn_bonus(Game_t *game);

#endif /* GAME_H */
