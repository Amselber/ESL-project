#include "nvm_storage.h"
#include "nrf_log.h"

#define NVM_PAGE_SIZE         0x1000UL // Размер одной страницы Flash (4096 байт)
#define NVM_BOOTLOADER_MARGIN (3 * NVM_PAGE_SIZE) // Тройной запас от бутлоадера (12 КБ)
#define NVM_WORDS_PER_PAGE    (NVM_PAGE_SIZE / sizeof(uint32_t)) // 1024 слота

// Безопасные границы Flash: выше конца кода (0x80000) и ниже буферной зоны (0xDF000)
#define NVM_MIN_SAFE_ADDR     0x00080000UL
#define NVM_MAX_SAFE_ADDR     0x000DF000UL

// Читаем адрес загрузчика из UICR->NRFFW[0] (если регистр не задан, берём 0x000E0000)
#define NVM_BOOTLOADER_ADDR   ((NRF_UICR->NRFFW[0] != 0xFFFFFFFFUL) ? (uint32_t)NRF_UICR->NRFFW[0] : 0x000E0000UL)
#define NVM_STORAGE_ADDR      (NVM_BOOTLOADER_ADDR - NVM_BOOTLOADER_MARGIN)

#define RECORD_MARKER         0xA5UL // Метка валидной записи в старшем байте

static void nvmc_wait_ready(void) {
  while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
  }
}

// Защитная проверка на этапе выполнения: запрещаем работу за пределами безопасного окна
static bool is_storage_addr_safe(uint32_t addr) {
  if (addr < NVM_MIN_SAFE_ADDR || addr >= NVM_MAX_SAFE_ADDR || (addr % NVM_PAGE_SIZE) != 0) {
    NRF_LOG_ERROR("[NVM] Unsafe or unaligned flash address: 0x%08X!", addr);
    return false;
  }
  return true;
}

// Упаковка HSV в одно 32-битное слово:
// [31:24] = 0xA5 (маркер) | [23:16] = V (0..100) | [15:9] = S (0..100) | [8:0] = H (0..359)
static uint32_t pack_hsv(const hsv_color_t *p_color) {
  return (RECORD_MARKER << 24)                 |
         ((uint32_t)(p_color->v & 0x7F) << 16) |
         ((uint32_t)(p_color->s & 0x7F) << 9)  |
         ((uint32_t)(p_color->h & 0x1FF));
}

// Распаковка 32-битного слова обратно в HSV
static bool unpack_hsv(uint32_t raw, hsv_color_t *p_color) {
  if (((raw >> 24) & 0xFF) != RECORD_MARKER) {
    return false;
  }

  uint16_t h = (uint16_t)(raw & 0x1FF);
  uint8_t  s = (uint8_t)((raw >> 9) & 0x7F);
  uint8_t  v = (uint8_t)((raw >> 16) & 0x7F);

  if (h >= 360 || s > 100 || v > 100) {
    return false;
  }

  p_color->h = h;
  p_color->s = s;
  p_color->v = v;
  return true;
}

bool nvm_load_hsv(hsv_color_t *p_color) {
  uint32_t storage_addr = NVM_STORAGE_ADDR;
  if (!is_storage_addr_safe(storage_addr)) {
    return false;
  }

  const uint32_t *p_slots = (const uint32_t *)storage_addr;
  uint32_t last_valid_word = 0xFFFFFFFFUL;

  // Ищем последнюю непустую ячейку на странице
  for (uint32_t i = 0; i < NVM_WORDS_PER_PAGE; i++) {
    if (p_slots[i] == 0xFFFFFFFFUL) {
      break; // Дошли до чистой области Flash
    }
    last_valid_word = p_slots[i];
  }

  if (last_valid_word == 0xFFFFFFFFUL) {
    return false; // Страница полностью пуста (первый запуск)
  }

  return unpack_hsv(last_valid_word, p_color);
}

void nvm_save_hsv(const hsv_color_t *p_color) {
  uint32_t storage_addr = NVM_STORAGE_ADDR;
  if (!is_storage_addr_safe(storage_addr)) {
    return;
  }

  uint32_t new_word = pack_hsv(p_color);
  uint32_t *p_slots = (uint32_t *)storage_addr;

  // Ищем первый свободный слот (0xFFFFFFFF)
  uint32_t free_idx = NVM_WORDS_PER_PAGE;
  for (uint32_t i = 0; i < NVM_WORDS_PER_PAGE; i++) {
    if (p_slots[i] == 0xFFFFFFFFUL) {
      free_idx = i;
      break;
    }
  }

  // Если цвет совпадает с уже записанным в предыдущем слоте — ничего не пишем
  if (free_idx > 0 && p_slots[free_idx - 1] == new_word) {
    NRF_LOG_INFO("[NVM] Color unchanged, skip write");
    return;
  }

  // Если все 1024 слота заняты — стираем страницу и пишем в слот 0
  if (free_idx >= NVM_WORDS_PER_PAGE) {
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een;
    nvmc_wait_ready();
    NRF_NVMC->ERASEPAGE = storage_addr;
    nvmc_wait_ready();
    free_idx = 0;
    NRF_LOG_INFO("[NVM] Page full, erased 0x%08X", storage_addr);
  }

  // Пишем одно 32-битное слово в свободный слот
  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
  nvmc_wait_ready();

  p_slots[free_idx] = new_word;
  nvmc_wait_ready();

  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
  nvmc_wait_ready();

  NRF_LOG_INFO("[NVM] Saved HSV(%d, %d, %d) to slot %d (0x%08X)",
               p_color->h, p_color->s, p_color->v,
               free_idx, (uint32_t)&p_slots[free_idx]);
}
