#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define SNAKE_MAX_LENGTH 256

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction_t;

typedef struct {
    int16_t x;
    int16_t y;
} Point_t;

typedef enum {
    INPUT_NONE,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_RESTART,
    INPUT_QUIT
} InputEvent_t;

typedef struct {
    uint16_t board_width;
    uint16_t board_height;
    uint16_t initial_length;
    uint16_t tick_ms;
} GameConfig_t;

#endif /* TYPES_H */
