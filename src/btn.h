#ifndef BTN_H
#define BTN_H

#include <stdbool.h>

void btn_init(void);

extern volatile bool g_isBlinkingActive;

#endif // !BTN_H
