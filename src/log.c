#include "log.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

#include "app_timer.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"

#include "nrf_drv_clock.h"

#if !LOG_BACKEND_USB_INIT_STACK

static void usbd_user_ev_handler(app_usbd_event_type_t event);

#endif /* !LOG_BACKEND_USB_INIT_STACK */

/**
 * Timer-related definitions
 */
APP_TIMER_DEF(gs_log_timer);

static void log_timer_timeout_handler(void *p_context);

static uint32_t gs_timer_ticks;

void log_init(void) {
  ret_code_t ret = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(ret);

  NRF_LOG_INFO("Starting up the test project with USB logging");

#if !LOG_BACKEND_USB_INIT_STACK
  /* Init the clock for USB */
  NRF_LOG_DEBUG("Init the LF clock for USB - Start");
  ret = nrf_drv_clock_init();
  APP_ERROR_CHECK(ret);

  nrf_drv_clock_lfclk_request(NULL);
  while (!nrf_drv_clock_lfclk_is_running()) {
    /* Just waiting until the lf clock starts up */
  }
  NRF_LOG_DEBUG("Init the LF clock for USB - End");

  /* Generate the USB serial number */
  NRF_LOG_DEBUG("Generate the USB serial number - Start");
  app_usbd_serial_num_generate();
  NRF_LOG_DEBUG("Generate the USB serial number - End");

  /* Init USBD */
  static const app_usbd_config_t usbd_config = {.ev_state_proc =
                                                    usbd_user_ev_handler};

  NRF_LOG_DEBUG("Initialize the USBD - Start");
  ret = app_usbd_init(&usbd_config);
  APP_ERROR_CHECK(ret);
  NRF_LOG_DEBUG("Initialize the USBD - End");
#endif /* !LOG_BACKEND_USB_INIT_STACK */

  NRF_LOG_DEFAULT_BACKENDS_INIT();

#if !LOG_BACKEND_USB_INIT_STACK
  /* Enable USB power detection if it wasn't already enabled */
  if (!nrfx_usbd_is_enabled()) {
    ret = app_usbd_power_events_enable();
    APP_ERROR_CHECK(ret);
  }
#endif /* !LOG_BACKEND_USB_INIT_STACK */

  NRF_LOG_DEBUG("Init timers");
  ret = app_timer_init();
  // APP_ERROR_CHECK(ret);

  NRF_LOG_DEBUG("Init log timer");
  ret = app_timer_create(&gs_log_timer, APP_TIMER_MODE_REPEATED,
                         log_timer_timeout_handler);
  APP_ERROR_CHECK(ret);

  NRF_LOG_DEBUG("Start log timer");
  ret = app_timer_start(gs_log_timer, APP_TIMER_TICKS(1000), NULL);
  APP_ERROR_CHECK(ret);
}

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

void log_usb_process() {
#if !LOG_BACKEND_USB_INIT_STACK
  while (app_usbd_event_queue_process()) {
    /* Nothing to do */
  }
#endif /* !LOG_BACKEND_USB_INIT_STACK */

  LOG_BACKEND_USB_PROCESS();

  if (!NRF_LOG_PROCESS()) {
  }
}

static void log_timer_timeout_handler(void *p_context) {
  NRF_LOG_INFO("Healthcheck: tick count is %u", gs_timer_ticks);
}
