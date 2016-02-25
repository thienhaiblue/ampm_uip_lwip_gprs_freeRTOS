/**
* \file
*         uart2 driver
* \author
*         Nguyen Van Hai <blue@ambo.com.vn>
*/

#include "uart3.h"
extern RINGBUF comRingBuf;
extern uint8_t logEnable;
extern void RFID_Input(char c);
uint32_t uart3_parity = 0;
/*----------------------------------------------------------------------------
 Define  Baudrate setting (BRR) for USART3 
 *----------------------------------------------------------------------------*/
#define __DIV(__PCLK, __BAUD)       ((__PCLK*25)/(4*__BAUD))
#define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
#define __DIVFRAQ(__PCLK, __BAUD)   (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) * 16 + 50) / 100)
#define __USART3_BRR(__PCLK, __BAUD) ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

 uint8_t USART3_RxBuff[1024] = {0};
 RINGBUF USART3_RxRingBuff;

#ifdef UART3_USE_TX_RINGBUFF
 uint8_t USART3_TxBuff[512] = {0};
 RINGBUF USART3_TxRingBuff;
#endif

void USART3_Init(uint32_t pclk1,uint32_t baudrate,uint16_t databits,uint16_t stopbits,uint16_t parity)
{
		RCC->APB1ENR |= RCC_APB1ENR_USART3EN;                     // enable clock for Alternate Function
		//AFIO->MAPR   &= ~(3 << 4);                              // clear USART2 remap
		//UART3 Pin
		GPIOB->CRH 	 &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10 | GPIO_CRH_MODE11 | GPIO_CRH_CNF11);  					// Clear PA11, PA10               
		GPIOB->CRH   |=   GPIO_CRH_CNF10_1 | GPIO_CRH_MODE10_0 | GPIO_CRH_MODE10_1; // USART3 Tx (PA10)   Alternate function output Open-drain
		GPIOB->CRH   |=  (GPIO_CRH_CNF11_1);                      // USART3 Rx (PA11) input floating
		GPIOB->ODR	|=  GPIO_IDR_IDR11;
		uart3_parity = parity;
    USART3->BRR  = __USART3_BRR(pclk1, baudrate); // set baudrate
    USART3->CR1  = databits;                       // set Data bits
    USART3->CR2  = stopbits;                       // set Stop bits
    USART3->CR1 |= USART_Parity_No;                         // set Parity
    USART3->CR3  = __USART3_FLOWCTRL;                       // Set Flow Control
	
    USART3->CR1 |= (USART_CR1_RE | USART_CR1_TE);           // RX, TX enable
		// interrupts used
		USART3->CR1 |= USART_CR1_RXNEIE;
			
		RINGBUF_Init(&USART3_RxRingBuff,USART3_RxBuff,sizeof(USART3_RxBuff));
		#ifdef UART3_USE_TX_RINGBUFF
		USART3->CR1 |= USART_CR1_TXEIE;
		RINGBUF_Init(&USART3_TxRingBuff,USART3_TxBuff,sizeof(USART3_TxBuff));
		#endif
	
		/* preemption = GPS_PRIORITY, sub-priority = 1 */
		NVIC_SetPriority(USART3_IRQn, ((0x01<<3)| 2));
		NVIC_EnableIRQ(USART3_IRQn);
    USART3->CR1 |= USART_CR1_UE;                            // USART enable
}

void USART3_DeInit(void)
{
	RCC->APB1ENR &= ~RCC_APB1ENR_USART3EN;                     // enable clock for Alternate Function
	GPIOA->CRL   &= ~( GPIO_CRL_CNF3 | GPIO_CRL_MODE3);           // Clear PA3
}

void USART3_PutString (char *s)
{
   while(*s)
	{
		USART3_PutChar(*s++);
	}
}
   

uint8_t USART3_PutChar (uint8_t ch) 
{
	uint32_t timeOut = 100000000;
#ifndef UART3_USE_TX_RINGBUFF
  while (!(USART3->SR & USART_SR_TXE));
	USART3->DR = ch;
#else
	while((RINGBUF_GetFill(&USART3_TxRingBuff) >= (USART3_TxRingBuff.size - 1)) && timeOut--){}
	RINGBUF_Put(&USART3_TxRingBuff,ch);
	if((USART3->CR1 & USART_CR1_TXEIE) != USART_CR1_TXEIE)
	{
		USART3->CR1 |= USART_CR1_TXEIE;
	}
#endif
	return ch;
}


uint8_t USART3_GetChar (void) 
{
  while (!(USART3->SR & USART_SR_RXNE));
  return ((uint8_t)(USART3->DR & 0x1FF));
}


void USART3_IRQHandler(void) 
{
		uint8_t c;uint32_t s,even;
	if(USART3->SR & (USART_SR_RXNE | USART_SR_ORE))
	{	  
			c = (uint8_t)(USART3->DR & 0x1FF);
			if(uart3_parity == USART_Parity_Even){
				//c &= 0x7F;
				s = c;
				even = (s & 1) + (s>>1 & 1) + (s>>2 & 1) + (s>>3 & 1) + (s>>4 & 1) + (s>>5 & 1) + (s>>6 & 1);
				if(even & 1)
				{
					if(s & 0x80)
						RINGBUF_Put(&USART3_RxRingBuff,(c & 0x7F));
				}
				else
				{
					if((s & 0x80) == 0)
						RINGBUF_Put(&USART3_RxRingBuff,(c & 0x7F));
				}
			}
			else
				RINGBUF_Put(&USART3_RxRingBuff,c);
	}

	#ifdef UART3_USE_TX_RINGBUFF
	if(USART3->SR & USART_SR_TXE)
	{
		if(RINGBUF_Get(&USART3_TxRingBuff,&c) == 0)
		{
			if(uart3_parity == USART_Parity_Even){
				s = c;
				even = (s & 1) + (s>>1 & 1) + (s>>2 & 1) + (s>>3 & 1) + (s>>4 & 1) + (s>>5 & 1) + (s>>6 & 1);
				if(even & 1)
				{
					USART3->DR = c | 0x80;
				}
				else
				{
					USART3->DR = c & 0x7F;
				}
			}else{
				USART3->DR = c;
			}
		}
		else
		{
			USART3->CR1 &= (~USART_CR1_TXEIE);
		}
	}
	#endif
}






/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/




