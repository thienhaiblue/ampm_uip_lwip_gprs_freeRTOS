

#ifndef __ADC_TASK_H__
#define __ADC_TASK_H__
#include <stm32f10x.h>

#define ADC_CHANNELS_IN_USE 2
#define ADC_BUF_LEN       ADC_CHANNELS_IN_USE*10

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define Avg_Slope	0.0043 // V/°C
#define V25			1.43

extern int16_t ADC1ConvertedValue[ADC_BUF_LEN];
extern int32_t ADC1ConvertedValueTotal[ADC_CHANNELS_IN_USE];
extern float ADC1_0_MaxValue;
extern  float ADC1_0_MinValue;
extern uint8_t ADC_Updated;
void ADC_TaskInit(void);
void ADC_TaskDeInit(void);
void ADC_Task(uint32_t rtcTime);

#endif


