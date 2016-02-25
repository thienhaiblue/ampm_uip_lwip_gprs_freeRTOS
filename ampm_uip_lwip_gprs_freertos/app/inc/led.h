#ifndef __LED__H__
#define __LED__H__


#include <stdint.h>
#include "hw_config.h"





typedef struct _ledDEF
{
    uint32_t ledOnTime;
		uint32_t ledOffTime;
		uint32_t ledCounter;
		uint8_t ledStatus;
		uint8_t ledEnable;
		uint32_t ledTimes;
} LED_TYPE;


extern LED_TYPE	led1Ctr,led2Ctr;	

#define TIMER_PERIOD	1	//ms


#define LED_ON_TIME_DFG	(500 / TIMER_PERIOD) /*1s */
#define LED_OFF_TIME_DFG	(500 / TIMER_PERIOD) /*1s */
#define LED_TURN_ON	1
#define LED_TURN_OFF 0


#define LED1_PORT	GPIOB
#define LED1_PIN	GPIO_BSRR_BS6
#define LED1_SET_OUTPUT		LED1_PORT->CRL	&= ~(GPIO_CRL_MODE6 | GPIO_CRL_CNF6); LED1_PORT->CRL	|= (GPIO_CRL_MODE6_0)
#define LED1_SET_INPUT		LED1_PORT->CRL	&= ~(GPIO_CRL_MODE6 | GPIO_CRL_CNF6); LED1_PORT->CRL	|= (GPIO_CRL_CNF6_0)
#define LED1_PIN_SET					LED1_PORT->BRR = LED1_PIN
#define LED1_PIN_CLR					LED1_PORT->BSRR = LED1_PIN

#define LED2_PORT	GPIOB
#define LED2_PIN	GPIO_BSRR_BS7
#define LED2_SET_OUTPUT		LED2_PORT->CRL	&= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7); LED2_PORT->CRL	|= (GPIO_CRL_MODE7_0)
#define LED2_SET_INPUT		LED2_PORT->CRL	&= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7); LED2_PORT->CRL	|= (GPIO_CRL_CNF7_0)
#define LED2_PIN_SET					LED2_PORT->BRR = LED2_PIN
#define LED2_PIN_CLR					LED2_PORT->BSRR = LED2_PIN




#define LED_ON	1
#define LED_OFF 0

void LedCtr(LED_TYPE *ledCtr,uint32_t times);
void LedSetStatus(LED_TYPE *ledCtr,uint32_t onTime,uint32_t offTime,uint32_t ledEnable,uint32_t times);
void LedInit(void);
void CtrLed(uint32_t time);

#endif

