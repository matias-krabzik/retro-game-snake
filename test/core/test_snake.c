#include "../test.h"
#include "core/snake.h"

/* --- snake_init --- */

static void test_init_position(void)
{
    Snake_t s;
    Point_t start = {10, 5};
    snake_init(&s, start, 3, DIR_RIGHT);

    ASSERT_EQ(s.length, 3);
    ASSERT_EQ(s.direction, DIR_RIGHT);
    /* Head at start, body extends left */
    ASSERT_EQ(s.body[0].x, 10);
    ASSERT_EQ(s.body[0].y, 5);
    ASSERT_EQ(s.body[1].x, 9);
    ASSERT_EQ(s.body[2].x, 8);
}

static void test_init_clears_fat(void)
{
    Snake_t s;
    /* Pre-fill fat array to verify it's cleared */
    memset(s.fat, 1, sizeof(s.fat));
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);

    for (int i = 0; i < 3; i++) {
        ASSERT_FALSE(s.fat[i]);
    }
}

static void test_init_clears_dir_buffer(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    ASSERT_EQ(s.dir_buf_len, 0);
    ASSERT_EQ(s.grow_pending, 0);
}

/* --- snake_head --- */

static void test_head_returns_body0(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){7, 3}, 2, DIR_RIGHT);
    Point_t h = snake_head(&s);
    ASSERT_EQ(h.x, 7);
    ASSERT_EQ(h.y, 3);
}

/* --- snake_move --- */

static void test_move_right(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    snake_move(&s);
    ASSERT_EQ(s.body[0].x, 6);
    ASSERT_EQ(s.body[0].y, 5);
    /* Tail shifted: was at x=3, now at x=4 (old body[1]) */
    ASSERT_EQ(s.body[2].x, 4);
}

static void test_move_left(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 1, DIR_LEFT);
    snake_move(&s);
    ASSERT_EQ(s.body[0].x, 4);
}

static void test_move_up(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 1, DIR_UP);
    snake_move(&s);
    ASSERT_EQ(s.body[0].y, 4);
}

static void test_move_down(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 1, DIR_DOWN);
    snake_move(&s);
    ASSERT_EQ(s.body[0].y, 6);
}

static void test_move_shifts_fat(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    s.fat[0] = true;
    snake_move(&s);
    /* Fat should have shifted from index 0 to index 1 */
    ASSERT_FALSE(s.fat[0]);
    ASSERT_TRUE(s.fat[1]);
}

/* --- snake_grow --- */

static void test_grow_deferred(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    ASSERT_EQ(s.length, 3);

    snake_grow(&s);
    /* Length doesn't change until move */
    ASSERT_EQ(s.length, 3);
    ASSERT_EQ(s.grow_pending, 1);

    snake_move(&s);
    ASSERT_EQ(s.length, 4);
    ASSERT_EQ(s.grow_pending, 0);
}

static void test_grow_multiple(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    snake_grow(&s);
    snake_grow(&s);
    ASSERT_EQ(s.grow_pending, 2);

    snake_move(&s);
    ASSERT_EQ(s.length, 4);
    ASSERT_EQ(s.grow_pending, 1);

    snake_move(&s);
    ASSERT_EQ(s.length, 5);
    ASSERT_EQ(s.grow_pending, 0);
}

/* --- snake_occupies --- */

static void test_occupies_body(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    ASSERT_TRUE(snake_occupies(&s, (Point_t){5, 5}));
    ASSERT_TRUE(snake_occupies(&s, (Point_t){4, 5}));
    ASSERT_TRUE(snake_occupies(&s, (Point_t){3, 5}));
    ASSERT_FALSE(snake_occupies(&s, (Point_t){6, 5}));
    ASSERT_FALSE(snake_occupies(&s, (Point_t){5, 4}));
}

/* --- snake_collides_self --- */

static void test_no_self_collision(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    ASSERT_FALSE(snake_collides_self(&s));
}

static void test_self_collision(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 5, DIR_RIGHT);
    /* Manually place head on body segment */
    s.body[0] = s.body[3];
    ASSERT_TRUE(snake_collides_self(&s));
}

/* --- snake_set_direction --- */

static void test_set_direction_buffers(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    snake_set_direction(&s, DIR_UP);
    ASSERT_EQ(s.dir_buf_len, 1);
    ASSERT_EQ(s.dir_buf[0], DIR_UP);
}

static void test_set_direction_rejects_opposite(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    snake_set_direction(&s, DIR_LEFT);
    ASSERT_EQ(s.dir_buf_len, 0);
}

static void test_set_direction_rejects_same(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    snake_set_direction(&s, DIR_RIGHT);
    ASSERT_EQ(s.dir_buf_len, 0);
}

static void test_set_direction_max_two(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    snake_set_direction(&s, DIR_UP);
    snake_set_direction(&s, DIR_LEFT);  /* valid: not opposite to UP */
    snake_set_direction(&s, DIR_DOWN);  /* should be rejected: buffer full */
    ASSERT_EQ(s.dir_buf_len, 2);
}

static void test_direction_buffer_consumed_on_move(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    snake_set_direction(&s, DIR_UP);
    snake_set_direction(&s, DIR_LEFT);
    ASSERT_EQ(s.dir_buf_len, 2);

    snake_move(&s);
    ASSERT_EQ(s.direction, DIR_UP);
    ASSERT_EQ(s.dir_buf_len, 1);

    snake_move(&s);
    ASSERT_EQ(s.direction, DIR_LEFT);
    ASSERT_EQ(s.dir_buf_len, 0);
}

static void test_set_direction_validates_against_last_buffered(void)
{
    Snake_t s;
    snake_init(&s, (Point_t){5, 5}, 3, DIR_RIGHT);
    /* Buffer UP, then try DOWN (opposite of buffered UP, not of current RIGHT) */
    snake_set_direction(&s, DIR_UP);
    snake_set_direction(&s, DIR_DOWN);
    /* DOWN is opposite of UP (last buffered), should be rejected */
    ASSERT_EQ(s.dir_buf_len, 1);
}

int main(void)
{
    printf("[test_snake]\n");

    RUN(test_init_position);
    RUN(test_init_clears_fat);
    RUN(test_init_clears_dir_buffer);
    RUN(test_head_returns_body0);
    RUN(test_move_right);
    RUN(test_move_left);
    RUN(test_move_up);
    RUN(test_move_down);
    RUN(test_move_shifts_fat);
    RUN(test_grow_deferred);
    RUN(test_grow_multiple);
    RUN(test_occupies_body);
    RUN(test_no_self_collision);
    RUN(test_self_collision);
    RUN(test_set_direction_buffers);
    RUN(test_set_direction_rejects_opposite);
    RUN(test_set_direction_rejects_same);
    RUN(test_set_direction_max_two);
    RUN(test_direction_buffer_consumed_on_move);
    RUN(test_set_direction_validates_against_last_buffered);

    TEST_REPORT();
}
