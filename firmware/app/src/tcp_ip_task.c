
#include "ampm_gsm_common.h"
#include "ppp.h"
#include "resolv.h"
#include "resolver.h"
#include "lib/sys_tick.h"
#include "lib/sys_time.h"
#include "tcp_ip_task.h"

static sys_mbox_t mbox;

void	tcpip_thread (void *arg);
uint8_t vTcpIpTaskInit(void)
{
	return 0;
}

uint8_t vTcpIpTask(void)
{
	tcpip_thread(NULL);
 return 0;
}

void	tcpip_thread (void *arg)
{	
	struct tcpip_msg *msg;
	//while(1)
	{
		sys_arch_mbox_fetch(mbox, (void *)&msg,1);
		if(msg != NULL)
		{
			switch (msg->type) {
			case TCPIP_MSG_API:
				LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: API message %p\n", (void *)msg));
				api_msg_input(msg->msg.apimsg);
				break;
			case TCPIP_MSG_INPUT:
				break;
			case TCPIP_MSG_CALLBACK:
				LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: CALLBACK %p\n", (void *)msg));
				msg->msg.cb.f(msg->msg.cb.ctx);
				break;
			default:
				break;
			}
			memp_free(MEMP_TCPIP_MSG, msg);
		}
	}
}


void tcpip_apimsg(struct api_msg *apimsg)
{
  struct tcpip_msg *msg;
  msg = memp_malloc(MEMP_TCPIP_MSG);
  if (msg == NULL) {
    memp_free(MEMP_API_MSG, apimsg);
    return;
  }
  msg->type = TCPIP_MSG_API;
  msg->msg.apimsg = apimsg;
  sys_mbox_post(mbox, msg);
}

void tcpip_init(void (* initfunc)(void *), void *arg)
{
  mbox = sys_mbox_new();
  //sys_thread_new(tcpip_thread, NULL, TCPIP_THREAD_PRIO);
}
