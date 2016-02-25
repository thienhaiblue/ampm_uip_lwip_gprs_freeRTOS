
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stm32f10x.h>
#include "uip.h"
#include "lwip/api.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api_msg.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip/sys.h"
#include "tcp_ip_task.h"
#include "dbg.h"

#include "uart1.h"
#include "stm32f10x_iwdg.h"
#include "lib/ringbuf.h"
#include "lib/tick.h"
#include "lib/sys_tick.h"
#include "rtc.h"
#include "lib/data_cmp.h"
#include "led.h"
#include "lib/sys_time.h"
#include "lib/encoding.h"
#include "lib/ampm_sprintf.h"
#include "ampm_gsm_main_task.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "adc_task.h"
#include "DTMF_app.h"

#define main_GSM_GPRS_PRIORITY				( tskIDLE_PRIORITY + 1 )
/* The check task uses the sprintf function so requires a little more stack. */
#define main_GSM_GPRS_STACK_SIZE			( 128 )

#define MAIN_Info(...)  //DbgCfgPrintf(__VA_ARGS__)

const char *sendfile = "\
-----BEGIN CERTIFICATE-----\n\
MIIEqjCCA5KgAwIBAgIJANmAOsPS9No3MA0GCSqGSIb3DQEBCwUAMIGUMQswCQYD\r\n\
VQQGEwJVUzEQMA4GA1UECAwHTW9udGFuYTEQMA4GA1UEBwwHQm96ZW1hbjERMA8G\r\n\
A1UECgwIU2F3dG9vdGgxEzARBgNVBAsMCkNvbnN1bHRpbmcxGDAWBgNVBAMMD3d3\r\n\
dy53b2xmc3NsLmNvbTEfMB0GCSqGSIb3DQEJARYQaW5mb0B3b2xmc3NsLmNvbTAe\r\n\
Fw0xNTA1MDcxODIxMDFaFw0xODAxMzExODIxMDFaMIGUMQswCQYDVQQGEwJVUzEQ\r\n\
MA4GA1UECAwHTW9udGFuYTEQMA4GA1UEBwwHQm96ZW1hbjERMA8GA1UECgwIU2F3\r\n\
dG9vdGgxEzARBgNVBAsMCkNvbnN1bHRpbmcxGDAWBgNVBAMMD3d3dy53b2xmc3Ns\r\n\
LmNvbTEfMB0GCSqGSIb3DQEJARYQaW5mb0B3b2xmc3NsLmNvbTCCASIwDQYJKoZI\r\n\
hvcNAQEBBQADggEPADCCAQoCggEBAL8Myi0Ush6EQlvNOB9K8k11EPG2NZ/fyn0D\r\n\
mNOs3gNm7irx2LB9bgdUCxCYIU2AyxIg58xP3kV9yXJ3MurKkLtpUhADL6jzlcXx\r\n\
i2JWG+9nb6QQQZWtCpvjpcCw0nB2UDBbqOgILHztp6J6jTgpHKzH7fJ8lbCVgn1J\r\n\
XDjNdyXvvYB1U5Q8PcpjW58VtdMdEy8Z0TzbdjrMuH3J5cLX2kBv2CHccxtCLVOc\r\n\
/hr8fat6Nj+Y3oR8BWfOahQ4h6nxjLVoy2h/cSAr9aBj9VYvoybSt2+xWhfXOJkI\r\n\
/pNYb/7DE0kIFgunTWcAUjFnI06Y7VFFHbkE2Qvs2CizS73tNnkCAwEAAaOB/DCB\r\n\
+TAdBgNVHQ4EFgQUJ45nEXTDJh0/7TNjs6TYHTDl6NUwgckGA1UdIwSBwTCBvoAU\r\n\
J45nEXTDJh0/7TNjs6TYHTDl6NWhgZqkgZcwgZQxCzAJBgNVBAYTAlVTMRAwDgYD\r\n\
VQQIDAdNb250YW5hMRAwDgYDVQQHDAdCb3plbWFuMREwDwYDVQQKDAhTYXd0b290\r\n\
aDETMBEGA1UECwwKQ29uc3VsdGluZzEYMBYGA1UEAwwPd3d3LndvbGZzc2wuY29t\r\n\
MR8wHQYJKoZIhvcNAQkBFhBpbmZvQHdvbGZzc2wuY29tggkA2YA6w9L02jcwDAYD\r\n\
VR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAeq9EO6pvU0KyM6pDX1Yw07mW\r\n\
C5pVWjkqC07kLvGVZsmGNoKNY3xNou5IugPHkNenxnRgSF8xovlePsOC4eUvQYGD\r\n\
KSV50VMAaTztCjA7QR2SoSyonSzjI4d54FVukahQ2kYvwiBQPitHlxSwfQS6RVHQ\r\n\
buFaokuEnE3NhQT5KDGCk7zHWUmRA+jfauRWrWrLHw035F6955/V7J08GCWb8S9Q\r\n\
fesxy/FjIp1X/POEIBrGB4eSJp4VGFkzBtz7sLZ2XfHBL8gvYpzA1t7rZXfzXKbD\r\n\
iCeWdbT0VM3/LSEulvAHc0vpk5KQ3mLZozusbiRfJ0qzlHD/MBfnfjKPZbd1WA==\r\n\
-----END CERTIFICATE-----\
";

#define SMS_TIME_CHECK 600

#define BOSS_PHONE_NUMBER	"0978779222"

#define EXTI_Line8       ((uint32_t)0x00100)  /* External interrupt line 8 */
SMS_LIST_TYPE smsUser;
CALL_LIST_TYPE callUser;
Timeout_Type tSendSmsTime;
RINGBUF dtmfRingBuf;
uint8_t dtmfBuf[64];
uint8_t pdu2uniBuf[256];
uint8_t replySmsBuf[256];
uint8_t smsLen,c;

void vAmpmNetTestTask1( void *pvParameters );
void vAmpmNetTestTask( void *pvParameters );
void vGSM_GPRS_Task(void *arg);
void SysSleep(void);
void SysInit(void);
void PowerWarningTask(void);
void TIMER2_Init(uint32_t pclk);

enum{
	SYS_SLEEP_MODE,
	SYS_RUN_MODE
}sysStatus = SYS_RUN_MODE;


void HardFault_Handler(void)
{
	//NVIC_SystemReset();
	while(1);
}

void SysInit(void)
{
	uint32_t i,j,len;
	uint8_t buf[32];
	//AMPM_FIL fsFile;
	__enable_irq();
	SystemInit();
	//TICK_Init(1);
	RCC->APB2ENR = (RCC_APB2ENR_AFIOEN |  /*enable clock for Alternate Function*/
								 RCC_APB2ENR_IOPAEN |  /* enable clock for GPIOA*/
								 RCC_APB2ENR_IOPBEN |	/*enable clock for GPIOB*/
								 RCC_APB2ENR_IOPCEN |/*enable clock for GPIOc*/ 									 
								 RCC_APB2ENR_IOPDEN	
								);   
	AFIO->MAPR = AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;   // clear used pin
	AFIO->EXTICR[1] &= ~AFIO_EXTICR1_EXTI1;   // clear used pin
	AFIO->EXTICR[2] &= ~AFIO_EXTICR1_EXTI2;   // clear used pin
	AFIO->EXTICR[3] &= ~AFIO_EXTICR1_EXTI3;    // clear used pin


	/*Use PA8 is Modem RI*/
	EXTI->PR = EXTI_Line8;
	AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI8_PA;           // set pin to use
	EXTI->IMR       &= ~EXTI_Line8;             // mask interrupt
	EXTI->EMR       &= ~EXTI_Line8;             // mask event
	EXTI->IMR       |= 	EXTI_Line8;             // mask interrupt
	EXTI->EMR       |= EXTI_Line8;             // mask event
	EXTI->RTSR      &= ~EXTI_Line8;            // clear rising edge
	EXTI->FTSR      |= EXTI_Line8;            // set falling edge
	/* preemption = MODEMRI_PRIORITY, sub-priority = 1 */
	NVIC_SetPriority(EXTI9_5_IRQn, ((0x01<<3)| 3));
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	
	
	GSM_POWER_PIN_SET_OUTPUT;
	MODEM_MOSFET_On();
	RTC_Init();
	LedInit();
	TIMER2_Init(SystemCoreClock);
	ADC_TaskInit();
/*GSM Init*/
 	USART1_Init(SystemCoreClock,__USART1_BAUDRATE);
}


int main(void)
{
	SysInit();
	sys_init();
	mem_init();
	memp_init();
	pbuf_init();
	tcpip_init( NULL, NULL );
/* Start the tasks defined within this file/specific to this demo. */
	xTaskCreate( vGSM_GPRS_Task, "GSM_GPRS_TASK", 256 , NULL,tskIDLE_PRIORITY + 1 , NULL );
  xTaskCreate( vAmpmNetTestTask, "ampm net test", 128 , "\r\nThienhaiblue->",tskIDLE_PRIORITY + 1 , NULL );
	xTaskCreate( vAmpmNetTestTask1, "ampm net test1", 128 , "\r\nTony->",tskIDLE_PRIORITY + 1 , NULL );
	/* Start the scheduler. */
	vTaskStartScheduler();
	while(1)
	{
		
		
	}
	return 0;
}


void vAmpmNetTestTask( void *pvParameters )
{
	char *pt =  pvParameters;
	struct netconn *ampm_net;
	struct ip_addr remote_addr;
  u16_t remote_port = 1880;
	struct netbuf *data;
  err_t err;
	((uint8_t *)&remote_addr)[0] = 118;
	((uint8_t *)&remote_addr)[1] = 69;
	((uint8_t *)&remote_addr)[2] = 60;
	((uint8_t *)&remote_addr)[3] = 174;
	ampm_net = netconn_new(NETCONN_TCP);
	err = netconn_connect(ampm_net, &remote_addr, remote_port);
	err = netconn_write(ampm_net, pt, strlen(pt), NETCONN_COPY);
	while(1)
	{
		data = netconn_recv(ampm_net);
		if(data)
		{
			//netconn_write(ampm_net, (uint8_t *)sendfile, strlen(sendfile), NETCONN_COPY);
			netconn_write(ampm_net, data->p->payload, data->p->len, NETCONN_COPY);
			netbuf_delete(data);
		}
	}
}

void vAmpmNetTestTask1( void *pvParameters )
{
	int xClientSocket, newconn, size;
	char *pt =  pvParameters;
  struct sockaddr_in xConnection;
	uint32_t lBytes;
  /* Set family and port for client socket. */
	memset( ( void * ) &xConnection, 0x00, sizeof( struct sockaddr_in ) );
	xConnection.sin_family = AF_INET;
	xConnection.sin_addr.s_addr = inet_addr("118.69.60.174");
	xConnection.sin_port = htons( 1880 );
	/* create a TCP socket */
  xClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	connect(xClientSocket,(struct sockaddr *)&xConnection,sizeof(xConnection));
	send( xClientSocket, pt, strlen(pt),0);
	while(1){
		lBytes = recv( xClientSocket, replySmsBuf, sizeof(replySmsBuf),0);
		if(lBytes)
		{
			send( xClientSocket, replySmsBuf, lBytes,0);
		}
	}
}



void vGSM_GPRS_Task(void *arg)
{
		Timeout_Type tApp_100MS;
		uint32_t timer100msCnt = 0;
		InitTimeout(&tApp_100MS,SYSTICK_TIME_MS(100));
		AMPM_GSM_Init("internet","mms","mms",vTcpIpTask,vTcpIpTaskInit);
		while(1)
		{
			AMPM_GSM_MainTask();
			
			//Call and dtmf test
			if(incomingCall.state == CALL_INCOMING)
			{
					if(ampm_GotIncomingNumberFlag)
					{
						if(Ampm_ComparePhoneNumber((char *)ampm_IncomingCallPhoneNo,BOSS_PHONE_NUMBER))
						{
							incomingCall.dtmf = &dtmfRingBuf;
							Ampm_VoiceCallSetAction(&incomingCall,CALL_PICKUP_WHEN_INCOMING);
						}
						else
						{
							Ampm_VoiceCallCancel(&incomingCall);
						}		
					}
			}
			
			if(incomingCall.state == CALL_ACTIVE)
			{
				if(RINGBUF_Get(&dtmfRingBuf,&c) == 0)
				{
					DbgCfgPrintf("\r\nDTMF:%c",c);
					if(c == '1')
					{
						Ampm_VoiceCallCancel(&incomingCall);
						Ampm_VoiceCallSetup(&callUser,(uint8_t *)BOSS_PHONE_NUMBER,&dtmfRingBuf,CALL_HANGUP_WHEN_ALERTING,NULL,6,60,1);
						//Ampm_VoiceCallSetup(&callUser,BOSS_PHONE_NUMBER,&dtmfRingBuf,CALL_HANGUP_WHEN_ACTIVE,NULL,6,60,1);
					}
				}
			}
			//SMS test
			//Send Sms with interval = 3600s
			if(CheckTimeout(&tSendSmsTime) == SYSTICK_TIMEOUT)
			{
				InitTimeout(&tSendSmsTime,SYSTICK_TIME_SEC(3600));
				smsLen =	sprintf((char *)pdu2uniBuf,"Xin chào!\n");
				smsLen = utf8s_to_ucs2s((int16_t *)replySmsBuf,pdu2uniBuf);
				big2litel_endian((uint16_t *)replySmsBuf,unilen((uint16_t *)replySmsBuf));
				smsLen *= 2;
				Ampm_Sms_SendMsg(&smsUser,(uint8_t *)BOSS_PHONE_NUMBER,(uint8_t *)replySmsBuf,smsLen,SMS_PDU16_MODE,30/*interval resend*/,3/*resend times*/);
			}
			// 0.1 sec process 
			if(CheckTimeout(&tApp_100MS) == SYSTICK_TIMEOUT)
			{
				InitTimeout(&tApp_100MS,SYSTICK_TIME_MS(100));
				timer100msCnt += 100;
				CtrLed(timer100msCnt);
			}
		}
}



//recv SMS function
void Ampm_MainSmsRecvCallback(uint8_t *buf)
{
	//CMD_CfgParse((char *)buf, 0,0);
	Ampm_Sms_SendMsg(&smsUser,(uint8_t *)BOSS_PHONE_NUMBER,buf,strlen((char *)buf),SMS_TEXT_MODE,30,3);
}


void EXTI9_5_IRQHandler(void)
{
	if(EXTI->PR & EXTI_Line8)
	{
		Ampm_SetRinging();
		EXTI->PR = EXTI_Line8;
	}
}

uint32_t timer2Cnt = 0;

void TIM2_IRQHandler(void)
{
	if(TIM2->SR & 1)
	{
		TIM2->SR = (uint16_t)~0x0001;
		timer2Cnt++;

		if(timer2Cnt % 50 == 0)
			CtrLed(timer2Cnt);
		
		DTMF_Task();
	}
}

void TIMER2_Init(uint32_t pclk)
{
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;                     // enable clock for TIM2
    TIM2->PSC = (uint16_t)(pclk/1000000) - 1;            // set prescaler
    TIM2->ARR = (uint16_t)(1000000*TIMER_PERIOD/1000 - 1);  //1ms          // set auto-reload
    TIM2->CR1 = 0;                                          // reset command register 1
    TIM2->CR2 = 0;                                          // reset command register 2
		TIM2->DIER = 1;                             
		NVIC_SetPriority (TIM2_IRQn,((0x01<<3) | TMR2_PRIORITY));
		NVIC_EnableIRQ(TIM2_IRQn);// enable interrupt
    TIM2->CR1 |= 1;                              // enable timer
}

