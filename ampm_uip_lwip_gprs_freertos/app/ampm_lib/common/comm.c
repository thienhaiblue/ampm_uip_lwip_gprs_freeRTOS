
#include "comm.h"
#include "uart1.h"
#include "hw_config.h"
#include "lib/sys_tick.h"
RINGBUF *commTxRingBuf = &USART1_TxRingBuff;
RINGBUF *commRxRingBuf = &USART1_RxRingBuff;

void COMM_Putc(uint8_t c)
{
	USART1_PutChar(c);
}
void COMM_Puts(uint8_t *s)
{
	USART1_PutString(s);
}

uint8_t COMM_CarrierDetected(void)
{
	return 0;
}

void MODEM_RTS_Set(void)
{
	RTS_PIN_SET;
}
void MODEM_RTS_Clr(void)
{
	RTS_PIN_CLR;
}

void MODEM_DTR_Set(void)
{
	DTR_PIN_SET;
}
void MODEM_DTR_Clr(void)
{
	DTR_PIN_CLR;
}

void MODEM_MOSFET_On(void)
{
	GSM_POWER_PIN_SET;
}

void MODEM_MOSFET_Off(void)
{
	GSM_POWER_PIN_CLR;
}

void MODEM_POWER_Set(void)
{
	POWER_PIN_SET;
}

void MODEM_POWER_Clr(void)
{
	POWER_PIN_CLR;
}

void MODEM_RESET_Set(void)
{
	RESET_PIN_SET;
}

void MODEM_RESET_Clr(void)
{
	RESET_PIN_CLR;
}

void MODEM_Wakeup(void)
{
	MODEM_RTS_Set();
	SysTick_DelayMs(100);
	MODEM_RTS_Clr();
}
				
void MODEM_UartInit(uint32_t baudrate)
{
	USART1_Init(SystemCoreClock,baudrate);
	GSM_POWER_PIN_SET_OUTPUT;
	POWER_PIN_SET_OUTPUT;
	RESET_PIN_SET_OUTPUT;
	DTR_PIN_SET_OUTPUT;
}



