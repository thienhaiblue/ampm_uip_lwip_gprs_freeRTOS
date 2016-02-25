#include "led.h"


LED_TYPE	led1Ctr;	
LED_TYPE	led2Ctr;	

void LedInit(void)			
{
	LED1_SET_OUTPUT;
	LED2_SET_OUTPUT;
	LedSetStatus(&led1Ctr,LED_ON_TIME_DFG,LED_OFF_TIME_DFG,LED_TURN_ON,0xffffffff);
	LedSetStatus(&led2Ctr,LED_ON_TIME_DFG,LED_OFF_TIME_DFG,LED_TURN_ON,1);
}

void LedSetStatus(LED_TYPE *ledCtr,uint32_t onTime,uint32_t offTime,uint32_t ledEnable,uint32_t times)		
{
	ledCtr->ledOnTime = onTime;
	ledCtr->ledOffTime = offTime;
	ledCtr->ledCounter = 0;
	ledCtr->ledEnable = ledEnable;
	ledCtr->ledTimes = times;
}													


void CtrLed(uint32_t time)
{
		static uint64_t ledTimeNew = 0,ledTimeOld = 0;
		static uint32_t timeOld = 0;
		ledTimeNew += time - timeOld;
		LedCtr(&led1Ctr,ledTimeNew - ledTimeOld);
		LedCtr(&led2Ctr,ledTimeNew - ledTimeOld);
		ledTimeOld = ledTimeNew; 
		timeOld = time;
	
	if(led1Ctr.ledStatus == LED_ON)
	{
		LED1_PIN_SET;
	}
	else 
	{
		LED1_PIN_CLR;
	}
	
	if(led2Ctr.ledStatus == LED_ON)
	{
		LED2_PIN_SET;
	}
	else 
	{
		LED2_PIN_CLR;
	}
}	

											
void LedCtr(LED_TYPE *ledCtr, uint32_t times)	
{
	if(ledCtr->ledEnable == LED_TURN_ON) 
	{
			if(ledCtr->ledCounter > times)
				ledCtr->ledCounter -= times;
			else ledCtr->ledCounter = 0;
				
			if(ledCtr->ledCounter == 0) 
			{
				if(ledCtr->ledTimes) 
				{
					ledCtr->ledTimes--;
					ledCtr->ledCounter = ledCtr->ledOffTime + ledCtr->ledOnTime;
					ledCtr->ledStatus = LED_ON;
				}
			}
			
			if(ledCtr->ledCounter <= ledCtr->ledOffTime) 
				ledCtr->ledStatus = LED_OFF;
	}
}

