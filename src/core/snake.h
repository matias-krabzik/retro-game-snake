#ifndef SNAKE_H
#define SNAKE_H

#include "types.h"

typedef struct {
    Point_t body[SNAKE_MAX_LENGTH];
    bool fat[SNAKE_MAX_LENGTH];   /* segment has food bulge */
    uint16_t length;
    Direction_t direction;
    Direction_t dir_buf[2];       /* buffered direction inputs (max 2) */
    uint8_t dir_buf_len;          /* number of buffered directions */
    uint8_t grow_pending;         /* deferred growth: tail stays on next move(s) */
} Snake_t;

void snake_init(Snake_t *snake, Point_t start, uint16_t initial_length, Direction_t dir);
void snake_move(Snake_t *snake);
void snake_grow(Snake_t *snake);
bool snake_collides_self(const Snake_t *snake);
bool snake_occupies(const Snake_t *snake, Point_t p);
Point_t snake_head(const Snake_t *snake);
void snake_set_direction(Snake_t *snake, Direction_t dir);

#endif /* SNAKE_H */
