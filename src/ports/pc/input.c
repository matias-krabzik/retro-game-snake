#include "input.h"

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

static struct termios orig_termios;

void input_enable_raw_mode(void)
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= (tcflag_t)~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    /* Set stdin to non-blocking */
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void input_disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);

    /* Restore blocking mode */
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}

UiInput_t input_poll(void)
{
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1)
        return UI_INPUT_NONE;

    if (c == 'q' || c == 'Q') return UI_INPUT_QUIT;
    if (c == 'w' || c == 'W') return UI_INPUT_UP;
    if (c == 's' || c == 'S') return UI_INPUT_DOWN;
    if (c == 'a' || c == 'A') return UI_INPUT_LEFT;
    if (c == 'd' || c == 'D') return UI_INPUT_RIGHT;
    if (c == '\n' || c == '\r') return UI_INPUT_CONFIRM;

    /* Arrow keys: ESC [ {A,B,C,D} */
    if (c == '\x1b') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return UI_INPUT_BACK;  /* standalone ESC = back */
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return UI_INPUT_NONE;
        if (seq[0] == '[') {
            switch (seq[1]) {
            case 'A': return UI_INPUT_UP;
            case 'B': return UI_INPUT_DOWN;
            case 'C': return UI_INPUT_RIGHT;
            case 'D': return UI_INPUT_LEFT;
            }
        }
    }

    return UI_INPUT_NONE;
}
