#ifndef BTN_H
#define BTN_H

#include <stdbool.h>

#define BUTTON_ACTIVE_STATE 0

void btn_init(void);
bool btn_get_state();

extern volatile bool is_button_pressed;

#endif // !BTN_H
