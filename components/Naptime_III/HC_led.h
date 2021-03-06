#ifndef HC_LED_H__
#define HC_LED_H__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "HC_pwm.h"
#include "HC_timer.h"

typedef enum
{
	  BSP_INDICATE_IDLE,
	  BSP_INDICATE_CONNECTED,
	  BSP_INDICATE_CONNECTED_BAT_LOW,
	  BSP_INDICATE_WITH_WHITELIST,
	  BSP_INDICATE_WITH_WHITELIST_BAT_LOW,
	  BSP_INDICATE_WITHOUT_WHITELIST,
	  BSP_INDICATE_WITHOUT_WHITELIST_BAT_LOW,
	  BSP_INDICATE_BATTERY_CHARGING,
	  BSP_INDICATE_BATTERY_CHARGEOVER,
	  BSP_INDICATE_FACTORY_LED_TEST,
} led_indication_t;

void LED_timeout_restart(void);
uint32_t bsp_led_indication(led_indication_t indicate);
void leds_state_update(void);

#endif 
