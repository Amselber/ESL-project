#include "log.h"

#include "nrf_log.h"
#include "nrf_log_backend_usb.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"

#if !LOG_BACKEND_USB_INIT_STACK

static void usbd_user_ev_handler(app_usbd_event_type_t event) {
  switch (event) {
  case APP_USBD_EVT_STARTED:
    break;
  case APP_USBD_EVT_STOPPED:
    app_usbd_disable();
    break;
  case APP_USBD_EVT_POWER_DETECTED:
    NRF_LOG_INFO("USB power detected");
    if (!nrf_drv_usbd_is_enabled()) {
      app_usbd_enable();
    }
    break;
  case APP_USBD_EVT_POWER_REMOVED:
    NRF_LOG_INFO("USB power removed");
    app_usbd_stop();
    break;
  case APP_USBD_EVT_POWER_READY:
    NRF_LOG_INFO("USB ready");
    app_usbd_start();
    break;
  default:
    break;
  }
}

#endif /* !LOG_BACKEND_USB_INIT_STACK */

void log_init(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_INFO("Starting up the project with USB logging");

#if !LOG_BACKEND_USB_INIT_STACK
  app_usbd_serial_num_generate();

  static const app_usbd_config_t usbd_config = {
      .ev_state_proc = usbd_user_ev_handler};

  err_code = app_usbd_init(&usbd_config);
  APP_ERROR_CHECK(err_code);
#endif /* !LOG_BACKEND_USB_INIT_STACK */

  NRF_LOG_DEFAULT_BACKENDS_INIT();

#if !LOG_BACKEND_USB_INIT_STACK
  if (!nrfx_usbd_is_enabled()) {
    err_code = app_usbd_power_events_enable();
    APP_ERROR_CHECK(err_code);
  }
#endif /* !LOG_BACKEND_USB_INIT_STACK */
}

void log_usb_process(void) {
#if !LOG_BACKEND_USB_INIT_STACK
  while (app_usbd_event_queue_process()) {
    /* Разбираем события USB-стека */
  }
#endif /* !LOG_BACKEND_USB_INIT_STACK */

  while (NRF_LOG_PROCESS()) {
    LOG_BACKEND_USB_PROCESS();
  }

  LOG_BACKEND_USB_PROCESS();
}
