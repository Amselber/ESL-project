/** @file
 *
 * @brief Application entry point.
 *
 */

#include "app.h"
#include <stdbool.h>

/**
 * @brief Entry point
 */
int main(void) {
  app_init();

  while (true) {
    // device ID: #6593
    app_process();
  }
}
