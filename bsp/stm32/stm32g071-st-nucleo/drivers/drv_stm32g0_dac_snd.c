#include <rtthread.h>
#include "stm32g0xx_hal.h"
//************************************************************************************************************
TIM_HandleTypeDef htim6;
DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch1;
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
//	MX_TIM6_Init();

	return 0;
}
//************************************************************************************************************
//by yangwensen@20191209
extern int8_t stm32g0_dac_snd_start(void)
{
	const uint8_t aEscalator8bit[6] = {0x0, 0x33, 0x66, 0x99, 0xCC, 0xFF};
	const uint8_t wave8[9] = {0, 64, 128, 192, 0xff, 0xff, 0xff, 0xff, 0xff};
	const uint16_t wave16[9] = {0, 512, 1024, 1536, 2048, 2560, 3072, 3584, 4095};
 
#if 0
#endif
	HAL_DAC_DeInit(&hdac1);

	if (HAL_DAC_Init(&hdac1) != HAL_OK)
	{
		return -1;
	}

	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)wave16, 9, DAC_ALIGN_12B_R) != HAL_OK)
//	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)wave8, 5, DAC_ALIGN_8B_R) != HAL_OK)
	{
		return -2;
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

	return 0;
}
//************************************************************************************************************
//by yangwensen@20191209
extern int8_t stm32g0_dac_snd_transfer(uint8_t *dat, uint32_t len)
{
//	if (HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)dat, len, DAC_ALIGN_8B_R) != HAL_OK)
	{
//		return -1;
	}

	return 0;
}
//************************************************************************************************************
void DMA1_Channel2_3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_dac1_ch1);
}
//************************************************************************************************************
//by yanwensen@20191210
extern void stm32g0_dac_snd_samplerate_set(uint32_t samplerate)
{
	HAL_TIM_Base_DeInit(&htim6);
	MX_TIM6_Init(64000000UL/samplerate);
}
//************************************************************************************************************
