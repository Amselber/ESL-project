#ifndef BTN_H
#define BTN_H

typedef enum {
  BTN_EVENT_DOUBLE_CLICK, // Зафиксирован двойной клик
  BTN_EVENT_HOLD_START,   // Началось длительное удержание кнопки (> 250 мс)
  BTN_EVENT_HOLD_STOP     // Кнопка отпущена после удержания
} btn_event_t;

// Тип callback-функции, которую модуль кнопки будет вызывать при событиях
typedef void (*btn_event_handler_t)(btn_event_t event);

/**
 * @brief Инициализация кнопки с регистрацией функции обратного вызова
 */
void btn_init(btn_event_handler_t handler);

#endif // !BTN_H
