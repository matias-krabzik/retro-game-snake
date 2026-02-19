#ifndef PC_INPUT_H
#define PC_INPUT_H

#include "../../core/ui.h"

void input_enable_raw_mode(void);
void input_disable_raw_mode(void);
UiInput_t input_poll(void);

#endif /* PC_INPUT_H */
