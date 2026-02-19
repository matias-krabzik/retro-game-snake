#ifndef PLATFORM_H
#define PLATFORM_H

#include "core/lcd.h"
#include "core/theme.h"
#include "core/ui.h"
#include "core/audio_types.h"
#include <stdbool.h>
#include <stdint.h>

/*
 * Platform abstraction interface.
 * Each port must implement all functions declared here.
 *
 * Ports are thin UI library adapters: they present the framebuffer,
 * poll input, play sounds, and provide timing. All game logic,
 * rendering, and UI state lives in core/.
 */

/* Initialize platform resources (window, audio, etc.).
 * Returns 0 on success, non-zero on failure. */
int platform_init(void);

/* Shut down platform and release resources. */
void platform_shutdown(void);

/* Present the LCD framebuffer to screen using the given palette. */
void platform_present(const uint8_t (*fb)[LCD_WIDTH],
                      const LcdPalette_t *palette);

/* Poll for input. Returns immediately (non-blocking). */
UiInput_t platform_get_input(void);

/* Get current time in seconds (monotonic). */
double platform_get_time(void);

/* Check if the platform wants to close (window X button, etc.). */
bool platform_should_close(void);

/* Play a sound effect. */
void platform_play_sound(SoundType_t sound);

/* Set audio volume (0..VOLUME_MAX). */
void platform_set_volume(int level);

/* Sleep for the given number of milliseconds. */
void platform_sleep_ms(uint32_t ms);

#endif /* PLATFORM_H */
