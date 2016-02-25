/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/

#ifndef __AMPM_GSM_CALL_H__
#define __AMPM_GSM_CALL_H__
#include <stdint.h>
#include "lib/ringbuf.h"

#define PHONE_NO_LENGTH									16
#define SAME_NUMBER											1
#define DIFFERENT_NUMBER								0
#define PHONE_DIGIT_THRESHOLD						6
#define CALL_DELAY_TIME				3

typedef enum{
	CALL_RESPONSE_NONE = 0,
	CALL_RESPONSE_TIMEOUT,
	CALL_RESPONSE_MISS,
	CALL_RESPONSE_BUSY,
	CALL_RESPONSE_ALERTING_BUT_NO_ANSWER,
	CALL_RESPONSE_NO_CARRIER,
	CALL_RESPONSE_UNSUCCESS,
	CALL_RESPONSE_NORMAL_CALL_CLEARING,
	CALL_RESPONSE_CANCEL_BY_SOFTWARE,
	CALL_RESPONSE_SUCCESS
}CALL_RESPONSE;

typedef enum{
	CALL_START = 0,
	CALL_ACTIVE,
	CALL_HELD,
	CALL_DIALING,
	CALL_ALERTING,
	CALL_INCOMING,
	CALL_WAITING,
	CALL_ENDED
}CALL_STATE_RESPONSE;

typedef enum{
	CALL_NONE = 0,
	CALL_HANGUP_WHEN_ALERTING,
	CALL_HANGUP_WHEN_ACTIVE,
	CALL_HANGUP_AFTER_ACTIVE_action_timeout,
	CALL_PICKUP_WHEN_INCOMING,
	CALL_HANGUP_NOW
}CALL_ACTION;

typedef struct{
	struct CALL_LIST_TYPE *next;
	uint8_t 	*phone;
	uint8_t index;
	CALL_STATE_RESPONSE state;
	uint16_t 	timeout; //second
	uint16_t timeoutReload;
	uint8_t 	tryNum;
	RINGBUF	*dtmf;
	CALL_RESPONSE response;
	CALL_ACTION	action;
	void (*action_callback)(void);
	uint8_t action_timeout;//sec
}CALL_LIST_TYPE;

extern CALL_LIST_TYPE incomingCall;
extern uint8_t ampm_IncomingCallPhoneNo[16];
extern uint8_t ampm_GotIncomingNumberFlag;
uint8_t Ampm_CallTask(void);
uint8_t Ampm_ComparePhoneNumber_1(char* phone1, char* phone2,uint8_t digitThreshold);
uint8_t Ampm_ComparePhoneNumber(char* phone1, char* phone2);
void Ampm_CallTask_Init(void (*callback)(uint8_t *buf));
uint8_t Ampm_CallTaskPeriodic_1Sec(void);
uint16_t Ampm_VoiceCallSetup(CALL_LIST_TYPE *call,
uint8_t *phone,
RINGBUF *dtmf,
CALL_ACTION action,
void (*callback)(void),
uint8_t action_timeout,
uint16_t timeout,uint8_t tryNum
);
uint8_t Ampm_VoiceCallCheckList_IsEmpty(void);
uint32_t Ampm_CallListSetEmpty(uint16_t cnt,uint8_t c);
void Ampm_VoiceCallCancel(CALL_LIST_TYPE *call);
void Ampm_VoiceCallStartRecvCall(void);
void Ampm_VoiceCallCancel(CALL_LIST_TYPE *call);
void Ampm_VoiceCallSetAction(CALL_LIST_TYPE *call,CALL_ACTION action);
#endif





