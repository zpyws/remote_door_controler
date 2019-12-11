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
//	static uint32_t cnt = 0;
//	rt_kprintf("WAV_PACK[%d]", cnt++);
//	memdump(dat, len);
#if 0
	uint32_t i;
	uint32_t point;
	uint32_t *p;

	point = 10;
	len = point*4;
	p = (uint32_t *)dat;
	for(i=0; i<point; i++)
	{
		p[i] = (i*(4096/point)) << 16;
	}
#endif
	len = sizeof(wave32);
	rt_memcpy(dat, wave32, len);

	rt_hw_led_tog(0);
	current_audio_device = device;
//	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)wave32, sizeof(wave32)/4, DAC_ALIGN_12B_L) == HAL_OK);
	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)dat, len/4, DAC_ALIGN_12B_L) != HAL_OK)
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
	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)wave32, sizeof(wave32)/4, DAC_ALIGN_12B_L) == HAL_OK);

	HAL_TIM_Base_Start(&htim6);
	rt_hw_led_tog(0);
}
MSH_CMD_EXPORT(dac_test,  play wav file);
//************************************************************************************************************
