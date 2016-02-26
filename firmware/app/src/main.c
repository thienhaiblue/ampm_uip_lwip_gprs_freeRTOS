
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
#include "ampm_ff/ampm_ff.h"
/* wolfSSL includes. */
#include "wolfssl/ssl.h"
#define main_GSM_GPRS_PRIORITY				( tskIDLE_PRIORITY + 1 )
/* The check task uses the sprintf function so requires a little more stack. */
#define main_GSM_GPRS_STACK_SIZE			( 128 )

#define MAIN_Info(...)  //DbgCfgPrintf(__VA_ARGS__)

const char *ca_cert = "\
-----BEGIN CERTIFICATE-----\r\n\
MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\r\n\
yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\r\n\
ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\r\n\
U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\r\n\
ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\r\n\
aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\r\n\
MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\r\n\
ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\r\n\
biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\r\n\
U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\r\n\
aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\r\n\
nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\r\n\
t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\r\n\
SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\r\n\
BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\r\n\
rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\r\n\
NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\r\n\
BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\r\n\
BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\r\n\
aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\r\n\
MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\r\n\
p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\r\n\
5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\r\n\
WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\r\n\
4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\r\n\
hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\r\n\
-----END CERTIFICATE-----\
";

const char *server_cert = "\
-----BEGIN CERTIFICATE-----\r\n\
MIIDWTCCAkGgAwIBAgIUWYCxn9tcU4UyrW1olimQCgU85BcwDQYJKoZIhvcNAQEL\r\n\
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\r\n\
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE2MDEyOTE4MjMz\r\n\
N1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\r\n\
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJwJA0KJ6EnDJv/uvm6C\r\n\
WTuUstTEebpaLzsjm/sjkjuJQdkcaotFlSAghhWGrX2QEzW+OqZ3d+61tWHuXBzn\r\n\
knFE1JxhsQN2xu9bFWGCXe8FFLg6n3vjH93JCG9JmAErCwzSjIm8xmX0nZFzVgk8\r\n\
AzMfqSQYZtguGJwKELGEZhMJrwmyM4v+DD+/fQbwyoxsp3tWQ9DmkR1dsvXfA2BA\r\n\
dkUsS3YC6jXRNimqGDAw+nllzWUM4TK4jQY1pNypVHKVth/15aSb3eLFJeWylv2M\r\n\
qnZDS3cdApiZ+ss+I6nHJPAMDkfQIPBsEnq/w1W4Umz5eRfuUZmz1fyfnF13U6f5\r\n\
YPcCAwEAAaNgMF4wHwYDVR0jBBgwFoAUkNH8iRp4miRwAJ9TS5ihy2CaDRMwHQYD\r\n\
VR0OBBYEFG+Y9RwubLZECJAORAIuIAlpsa2sMAwGA1UdEwEB/wQCMAAwDgYDVR0P\r\n\
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAEfFw3d97C/lu7eathAqIEI3Hh\r\n\
+hWNHwn5OFvQDpseGL8cL6Nilawvb9ol3x2VMSg0UJTz7K+M5PsWu64l58TyJT1o\r\n\
g5oAJJTFGXf9eIyWvr4X4bndCHttlICMmuUmoknY9sMRBb8GR15v00z3pWaXmi+O\r\n\
qVqnPMRstKo0wdBgUsYsvtz2TZl2yDo0p5X/9YqDfWyjDhNGQFLE0e3dgqzWS+qg\r\n\
QmcKOR0dbp/SRDhg12l7pJKznVfgxabTtaadH5ZKhQHHGx2z9BMM88qdr5ifJL2I\r\n\
RDRWzFfR2WEyGEauP9X27MZRyQEqVNLOkP574K2sONlnS7zyNgDO7m9FzpcB\r\n\
-----END CERTIFICATE-----\
";

const char *server_key = "\
-----BEGIN RSA PRIVATE KEY-----\r\n\
MIIEowIBAAKCAQEAnAkDQonoScMm/+6+boJZO5Sy1MR5ulovOyOb+yOSO4lB2Rxq\r\n\
i0WVICCGFYatfZATNb46pnd37rW1Ye5cHOeScUTUnGGxA3bG71sVYYJd7wUUuDqf\r\n\
e+Mf3ckIb0mYASsLDNKMibzGZfSdkXNWCTwDMx+pJBhm2C4YnAoQsYRmEwmvCbIz\r\n\
i/4MP799BvDKjGyne1ZD0OaRHV2y9d8DYEB2RSxLdgLqNdE2KaoYMDD6eWXNZQzh\r\n\
MriNBjWk3KlUcpW2H/XlpJvd4sUl5bKW/YyqdkNLdx0CmJn6yz4jqcck8AwOR9Ag\r\n\
8GwSer/DVbhSbPl5F+5RmbPV/J+cXXdTp/lg9wIDAQABAoIBAGcD/Eb9zYKFeUYX\r\n\
VVqYbFlNvxOB8+v4hp5A0EZqIa103Sh9/kmc0uu3DU9A72GqsUQWJ5qn3WKYTPwu\r\n\
5lme+awWiVgFl1x2GrkEJYWPEH0xmZBYA9tDBFLk2tC+gcCjrlP03hxBqaq+aRkS\r\n\
UIcgO+yQxpayFZQ8OrsqWgMZKlr+TYh+2BhEhJrHadjR2uDpTDzG1DJp/l/n0/fz\r\n\
2um9fAnbfIO64tkDQkoH2WsuKS3+e7EOYLhH7EW5AT6OWzssBEKajItLv1fj1RvK\r\n\
7V6uSBc4EvoZOsLDviYQt0O+cMfBBVF6V7ZvDr7Q9dgpyv7sCiqvfCiNwuqpqvB8\r\n\
hqaADMECgYEA0SQRn8biYoVsbX9BJAPXREKFn982aoQ3qkWBzbysRN4ovBCw0NWu\r\n\
R88nr4nullivuX8QlZrSB08/MveW0b/75iv7UZHjeujh+Wu2URi5sBwzharF8Fye\r\n\
6kok/BMn8viQ5dNuFUH9ZtqU6/hFUxpN61QENZ8sYQE3BDuvE4DvI98CgYEAvv7m\r\n\
mk6hU3GxJ57/1ZDpwG6LGkTc2h7l+DfnJPHXazkyztmlFQWBUSdCRqWBJwUkTlt0\r\n\
G6cvaWZe8Dqa7+K4G9X6EBMMw/yvVprhsE/21wJIIhWiRzMpeA7X5ft0WxYGGPA1\r\n\
Nt443p2jA/5J3rrhFLH9PRZ3xOzBbbuL2+ItpekCgYBIfOLq6SkwEqSpMxEl5Xro\r\n\
OtJLvjcDJj6Q8rRx1bIz0Hh37qUKTPWyB/fuXLVoQObvOT5LTDT9uZbjGHOa0ZsC\r\n\
hT3/YLxirnMcWxv+8b3yb2PgMXeXvtKJzBcTk6QmD8dRET43ef7Vdm9ldlC45AYb\r\n\
yawH1dqw6JXkDre439iDKwKBgH4cfxV6P8mCM3Au60wP4FhgZVbWC2G2rzBDcUsS\r\n\
qKOy49pzGS6VMPrtyjQtiebC3WMjvbmYnZShtKQ4fh9Q+zHeCrxcZ7xs9zVfFRA+\r\n\
7ISxjAF9eWY74PDWFDZV31FZbHNBAFIXT0OmoIG9gPchPAdXMxgH4tNTZLDY+hMY\r\n\
9/0pAoGBAJSjYTNb/nnJuoH7MZKir0GdznXytPUgzWItxpZDZo0fkgglU8aoa4sM\r\n\
7tSnqXJXHPVO1OfSvHqFp5+FWjjNX4hu4wbzUkbupHRAQXlskhcr70mMEDnHGKWo\r\n\
15y/1G/bdvpMRmsKj6vi5p1wxrE7wKc/joYDhDbT5epI0hk+puOB\r\n\
-----END RSA PRIVATE KEY-----\
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
	AMPM_FIL fil;
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
	
	ampm_f_init();
	if(ampm_f_open(&fil,"ca-cert.pem",AMPM_FA_READ) != FR_OK)
	{
		ampm_f_close(&fil);
		if(ampm_f_open(&fil,"ca-cert.pem",AMPM_FA_CREATE_ALWAYS) == FR_OK)
		{
			ampm_f_write(&fil,(char *)ca_cert,strlen(ca_cert),&i);
		}
	}
	ampm_f_close(&fil);
	if(ampm_f_open(&fil,"server-cert.pem",AMPM_FA_READ) != FR_OK)
	{
		ampm_f_close(&fil);
		if(ampm_f_open(&fil,"server-cert.pem",AMPM_FA_CREATE_ALWAYS) == FR_OK)
		{
			ampm_f_write(&fil,(char *)server_cert,strlen(server_cert),&i);
		}
	}
	ampm_f_close(&fil);
	if(ampm_f_open(&fil,"server-key.pem",AMPM_FA_READ) != FR_OK)
	{
		ampm_f_close(&fil);
		if(ampm_f_open(&fil,"server-key.pem",AMPM_FA_CREATE_ALWAYS) == FR_OK)
		{
			ampm_f_write(&fil,(char *)server_key,strlen(server_key),&i);
		}
	}
	ampm_f_close(&fil);
	
	
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


//void vAmpmNetTestTask2( void *pvParameters )
//{
//	int xClientSocket, newconn, size;
//	char *pt =  pvParameters;
//  struct sockaddr_in xConnection;
//	WOLFSSL* xWolfSSL_Object;
//	WOLFSSL_CTX* xWolfSSL_ClientContext = NULL;
//	BaseType_t lReturned;
//	uint32_t lBytes;
//	char cString[ 50 ];
//	uint8_t cReceivedString[60];
//	/* Initialise wolfSSL.  This must be done before any other wolfSSL functions
//	are called. */
//	wolfSSL_Init();
//	xWolfSSL_ClientContext = wolfSSL_CTX_new( wolfTLSv1_2_client_method() );
//	wolfSSL_CTX_load_verify_locations(xWolfSSL_ClientContext, "ca-cert.pem", 0);
//	wolfSSL_CTX_use_certificate_file(xWolfSSL_ClientContext, "server-cert.pem", SSL_FILETYPE_PEM);
//	wolfSSL_CTX_use_PrivateKey_file(xWolfSSL_ClientContext, "server-key.pem", SSL_FILETYPE_PEM);
//	/* bind to port 80 at any interface */
//  /* Set family and port for client socket. */
//	memset( ( void * ) &xConnection, 0x00, sizeof( struct sockaddr_in ) );
//	xConnection.sin_family = AF_INET;
//	xConnection.sin_addr.s_addr = inet_addr("52.32.95.12");
//	xConnection.sin_port = htons( 8883 );
//	/* create a TCP socket */
//  xClientSocket = socket(AF_INET, SOCK_STREAM, 0);
//	
//	connect(xClientSocket,(struct sockaddr *)&xConnection,sizeof(xConnection));
//	
//	/* The connect was successful.  Create a wolfSSL object to associate
//			with this connection. */
//	//xWolfSSL_Object = wolfSSL_new( xWolfSSL_ClientContext );
//	
//		if( xWolfSSL_Object != NULL )
//		{
//			/* Associate the created wolfSSL object with the connected
//			socket. */
//			lReturned = wolfSSL_set_fd( xWolfSSL_Object, xClientSocket );
//			configASSERT( lReturned == SSL_SUCCESS );

//			do
//			{
//				/* Create the string that is sent to the secure server. */
//				uint8_t cString[28] = { 0x10, 0x1A, 0x00, 0x04, 0x4D, 0x51, 0x54, 0x54, 0x04, 0x02, 0x00, 0x3C, 0x00, 0x0E, 0x4D, 0x51, 0x54, 0x54, 0x5F, 0x46, 0x58, 0x5F, 0x43, 0x6C, 0x69, 0x65, 0x6E, 0x74};
//				/* The next line is the secure equivalent of the standard
//				sockets call:
//				lReturned = send( xClientSocket, cString, strlen( cString ) + 1, 0 ); */
//				//lReturned = wolfSSL_write(xWolfSSL_Object, cString, sizeof(cString) + 1);


//				/* Short delay to prevent the messages streaming up the
//				console too quickly. */
//				vTaskDelay( 50 );
////				lBytes = wolfSSL_read(xWolfSSL_Object, cReceivedString, sizeof(cReceivedString));

////				/* Print the received characters. */
////				if (lBytes > 0)
////				{
////					printf("Received by the secure server: %s\r\n", cReceivedString);
////				}

//			} while( 1);
//		}
//		while(1);
//		wolfSSL_free( xWolfSSL_Object );
//}



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

