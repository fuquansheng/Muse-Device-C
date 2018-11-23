#include "HC_timer.h"

//定时器参数
#define APP_TIMER_OP_QUEUE_SIZE          9                                           /**< Size of timer operation queues. */
#define APP_TIMER_PRESCALER              0                                           /**< Value of the RTC1 PRESCALER register. */

APP_TIMER_DEF(m_leds_timer_id);
APP_TIMER_DEF(m_leds_test_timer_id);
APP_TIMER_DEF(m_wdts_timer_id); 
APP_TIMER_DEF(m_buttons_timer_id);
APP_TIMER_DEF(m_batterys_timer_id); 
APP_TIMER_DEF(m_connects_timer_id); 
APP_TIMER_DEF(m_ledFlips_timer_id); 
//cole add
APP_TIMER_DEF(m_pps960_rd_raw_timer_id);                     
APP_TIMER_DEF(m_pps960_alg_timer_id); 

//全局变量
bool Is_led_timer_start = false;    //LED亮灯时间计时判断，如果亮灯中途切换状态，重新计时
bool led_timerout = false;          //led亮灯时间超时标志    

extern uint16_t                    m_conn_handle;  
extern led_indication_t            m_stable_state;
extern nrf_drv_wdt_channel_id      m_channel_id;
extern bsp_button_event_cfg_t      m_buttin_events;

extern void button_event_handler(button_event_t event);
extern void pps960_sensor_task(void *params);
extern void pps960_sensor_task2(void *params);

#define wdt_timer_interval           APP_TIMER_TICKS(4000, APP_TIMER_PRESCALER)
#define led_timer_interval           APP_TIMER_TICKS(120000, APP_TIMER_PRESCALER)     
#define button_timer_interval        APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)         //按键50ms检测一次
#define battery_timer_interval       APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER) 
#define led_test_timer_interval      APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)
#define connect_timer_interval       APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)      //时间30s待定
//cole add...
#define PPS960_ALG_INTERVAL          APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)  /**< algorithm calculate HR every 1s. */
#define PPS960_RD_RAW_INTERVAL       APP_TIMER_TICKS(40, APP_TIMER_PRESCALER)  /**< 40ms for PPS960 read raw data. */

void timers_init(void)
{
    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create timers.
	  wdt_timer_init();
    led_timer_init();
  	button_timer_init();
		battery_timer_init();
	  connects_timer_init();
	  led_test_timer_init();
	  ledFlips_timer_init();
		pps960_rd_raw_timer_init();
		pps960_alg_timer_init();
}
//LED定时器
void leds_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
	  Is_led_timer_start = false;              //单次触发，所以需要将标志位置0
	  leds_state_update();                     //更新LED状态和相关标志位状态
}
void led_timer_init(void)
{
    uint32_t err_code;
	  err_code = app_timer_create(&m_leds_timer_id, APP_TIMER_MODE_SINGLE_SHOT, leds_timer_handler);  //单次触发
	  APP_ERROR_CHECK(err_code);
}

void led_timer_start(void)
{
    uint32_t err_code;
    err_code = app_timer_start(m_leds_timer_id,led_timer_interval, NULL); 
	  APP_ERROR_CHECK(err_code);
	  led_timerout = false;
	  Is_led_timer_start = true;
}

void led_timer_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_leds_timer_id);
    APP_ERROR_CHECK(err_code);
	  Is_led_timer_start = false;
}

//LED测试定时器
void leds_test_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    UNUSED_VARIABLE(bsp_led_indication(BSP_INDICATE_FACTORY_LED_TEST));
}

void led_test_timer_init(void)
{
    uint32_t err_code;
	  err_code = app_timer_create(&m_leds_test_timer_id, APP_TIMER_MODE_SINGLE_SHOT, leds_test_timer_handler);   //单次触发
	  APP_ERROR_CHECK(err_code);
}

void led_test_timer_start(void)
{
    uint32_t err_code;
    err_code = app_timer_start(m_leds_test_timer_id,led_test_timer_interval, NULL); 
	  APP_ERROR_CHECK(err_code);
}

void led_test_timer_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_leds_test_timer_id);
    APP_ERROR_CHECK(err_code);
}

//看门狗定时器
void wdts_timer_handler(void * p_context)
{
	  UNUSED_PARAMETER(p_context);
    nrf_drv_wdt_channel_feed(m_channel_id);  //喂狗，WDT设置时间为5s，喂狗时间为4s
}

void wdt_timer_init(void)
{
    uint32_t err_code;
    err_code = app_timer_create(&m_wdts_timer_id,APP_TIMER_MODE_REPEATED,wdts_timer_handler);
    APP_ERROR_CHECK(err_code);
}

void wdt_timer_start(void)
{
	  uint32_t err_code;
	  err_code = app_timer_start(m_wdts_timer_id, wdt_timer_interval, NULL);
    APP_ERROR_CHECK(err_code);
}

void wdt_timer_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_wdts_timer_id);
    APP_ERROR_CHECK(err_code);
}

//按键定时器
void buttons_timer_handler(void * p_context)
{
	  UNUSED_PARAMETER(p_context);
	  buttons_state_update();
}

void button_timer_init(void)
{
    uint32_t err_code;
    err_code = app_timer_create(&m_buttons_timer_id,APP_TIMER_MODE_REPEATED,buttons_timer_handler);
    APP_ERROR_CHECK(err_code);
}

void button_timer_start(void)
{
	  uint32_t err_code;
	  err_code = app_timer_start(m_buttons_timer_id, button_timer_interval, NULL);
    APP_ERROR_CHECK(err_code);
}

void button_timer_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_buttons_timer_id);
    APP_ERROR_CHECK(err_code);
}

//SAADC定时器
void batterys_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    battery_level_update();
}

void battery_timer_init(void)
{
    uint32_t err_code;
    err_code = app_timer_create(&m_batterys_timer_id,APP_TIMER_MODE_REPEATED,batterys_timer_handler);
    APP_ERROR_CHECK(err_code);
}

void battery_timer_start(void)
{
    uint32_t err_code;
	  err_code = app_timer_start(m_batterys_timer_id, battery_timer_interval, NULL);
    APP_ERROR_CHECK(err_code);
}

void battery_timer_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_batterys_timer_id);
    APP_ERROR_CHECK(err_code);
}

//连接超时定时器
void connects_timer_handler(void * p_context)
{
	   uint32_t err_code;
     UNUSED_PARAMETER(p_context);
	   if(m_conn_handle != BLE_CONN_HANDLE_INVALID)
		 {
					err_code = sd_ble_gap_disconnect(m_conn_handle,BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
					APP_ERROR_CHECK(err_code);
		 }
}

void connects_timer_init(void)
{
	  uint32_t err_code;
    err_code = app_timer_create(&m_connects_timer_id,APP_TIMER_MODE_SINGLE_SHOT,connects_timer_handler);
    APP_ERROR_CHECK(err_code);
}

void connects_timer_start(void)
{
	  uint32_t err_code;
		err_code = app_timer_start(m_connects_timer_id, connect_timer_interval, NULL); 
    APP_ERROR_CHECK(err_code);
}

void connects_timer_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_connects_timer_id);
    APP_ERROR_CHECK(err_code);
}
//led翻转定时器
void ledFlips_timer_handler(void * p_context)
{
     UNUSED_PARAMETER(p_context);
	   UNUSED_VARIABLE(bsp_led_indication(m_stable_state));
}

void ledFlips_timer_init(void)
{
	  uint32_t err_code;
    err_code = app_timer_create(&m_ledFlips_timer_id,APP_TIMER_MODE_SINGLE_SHOT,ledFlips_timer_handler);
    APP_ERROR_CHECK(err_code);
}

void ledFlips_timer_start(uint16_t ms)
{
	  uint32_t err_code;
		err_code = app_timer_start(m_ledFlips_timer_id, APP_TIMER_TICKS(ms, APP_TIMER_PRESCALER), NULL); 
    APP_ERROR_CHECK(err_code);
}
void ledFlips_timer_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_ledFlips_timer_id);
    APP_ERROR_CHECK(err_code);
}
//采集心率原始数据
void pps960_rd_raw_timer_handler(void * p_context)
{
//	  SEGGER_RTT_printf(0," 40ms \n");
    UNUSED_PARAMETER(p_context);
	  pps960_sensor_task(NULL);
}

void pps960_rd_raw_timer_init(void)
{
	  uint32_t err_code;
    err_code = app_timer_create(&m_pps960_rd_raw_timer_id,APP_TIMER_MODE_REPEATED,pps960_rd_raw_timer_handler);
    APP_ERROR_CHECK(err_code);
}

void pps960_rd_raw_timer_start(void)
{
	  uint32_t err_code;
		err_code = app_timer_start(m_pps960_rd_raw_timer_id, PPS960_RD_RAW_INTERVAL, NULL); 
    APP_ERROR_CHECK(err_code);
}
void pps960_rd_raw_timer_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_pps960_rd_raw_timer_id);
    APP_ERROR_CHECK(err_code);
}

//心率数据处理
void pps960_alg_timer_handler(void * p_context)
{
//	  SEGGER_RTT_printf(0," 1s \n");
    UNUSED_PARAMETER(p_context);
    pps960_sensor_task2(NULL);	
}
void pps960_alg_timer_init(void)
{
	  uint32_t err_code;
    err_code = app_timer_create(&m_pps960_alg_timer_id,APP_TIMER_MODE_REPEATED,pps960_alg_timer_handler);
    APP_ERROR_CHECK(err_code);	
}

void pps960_alg_timer_start(void)
{
	  uint32_t err_code;
		err_code = app_timer_start(m_pps960_alg_timer_id,PPS960_ALG_INTERVAL, NULL); 
    APP_ERROR_CHECK(err_code);
}
void pps960_alg_timer_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_pps960_alg_timer_id);
    APP_ERROR_CHECK(err_code);
}
