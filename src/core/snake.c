#include "snake.h"
#include <string.h>

void snake_init(Snake_t *snake, Point_t start, uint16_t initial_length, Direction_t dir)
{
    snake->direction = dir;
    snake->length = initial_length;
    memset(snake->fat, 0, sizeof(snake->fat));

    for (uint16_t i = 0; i < initial_length; i++) {
        snake->body[i].x = start.x - i;
        snake->body[i].y = start.y;
    }
}

void snake_move(Snake_t *snake)
{
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
    if (snake->length < SNAKE_MAX_LENGTH) {
        /* Duplicate tail — next move will naturally separate it */
        snake->body[snake->length] = snake->body[snake->length - 1];
        snake->length++;
    }
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

void snake_set_direction(Snake_t *snake, Direction_t dir)
{
    /* Prevent 180-degree reversal */
    if ((snake->direction == DIR_UP    && dir == DIR_DOWN)  ||
        (snake->direction == DIR_DOWN  && dir == DIR_UP)    ||
        (snake->direction == DIR_LEFT  && dir == DIR_RIGHT) ||
        (snake->direction == DIR_RIGHT && dir == DIR_LEFT)) {
        return;
    }
    snake->direction = dir;
}
