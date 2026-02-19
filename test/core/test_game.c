#include "../test.h"
#include "core/game.h"

static GameConfig_t cfg = {
    .board_width = 20,
    .board_height = 9,
    .initial_length = 3,
    .tick_ms = 150
};

/* --- game_init --- */

static void test_init_state(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    ASSERT_EQ(g.status, STATE_PLAYING);
    ASSERT_EQ(g.score, 0);
    ASSERT_EQ(g.food_count, 0);
    ASSERT_FALSE(g.bonus_active);
    ASSERT_EQ(g.snake.length, cfg.initial_length);
    ASSERT_EQ(g.snake.direction, DIR_RIGHT);
}

static void test_init_snake_centered(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    ASSERT_EQ(g.snake.body[0].x, cfg.board_width / 2);
    ASSERT_EQ(g.snake.body[0].y, cfg.board_height / 2);
}

static void test_init_food_on_board(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    ASSERT_TRUE(g.food.x >= 0 && g.food.x < cfg.board_width);
    ASSERT_TRUE(g.food.y >= 0 && g.food.y < cfg.board_height);
}

static void test_init_food_not_on_snake(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    ASSERT_FALSE(snake_occupies(&g.snake, g.food));
}

/* --- game_update: movement --- */

static void test_update_moves_snake(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    int16_t old_x = g.snake.body[0].x;
    game_update(&g);
    ASSERT_EQ(g.snake.body[0].x, old_x + 1);
}

/* --- game_update: wrap around --- */

static void test_wrap_right(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    g.snake.body[0].x = cfg.board_width - 1;
    g.snake.direction = DIR_RIGHT;
    game_update(&g);
    ASSERT_EQ(g.snake.body[0].x, 0);
}

static void test_wrap_left(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    g.snake.body[0].x = 0;
    g.snake.direction = DIR_LEFT;
    game_update(&g);
    ASSERT_EQ(g.snake.body[0].x, cfg.board_width - 1);
}

static void test_wrap_down(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    g.snake.body[0].y = cfg.board_height - 1;
    g.snake.direction = DIR_DOWN;
    game_update(&g);
    ASSERT_EQ(g.snake.body[0].y, 0);
}

static void test_wrap_up(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    g.snake.body[0].y = 0;
    g.snake.direction = DIR_UP;
    game_update(&g);
    ASSERT_EQ(g.snake.body[0].y, cfg.board_height - 1);
}

/* --- game_update: self collision --- */

static void test_self_collision_game_over(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    /* Create a loop: length 5, manually curl into itself */
    g.snake.length = 5;
    g.snake.body[0] = (Point_t){5, 5};
    g.snake.body[1] = (Point_t){6, 5};
    g.snake.body[2] = (Point_t){6, 6};
    g.snake.body[3] = (Point_t){5, 6};
    g.snake.body[4] = (Point_t){4, 6};
    g.snake.direction = DIR_DOWN;
    /* After move: head goes to (5,6) which is body[3] */
    game_update(&g);
    ASSERT_EQ(g.status, STATE_GAME_OVER);
}

/* --- game_update: food --- */

static void test_eat_food_scores(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    /* Place food directly ahead */
    g.food = (Point_t){(int16_t)(g.snake.body[0].x + 1), g.snake.body[0].y};
    game_update(&g);
    ASSERT_EQ(g.score, 7);
    ASSERT_EQ(g.food_count, 1);
    ASSERT_TRUE(g.evt_ate_food);
}

static void test_eat_food_sets_fat(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    g.food = (Point_t){(int16_t)(g.snake.body[0].x + 1), g.snake.body[0].y};
    game_update(&g);
    ASSERT_TRUE(g.snake.fat[0]);
}

static void test_eat_food_respawns(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    g.food = (Point_t){(int16_t)(g.snake.body[0].x + 1), g.snake.body[0].y};
    Point_t old_food = g.food;
    game_update(&g);
    /* New food should be somewhere (might be same spot by RNG, but not on snake) */
    ASSERT_FALSE(snake_occupies(&g.snake, g.food));
    (void)old_food;
}

static void test_eat_food_grows_snake(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    uint16_t old_len = g.snake.length;
    g.food = (Point_t){(int16_t)(g.snake.body[0].x + 1), g.snake.body[0].y};
    game_update(&g);
    /* grow is deferred, triggers on next move */
    game_update(&g);
    ASSERT_EQ(g.snake.length, old_len + 1);
}

/* --- game_update: event flags --- */

static void test_event_flags_cleared_each_tick(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    /* Eat food to set flag */
    g.food = (Point_t){(int16_t)(g.snake.body[0].x + 1), g.snake.body[0].y};
    game_update(&g);
    ASSERT_TRUE(g.evt_ate_food);

    /* Next tick should clear it */
    game_update(&g);
    ASSERT_FALSE(g.evt_ate_food);
    ASSERT_FALSE(g.evt_ate_bonus);
}

/* --- game_update: bonus --- */

static void test_bonus_spawns_every_5_foods(void)
{
    Game_t g;
    game_init(&g, cfg, 1);
    g.food_count = 4;
    /* Eat the 5th food */
    g.food = (Point_t){(int16_t)(g.snake.body[0].x + 1), g.snake.body[0].y};
    game_update(&g);
    ASSERT_EQ(g.food_count, 5);
    ASSERT_TRUE(g.bonus_active);
    ASSERT_EQ(g.bonus_steps, 20);
}

static void test_bonus_countdown(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    g.bonus_active = true;
    g.bonus_steps = 2;
    g.bonus = (Point_t){0, 0};  /* far from snake */

    game_update(&g);
    ASSERT_EQ(g.bonus_steps, 1);
    ASSERT_TRUE(g.bonus_active);

    game_update(&g);
    ASSERT_EQ(g.bonus_steps, 0);
    ASSERT_FALSE(g.bonus_active);
}

static void test_eat_bonus_scores(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    g.bonus_active = true;
    g.bonus_steps = 10;
    /* Place bonus directly ahead (2 cells wide) */
    g.bonus = (Point_t){(int16_t)(g.snake.body[0].x + 1), g.snake.body[0].y};
    game_update(&g);
    ASSERT_EQ(g.score, 77);
    ASSERT_TRUE(g.evt_ate_bonus);
    ASSERT_FALSE(g.bonus_active);
}

/* --- game_handle_input --- */

static void test_handle_input_directions(void)
{
    Game_t g;
    game_init(&g, cfg, 42);

    game_handle_input(&g, INPUT_UP);
    ASSERT_EQ(g.snake.dir_buf_len, 1);
    ASSERT_EQ(g.snake.dir_buf[0], DIR_UP);
}

static void test_handle_input_none_ignored(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    game_handle_input(&g, INPUT_NONE);
    ASSERT_EQ(g.snake.dir_buf_len, 0);
}

/* --- game_update: no-op when game over --- */

static void test_update_noop_when_game_over(void)
{
    Game_t g;
    game_init(&g, cfg, 42);
    g.status = STATE_GAME_OVER;
    int16_t old_x = g.snake.body[0].x;
    game_update(&g);
    ASSERT_EQ(g.snake.body[0].x, old_x);
}

int main(void)
{
    printf("[test_game]\n");

    RUN(test_init_state);
    RUN(test_init_snake_centered);
    RUN(test_init_food_on_board);
    RUN(test_init_food_not_on_snake);
    RUN(test_update_moves_snake);
    RUN(test_wrap_right);
    RUN(test_wrap_left);
    RUN(test_wrap_down);
    RUN(test_wrap_up);
    RUN(test_self_collision_game_over);
    RUN(test_eat_food_scores);
    RUN(test_eat_food_sets_fat);
    RUN(test_eat_food_respawns);
    RUN(test_eat_food_grows_snake);
    RUN(test_event_flags_cleared_each_tick);
    RUN(test_bonus_spawns_every_5_foods);
    RUN(test_bonus_countdown);
    RUN(test_eat_bonus_scores);
    RUN(test_handle_input_directions);
    RUN(test_handle_input_none_ignored);
    RUN(test_update_noop_when_game_over);

    TEST_REPORT();
}
