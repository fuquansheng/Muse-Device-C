#include "HC_eeg_data_send.h"

ble_eeg_t               m_eeg;                                     /**< Structure used to identify the heart rate service. */
ble_bas_t               m_bas;                                     /**< Structure used to identify the battery service. */
ble_com_t               m_com;                                     /**< Structure to identify the Nordic UART Service. */

/*******************数据吞吐量测试*********************/
uint16_t data_len = 300;             //发送数据总长度
uint16_t m_data_left_to_send = 0;    //剩余需要发送的数据长度
static uint8_t Data_send[20];        //发送数据缓存
static uint8_t m_ble_pl_len = 18;    //每一次发送数据长度
static uint8_t Num_Time = 0;         //帧头
//static bool m_data_q;              //发送成功True
extern uint8_t EEG_DATA_SEND[320];   //需要发送的数据
extern bool Global_connected_state;
extern bool ads1291_is_init;         //1291初始化标志位
extern uint8_t send_bat_data;
extern uint8_t bat_vol_pre;          //当前电量百分比

extern uint8_t device_id_send[17];   //发送的device_id
extern uint8_t device_sn_send[17];   //发送的SN
extern uint8_t user_id_send[5];      //发送的SN

extern uint16_t lifeQhrm;
extern int8_t skin;
extern uint8_t LOFF_State;

//调用该函数发送第一帧数据
void ble_send_data(void)
{
    uint32_t err_code;  
    m_data_left_to_send = data_len;
	  
//		Data_send[0] = Num_Time >> 8;    //添加帧头--2个字节
//		Data_send[1] = Num_Time & 0xFF;

	  for(uint8_t i = 0; i < m_ble_pl_len; i++)
		{
			Data_send[i] = *(EEG_DATA_SEND + data_len - m_data_left_to_send + i);
		}
		Data_send[18] = lifeQhrm;
		Data_send[19] = LOFF_State;
		m_data_left_to_send -= m_ble_pl_len;
			 
		err_code = ble_EEG_DATA_send(&m_eeg, Data_send, m_ble_pl_len+2);   //数据发送，长度17字节
		if(RTT_PRINT)
		{
			 SEGGER_RTT_printf(0,"err_code1:%x\r",err_code);		
		} 		
		if (err_code == BLE_ERROR_NO_TX_PACKETS ||
			err_code == NRF_ERROR_INVALID_STATE || 
			err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
			{ 
				 return;
			}
		else if (err_code == NRF_SUCCESS) 
		{
			  Num_Time ++;
		}
		else
    {  
			  APP_ERROR_CHECK(err_code);
    }
}

//这个函数完成后续数据发送,放在BLE_EVT_TX_comPLETE时间处理函数中
void ble_send_more_data(void)
{
	uint32_t err_code;
	while(m_data_left_to_send != 0 && Global_connected_state)
	{
//		Data_send[0] = Num_Time >> 8;
//		Data_send[1] = Num_Time & 0xFF;
		if(m_data_left_to_send >= 18)
		{
			for(uint8_t i = 0; i < m_ble_pl_len ;i++)
			{
				Data_send[i]= *(EEG_DATA_SEND + data_len - m_data_left_to_send + i);
			}
			m_data_left_to_send -= m_ble_pl_len;

			err_code = ble_EEG_DATA_send(&m_eeg,Data_send, m_ble_pl_len);
		}
		else
		{
			for(uint8_t i = 0; i < m_data_left_to_send ;i++)
			{
				Data_send[i]= *(EEG_DATA_SEND + data_len - m_data_left_to_send + i);
			}
			err_code = ble_EEG_DATA_send(&m_eeg,Data_send, m_data_left_to_send);
			m_data_left_to_send -= m_data_left_to_send;			
		}
		if(RTT_PRINT)
		{
			 SEGGER_RTT_printf(0,"err_code2:%x\r",err_code);		
		}      	
		if (err_code == BLE_ERROR_NO_TX_PACKETS ||
			err_code == NRF_ERROR_INVALID_STATE || 
			err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
			{
				 m_data_left_to_send += m_ble_pl_len; 
				 break;
			}
		else if (err_code != NRF_SUCCESS) 
		{
			APP_ERROR_CHECK(err_code);
		}
	}
	while(send_bat_data == 1 && Global_connected_state)
	{
		err_code = ble_bas_battery_level_update(&m_bas, bat_vol_pre,1);
		if(RTT_PRINT)
		{
			 SEGGER_RTT_printf(0,"err_code4:%x\r",err_code);			
		}     
		if (err_code == BLE_ERROR_NO_TX_PACKETS ||
			err_code == NRF_ERROR_INVALID_STATE || 
			err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
			{
				 break;
			}
		else if (err_code == NRF_SUCCESS) 
		{
			send_bat_data = 0;
		}
		else
		{
			APP_ERROR_CHECK(err_code);
		}
	}
	while(device_id_send[0] != 0 && Global_connected_state)
	{
		err_code = ble_com_string_send(&m_com, device_id_send , 17);
		if(RTT_PRINT)
		{
			 SEGGER_RTT_printf(0,"err_code6:%x\r",err_code);			
		} 	
		if (err_code == BLE_ERROR_NO_TX_PACKETS ||
			err_code == NRF_ERROR_INVALID_STATE || 
			err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
			{
				 break;
			}
		else if (err_code == NRF_SUCCESS) 
		{
			device_id_send[0] = 0;
		}
		else
		{
			APP_ERROR_CHECK(err_code);
		}
	}
	while(device_sn_send[0] != 0 && Global_connected_state)
	{
		err_code = ble_com_string_send(&m_com, device_sn_send , 17);
		if(RTT_PRINT)
		{
			 SEGGER_RTT_printf(0,"err_code7:%x\r",err_code);	
		}     	
		if (err_code == BLE_ERROR_NO_TX_PACKETS ||
			err_code == NRF_ERROR_INVALID_STATE || 
			err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
			{
				 break;
			}
		else if (err_code == NRF_SUCCESS) 
		{
			device_sn_send[0] = 0;
		}
		else
		{
			APP_ERROR_CHECK(err_code);
		}
	}
	while(user_id_send[0] != 0 && Global_connected_state)
	{
		err_code = ble_com_string_send(&m_com, user_id_send , 5);
		if(RTT_PRINT)
		{
			 SEGGER_RTT_printf(0,"err_code8:%x\r",err_code);
		}    		
		if (err_code == BLE_ERROR_NO_TX_PACKETS ||
		err_code == NRF_ERROR_INVALID_STATE || 
		err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
		{
			 break;
		}
		else if (err_code == NRF_SUCCESS) 
		{
		user_id_send[0] = 0;
		}
		else
		{
		APP_ERROR_CHECK(err_code);
		}
	}
}

void ble_state_send(uint8_t pdata)
{
   uint32_t err_code;
	 err_code = ble_EEG_ELE_STATE_send(&m_eeg,pdata, 1);
	 if(RTT_PRINT)
	 {
			 SEGGER_RTT_printf(0,"err_code3:%x\r\n",err_code);
	 }	
	 if (err_code == BLE_ERROR_NO_TX_PACKETS ||
	 err_code == NRF_ERROR_INVALID_STATE || 
	 err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING)
	 {
		 return;
	 }
	 else if (err_code != NRF_SUCCESS) 
	 {
		 APP_ERROR_CHECK(err_code);
	 }
}
