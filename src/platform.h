#ifndef PLATFORM_H
#define PLATFORM_H

#include "core/types.h"
#include "core/game.h"

/*
 * Platform abstraction interface.
 * Each port must implement all functions declared here.
 */

/* Initialize platform resources (terminal, display, etc.).
 * Returns 0 on success, non-zero on failure. */
int platform_init(const GameConfig_t *config);

/* Shut down platform and release resources. */
void platform_shutdown(void);

/* Render the current game state to screen. */
void platform_render(const Game_t *game);

/* Poll for input. Returns immediately (non-blocking). */
InputEvent_t platform_get_input(void);

/* Sleep for the given number of milliseconds. */
void platform_sleep_ms(uint32_t ms);

#endif /* PLATFORM_H */
