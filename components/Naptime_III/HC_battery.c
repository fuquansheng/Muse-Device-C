#include "HC_battery.h"

extern ble_bas_t                         m_bas;                                      /**< Structure used to identify the battery service. */

#define VOLTAGE_AVG_NUM  5              //��ص�ѹ�˲������С

static double bat_vol;                  //ʵ�����
//ȫ�ֱ���
uint8_t bat_vol_pre;                    //��ǰ�����ٷֱ�
uint8_t bat_vol_pre_work = 57;          //����3.67V(57%)��ʾ�͵���
uint8_t send_bat_data = 0;              //�����ݷ�����1
uint8_t battery_level_last = 0;

extern bool Into_factory_test_mode;     //�Ƿ���빤������ģʽ
extern bool Global_connected_state;     //����+���ֳɹ���־
extern void sleep_mode_enter(void);

//ȡ�������н�С����
uint8_t min(uint8_t a, uint8_t b)
{
	return a > b ? b : a;
}
//��ط����ʼ��
void ble_battory_serv_init(void)    
{
	  uint32_t        err_code;
	  ble_bas_init_t  bas_init;

    memset(&bas_init, 0, sizeof(bas_init));

    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);

    bas_init.evt_handler          = NULL;
    bas_init.support_notification = false;
    bas_init.p_report_ref         = NULL;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);
}

//SAADC�ص�����
void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{

}																	 

//SAADC��ʼ��-PIN30-ADC����ͨ��-�ɼ���ص�ѹ
void saadc_init(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN6);   //PIN30��adc����ͨ����
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);
	
	  nrf_gpio_cfg_output(TPS_CTRL);                 //��TPS62746�ڲ����ز��ܲɼ�����ص�ѹ
	  NRF_GPIO->OUTSET = 1<<TPS_CTRL;	 
}


//��ص������µ�bat_vol��ÿ��1s����һ�Σ�Ȼ��ȡ5�ε����ݵ�ƽ��ֵ����
void battery_level_update(void)                   
{
    uint32_t err_code;
	  static uint8_t count = 0;
	  
    static uint8_t bat_vol_arrary_index = 0;
    static double bat_vol_arrary[VOLTAGE_AVG_NUM] = {0};
    
	  nrf_saadc_value_t  ADC_value = 0;	                   //ADC��ȡ����
  	nrf_drv_saadc_sample_convert(0,&ADC_value);
  	bat_vol = ADC_value * 3.60 / 1024.0 * 2;             //��ز�����ѹ

		if( bat_vol_arrary[0] == 0 )                         //��һ�ν��룬����Ϊ��             
		{
				for( int i = 0; i < VOLTAGE_AVG_NUM; i++ )
			  {
						bat_vol_arrary[i] = bat_vol;
				}
		}
		else
		{
				bat_vol_arrary[bat_vol_arrary_index] = bat_vol;
				bat_vol_arrary_index = ( bat_vol_arrary_index + 1 )%VOLTAGE_AVG_NUM;
				if(bat_vol_arrary_index == 0)                    //5�����ݺ����һ��ƽ����
				{
					  count ++;
						bat_vol = 0;
						for( int i = 0; i < VOLTAGE_AVG_NUM; i++ )
						{
								bat_vol += bat_vol_arrary[i];
						}
						bat_vol = bat_vol / VOLTAGE_AVG_NUM;
						bat_vol_pre = (uint8_t)((bat_vol - 3.10 ) * 100);         //�����ٷֱ�  3.1-4.1
						
//						bat_vol_pre = min(bat_vol_pre,m_bas.battery_level_last);  //ȡ���ϴμ������������н�С�����������ֻ������ݻ�����
						bat_vol_pre = min(bat_vol_pre,battery_level_last);  //ȡ���ϴμ������������н�С�����������ֻ������ݻ�����
						m_bas.battery_level_last = bat_vol_pre;
						
						if(bat_vol_pre > 100)                                     //�����ʾ����100%
							 bat_vol_pre = 100;
						
//						err_code = update_database(&m_bas,bat_vol_pre);
//						APP_ERROR_CHECK(err_code);
						battery_level_last = bat_vol_pre;
 
//						if (count >= 6 && m_bas.is_battery_notification_enabled)  //30s�ϴ�һ�ε�ص���ֵ
//						{	
//                count = 0;
//								send_bat_data = 1;
//								err_code = ble_bas_battery_level_update(&m_bas, bat_vol_pre,1);
//								if(RTT_PRINT)
//								{
//									SEGGER_RTT_printf(0,"err_code4:%x\r",err_code);		
//								}								
//								if (err_code == BLE_ERROR_NO_TX_PACKETS ||
//									err_code == NRF_ERROR_INVALID_STATE || 
//									err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
//									{
//										 return;
//									}
//								else if (err_code == NRF_SUCCESS) 
//								{
//									send_bat_data = 0;
//								}
//								else 
//								{
//									APP_ERROR_CHECK(err_code);
//								}
//						}		
						
						if(bat_vol_pre < 45)                                //����3.55V��45%��,�ػ�
						{
							  Global_connected_state = false;
								sleep_mode_enter();
						}
			 }
		}
}

//������ص�ѹCheck���͵�ѹ���ܿ���-----����һ��
void Power_Check(void)
{
    uint32_t err_code;
	
	  double bat_V[3] = {0};
	  nrf_saadc_value_t  ADC_value = 0;	           //ADC��ȡ����

  	nrf_drv_saadc_sample_convert(0,&ADC_value);
	  bat_V[0] = ADC_value * 3.6 / 1024.0 * 2;     //��ص�ѹʵ�ʵ�ѹ
  	nrf_delay_ms(10);
		nrf_drv_saadc_sample_convert(0,&ADC_value);
	  bat_V[1] = ADC_value * 3.6 / 1024.0 * 2;     //��ص�ѹʵ�ʵ�ѹ
  	nrf_delay_ms(10);
  	nrf_drv_saadc_sample_convert(0,&ADC_value);
	  bat_V[2] = ADC_value * 3.6 / 1024.0 * 2;     //��ص�ѹʵ�ʵ�ѹ

	  bat_vol = (bat_V[0] + bat_V[1] + bat_V[2]) / 3;

		bat_vol_pre = (uint8_t)((bat_vol - 3.10 ) * 100);   //�õ�����ʱ�����ٷֱ�
		if(bat_vol_pre > 100)                               //�����ʾ����100%
				bat_vol_pre = 100;
		
		if(RTT_PRINT)
	  {
			SEGGER_RTT_printf(0," bat_vol_pre:%d \n",bat_vol_pre);
	  }

//    err_code = update_database(&m_bas,bat_vol_pre);
//		APP_ERROR_CHECK(err_code);
		battery_level_last = bat_vol_pre;
		
		if(bat_vol_pre < 40)               //����3.5V(40%)�޷�����
		{
			  Global_connected_state = false;
			  sleep_mode_enter();
		}
}

//USB������Ϊ�ǹ�������ģʽʱ���룬��粻ִ����������
void charging_check(void)
{
		uint32_t err_code;	   
		while(nrf_gpio_pin_read(BQ_PG) == 0 && !Into_factory_test_mode)    
		{
			 if(nrf_gpio_pin_read(BQ_CHG) == 0 && nrf_gpio_pin_read(BQ_PG) == 0)   //charging
			 {
				   if(RTT_PRINT)
					 {
							SEGGER_RTT_printf(0,"\r charging \r\n");
					 }
				   err_code = bsp_led_indication(BSP_INDICATE_BATTERY_CHARGING);
           APP_ERROR_CHECK(err_code);
			 }
			 if(nrf_gpio_pin_read(BQ_CHG) == 1 && nrf_gpio_pin_read(BQ_PG) == 0)     //charging_over
			 {
				   if(RTT_PRINT)
					 {
							SEGGER_RTT_printf(0,"\r charging over \r\n");
					 }
				   err_code = bsp_led_indication(BSP_INDICATE_BATTERY_CHARGEOVER);
           APP_ERROR_CHECK(err_code);
			 }
			 nrf_delay_ms(1000);
		}
		err_code = bsp_led_indication(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
		
		battery_timer_start();   //������ʱ�ɼ���ѹ
}
