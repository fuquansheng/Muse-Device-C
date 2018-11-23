#ifndef HC_TIMER_H__
#define HC_TIMER_H__

#include <stdint.h>
#include <string.h>
#include "ble.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "app_timer.h"
#include "Naptime_III.h"
#include "HC_wdt.h"
#include "HC_key.h"
#include "HC_led.h"
#include "HC_battery.h"
#include "HC_command_analysis.h"
#include "softdevice_handler.h"

void timers_init(void);
void led_timer_init(void);
void led_timer_start(void);
void led_timer_stop(void);
void led_test_timer_init(void);
void led_test_timer_start(void);
void led_test_timer_stop(void);
void wdt_timer_init(void);
void wdt_timer_start(void);
void wdt_timer_stop(void);
void button_timer_init(void);
void button_timer_start(void);
void button_timer_stop(void);
void battery_timer_init(void);
void battery_timer_start(void);
void battery_timer_stop(void);
void connects_timer_init(void);
void connects_timer_start(void);
void connects_timer_stop(void);
void ledFlips_timer_init(void);
void ledFlips_timer_start(uint16_t ms);
void ledFlips_timer_stop(void);

void pps960_rd_raw_timer_init(void);
void pps960_rd_raw_timer_start(void);
void pps960_rd_raw_timer_stop(void);
void pps960_alg_timer_init(void);
void pps960_alg_timer_start(void);
void pps960_alg_timer_stop(void);

#endif
