#include "snake.h"
#include <string.h>

void snake_init(Snake_t *snake, Point_t start, uint16_t initial_length, Direction_t dir)
{
    snake->direction = dir;
    snake->length = initial_length;
    snake->dir_buf_len = 0;
    snake->grow_pending = 0;
    memset(snake->fat, 0, sizeof(snake->fat));

    for (uint16_t i = 0; i < initial_length; i++) {
        snake->body[i].x = start.x - i;
        snake->body[i].y = start.y;
    }
}

void snake_move(Snake_t *snake)
{
    /* Apply one buffered direction per tick */
    if (snake->dir_buf_len > 0) {
        snake->direction = snake->dir_buf[0];
        /* Shift buffer down */
        snake->dir_buf[0] = snake->dir_buf[1];
        snake->dir_buf_len--;
    }

    /* Grow: extend length before shifting so the tail stays in place */
    if (snake->grow_pending > 0 && snake->length < SNAKE_MAX_LENGTH) {
        snake->length++;
        snake->grow_pending--;
    }

    /* Shift body segments and fat flags */
    for (uint16_t i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
        snake->fat[i] = snake->fat[i - 1];
    }
    snake->fat[0] = false;

    /* Move head in current direction */
    switch (snake->direction) {
    case DIR_UP:    snake->body[0].y--; break;
    case DIR_DOWN:  snake->body[0].y++; break;
    case DIR_LEFT:  snake->body[0].x--; break;
    case DIR_RIGHT: snake->body[0].x++; break;
    }
}

void snake_grow(Snake_t *snake)
{
    snake->grow_pending++;
}

bool snake_collides_self(const Snake_t *snake)
{
    Point_t head = snake->body[0];
    for (uint16_t i = 1; i < snake->length; i++) {
        if (snake->body[i].x == head.x && snake->body[i].y == head.y) {
            return true;
        }
    }
    return false;
}

bool snake_occupies(const Snake_t *snake, Point_t p)
{
    for (uint16_t i = 0; i < snake->length; i++) {
        if (snake->body[i].x == p.x && snake->body[i].y == p.y) {
            return true;
        }
    }
    return false;
}

Point_t snake_head(const Snake_t *snake)
{
    return snake->body[0];
}

static bool directions_opposite(Direction_t a, Direction_t b)
{
    return (a == DIR_UP    && b == DIR_DOWN)  ||
           (a == DIR_DOWN  && b == DIR_UP)    ||
           (a == DIR_LEFT  && b == DIR_RIGHT) ||
           (a == DIR_RIGHT && b == DIR_LEFT);
}

void snake_set_direction(Snake_t *snake, Direction_t dir)
{
    /* Validate against the last buffered direction (or current if buffer empty) */
    Direction_t ref = snake->dir_buf_len > 0
                    ? snake->dir_buf[snake->dir_buf_len - 1]
                    : snake->direction;

    if (dir == ref || directions_opposite(ref, dir)) {
        return;
    }

    if (snake->dir_buf_len < 2) {
        snake->dir_buf[snake->dir_buf_len++] = dir;
    }
}
