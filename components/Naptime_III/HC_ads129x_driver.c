#include "HC_ads129x_driver.h"

extern ble_eeg_t                   m_eeg;                                      /**< Structure used to identify the heart rate service. */

static uint8_t ADCData1[750];

static uint8_t Data_Num;             //采集数据到250个触发发送函数
static ADS_ConfigDef ADS_Config1;

uint8_t EEG_DATA_SEND[320];
uint8_t LOFF_State = 0;
bool ads1291_is_init = false; //1291是否初始化完成标志位

extern uint16_t data_len;     //发送数据长度
extern uint16_t m_data_left_to_send;

#define SPI_INSTANCE  1   /**< SPI instance index. */
nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

void ADS1291_disable(void)
{
	  ads1291_is_init = false;
		nrf_gpio_cfg_output(AEF_PM_EN);  //关闭1291供电
		NRF_GPIO->OUTCLR = 1<<AEF_PM_EN;
	  nrf_drv_spi_uninit(&spi);
	  nrf_drv_gpiote_in_uninit(AEF_RDRDY);
	  m_data_left_to_send = 0;
	  m_eeg.last_state = 0x24;
}

void ads1291_init(void)
{
	  nrf_gpio_cfg_output(AEF_PM_EN);
	  NRF_GPIO->OUTSET = 1<<AEF_PM_EN;
	  nrf_delay_ms(50);
	
    nrf_gpio_cfg_output(AEF_START);
    nrf_gpio_cfg_output(AEF_RESET);
    nrf_gpio_cfg_output(AEF_MAIN_CLKSEL);
    nrf_gpio_cfg_input(AEF_RDRDY,NRF_GPIO_PIN_PULLUP);
	
	  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG(SPI_INSTANCE);
    spi_config.ss_pin = SPI_CS_PIN;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL));

	  ADS_PIN_Mainclksel_H();
	  nrf_delay_ms(100);
  	ADS_PIN_Reset_H();
  	nrf_delay_ms(1000);
	  ADS_PIN_Reset_L();
	  nrf_delay_ms(10);
	  ADS_PIN_Reset_H();
	  ADS_PIN_Start_L();
    nrf_delay_ms(10);
  	ADS_Command(ADS_SDATAC);
  	ADS_init();
		ADS_PIN_Start_H();	
	  ADS_Command(ADS_RDATAC);	
		
		gpiote_init();
		Data_Num = 0;
		m_data_left_to_send = 0;
		ads1291_is_init = true;
}

void ADS_init(void)
{
    ADS_Config_Init(&ADS_Config1);
	  ADS_Config1.CONFIG1.Value = ADS_DR(1);
    ADS_Config1.CONFIG2.Value |= ADS_PDB_LOFF_COMP + ADS_PDB_REFBUF ;//+ ADS_INT_TEST + ADS_TEST_FREQ 
    ADS_Config1.LOFF.Value |= ADS_COMP_TH(0) + ADS_ILEAD_OFF(1);//
    ADS_Config1.CH1SET.Value |=ADS_GAIN1(6)+ADS_MUX1(0);
	  ADS_Config1.CH2SET.Value |=ADS_GAIN1(6)+ADS_MUX1(0);
    ADS_Config1.RLD_SENS.Value |= ADS_RLD_LOFF_SENS + ADS_CHOP(0)  + ADS_PDB_RLD + ADS_RLD1N + ADS_RLD1P + ADS_RLD2N + ADS_RLD2P;//
    ADS_Config1.LOFF_SENS.Value |=  ADS_FLIP1 +ADS_LOFF1N + ADS_LOFF1P + ADS_FLIP2 +ADS_LOFF2N + ADS_LOFF2P;//NULL
    ADS_Config1.LOFF_STAT.Value |= NULL;
    ADS_Config1.RESP1.Value |= NULL;
    ADS_Config1.RESP2.Value |= ADS_RLDREF_INT ;//+ ADS_CALIB_ON
    ADS_Config1.GPIO.Value |= NULL;
    ADS_Config(&ADS_Config1);
}
    
void ADS_Config_Init(ADS_ConfigDef *Config)
{
    (*Config).CONFIG1.Address        =    ADS_REG_CONFIG1_ADDRESS;
    (*Config).CONFIG1.Value          =    ADS_REG_CONFIG1_DEFAULT;
    (*Config).CONFIG2.Address        =    ADS_REG_CONFIG2_ADDRESS;
    (*Config).CONFIG2.Value          =    ADS_REG_CONFIG2_DEFAULT;
    (*Config).LOFF.Address           =    ADS_REG_LOFF_ADDRESS;
    (*Config).LOFF.Value             =    ADS_REG_LOFF_DEFAULT;
    (*Config).CH1SET.Address         =    ADS_REG_CH1SET_ADDRESS;
    (*Config).CH1SET.Value           =    ADS_REG_CH1SET_DEFAULT;
    (*Config).CH2SET.Address         =    ADS_REG_CH2SET_ADDRESS;
    (*Config).CH2SET.Value           =    ADS_REG_CH2SET_DEFAULT;
    (*Config).RLD_SENS.Address       =    ADS_REG_RLDSENS_ADDRESS;
    (*Config).RLD_SENS.Value         =    ADS_REG_RLDSENS_DEFAULT;
    (*Config).LOFF_SENS.Address      =    ADS_REG_LOFFSENS_ADDRESS;
    (*Config).LOFF_SENS.Value        =    ADS_REG_LOFFSENS_DEFAULT;
    (*Config).LOFF_STAT.Address      =    ADS_REG_LOFFSTAT_ADDRESS;
    (*Config).LOFF_STAT.Value        =    ADS_REG_LOFFSTAT_DEFAULT;
    (*Config).RESP1.Address          =    ADS_REG_RESP1_ADDRESS;
    (*Config).RESP1.Value            =    ADS_REG_RESP1_DEFAULT;
    (*Config).RESP2.Address          =    ADS_REG_RESP2_ADDRESS;
    (*Config).RESP2.Value            =    ADS_REG_RESP2_DEFAULT;    
    (*Config).GPIO.Address           =    ADS_REG_GPIO_ADDRESS;
    (*Config).GPIO.Value             =    ADS_REG_GPIO_DEFAULT;
}    
void ADS_Config(ADS_ConfigDef *Config)
{    
    ADS_Setting((*Config).CONFIG1.Address,0,&(*Config).CONFIG1.Value,1);
    ADS_Setting((*Config).CONFIG2.Address,0,&(*Config).CONFIG2.Value,1);
    ADS_Setting((*Config).LOFF.Address,0,&(*Config).LOFF.Value,1);
    ADS_Setting((*Config).CH1SET.Address,0,&(*Config).CH1SET.Value,1);
    ADS_Setting((*Config).CH2SET.Address,0,&(*Config).CH2SET.Value,1);
    ADS_Setting((*Config).RLD_SENS.Address,0,&(*Config).RLD_SENS.Value,1);
    ADS_Setting((*Config).LOFF_SENS.Address,0,&(*Config).LOFF_SENS.Value,1);
    ADS_Setting((*Config).LOFF_STAT.Address,0,&(*Config).LOFF_STAT.Value,1);
    ADS_Setting((*Config).RESP1.Address,0,&(*Config).RESP1.Value,1);
    ADS_Setting((*Config).RESP2.Address,0,&(*Config).RESP2.Value,1);
    ADS_Setting((*Config).GPIO.Address,0,&(*Config).GPIO.Value,1);    
}    

void ADS_Setting(uint8_t REG,uint8_t Num,uint8_t *pData,uint8_t Size )
{
	  uint8_t SendData[3];
	  REG |=ADS_WREG;
	  SendData[0] = REG;
	  SendData[1] = Num;
	  SendData[2] = *pData;
    ADS_SPI_Write(SendData,3);
	  ADS_SPI_Delay(1);
}

void ADS_Command(uint8_t CMD)
{
    ADS_SPI_Write(&CMD,1);
	  ADS_SPI_Delay(1);
}

void ADS_ReadData(uint8_t *pRxData,uint8_t Size)
{
    ADS_SPI_Read(pRxData,Size);
}

void ADS_ReadStatue(uint8_t REG,uint8_t Num,uint8_t *pData,uint8_t Size)
{
    REG |= ADS_RREG;
    ADS_SPI_Write(&REG,1);
    ADS_SPI_Write(&Num,1);
    ADS_SPI_Read(pData,Size);
}

void pin_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if(m_eeg.is_eeg_notification_enabled && ads1291_is_init)
    {
	     uint8_t Rx[9];
  	   uint32_t ADCData11,ADCData22;
	     uint8_t Data[6] = {0};

       ADS_ReadData(Rx,9);
	     ADCData11 = ((Rx[3]*0xFFFFFF)+(Rx[4]*0xFFFF)+Rx[5]*0xFF+0x80000000)>>8;
			 ADCData22 = ((Rx[6]*0xFFFFFF)+(Rx[7]*0xFFFF)+Rx[8]*0xFF+0x80000000)>>8;
       Data[0] = ADCData11 >> 16;
		   Data[1] = ADCData11 >> 8 % 0xFF;
		   Data[2] = ADCData11 & 0xFE;
			 Data[3] = ADCData22 >> 16;
		   Data[4] = ADCData22 >> 8 % 0xFF;
		   Data[5] = ADCData22 & 0xFE;

		   memcpy((ADCData1 + Data_Num * 6),Data,6);

		   Data_Num ++;
       if(Data_Num == 48)  
	     {
				  LOFF_State = ((Rx[0]<<4) & 0x70) | ((Rx[1] & 0x80)>>4);
			    Data_Num = 0;
			    memcpy(EEG_DATA_SEND,ADCData1,288);			
				  memset(ADCData1,0,sizeof(ADCData1));
			    ble_send_data();
		   }
			 
	 }
		else
		{
				  Data_Num = 0;
		}
}

void gpiote_init(void)
{
	ret_code_t err_code;
 
  nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
  in_config.pull = NRF_GPIO_PIN_PULLUP;

  err_code = nrf_drv_gpiote_in_init(AEF_RDRDY, &in_config, pin_event_handler);
  APP_ERROR_CHECK(err_code);

  nrf_drv_gpiote_in_event_enable(AEF_RDRDY, true);
}

