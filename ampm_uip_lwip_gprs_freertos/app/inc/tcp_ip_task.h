#ifndef	__TCP_IP_TASK__H__
#define __TCP_IP_TASK__H__
#include <stdint.h>
#include "ppp.h"
#include "lib/sys_tick.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api_msg.h"
#include "lwip/memp.h"
#include "lwip/sys.h"

#define PPP_RESET_TIMEOUT	120
#define TCP_BUSY_TIMEOUT	30

typedef enum {
	INITIAL,
	CONNECT,
	CONNECTED,
	LOGGED_IN,
	REPORTING,
	WAIT_TIMEOUT_RECONNECT
}TCP_STATE_TYPE;

extern Timeout_Type tcpIpReset;
extern Timeout_Type tTcpDataIsBusy;
extern uint8_t tcpIpTryCnt;
uint8_t vTcpIpTask(void);
uint8_t vTcpIpTaskInit(void);

void tcpip_init(void (* initfunc)(void *), void *arg);

void tcpip_apimsg(struct api_msg *apimsg);
err_t tcpip_input(struct pbuf *p, struct netif *inp);
err_t tcpip_callback(void (*f)(void *ctx), void *ctx);

void tcpip_tcp_timer_needed(void);

enum tcpip_msg_type {
  TCPIP_MSG_API,
  TCPIP_MSG_INPUT,
  TCPIP_MSG_CALLBACK
};

struct tcpip_msg {
  enum tcpip_msg_type type;
  sys_sem_t *sem;
  union {
    struct api_msg *apimsg;
    struct {
      struct pbuf *p;
      struct netif *netif;
    } inp;
    struct {
      void (*f)(void *ctx);
      void *ctx;
    } cb;
  } msg;
};

#endif

