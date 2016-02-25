/**
  ******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "adc_task.h"
#include "hw_config.h"
#include "string.h"
/** @addtogroup ScanContinuous
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Comment or incomment this define to use or not the DMA */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

int16_t ADC1ConvertedValue[ADC_BUF_LEN];



float ADC1_0_MaxValue = 0;
float ADC1_0_MinValue = 0;
 
int32_t ADC1ConvertedValueTotal[ADC_CHANNELS_IN_USE];
uint8_t ADC_CntValue = 0;
uint8_t ADC_Updated= 0;
/* Private functions ---------------------------------------------------------*/
/*
Select the ADCx_IN16 input channe	
	Obtain the temperature using the following formula:
	Temperature (in °C) = {(V25- VSENSE) / Avg_Slope} + 25.
	Where, 
	1.
	VSENSE linearity with temperature ? ? °C
	Avg_Slope is Average slope 4.3 mV/°C
	V25 is Voltage at 25°C 1.43V
	tSTART(2)
	2. Guaranteed by design, not tested in production.
	Startup time 4 10 µs
	3. Shortest sampling time can be determined in the application by multiple iterations.
	ADC sampling time when reading the 
	temperature 17.1 µs
*/
void ADC_Task(uint32_t rtcTime)
{
	static uint32_t time =  0;
	uint8_t i,j;
	float f1;
	if(ADC1ConvertedValue[ADC_BUF_LEN - 1] != 0)
	{
		memset(ADC1ConvertedValueTotal,0,ADC_CHANNELS_IN_USE*4);
		
		for(i = 0; i < ADC_BUF_LEN ;i += ADC_CHANNELS_IN_USE)
		{
			for(j = 0;j < ADC_CHANNELS_IN_USE - 1;j++)
				ADC1ConvertedValueTotal[j] += (ADC1ConvertedValue[i + j] - (ADC1ConvertedValue[i + ADC_CHANNELS_IN_USE - 1] - 1489));
		}
		ADC1ConvertedValue[ADC_BUF_LEN - 1] = 0;

		for(i = 0; i < ADC_CHANNELS_IN_USE - 1;i++)
		{
			ADC1ConvertedValueTotal[i] /= 10;
		}
		
		ADC_Updated = 1;
	}
}
void ADC_TaskDeInit(void)
{
	/* Disable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
	ADC_DeInit(ADC1);
	ADC1ConvertedValue[ADC_BUF_LEN - 1] = 0;
}
/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
void ADC_TaskInit(void)
{
	ADC_InitTypeDef   ADC_InitStructure;
	DMA_InitTypeDef   DMA_InitStructure;
  /* GPIO configuration ------------------------------------------------------*/
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure PC.01 and PC.00 (Channel11 and Channel14) as analog input -----*/
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	

	ADC_TaskDeInit();
	
	/* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
/* Enable peripheral clocks --------------------------------------------------*/
  /* Enable ADC1 and GPIOC clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
	
  /* DMA1 channel1 configuration ---------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC1ConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = ADC_BUF_LEN;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = ADC_CHANNELS_IN_USE;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel11, channel14, channel16 and channel17 configurations */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 2, ADC_SampleTime_55Cycles5);
	
	 /* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  
  /* Enable TempSensor and Vrefint channels: channel16 and Channel17 */
  ADC_TempSensorVrefintCmd(ENABLE);

  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);
  
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);
  
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  
  /* Test on Channel 1 DMA1_FLAG_TC flag */
  while(!DMA_GetFlagStatus(DMA1_FLAG_TC1));
  
  /* Clear Channel 1 DMA1_FLAG_TC flag */
  DMA_ClearFlag(DMA1_FLAG_TC1);	
	ADC_Updated = 0;

}


