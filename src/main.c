/** @file
 *
 * @brief Application entry point.
 *
 */

#include "app.h"
// #include "log.h"
#include <stdbool.h>

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    // log_init();
    app_init();

    while (true)
    {
        // log_usb_process();
        app_process();
    }
}
