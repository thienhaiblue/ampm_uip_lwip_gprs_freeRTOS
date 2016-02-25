
/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "ampm_gsm_common.h"
#include "ampm_gsm_sms.h"
#include "ampm_gsm_call.h"
#include "lib/ampm_list.h"
#include "sms/pdu.h"
#include "at_command_parser.h"
#include "dtmf/dtmf_app.h"

LIST(callList);

CALL_LIST_TYPE incomingCall;
CALL_LIST_TYPE *voiceCall;
uint8_t callTaskFailCnt = 0;
uint8_t listOfCurrentCall[128];
uint8_t gotListOfCurrentCall = 0;
uint8_t callListIsEmpty = 0;

Timeout_Type callTimeout,callAlertingTimeout;
Timeout_Type tCallDelayTimeout;

uint16_t ceerId = 0;
uint8_t gotCeerId = 0;
uint8_t callDelayTime = CALL_DELAY_TIME;
uint8_t callTaskIsRunningCnt = 0;

uint8_t ampm_GotIncomingNumberFlag = 0;
uint8_t ampm_IncomingCallPhoneNo[16];
uint8_t Ampm_GsmCallDialEndCallback(AMPM_CMD_PHASE_TYPE phase);
uint8_t Ampm_GsmCallDialStartCallback(uint8_t *buf);
uint8_t Ampm_GsmCHLD_1X_Callback(uint8_t *buf);
uint32_t Ampm_GetListOfCurrentCall(uint16_t cnt,uint8_t c);
uint8_t Ampm_GsmVoiceCallCheckStateEndCallback(AMPM_CMD_PHASE_TYPE phase);
uint32_t Ampm_GetCeer(uint16_t cnt,uint8_t c);
uint8_t Ampm_GsmVoiceCallCheckCeerEndCallback(AMPM_CMD_PHASE_TYPE phase);
uint8_t Ampm_GsmCallEndCallback(AMPM_CMD_PHASE_TYPE phase);
uint8_t Ampm_GsmATM1EndCallback(AMPM_CMD_PHASE_TYPE phase);

const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_ATA = {"ATA\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1

const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CLVL = {"AT+CLVL=100\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_ATM = {"ATM1\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_ATL = {"ATL3\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1

const AMPM_GSM_AT_CMD_PACKET_TYPE callDial = {NULL,Ampm_GsmCallDialStartCallback,NULL,NULL,"OK","ERROR"};//delay = 500,timeout = 3000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CLCC = {"AT+CLCC\r",NULL,"+CLCC: ",Ampm_GetListOfCurrentCall,"OK","AT+CLCC\r\r\nOK"};//delay = 100,timeout = 3000,retry = 1

const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CEER = {"AT+CEER\r",NULL,"+CEER: ",Ampm_GetCeer,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CHLD_1X = {NULL,Ampm_GsmCHLD_1X_Callback,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_end	= {
		NULL,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmCallEndCallback,1,1000,100};

const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_AT_CHLD_1X = {
		(void *)&voiceCallSendCmdProcess_end,(void *)&ampmAtCmd_AT_CHLD_1X,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,3,1000,100};

const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_AT_CHLD = {
		(void *)&voiceCallSendCmdProcess_end,(void *)&ampmAtCmd_AT_CHLD,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,3,1000,100};

const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_ATH = {
		(void *)&voiceCallSendCmdProcess_end,(void *)&ampmAtCmd_ATH,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,3,1000,100};

		
const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_AT_CEER	= {
		(void *)&voiceCallSendCmdProcess_end,(void *)&ampmAtCmd_AT_CEER,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmVoiceCallCheckCeerEndCallback,1,3000,500};		
		
		
const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_AT_CLCC	= {
		(void *)&voiceCallSendCmdProcess_AT_CLCC,(void *)&ampmAtCmd_AT_CLCC,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmVoiceCallCheckStateEndCallback,1,3000,100};		

		
const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_ATD	= {
		(void *)&voiceCallSendCmdProcess_AT_CLCC,(void *)&callDial,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmCallDialEndCallback,0,5000,1000};		

//voice settup
const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_ATM = {
		(void *)&voiceCallSendCmdProcess_ATD,(void *)&ampmAtCmd_ATM,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmATM1EndCallback,1,1000,100};

const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_CLVL	= {
		(void *)&voiceCallSendCmdProcess_ATM,(void *)&ampmAtCmd_AT_CLVL,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};
		
const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_ATL	= {
		(void *)&voiceCallSendCmdProcess_CLVL,(void *)&ampmAtCmd_ATL,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};


const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_Start	= {
		(void *)&voiceCallSendCmdProcess_ATL,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};

//Incomming call
		const AMPM_CMD_PROCESS_TYPE voiceCallSendCmdProcess_ATA	= {
		(void *)&voiceCallSendCmdProcess_AT_CLCC,(void *)&ampmAtCmd_ATA,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,3,1000,100};


		
void Ampm_CallTask_Init(void (*callback)(uint8_t *buf))
{
	list_init(callList);
	callDelayTime = CALL_DELAY_TIME;
}

uint8_t Ampm_VoiceCallCheckList_IsEmpty(void)
{
	if(list_length(callList))
		return 0;
	return 1;
}

void Ampm_VoiceCallSetAction(CALL_LIST_TYPE *call,CALL_ACTION action)
{
	if(call->action == CALL_NONE)
	{
		call->action = action;
	}
}

void Ampm_VoiceCallStartRecvCall(void)
{
	incomingCall.phone = NULL;
	incomingCall.dtmf = NULL;
	incomingCall.action = CALL_NONE;
	incomingCall.action_timeout = 0;
	incomingCall.action_callback = NULL;
	incomingCall.index = 1;
	incomingCall.response = CALL_RESPONSE_NONE;
	incomingCall.timeout = 60;
	incomingCall.timeoutReload = 60;
	incomingCall.state = CALL_START;
	incomingCall.tryNum = 1;
}

void Ampm_VoiceCallAdd(CALL_LIST_TYPE *call)
{
	list_add(callList, call);
}

void Ampm_VoiceCallCancel(CALL_LIST_TYPE *call)
{
	call->action = CALL_HANGUP_NOW;
	if(call != &incomingCall)
	{
		list_remove(callList,call);
	}
}

uint16_t Ampm_VoiceCallSetup(CALL_LIST_TYPE *call,
uint8_t *phone,
RINGBUF *dtmf,
CALL_ACTION action,
void (*callback)(void),
uint8_t action_timeout,
uint16_t timeout,uint8_t tryNum
)
{
	call->phone = phone;
	call->dtmf = dtmf;
	call->action = action;
	call->action_timeout = action_timeout;
	call->action_callback = callback;
	call->index = 1;
	call->response = CALL_RESPONSE_NONE;
	call->timeout = 0;
	call->timeoutReload = timeout;
	call->state = CALL_START;
	call->tryNum = tryNum;
	Ampm_VoiceCallAdd(call);
	return 0;
}

uint8_t Ampm_CallTaskPeriodic_1Sec(void)
{
	CALL_LIST_TYPE *call;
	if(callTaskIsRunningCnt)
	{
		callTaskIsRunningCnt--;
	}
	else
	{
		incomingCall.state = CALL_ENDED;
	}
	
	if(incomingCall.timeout) incomingCall.timeout--;
	
	if(list_length(callList) == 0)
			return 0;
	call = list_head(callList);
	while(call != NULL)
	{
		if(call->timeout)
		{
			call->timeout--;
		}

		call = list_item_next(call);
	}
	
	return 1;
}

uint8_t Ampm_CallTask(void)
{
	
	static CALL_LIST_TYPE *call = NULL;
	
	callTaskIsRunningCnt = 1;
	if(Ampm_SendAtCheck_IsEmpty() 
	&& Ampm_CmdTask_IsIdle(ampm_GSM_CmdPhase)
	&& (CheckTimeout(&tCallDelayTimeout) == 0)
	)
	{
		if(incomingCall.state == CALL_START)
		{
			voiceCall = &incomingCall;
			Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_Start);
			return 0;
		}
		
		if(list_length(callList))
		{
			call = list_head(callList);
			while(call != NULL)
			{
					if(call->response != CALL_RESPONSE_SUCCESS
					&& call->phone != NULL 
					&& call->tryNum
					&& call->action != CALL_HANGUP_NOW
					&& call->action != CALL_ENDED
				)
				{
					//if(call->timeout == 0)
					{
						call->timeout = call->timeoutReload;
						if(call->tryNum){call->tryNum--;}
						call->state = CALL_START;
						voiceCall = call;
						callDelayTime = CALL_DELAY_TIME;
						InitTimeout(&tCallDelayTimeout,SYSTICK_TIME_SEC(callDelayTime));						
						Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_Start);
						return 0;
					}
					//else
//					{
//						call = list_item_next(call);
//					}
				}
				else
				{
					call->state = CALL_ENDED;
					list_remove(callList,call);
					call = list_item_next(call);
				}
			}
		}
		else
		{
				return 1;
		}
	}
	return 0;
}

uint8_t Ampm_GsmVoiceCallCheckCeerEndCallback(AMPM_CMD_PHASE_TYPE phase)
{
	
	if(phase == AMPM_CMD_OK)
	{
		if(gotCeerId && voiceCall->response == CALL_RESPONSE_NONE)
		{
			if(ceerId == 16)
			{
				voiceCall->response = CALL_RESPONSE_NORMAL_CALL_CLEARING;
			}
			else if(ceerId == 17)
			{
				voiceCall->response = CALL_RESPONSE_BUSY;
			}
			else if(ceerId == 19)
			{
				voiceCall->response = CALL_RESPONSE_ALERTING_BUT_NO_ANSWER;
			}
			else if(ceerId == 31)
			{
				voiceCall->response = CALL_RESPONSE_NO_CARRIER;
			}
			else
			{
				voiceCall->response = CALL_RESPONSE_NO_CARRIER;
			}
		}
	}
	DTMF_DeInit();
	voiceCall->state = CALL_ENDED;
	return 1;
}


uint8_t Ampm_GsmVoiceCallCheckStateEndCallback(AMPM_CMD_PHASE_TYPE phase)
{
	uint32_t idx,dir,stat = 0,mode,mpty;
	uint8_t c;
	char phoneNumber[32];
	if(voiceCall->timeout == 0)
	{
		voiceCall->response = CALL_RESPONSE_TIMEOUT;
		DTMF_DeInit();
		Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD);
		return 1;
	}
	
	while(RINGBUF_Get(&DTMF_ringBuff,&c) == 0)
	{
		if(voiceCall->dtmf)
			RINGBUF_Put(voiceCall->dtmf,c);
	}
	
	
	if(phase == AMPM_CMD_OK && gotListOfCurrentCall)
	{
		//pt = strstr((char *)listOfCurrentCall,"+CLCC: ");
		//if(pt)
		callListIsEmpty = 0;
		{
			sscanf((char *)listOfCurrentCall,"%d,%d,%d,%d,%d,\"%[^\", :\t\n\r]",&idx,&dir,&stat,&mode,&mpty,phoneNumber);
			if(voiceCall == &incomingCall)
			{
				memcpy(ampm_IncomingCallPhoneNo,phoneNumber,sizeof(phoneNumber));
				ampm_GotIncomingNumberFlag = 1;
			}
			//if(strstr(phoneNumber,(char *)voiceCall->phone) != NULL)
			{
				voiceCall->state = (CALL_STATE_RESPONSE)(stat + 1);
				voiceCall->index = idx;
				switch(voiceCall->state)
				{
					case CALL_START:
						
					break;
					case CALL_ACTIVE:
						if(DEMF_Enable == 0)
						{
							DTMF_Init();
						}
					break;
					case CALL_HELD:
						
					break;
					case CALL_DIALING:
						InitTimeout(&callTimeout,SYSTICK_TIME_SEC(voiceCall->action_timeout));
						InitTimeout(&callAlertingTimeout,SYSTICK_TIME_SEC(voiceCall->action_timeout));
					break;
					case CALL_ALERTING:
						InitTimeout(&callTimeout,SYSTICK_TIME_SEC(voiceCall->action_timeout));
					break;
					case CALL_INCOMING:
						if(voiceCall != &incomingCall)
						{
							voiceCall->state = CALL_ENDED;
							voiceCall->response = CALL_RESPONSE_UNSUCCESS;
							voiceCall = &incomingCall;
						}
					break;
					case CALL_WAITING:
						
					break;
					case CALL_ENDED:
						Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD_1X);
						return 1;
					break;
					default:
						
					break;
				}
			}
//			else
//			{
//				Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD);
//				voiceCall->response = CALL_RESPONSE_UNSUCCESS;
//			}
		}
	}
	else if(gotListOfCurrentCall == 0 && callListIsEmpty)
	{
		Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CEER);
		return 1;
	}
	
	switch(voiceCall->action)
	{
		case CALL_NONE:
			
		break;
		case CALL_HANGUP_WHEN_ALERTING:
			if((voiceCall->state == CALL_ALERTING && CheckTimeout(&callAlertingTimeout) == SYSTICK_TIMEOUT)
			||	voiceCall->state == CALL_ACTIVE
			||	voiceCall->state == CALL_INCOMING
			)
			{
				Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD_1X);
				voiceCall->response = CALL_RESPONSE_SUCCESS;
			}
		break;
		case CALL_HANGUP_WHEN_ACTIVE:
			if(voiceCall->state == CALL_ACTIVE)
			{
				Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD_1X);
				if(voiceCall->action_callback != NULL)
				{
					voiceCall->action_callback();
				}
				voiceCall->response = CALL_RESPONSE_SUCCESS;
			}
		break;
		case CALL_HANGUP_AFTER_ACTIVE_action_timeout:
			if(voiceCall->state == CALL_ALERTING && CheckTimeout(&callTimeout) == SYSTICK_TIMEOUT)
			{
				Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD_1X);
				voiceCall->response = CALL_RESPONSE_SUCCESS;
			}
		break;
		case CALL_PICKUP_WHEN_INCOMING:
			if(voiceCall->state == CALL_INCOMING)
			{
				Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_ATA);
			}
		break;
		case CALL_HANGUP_NOW:
			if(gotListOfCurrentCall || voiceCall->state == CALL_ACTIVE)
			{
				Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD_1X);
			}
			else
			{
				Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD);
			}
			voiceCall->response = CALL_RESPONSE_CANCEL_BY_SOFTWARE;
		break;
		default:
		break;
	}
	gotListOfCurrentCall = 0;
	return 1;
}
uint8_t Ampm_GsmCallEndCallback(AMPM_CMD_PHASE_TYPE phase)
{
	InitTimeout(&tCallDelayTimeout,SYSTICK_TIME_SEC(callDelayTime));
	DTMF_DeInit();
	voiceCall->state = CALL_ENDED;
	return 1;
}


uint8_t Ampm_GsmATM1EndCallback(AMPM_CMD_PHASE_TYPE phase)
{
	if(incomingCall.state == CALL_START)
	{
		voiceCall = &incomingCall;
		Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CLCC);
	}
	return 1;
}

uint8_t Ampm_GsmCallDialEndCallback(AMPM_CMD_PHASE_TYPE phase)
{
	if(phase == AMPM_CMD_ERROR || voiceCall->action == CALL_HANGUP_NOW)
	{
		if(phase == AMPM_CMD_ERROR)
		{
			callTaskFailCnt++;
			callDelayTime = 10;
			voiceCall->response = CALL_RESPONSE_UNSUCCESS;
			Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD_1X);
		}
		else
		{
			voiceCall->response = CALL_RESPONSE_CANCEL_BY_SOFTWARE;
			Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD_1X);
		}
	}
	gotListOfCurrentCall = 0;
	callListIsEmpty = 0;
	InitTimeout(&callTimeout,SYSTICK_TIME_SEC(voiceCall->action_timeout));
	return 1;

}
uint8_t Ampm_GsmCHLD_1X_Callback(uint8_t *buf)
{
	if(voiceCall)
	{
		voiceCall->state = CALL_ENDED;
		ampm_sprintf((char *)buf,"AT+CHLD=1%d\r",voiceCall->index);
	}
	else
	{
		Ampm_SendAtCmdNow(&voiceCallSendCmdProcess_AT_CHLD);
	}
	DTMF_DeInit();
	return 1;
}
uint8_t Ampm_GsmCallDialStartCallback(uint8_t *buf)
{
	if(voiceCall->action != CALL_HANGUP_NOW)
	{
		ampm_sprintf((char *)buf,"ATD%s;\r",voiceCall->phone);
	}
	else
	{
		ampm_sprintf((char *)buf,"AT\r");
	}
	return 1;
}


uint32_t Ampm_CallListSetEmpty(uint16_t cnt,uint8_t c)
{
	callListIsEmpty = 1;
	return 0;
}
uint32_t Ampm_GetCeer(uint16_t cnt,uint8_t c)
{
	static uint8_t getCeerFlag = 0;
	if(cnt == 0){
		ceerId = 0;
		getCeerFlag = 0;
	}

	if(getCeerFlag)
	{
		if(c >= '0' && c <= '9')
		{
			ceerId *= 10;
			ceerId += c - '0';
		}
		else
		{
			gotCeerId = 1;
			return 0;
		}
	}
	
	if(c == ',')
	{
		if(getCeerFlag == 0)
		{
			ceerId = 0;
			getCeerFlag = 1;
		}
		else
		{
			gotCeerId = 1;
			return 0;
		}
	}
	
	if(c == '\r' || c == '\n' || cnt >= 128)
	{
		gotCeerId = 1;
		return 0;
	}
 return 0xff;
}
/*
[+CLCC: <idx>, <dir>, <stat>, <mode>, <mpty>[, <number>, <type>[, <alpha>]]]
*/
uint32_t Ampm_GetListOfCurrentCall(uint16_t cnt,uint8_t c)
{
	static uint8_t len = 0;
	if(cnt == 0){
		len = 0;
	}
	 listOfCurrentCall[len] = c;
	 len++;
	 listOfCurrentCall[len] = '\0';
	if(c == '\r' || c == '\n' || len >= (sizeof(listOfCurrentCall) - 1))
	{
		gotListOfCurrentCall = 1;
		return 0;
	}
 return 0xff;
}


uint8_t Ampm_ComparePhoneNumber_1(char* phone1, char* phone2,uint8_t digitThreshold)
{
	uint8_t i = strlen(phone1);
	uint8_t j = strlen(phone2);
	uint8_t l1;
	uint8_t l2;
	uint8_t minL;
	uint8_t count = 0;
	
	l1 = i;
	if (phone1[0] == '+')
	{
		if (i > 4)
		{
			l1 = i - 4;
		}
	}
	else if (phone1[0] == '0')
	{
		if (i > 1)
		{
			l1 = i - 1;
		}
	}
	
	l2 = j;
	if (phone2[0] == '+')
	{
		if (j > 4)
		{
			l2 = j - 4;
		}
	}
	else if (phone2[0] == '0')
	{
		if (j > 1)
		{
			l2 = j - 1;
		}
	}
	
	minL = l1;
	if (l2 < l1)
	{
		minL = l2;
	}
	
	while ((i != 0) && (j != 0))
	{
		i--;
		j--;
		if (phone1[i] == phone2[j])
		{
			count++;
		}
		else
		{
			break;
		}
	}
	
	if ((count >= digitThreshold) && (count >= minL))
	{
		return SAME_NUMBER;
	}
	
	return DIFFERENT_NUMBER;
}


uint8_t Ampm_ComparePhoneNumber(char* phone1, char* phone2)
{
	uint8_t i = strlen(phone1);
	uint8_t j = strlen(phone2);
	uint8_t l1;
	uint8_t l2;
	uint8_t minL;
	uint8_t count = 0;
	
	l1 = i;
	if (phone1[0] == '+')
	{
		if (i > 4)
		{
			l1 = i - 4;
		}
	}
	else if (phone1[0] == '0')
	{
		if (i > 1)
		{
			l1 = i - 1;
		}
	}
	
	l2 = j;
	if (phone2[0] == '+')
	{
		if (j > 4)
		{
			l2 = j - 4;
		}
	}
	else if (phone2[0] == '0')
	{
		if (j > 1)
		{
			l2 = j - 1;
		}
	}
	
	minL = l1;
	if (l2 < l1)
	{
		minL = l2;
	}
	
	while ((i != 0) && (j != 0))
	{
		i--;
		j--;
		if (phone1[i] == phone2[j])
		{
			count++;
		}
		else
		{
			break;
		}
	}
	
	if ((count >= PHONE_DIGIT_THRESHOLD) && (count >= minL))
	{
		return SAME_NUMBER;
	}
	
	return DIFFERENT_NUMBER;
}




