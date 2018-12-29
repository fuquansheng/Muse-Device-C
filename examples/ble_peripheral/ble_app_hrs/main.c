/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
/** @example examples/ble_peripheral/ble_app_eeg/main.c
 *
 * @brief Heart Rate Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Heart Rate service
 * (and also Battery and Device Information services). This application uses the
 * @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_rng.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_wdt.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "app_timer.h"
#include "app_error.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "ble_bas.h"
#include "ble_com.h"
#include "ble_conn.h"
#include "ble_dis.h"
#include "ble_eeg.h"
#ifdef BLE_DFU_APP_SUPPORT
#include "ble_dfu.h"
#include "dfu_app_handler.h"
#endif 
#include "pstorage.h"
#include "nordic_common.h"
#include "device_manager.h"
#include "softdevice_handler.h"
#include "SEGGER_RTT_Conf.h"
#include "SEGGER_RTT.h"
#include "HC_led.h"
#include "HC_pwm.h"
#include "HC_wdt.h"
#include "HC_timer.h"
#include "HC_key.h"
#include "HC_uart.h"
#include "HC_Battery.h"
#include "HC_data_flash.h"
#include "HC_factory_test.h"
#include "HC_device_info.h"
#include "HC_random_number.h"
#include "HC_eeg_data_send.h"
#include "HC_ads129x_driver.h"
#include "HC_command_analysis.h"
#include "protocol_analysis.h"

//cole add..
#include "afe4404_hw.h"
#include "agc_V3_1_19.h"
#include "hqerror.h"
#include "pps960.h"

//服务发现
#define IS_SRVC_CHANGED_CHARACT_PRESENT  1                                           /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/
//主从机连接数量参数
#define CENTRAL_LINK_COUNT               0                                           /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT            1                                           /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
//广播参数
#define APP_ADV_FAST_INTERVAL            0x00a0       //100ms                        /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_SLOW_INTERVAL            0x0320       //500ms                        /**< Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds). */
#define APP_ADV_FAST_TIMEOUT             120                                         /**< The duration of the fast advertising period (in seconds). */
#define APP_ADV_SLOW_TIMEOUT             0                                           /**< The duration of the slow advertising period (in seconds). */
//定时器参数
#define APP_TIMER_PRESCALER              0                                           /**< Value of the RTC1 PRESCALER register. */
//连接参数
#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(15, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (0.4 seconds). */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (0.65 second). */
#define SLAVE_LATENCY                    0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 (4 * 100)                                   /**< Connection supervisory timeout (4 seconds). */
//连接间隔更新参数
#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(500, APP_TIMER_PRESCALER)   /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                           /**< Number of attempts before giving up the connection parameter negotiation. */
//堆栈错误调试测参数
#define DEAD_BEEF                        0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define APP_FEATURE_NOT_SUPPORTED        BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */
//安全参数                               
#define SEC_PARAM_BOND                   1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                          /**< Maximum encryption key size. */
//DFU固件升级
#ifdef  BLE_DFU_APP_SUPPORT
#define DFU_REV_MAJOR                    0x00                                        /** DFU Major revision number to be exposed. */
#define DFU_REV_MINOR                    0x01                                        /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                     ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)      /** DFU Revision number to be exposed. combined of major and minor versions. */
#define APP_SERVICE_HANDLE_START         0x000C                                      /**< Handle of first application specific service when when service changed characteristic is present. */
#define BLE_HANDLE_MAX                   0xFFFF                                      /**< Max handle value in BLE. */
STATIC_ASSERT(IS_SRVC_CHANGED_CHARACT_PRESENT);                                      /** When having DFU Service support in application the Service Changed Characteristic should always be present. */
static ble_dfu_t                         m_dfus;                                     /**< Structure used to identify the DFU service. */
#endif // BLE_DFU_APP_SUPPORT
//服务变量
//extern ble_bas_t                         m_bas;                                      /**< Structure used to identify the battery service. */
//extern ble_com_t                         m_com;                                      /**< Structure to identify the Nordic UART Service. */
extern ble_eeg_t                         m_eeg;                                      /**< Structure used to identify the heart rate service. */
//extern ble_conn_t                        m_conn;                                     /**< Structure to identify the Nordic UART Service. */
uint16_t                                 m_conn_handle;                              /**< Handle of the current connection. */
static dm_application_instance_t         m_app_handle;                               /**< Application identifier allocated by device manager. */
//eeg数据传输变量与标志位
extern bool ads1291_is_init;             //1291初始化标志位
extern uint8_t PPS960_readReg_faile;
//连接状态标志位
extern bool Is_white_adv;                //是否白名单广播
extern bool ID_is_change;                //接收到的ID与原先ID不同，可能需要更新绑定ID
extern bool Is_device_bond;              //设备是否绑定
extern uint8_t communocate_state[5];     //握手状态返回
bool ble_is_connect = false;
bool Global_connected_state = false;     //连接+握手成功标志
//LED状态控制与标志位
extern bool led_timerout;                //led亮灯时间超时标志    
extern bool led_red_timerout;            //红灯亮灯时间超时标志
extern led_indication_t m_stable_state;
//电池电量变量
extern uint8_t bat_vol_pre;              //当前电量百分比
extern uint8_t bat_vol_pre_work;         //低电量提示的电量百分比60% -> 3.7V
//工厂测试
extern bool Into_factory_test_mode;      //是否进入工厂测试模式
extern bool deleteUserid;                //是否删除UserID
extern bool StoryDeviceID;               //是否存储deviceID
extern bool StorySN;                     //是否存储SN
//看门狗
extern nrf_drv_wdt_channel_id            m_channel_id;
//广播状态
bool ble_is_adv = false;                 //设备是否开启广播
extern uint16_t acc_check;
//广播UUID
#define BLE_UUID_Naptime_Profile 0xFF30
static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_Naptime_Profile, BLE_UUID_TYPE_VENDOR_BEGIN}}; /**< Universally unique service identifiers. */

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_UNKNOWN);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);	
}

#ifdef BLE_DFU_APP_SUPPORT
/**@brief Function for stopping advertising.
 */
static void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);

    err_code = bsp_led_indication(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for loading application-specific context after establishing a secure connection.
 *
 * @details This function will load the application context and check if the ATT table is marked as
 *          changed. If the ATT table is marked as changed, a Service Changed Indication
 *          is sent to the peer if the Service Changed CCCD is set to indicate.
 *
 * @param[in] p_handle The Device Manager handle that identifies the connection for which the context
 *                     should be loaded.
 */
static void app_context_load(dm_handle_t const * p_handle)
{
    uint32_t                 err_code;
    static uint32_t          context_data;
    dm_application_context_t context;

    context.len    = sizeof(context_data);
    context.p_data = (uint8_t *)&context_data;

    err_code = dm_application_context_get(p_handle, &context);
    if (err_code == NRF_SUCCESS)
    {
        // Send Service Changed Indication if ATT table has changed.
        if ((context_data & (DFU_APP_ATT_TABLE_CHANGED << DFU_APP_ATT_TABLE_POS)) != 0)
        {
            err_code = sd_ble_gatts_service_changed(m_conn_handle, APP_SERVICE_HANDLE_START, BLE_HANDLE_MAX);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != BLE_ERROR_INVALID_CONN_HANDLE) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != BLE_ERROR_NO_TX_PACKETS) &&
                (err_code != NRF_ERROR_BUSY) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
            {
                APP_ERROR_HANDLER(err_code);
            }
        }

        err_code = dm_application_context_delete(p_handle);
        APP_ERROR_CHECK(err_code);
    }
    else if (err_code == DM_NO_APP_CONTEXT)
    {
        // No context available. Ignore.
    }
    else
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/** @snippet [DFU BLE Reset prepare] */
/**@brief Function for preparing for system reset.
 *
 * @details This function implements @ref dfu_app_reset_prepare_t. It will be called by
 *          @ref dfu_app_handler.c before entering the bootloader/DFU.
 *          This allows the current running application to shut down gracefully.
 */
static void reset_prepare(void)
{
    uint32_t err_code;

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Disconnect from peer.
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        err_code = bsp_led_indication(BSP_INDICATE_IDLE);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        // If not connected, the device will be advertising. Hence stop the advertising.
        advertising_stop();
    }

    err_code = ble_conn_params_stop();
    APP_ERROR_CHECK(err_code);

    nrf_delay_ms(500);
}
/** @snippet [DFU BLE Reset prepare] */
#endif // BLE_DFU_APP_SUPPORT
/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
static void services_init(void)
{
    uint32_t         err_code;
//	  ble_com_init_t   com_init;
//	  ble_conn_init_t  conn_init;
    ble_eeg_init_t   eeg_init;

//	  memset(&com_init, 0, sizeof(com_init));
//    com_init.data_handler = NULL;   
//    err_code = ble_com_init(&m_com, &com_init);
//    APP_ERROR_CHECK(err_code);

//	  memset(&conn_init, 0, sizeof(conn_init));
//    conn_init.data_handler = NULL;   
//    err_code = ble_conn_init(&m_conn, &conn_init);
//    APP_ERROR_CHECK(err_code);

    memset(&eeg_init, 0, sizeof(eeg_init));
    eeg_init.evt_handler = NULL;
    err_code = ble_eeg_init(&m_eeg, &eeg_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Battery Service.
//    ble_battory_serv_init();
    // Initialize Device Information Service.
//    ble_devinfo_serv_init();
		
#ifdef BLE_DFU_APP_SUPPORT
    /** @snippet [DFU BLE Service initialization] */
    ble_dfu_init_t   dfus_init;

    // Initialize the Device Firmware Update Service.
    memset(&dfus_init, 0, sizeof(dfus_init));

    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.error_handler = NULL;
    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.revision      = DFU_REVISION;

    err_code = ble_dfu_init(&m_dfus, &dfus_init);
    APP_ERROR_CHECK(err_code);

    dfu_app_reset_prepare_set(reset_prepare);
    dfu_app_dm_appl_instance_set(m_app_handle);
    /** @snippet [DFU BLE Service initialization] */
#endif // BLE_DFU_APP_SUPPORT
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

static void gpio_reset(void)
{
    while(nrf_gpio_pin_read(BUTTON) == 0)   //按键松开才进入休眠
    {
			 nrf_drv_wdt_channel_feed(m_channel_id);
			 nrf_delay_ms(100);
		}
		
  	nrf_gpio_cfg_output(LED_GPIO_BLUE);
    nrf_gpio_cfg_output(LED_GPIO_RED);
	  nrf_gpio_cfg_output(LED_GPIO_GREEN);
	  nrf_gpio_cfg_output(AEF_PM_EN);
	  nrf_gpio_cfg_output(TPS_CTRL);
	  nrf_gpio_cfg_output(PPS_EN_PIN);

	  NRF_GPIO->OUTCLR = 1<<PPS_EN_PIN;
	  NRF_GPIO->OUTCLR = 1<<LED_GPIO_BLUE;
	  NRF_GPIO->OUTCLR = 1<<LED_GPIO_RED;
	  NRF_GPIO->OUTCLR = 1<<LED_GPIO_GREEN;
		NRF_GPIO->OUTCLR = 1<<AEF_PM_EN;
	  NRF_GPIO->OUTCLR = 1<<TPS_CTRL;
}
/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
void sleep_mode_enter(void)
{
    uint32_t err_code;

	  gpio_reset();
    // Prepare wakeup buttons.
    err_code = bsp_wakeup_buttons_set();
    APP_ERROR_CHECK(err_code);
	
    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_IDLE:                  //快速广播结束,关机
					   Global_connected_state = false;
             err_code = bsp_led_indication(BSP_INDICATE_IDLE);        //关机闪烁
             APP_ERROR_CHECK(err_code);
					   sleep_mode_enter();
             break;

        case BLE_ADV_EVT_WITH_WHITELIST:        //普通广播
					   advertising_buttons_configure();
						 LED_timeout_restart();
						 if(bat_vol_pre < bat_vol_pre_work) //低电量
						 {
							 err_code = bsp_led_indication(BSP_INDICATE_WITH_WHITELIST_BAT_LOW);     //红灯闪烁
							 APP_ERROR_CHECK(err_code);
						 }
						 else
						 {
							 err_code = bsp_led_indication(BSP_INDICATE_WITH_WHITELIST);  //蓝灯闪烁
							 APP_ERROR_CHECK(err_code);
						 }
             break;

				case BLE_ADV_EVT_WITHOUT_WHITELIST:     //快速广播
					   pairing_buttons_configure();
				     LED_timeout_restart();
						 if(bat_vol_pre < bat_vol_pre_work) //低电量
						 {
							 err_code = bsp_led_indication(BSP_INDICATE_WITHOUT_WHITELIST_BAT_LOW);     //红灯闪烁
							 APP_ERROR_CHECK(err_code);
						 }
						 else
						 {
							 err_code = bsp_led_indication(BSP_INDICATE_WITHOUT_WHITELIST);  //蓝灯闪烁
							 APP_ERROR_CHECK(err_code);
						 }
						 break;

        case BLE_ADV_EVT_WITH_WHITELIST_SLOW:   //慢广播
						 advertising_buttons_configure();
					   err_code = bsp_led_indication(BSP_INDICATE_IDLE);              //led全灭
             APP_ERROR_CHECK(err_code);
             break;
				
        default:
            break;
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;
	  
    switch (p_ble_evt->header.evt_id)
         {
         case BLE_GAP_EVT_CONNECTED:
				 	if(RTT_PRINT)
						{
							SEGGER_RTT_printf(0,"\r BLE_GAP_EVT_CONNECTED \r\n");
						}
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
				    ble_is_adv = false;
						ble_is_connect = true;
						if(Into_factory_test_mode)    //工厂测试模式
						{
							  Global_connected_state = true;
						    app_uart_put(Nap_Tool_appconnectnap);
		            err_code = bsp_led_indication(BSP_INDICATE_CONNECTED);   //工厂测试下LED状态设置
                APP_ERROR_CHECK(err_code);
						}
						else                         //正常工作模式
						{
				        connection_buttons_configure();	  
//				        connects_timer_start();
								LED_timeout_restart();
								if(bat_vol_pre < bat_vol_pre_work)    //电量低于使用电压
								{
						  			err_code = bsp_led_indication(BSP_INDICATE_CONNECTED_BAT_LOW);   //LED状态设置
										APP_ERROR_CHECK(err_code);	
								}
								else
								{
										err_code = bsp_led_indication(BSP_INDICATE_CONNECTED);          //LED状态设置
										APP_ERROR_CHECK(err_code);	
								}
						}
						Global_connected_state = true;
						ads1291_init();		
						
            break;

          case BLE_GAP_EVT_DISCONNECTED:
					  if(RTT_PRINT)
						{
							SEGGER_RTT_printf(0,"\r BLE_GAP_EVT_DISCONNECTED \r\n");
							SEGGER_RTT_printf(0,"Disconnected, reason %x\r\n",
                                 p_ble_evt->evt.gap_evt.params.disconnected.reason);
						}
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
						ble_is_connect = false;
					  err_code = bsp_led_indication(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
						Global_connected_state = false;
//				    m_bas.is_battery_notification_enabled = false;
				    m_eeg.is_eeg_notification_enabled = false;
				    m_eeg.is_state_notification_enabled = false;
//				    m_com.is_com_notification_enabled = false;
//				    m_conn.is_Shakehands_notification_enabled = false;
//				    m_conn.is_state_notification_enabled = false;
            if(ads1291_is_init == true)
						{
					   	 ADS1291_disable();
						}
//					  connects_timer_stop();
            break;

          case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server and Client timeout events.
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

				  case BLE_EVT_TX_COMPLETE:			
	    	     ble_send_more_data(); 
				     break;

          case BLE_GATTS_EVT_SYS_ATTR_MISSING:
						if(RTT_PRINT)
						{
							SEGGER_RTT_printf(0,"\r BLE_GATTS_EVT_SYS_ATTR_MISSING \r\n");
						}
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;
				
          default:
            // No implementation needed.
            break;
    }
}  

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
//    ble_bas_on_ble_evt(&m_bas, p_ble_evt);
//	  ble_com_on_ble_evt(&m_com, p_ble_evt);
//	  ble_conn_on_ble_evt(&m_conn, p_ble_evt);
    ble_eeg_on_ble_evt(&m_eeg, p_ble_evt);
    on_ble_evt(p_ble_evt);
    dm_ble_evt_handler(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
#ifdef BLE_DFU_APP_SUPPORT
    /** @snippet [Propagating BLE Stack events to DFU Service] */
    ble_dfu_on_ble_evt(&m_dfus, p_ble_evt);
    /** @snippet [Propagating BLE Stack events to DFU Service] */
#endif // BLE_DFU_APP_SUPPORT
}

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
	  pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

  	nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
    
    // Initialize the SoftDevice handler module.	
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

#ifdef BLE_DFU_APP_SUPPORT
    ble_enable_params.gatts_enable_params.service_changed = 1;
#endif // BLE_DFU_APP_SUPPORT
    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void button_event_handler(button_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
			  case BUTTON_EVENT_IDLE:
				     break;
						 
				case BUTTON_EVENT_POWER_ON:
             button_timer_stop();					
						 if(RTT_PRINT)
						 {
								SEGGER_RTT_printf(0," BUTTON_EVENT_POWER_ON \n");
						 }
						 err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
             APP_ERROR_CHECK(err_code);
             break;
				
        case BUTTON_EVENT_POWER_OFF:
						 if(RTT_PRINT)
						 {
								SEGGER_RTT_printf(0," BUTTON_EVENT_POWER_OFF \n");
						 }
						 err_code = bsp_led_indication(BSP_INDICATE_IDLE);
             APP_ERROR_CHECK(err_code);
						 sleep_mode_enter();
				     break;
				
				case BUTTON_EVENT_SLEEP:                                         
						 if(RTT_PRINT)
						 {					
								SEGGER_RTT_printf(0," BUTTON_EVENT_SLEEP \n");
						 }
				     Global_connected_state = false;
			 	     sleep_mode_enter();
             break;
				
        case BUTTON_EVENT_DISCONNECT:                                     
						 if(RTT_PRINT)
						 {					
								SEGGER_RTT_printf(0," BUTTON_EVENT_DISCONNECT \n");
						 }
						 err_code = sd_ble_gap_disconnect(m_conn_handle,
																							 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
						 APP_ERROR_CHECK(err_code);
      			 Is_white_adv = false;
				     Global_connected_state = false;
				     break;
				
        case BUTTON_EVENT_WHITELIST_OFF:                                
						 if(RTT_PRINT)
						 {
								SEGGER_RTT_printf(0," BUTTON_EVENT_WHITELIST_OFF \n");
						 }
						 Is_white_adv = false;
				     if(ble_is_adv)
						 {
				        ble_advertising_restart_without_whitelist();
						 }
						 else
						 {
							   err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
                 APP_ERROR_CHECK(err_code);
						 }
				     break;
						 
				case BUTTON_EVENT_LEDSTATE:
						 if(RTT_PRINT)
						 {
								SEGGER_RTT_printf(0," BUTTON_EVENT_LEDSTATE \n");
								SEGGER_RTT_printf(0,"led_timerout: %d\n",led_timerout);
							  SEGGER_RTT_printf(0,"Global_connected_state: %d \n",Global_connected_state);
							  SEGGER_RTT_printf(0,"bat_vol_pre: %d\n",bat_vol_pre);
						 }
						 if(led_timerout == true)         //红灯灭&&电量不足&&已连接
						 {
							   led_timerout = false;
						     LED_timeout_restart();
							   if(Global_connected_state == true)
								 {
									   if(bat_vol_pre < bat_vol_pre_work)
										 {
												 err_code = bsp_led_indication(BSP_INDICATE_CONNECTED_BAT_LOW);   //LED状态设置
												 APP_ERROR_CHECK(err_code);												  
										 }
										 else
										 {
												 err_code = bsp_led_indication(BSP_INDICATE_CONNECTED);          //LED状态设置
												 APP_ERROR_CHECK(err_code);												   
										 }
								 }
								 else
								 {
									   if(bat_vol_pre < bat_vol_pre_work)
										 {
												 err_code = bsp_led_indication(BSP_INDICATE_WITH_WHITELIST_BAT_LOW);    //LED状态设置
												 APP_ERROR_CHECK(err_code);												  
										 }
										 else
										 {
												 err_code = bsp_led_indication(BSP_INDICATE_WITH_WHITELIST);     //LED状态设置
												 APP_ERROR_CHECK(err_code);												   
										 }								 
								 }
						 }
						 break;

        default:
            break;
    }
}
/**@brief Function for handling the Device Manager events.
 *
 * @param[in] p_evt  Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
    APP_ERROR_CHECK(event_result);

#ifdef BLE_DFU_APP_SUPPORT
    if (p_event->event_id == DM_EVT_LINK_SECURED)
    {
        app_context_load(p_handle);
    }
#endif // BLE_DFU_APP_SUPPORT

    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.lesc         = SEC_PARAM_LESC;
    register_param.sec_param.keypress     = SEC_PARAM_KEYPRESS;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}
/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrsp;
		ble_advdata_manuf_data_t            manuf_data;	
    memset(&advdata, 0, sizeof(advdata));
    uint8_t data[2] = {0x55,0xAA};	
	
    if(Into_factory_test_mode)
		{
				app_uart_put(Tool_App_advdata);			
				app_uart_put(data[0]);
				app_uart_put(data[1]);
				manuf_data.data.p_data          = data;    
				manuf_data.data.size            = sizeof(data);	
				advdata.p_manuf_specific_data   = &manuf_data;
	  }
	
  	// Build advertising data struct to pass into @ref ble_advertising_init.
    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = false;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    
    memset(&scanrsp, 0, sizeof(scanrsp));  
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);  
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;  

		ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_FAST_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_FAST_TIMEOUT;
    options.ble_adv_slow_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_slow_interval = APP_ADV_SLOW_INTERVAL;
    options.ble_adv_slow_timeout  = APP_ADV_SLOW_TIMEOUT;
	
    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}
/**@brief Function for the Power manager.
 */
void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

static void gpio_init(void)
{
		nrf_gpio_cfg_input(BQ_PG ,NRF_GPIO_PIN_PULLUP);
	  nrf_gpio_cfg_input(BQ_CHG,NRF_GPIO_PIN_PULLUP);
	  nrf_gpio_cfg_input(BUTTON,NRF_GPIO_PIN_PULLUP);
	  nrf_gpio_cfg_input(FACTORY_TEST,NRF_GPIO_PIN_PULLUP);
	  nrf_delay_ms(100);
}

/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    bool erase_bonds = true;

	  SEGGER_RTT_Init();
	  if(RTT_PRINT)
	  {
			SEGGER_RTT_printf(0," FW_REV_STR:%s \n",FW_REV_STR);
	  }
	
	  // Initialize.
    gpio_init();
	  bootup_check();
 	  timers_init(); 
  	button_gpiote_init();
	  wdt_Init();
  	saadc_init();
  	flash_init();
    Read_User_ID();	

  	ble_stack_init();
    device_manager_init(erase_bonds);
    gap_params_init();
    services_init();
	  advertising_init();
    conn_params_init();
		
    charging_check();	
    Power_Check();
		button_power_on();
 
		if((!ble_is_adv) && (ble_is_connect == false))
		{
			  err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
		}
		/* Initializing TWI master interface for EEPROM */
		err_code = twi_master_init();
		APP_ERROR_CHECK(err_code);		

		init_pps960_sensor();
		acc_check = 1;
		
		pps960_rd_raw_timer_start();
		pps960_alg_timer_start();		
		
		while(1)
    {
		  if(nrf_gpio_pin_read(BQ_PG) == 0 && !Into_factory_test_mode)     //input vol is above battery vol
		  {
				   nrf_delay_ms(20);
				   if(nrf_gpio_pin_read(BQ_PG) == 0)
					 {
							 if(RTT_PRINT)
							 {
									SEGGER_RTT_printf(0," charging mode\r\n");
							 }							 
						   Global_connected_state = false;
							 err_code = sd_power_gpregret_set(0x55);
							 APP_ERROR_CHECK(err_code);
							 sleep_mode_enter();
					 }
		  }
			if(communocate_state[4] == 0x00 && ID_is_change)
			{
				   ID_is_change = false;
			     Story_User_ID();
				   communocate_state[4] = 0xFF;
				   Read_User_ID();	
					 Is_white_adv = true;
			}
			if(deleteUserid)
			{
				  deleteUserid = false;
				  delete_User_id();
				  Is_device_bond = false;
				  Is_white_adv = false;
			}
			if(StoryDeviceID)
			{
				  StoryDeviceID = false;
				  Story_Device_ID();
			}
			if(StorySN)
			{
				  StorySN = false;
				  Story_SN();
			}
		  if(PPS960_readReg_faile == 1)
			{
//					nrf_gpio_cfg_output(PPS_EN_PIN);
//					NRF_GPIO->OUTCLR = 1<<PPS_EN_PIN;				
//				  nrf_delay_ms(100);
				  SEGGER_RTT_printf(0," PPS960_readReg_faile %d\n",PPS960_readReg_faile);
				  init_pps960_sensor();
				  PPS960_readReg_faile = 0;
			}
			power_manage();
    }
}
