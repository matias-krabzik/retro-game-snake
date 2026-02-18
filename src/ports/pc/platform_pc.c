#include "../../platform.h"
#include "input.h"
#include "render.h"

#include <unistd.h>

int platform_init(const GameConfig_t *config)
{
    (void)config;
    input_enable_raw_mode();
    render_clear_screen();
    return 0;
}

void platform_shutdown(void)
{
    input_disable_raw_mode();
    render_clear_screen();
}

void platform_render(const Game_t *game)
{
    render_frame(game);
}

InputEvent_t platform_get_input(void)
{
    return input_poll();
}

void platform_sleep_ms(uint32_t ms)
{
    usleep(ms * 1000);
}
