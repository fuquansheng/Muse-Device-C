/*
 * pps960.c
 *
 *  Created on: 2016Äê9ÔÂ8ÈÕ
 *      Author: cole
 */

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "app_util_platform.h"
#include "nrf_drv_twi.h"
#include "nrf_gpio.h" 
#include "nrf_delay.h"
#include "app_timer.h"
#include "afe4404_hw.h"
#include "agc_V3_1_19.h"
#include "hqerror.h"
#include "pps960.h"
#include "Naptime_III.h"
#include "ble_hrs.h"
#include "SEGGER_RTT_Conf.h"
#include "SEGGER_RTT.h"
#include "HC_uart.h"

uint16_t lifeQhrm = 0;
int8_t snrValue = 0,skin = 0,sample = 0;
 
uint16_t acc_check=0;
uint16_t acc_check2=0;
uint8_t pps964_is_init = 0; 

int8_t hr_okflag=false;
int8_t Stablecnt=0;
int8_t Unstablecnt=0;

int8_t HR_HRV_enable=0;//0=>HR;1=>HRV;2=>HR+HRV;

uint32_t displayHrm = 0;
uint32_t pps_count;
uint32_t pps_intr_flag=0;
int8_t accPushToQueueFlag=0;
extern uint16_t AccBuffTail;

void PPS_DELAY_MS(uint32_t ms)
{
		nrf_delay_ms(ms);
}

void pps960_Rest_SW(void)
{
    PPS960_writeReg(0,0x8);
    PPS_DELAY_MS(50);
}

void pps960_disable(void)
{
	pps964_is_init = 0;
	nrf_gpio_pin_write(PPS_EN_PIN, 0);
	PPS_DELAY_MS(200);
}

void init_pps960_sensor(void)
{
	  pps964_is_init = 1;
	  nrf_gpio_cfg_output(PPS_EN_PIN);
	  nrf_gpio_pin_write(PPS_EN_PIN, 1);
	  PPS_DELAY_MS(200);
	  nrf_gpio_pin_write(PPS_REST_PIN, 0);//low
    PPS_DELAY_MS(30);
		nrf_gpio_pin_write(PPS_REST_PIN, 1);//high
    PPS_DELAY_MS(30);
	
    init_PPS960_register();
    PPS960_init();
}

extern uint8_t control;
uint8_t pps_test_flag=0;
//uint8_t pps960_init_flag = 0;
void pps960_sensor_task(void *params)
{
      pps_intr_flag = 0;
			if(acc_check){
							ALGSH_retrieveSamplesAndPushToQueue();//read pps raw data
							//move ALGSH_dataToAlg(); to message queue loop. and then send message at here.
							ALGSH_dataToAlg();
				//SEGGER_RTT_printf(0,"LED1~~~~\r\n");
			}
}

uint8_t cnt=0;
uint16_t lifeHR = 0;
uint16_t lifeskin = 0;
extern uint8_t EEG_DATA_SEND[750];
void pps960_sensor_task2(void *params)
{
//	  uint32_t err_code;
	 
		if(acc_check) {
				//if(GetHRSampleCount()==25) { // for 1s to update display
						sample=GetHRSampleCount();
						ClrHRSampleCount();
						cnt++;if(cnt>255)cnt=0;
						lifeQhrm = pps_getHR();

						snrValue=PP_GetHRConfidence();//for snr check
						//skin = PP_IsSensorContactDetected();//for skin detect
						skin = PPS_get_skin_detect();
						if(skin == 0)
						{
							lifeQhrm = 0;
						}
//						SEGGER_RTT_printf(0,"%d HR=%d snr=%d spl=%d skin=%d\r\n",cnt,lifeQhrm,snrValue,sample,skin);
//						app_uart_put(0x48);

//			      printf("%d\r\n",lifeQhrm);

//			      lifeHR = lifeQhrm;
//            lifeskin = skin;
						


						displayHrm = lifeQhrm;// 

				//}
		}//if(acc_check)
}

//=============================================================================

#define PPS960_ADDR (0x58U)

/**
 * @brief TWI master instance
 *
 * eeprom memory.
 */
static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(0);//twi0


void PPS960_writeReg(uint8_t regaddr, uint32_t wdata)
{

	  uint8_t temp[4];
	
    uint32_t wd=wdata;
    temp[0]=regaddr;
    temp[1]=(wd>>16) & 0xff;
    temp[2]=(wd>>8) & 0xff;
    temp[3]=wd & 0xff;
    ret_code_t ret;
    do
    {
        ret = nrf_drv_twi_tx(&m_twi_master, PPS960_ADDR, temp, 4, false);
    }while (0);
    //return ret;
		if(NRF_SUCCESS != ret)
		{
			if(0x8201==ret){SEGGER_RTT_printf(0,"i2c Ack error!");}
			else if(0x8202==ret){SEGGER_RTT_printf(0,"i2c Nack error!");}
//			SEGGER_RTT_printf(0,"PPS960_writeReg faile!!! %x\r\n",ret);
			//printf("PPS960_writeReg faile!!! %d\r\n",ret);
		}
}



uint32_t PPS960_readReg(uint8_t regaddr)
{
    ret_code_t ret;
		uint8_t temp[4];
    uint32_t rdtemp;
	
    do
    {
       uint8_t addr8 = (uint8_t)regaddr;
       ret = nrf_drv_twi_tx(&m_twi_master, PPS960_ADDR, &addr8, 1, true);
       if (NRF_SUCCESS != ret)
       {
				 			if(0x8201==ret){SEGGER_RTT_printf(0,"i2c Ack error!");}
			else if(0x8202==ret){SEGGER_RTT_printf(0,"i2c Nack error!");}
//			SEGGER_RTT_printf(0,"PPS960_readReg faile!!! %x\r\n",ret);
           //break;
       }
       ret = nrf_drv_twi_rx(&m_twi_master, PPS960_ADDR, temp, 3);
    }while (0);
		if(NRF_SUCCESS != ret)
		{
			if(0x8201==ret){SEGGER_RTT_printf(0,"i2c Ack error!");}
			else if(0x8202==ret){SEGGER_RTT_printf(0,"i2c Nack error!");}
//			SEGGER_RTT_printf(0,"PPS960_readReg faile!!! %x\r\n",ret);
			//printf("PPS960_readReg faile!!! %d\r\n",ret);
		}
		
    rdtemp = temp[0]<<16 | temp[1]<<8 | temp[2];
    //printf("rdtemp : %d\r\n ",rdtemp);
    
    return rdtemp;
}

/**
 * @brief Initialize the master TWI
 * @return NRF_SUCCESS or the reason of failure
 */

ret_code_t twi_master_init(void)
{
    ret_code_t ret;
    const nrf_drv_twi_config_t config =
    {
       .scl                = TWI_SCL_M,
       .sda                = TWI_SDA_M,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
    };

    ret = nrf_drv_twi_init(&m_twi_master, &config, NULL, NULL);

    if (NRF_SUCCESS == ret)
    {
        nrf_drv_twi_enable(&m_twi_master);
    }

    return ret;
}


/** @} */ /* End of group twi_master_with_twis_slave_example */

//#endif


