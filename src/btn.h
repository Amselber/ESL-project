#ifndef BTN_H
#define BTN_H

#include <stdbool.h>

#define BUTTON_ACTIVE_STATE 0

void btn_init(void);
bool btn_get_state();

#endif // !BTN_H
