#ifndef BTN_H
#define BTN_H

#include <stdbool.h>

void btn_init(void);

extern volatile bool is_button_pressed;

#endif // !BTN_H
