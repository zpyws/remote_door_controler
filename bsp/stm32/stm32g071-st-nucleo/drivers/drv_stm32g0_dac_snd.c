#include <rtthread.h>
#include "stm32g0xx_hal.h"
#include "drivers/audio.h"
//************************************************************************************************************
#define AUDIO_PA_PIN			6

#define AUDIO_PA_ON()			rt_pin_write(AUDIO_PA_PIN, PIN_LOW)
#define AUDIO_PA_OFF()			rt_pin_write(AUDIO_PA_PIN, PIN_HIGH)
//************************************************************************************************************
TIM_HandleTypeDef htim6;
DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch1;
//************************************************************************************************************
static struct rt_audio_device *current_audio_device = NULL;
static const uint32_t wave32[] = {0, 256*16, 512*16, 1024*16, 1536*16, 2048*16, 2560*16, 3072*16, 3584*16, 4095*16};
//	const uint8_t wave8[9] = {0, 64, 128, 192, 0xff, 0xff, 0xff, 0xff, 0xff};
//	const uint16_t wave16[9] = {0, 512, 1024, 1536, 2048, 2560, 3072, 3584, 4095};
const uint8_t wave[] = 
{
	0x44, 0x04, 0x65, 0x03, 0x9A, 0x04, 0x14, 0x04, 0x9B, 0x04, 0x6D, 0x04, 0x4D, 0x04, 0x70, 0x04,
	0xB6, 0x03, 0x2A, 0x04, 0xDF, 0x02, 0xAD, 0x03, 0xD0, 0x01, 0xF9, 0x02, 0xA1, 0x00, 0x08, 0x02,
	0x89, 0xFF, 0xEA, 0x00, 0xC0, 0xFE, 0xBC, 0xFF, 0x4B, 0xFE, 0xB4, 0xFE, 0x16, 0xFE, 0x05, 0xFE,
	0x10, 0xFE, 0xB9, 0xFD, 0x32, 0xFE, 0xBA, 0xFD, 0x75, 0xFE, 0xF1, 0xFD, 0xCE, 0xFE, 0x4D, 0xFE,
	0x2D, 0xFF, 0xC4, 0xFE, 0x7B, 0xFF, 0x45, 0xFF, 0xA6, 0xFF, 0xB7, 0xFF, 0xA7, 0xFF, 0x05, 0x00,
	0x79, 0xFF, 0x2D, 0x00, 0x22, 0xFF, 0x38, 0x00, 0xB0, 0xFE, 0x25, 0x00, 0x3A, 0xFE, 0xEE, 0xFF,
	0xDA, 0xFD, 0x9A, 0xFF, 0xA9, 0xFD, 0x3F, 0xFF, 0xB4, 0xFD, 0xF5, 0xFE, 0xF3, 0xFD, 0xCC, 0xFE,
	0x55, 0xFE, 0xC4, 0xFE, 0xC1, 0xFE, 0xD1, 0xFE, 0x24, 0xFF, 0xE6, 0xFE, 0x70, 0xFF, 0xF6, 0xFE,
	0xA6, 0xFF, 0xFF, 0xFE, 0xD0, 0xFF, 0x01, 0xFF, 0xF6, 0xFF, 0xFB, 0xFE, 0x20, 0x00, 0xEC, 0xFE,
	0x6E, 0x00, 0xE0, 0xFE, 0xFF, 0x00, 0xED, 0xFE, 0xC8, 0x01, 0x2E, 0xFF, 0xA5, 0x02, 0xB9, 0xFF,
	0x88, 0x03, 0x8A, 0x00, 0x77, 0x04, 0x8C, 0x01, 0x6F, 0x05, 0xAE, 0x02, 0x68, 0x06, 0xE3, 0x03,
	0x56, 0x07, 0x22, 0x05, 0x2D, 0x08, 0x5E, 0x06, 0xE0, 0x08, 0x7E, 0x07, 0x65, 0x09, 0x6C, 0x08,
	0xBE, 0x09, 0x22, 0x09, 0xEA, 0x09, 0xA5, 0x09, 0xD9, 0x09, 0x00, 0x0A, 0x7E, 0x09, 0x3D, 0x0A,
	0xEA, 0x08, 0x43, 0x0A, 0x44, 0x08, 0xF4, 0x09, 0x98, 0x07, 0x58, 0x09, 0xD8, 0x06, 0x90, 0x08,
	0xEA, 0x05, 0xAD, 0x07, 0xBF, 0x04, 0xAE, 0x06, 0x70, 0x03, 0x97, 0x05, 0x21, 0x02, 0x72, 0x04,
	0xE0, 0x00, 0x45, 0x03, 0xA3, 0xFF, 0x14, 0x02, 0x67, 0xFE, 0xE5, 0x00, 0x2E, 0xFD, 0xBF, 0xFF,
	0xFA, 0xFB, 0x9D, 0xFE, 0xCE, 0xFA, 0x79, 0xFD, 0xAE, 0xF9, 0x53, 0xFC, 0xA3, 0xF8, 0x30, 0xFB,
	0xB2, 0xF7, 0x18, 0xFA, 0xE6, 0xF6, 0x16, 0xF9, 0x50, 0xF6, 0x34, 0xF8, 0xFE, 0xF5, 0x7D, 0xF7,
	0xFE, 0xF5, 0xF4, 0xF6, 0x54, 0xF6, 0x9E, 0xF6, 0xFA, 0xF6, 0x8D, 0xF6, 0xE4, 0xF7, 0xD0, 0xF6,
	0x07, 0xF9, 0x63, 0xF7, 0x5A, 0xFA, 0x3A, 0xF8, 0xCC, 0xFB, 0x48, 0xF9, 0x46, 0xFD, 0x86, 0xFA,
	0xB8, 0xFE, 0xDF, 0xFB, 0x10, 0x00, 0x35, 0xFD, 0x40, 0x01, 0x70, 0xFE, 0x48, 0x02, 0x8C, 0xFF,
	0x37, 0x03, 0x90, 0x00, 0x16, 0x04, 0x80, 0x01, 0xE1, 0x04, 0x56, 0x02, 0x87, 0x05, 0x0F, 0x03,
	0x05, 0x06, 0xAA, 0x03, 0x61, 0x06, 0x33, 0x04, 0x92, 0x06, 0xB8, 0x04, 0x7F, 0x06, 0x36, 0x05,
	0x25, 0x06, 0x86, 0x05, 0x91, 0x05, 0x7E, 0x05, 0xD0, 0x04, 0x26, 0x05, 0xED, 0x03, 0x9F, 0x04,
	0xF4, 0x02, 0xF4, 0x03, 0xF9, 0x01, 0x26, 0x03, 0x0C, 0x01, 0x43, 0x02, 0x32, 0x00, 0x5F, 0x01,
	0x6E, 0xFF, 0x96, 0x00, 0xC4, 0xFE, 0xF9, 0xFF, 0x35, 0xFE, 0x86, 0xFF, 0xC3, 0xFD, 0x31, 0xFF,
	0x6E, 0xFD, 0xF7, 0xFE, 0x31, 0xFD, 0xD7, 0xFE, 0xFD, 0xFC, 0xCA, 0xFE, 0xBA, 0xFC, 0xBC, 0xFE,
	0x52, 0xFC, 0x9B, 0xFE, 0xC1, 0xFB, 0x60, 0xFE, 0x17, 0xFB, 0x0B, 0xFE, 0x67, 0xFA, 0x95, 0xFD,
	0xC5, 0xF9, 0x05, 0xFD, 0x4B, 0xF9, 0x6A, 0xFC, 0x1C, 0xF9, 0xE4, 0xFB, 0x5C, 0xF9, 0x92, 0xFB,
	0x14, 0xFA, 0x82, 0xFB, 0x32, 0xFB, 0xB5, 0xFB, 0x91, 0xFC, 0x2E, 0xFC, 0x0E, 0xFE, 0xEB, 0xFC,
	0x90, 0xFF, 0xE4, 0xFD, 0x08, 0x01, 0x02, 0xFF, 0x6C, 0x02, 0x2D, 0x00, 0xB2, 0x03, 0x4C, 0x01,
	0xC7, 0x04, 0x53, 0x02, 0x9D, 0x05, 0x3B, 0x03, 0x3A, 0x06, 0xF6, 0x03, 0xAE, 0x06, 0x76, 0x04
};
//************************************************************************************************************
/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

	rt_pin_mode(AUDIO_PA_PIN, PIN_MODE_OUTPUT);
	AUDIO_PA_OFF();
}
//************************************************************************************************************
/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Channel2_3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}
//************************************************************************************************************
/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(uint32_t period)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = period;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
//    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
//    Error_Handler();
  }
}
//************************************************************************************************************
/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{
  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */
  /** DAC Initialization 
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
//    Error_Handler();
  }
  /** DAC channel OUT1 config 
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
//    Error_Handler();
  }
}
//************************************************************************************************************
//by yangwensen@20191206
extern int8_t stm32g0_dac_snd_init(void)
{
	MX_GPIO_Init();
    MX_DMA_Init();
	MX_DAC1_Init();
	MX_TIM6_Init(1414);

	return 0;
}
//************************************************************************************************************
//by yangwensen@20191209
extern int8_t stm32g0_dac_snd_start(uint32_t *fifo, uint32_t samples)
{
	AUDIO_PA_ON();
#if 0
	HAL_DAC_DeInit(&hdac1);

	if (HAL_DAC_Init(&hdac1) != HAL_OK)
	{
		return -1;
	}
#endif

//	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, fifo, samples, DAC_ALIGN_12B_L) != HAL_OK)
//	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)wave16, 9, DAC_ALIGN_12B_R) != HAL_OK)
//	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)wave8, 5, DAC_ALIGN_8B_R) != HAL_OK)
	{
//		return -2;
	}

	/* Enable TIM peripheral counter */
	HAL_TIM_Base_Start(&htim6);

	return 0;
}
//************************************************************************************************************
//by yangwensen@20191209
extern int8_t stm32g0_dac_snd_stop(void)
{
	HAL_TIM_Base_Stop(&htim6);
	HAL_DMA_Abort(&hdma_dac1_ch1);
	AUDIO_PA_OFF();
	return 0;
}
//************************************************************************************************************
//by yangwensen@20191209
extern int8_t stm32g0_dac_snd_transfer(struct rt_audio_device *device, uint8_t *dat, uint32_t len)
{
	uint32_t i;
	uint32_t point;
	uint32_t *p;
#if 0
	point = 128;
	p = (uint32_t *)dat;
	len = point*4;
	for(i=0; i<point; i++)
	{
		p[i] = (i*(4096/point)) << 4;
	}
#else
	#if 0
	point = len / 4;
	p = (uint32_t *)dat;
	for(i=0; i<point; i++)
	{
		if(i && (i&15)==15)rt_kprintf("\r\n");
		p[i] &= 0x0000fff0;
		rt_kprintf("%03X ", p[i]>>4);
	}
	rt_kprintf("\n");
	#endif
#endif
	rt_memcpy(dat, wave, 512);
	len = 512;

	rt_hw_led_tog(0);
	current_audio_device = device;
	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)dat, len/4, DAC_ALIGN_8B_R) != HAL_OK)
	{
//		return -1;
	}

	return 0;
}
//************************************************************************************************************
//by yanwensen@20191210
extern void stm32g0_dac_snd_samplerate_set(uint32_t samplerate)
{
	HAL_TIM_Base_DeInit(&htim6);
	MX_TIM6_Init(64000000UL/samplerate);
}
//************************************************************************************************************
void DMA1_Channel2_3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_dac1_ch1);
}
//************************************************************************************************************
//by yanwensen@20191210
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
	UNUSED(hdac);
	rt_hw_led_tog(0);
	rt_audio_tx_complete(current_audio_device);
}
//************************************************************************************************************
//by yanwensen@20191210
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
	UNUSED(hdac);
	rt_hw_led_tog(0);
}
//************************************************************************************************************
//by yanwensen@20191210
void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
	UNUSED(hdac);
//	rt_hw_led_on(0);
}
//************************************************************************************************************
//by yanwensen@20191210
void HAL_DAC_DMAUnderrunCallbackCh1(DAC_HandleTypeDef *hdac)
{
	UNUSED(hdac);
//	rt_hw_led_on(0);
}
//************************************************************************************************************
extern void dac_test(void)
{
//	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)wave32, sizeof(wave32)/4, DAC_ALIGN_12B_L) == HAL_OK);
	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)wave, sizeof(wave)/4, DAC_ALIGN_8B_R) == HAL_OK);

	HAL_TIM_Base_Start(&htim6);
	rt_hw_led_tog(0);
}
MSH_CMD_EXPORT(dac_test,  play wav file);
//************************************************************************************************************
