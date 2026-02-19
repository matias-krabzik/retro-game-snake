#include "../../platform.h"
#include "input.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>

/* Render buffer: 84 cols * 24 rows, each cell up to ~50 bytes escape + 3 char */
#define RENDER_BUF_SIZE  131072
#define MIN_COLS  LCD_WIDTH   /* 84 */
#define MIN_ROWS  (LCD_HEIGHT / 2)  /* 24 */

static char render_buf[RENDER_BUF_SIZE];
static int term_cols;     /* cached terminal width */
static bool use_truecolor;

/* Convert LcdColor_t to nearest xterm 256-color index (6x6x6 cube) */
static int to_256(LcdColor_t c)
{
    int ri = (c.r < 48) ? 0 : (c.r < 115) ? 1 : (c.r - 35) / 40;
    int gi = (c.g < 48) ? 0 : (c.g < 115) ? 1 : (c.g - 35) / 40;
    int bi = (c.b < 48) ? 0 : (c.b < 115) ? 1 : (c.b - 35) / 40;
    return 16 + 36 * ri + 6 * gi + bi;
}

static int get_term_size(int *cols, int *rows)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
        return -1;
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
}

/* Write entire buffer, handling short writes and non-blocking EAGAIN.
 * stdin's O_NONBLOCK propagates to stdout (same pty), so write()
 * can return EAGAIN when the pty buffer is full. */
static void write_all(int fd, const char *buf, size_t len)
{
    while (len > 0) {
        ssize_t n = write(fd, buf, len);
        if (n > 0) {
            buf += n;
            len -= (size_t)n;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            usleep(200);  /* pty buffer full — pause and retry */
        } else {
            break;  /* real error */
        }
    }
}

int platform_init(void)
{
    int cols = 0, rows = 0;
    if (get_term_size(&cols, &rows) == 0) {
        if (cols < MIN_COLS || rows < MIN_ROWS) {
            fprintf(stderr,
                "Terminal too small: %dx%d (need at least %dx%d)\n"
                "Resize your terminal and try again.\n",
                cols, rows, MIN_COLS, MIN_ROWS);
            return -1;
        }
    }
    term_cols = cols;

    /* Detect truecolor support */
    const char *ct = getenv("COLORTERM");
    use_truecolor = (ct != NULL &&
        (strcmp(ct, "truecolor") == 0 || strcmp(ct, "24bit") == 0));

    input_enable_raw_mode();
    /* Alternate screen, hide cursor, reset scroll region, clear */
    const char init_seq[] = "\x1b[?1049h\x1b[?25l\x1b[r\x1b[H\x1b[2J";
    write_all(STDOUT_FILENO, init_seq, sizeof(init_seq) - 1);
    return 0;
}

void platform_shutdown(void)
{
    /* Show cursor, reset colors, restore main screen buffer */
    const char shutdown_seq[] = "\x1b[?25h\x1b[0m\x1b[?1049l";
    write_all(STDOUT_FILENO, shutdown_seq, sizeof(shutdown_seq) - 1);
    input_disable_raw_mode();
}

void platform_present(const uint8_t (*fb)[LCD_WIDTH],
                      const LcdPalette_t *palette)
{
    /* Horizontal padding to center LCD in terminal (1-based column) */
    int col_start = (term_cols > MIN_COLS) ? (term_cols - MIN_COLS) / 2 + 1 : 1;

    /*
     * Pre-compute the 4 possible color escape sequences.
     * Index = (top_on << 1) | bot_on  →  0=off/off, 1=off/on, 2=on/off, 3=on/on
     */
    char esc[4][64];
    int esc_len[4];

    if (use_truecolor) {
        LcdColor_t on  = palette->pixel_on;
        LcdColor_t off = palette->pixel_off;
        LcdColor_t fg_col[2] = { off, on };
        LcdColor_t bg_col[2] = { off, on };
        for (int i = 0; i < 4; i++) {
            LcdColor_t fg = fg_col[(i >> 1) & 1];
            LcdColor_t bg = bg_col[i & 1];
            esc_len[i] = sprintf(esc[i],
                "\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm",
                fg.r, fg.g, fg.b, bg.r, bg.g, bg.b);
        }
    } else {
        int c_on  = to_256(palette->pixel_on);
        int c_off = to_256(palette->pixel_off);
        int fg_idx[2] = { c_off, c_on };
        int bg_idx[2] = { c_off, c_on };
        for (int i = 0; i < 4; i++) {
            esc_len[i] = sprintf(esc[i],
                "\x1b[38;5;%dm\x1b[48;5;%dm",
                fg_idx[(i >> 1) & 1], bg_idx[i & 1]);
        }
    }

    int pos = 0;
    int text_rows = LCD_HEIGHT / 2;  /* 24 */

    /*
     * Render using Unicode upper-half-block (▀ U+2580).
     * Each text row covers 2 LCD pixel rows.
     * fg = top pixel color, bg = bottom pixel color.
     * Uses absolute cursor positioning per row (no \n — avoids scroll).
     */
    for (int tr = 0; tr < text_rows; tr++) {
        int row = tr * 2;
        int cur_state = -1;

        /* Position cursor at this row (1-based) */
        pos += sprintf(render_buf + pos, "\x1b[%d;%dH", tr + 1, col_start);

        for (int col = 0; col < LCD_WIDTH; col++) {
            int top = fb[row][col] ? 1 : 0;
            int bot = (row + 1 < LCD_HEIGHT && fb[row + 1][col]) ? 1 : 0;
            int state = (top << 1) | bot;

            /* Only emit escape codes when color state changes */
            if (state != cur_state) {
                memcpy(render_buf + pos, esc[state], (size_t)esc_len[state]);
                pos += esc_len[state];
                cur_state = state;
            }

            /* ▀ in UTF-8: 0xE2 0x96 0x80 */
            render_buf[pos++] = '\xe2';
            render_buf[pos++] = '\x96';
            render_buf[pos++] = '\x80';
        }

        /* Reset colors at end of row */
        memcpy(render_buf + pos, "\x1b[0m", 4);
        pos += 4;
    }

    /* Erase from cursor to end of screen (clear stale content below LCD) */
    pos += sprintf(render_buf + pos, "\x1b[%d;1H\x1b[J", text_rows + 1);

    write_all(STDOUT_FILENO, render_buf, (size_t)pos);
}

UiInput_t platform_get_input(void)
{
    return input_poll();
}

double platform_get_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

bool platform_should_close(void)
{
    return false;
}

void platform_play_sound(SoundType_t sound)
{
    (void)sound;
}

void platform_set_volume(int level)
{
    (void)level;
}

void platform_sleep_ms(uint32_t ms)
{
    usleep(ms * 1000);
}
