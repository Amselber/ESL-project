#ifndef NVM_STORAGE_H
#define NVM_STORAGE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint16_t h; // 0..359
  uint8_t s;  // 0..100
  uint8_t v;  // 0..100
} hsv_color_t;

/**
 * @brief Читает сохранённый цвет из Flash.
 * @return true, если найдены валидные данные; false при первом запуске.
 */
bool nvm_load_hsv(hsv_color_t *p_color);

/**
 * @brief Стирает страницу Flash и записывает новый цвет через NRF_NVMC.
 */
void nvm_save_hsv(const hsv_color_t *p_color);

#endif // !NVM_STORAGE_H
