
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
//#include "config.h"
//#include "eeprom_simulator.h"
#include "nrf_drv_twi.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "nrf.h"
#include "bsp.h"
#include "app_util_platform.h"
//#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"


#include "afe4404_hw.h"
#include "agc_V3_1_19.h"
#include "hqerror.h"
#include "pps960.h"

#define PPS960_ADDR (0x58U)

/**
 * @brief TWI master instance
 *
 * eeprom memory.
 */
static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(0);


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
			if(0x8201==ret){NRF_LOG_ERROR("i2c Ack error!");}
			else if(0x8202==ret){NRF_LOG_ERROR("i2c Nack error!");}
			NRF_LOG_ERROR("PPS960_writeReg faile!!! %x\r\n",ret);
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
				 			if(0x8201==ret){NRF_LOG_ERROR("i2c Ack error!");}
			else if(0x8202==ret){NRF_LOG_ERROR("i2c Nack error!");}
			NRF_LOG_ERROR("PPS960_readReg faile!!! %x\r\n",ret);
           //break;
       }
       ret = nrf_drv_twi_rx(&m_twi_master, PPS960_ADDR, temp, 3);
    }while (0);
		if(NRF_SUCCESS != ret)
		{
			if(0x8201==ret){NRF_LOG_ERROR("i2c Ack error!");}
			else if(0x8202==ret){NRF_LOG_ERROR("i2c Nack error!");}
			NRF_LOG_ERROR("PPS960_readReg faile!!! %x\r\n",ret);
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
#define TWI_SCL_M             7     // SCL signal pin
#define TWI_SDA_M             30    // SDA signal pin
ret_code_t twi_master_init(void)
{
    ret_code_t ret;
    const nrf_drv_twi_config_t config =
    {
       .scl                = TWI_SCL_M,
       .sda                = TWI_SDA_M,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    ret = nrf_drv_twi_init(&m_twi_master, &config, NULL, NULL);

    if (NRF_SUCCESS == ret)
    {
        nrf_drv_twi_enable(&m_twi_master);
    }

    return ret;
}


/** @} */ /* End of group twi_master_with_twis_slave_example */
