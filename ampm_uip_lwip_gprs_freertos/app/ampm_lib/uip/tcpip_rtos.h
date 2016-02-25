

#ifndef __AMPM_TCPIP_OS_H__
#define __AMPM_TCPIP_OS_H__


#include "uipopt.h"
#include "uip.h"
#include "lwip/err.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api_msg.h"
#include "lwip/memp.h"
#include "lwip/sys.h"
#include "tcpip.h"


struct tcp_pcb;

struct tcp_pcb * tcp_new(void);

void						tcp_arg(struct tcp_pcb *pcb, void *arg);

void             tcp_recv    (struct tcp_pcb *pcb,
            err_t (* recv)(void *arg, struct tcp_pcb *tpcb,struct pbuf *p,
								err_t err));
void             tcp_sent    (struct tcp_pcb *pcb,
            err_t (* sent)(void *arg, struct tcp_pcb *tpcb,
               u16_t len));
void             tcp_poll    (struct tcp_pcb *pcb,
            err_t (* poll)(void *arg, struct tcp_pcb *tpcb),
            u8_t interval);
void             tcp_err     (struct tcp_pcb *pcb,
            void (* err)(void *arg, err_t err));

int
tcp_pcb_connect(struct tcp_pcb *pcb,
                   const struct ip_addr *ipaddr,
                   uint16_t port);
									 
u16_t tcp_sndbuf(struct tcp_pcb *pcb);
									 
int
tcp_pcb_send(struct tcp_pcb *pcb,
                const uint8_t *data, int datalen);


enum tcp_state {
  CLOSED      = 0,
  LISTEN      = 1,
  SYN_SENT    = 2,
  SYN_RCVD    = 3,
  ESTABLISHED = 4,
  FIN_WAIT_1  = 5,
  FIN_WAIT_2  = 6,
  CLOSE_WAIT  = 7,
  CLOSING     = 8,
  LAST_ACK    = 9,
  TIME_WAIT   = 10
};

enum {
  TCP_PCB_FLAGS_NONE      = 0x00,
  TCP_PCB_FLAGS_LISTENING = 0x01,
  TCP_PCB_FLAGS_CLOSING   = 0x02,
};

typedef enum {
  TCP_PCB_CONNECTED,
  TCP_PCB_CLOSED,
  TCP_PCB_TIMEDOUT,
  TCP_PCB_ABORTED,
  TCP_PCB_DATA_SENT
} tcp_pcb_event_t;

#define TF_ACK_DELAY (u8_t)0x01U   /* Delayed ACK. */
#define TF_ACK_NOW   (u8_t)0x02U   /* Immediate ACK. */
#define TF_INFR      (u8_t)0x04U   /* In fast recovery. */
#define TF_RESET     (u8_t)0x08U   /* Connection was reset. */
#define TF_CLOSED    (u8_t)0x10U   /* Connection was sucessfully closed. */
#define TF_GOT_FIN   (u8_t)0x20U   /* Connection was closed by the remote end. */
#define TF_NODELAY   (u8_t)0x40U   /* Disable Nagle algorithm */


#define TCP_EVENT_ACCEPT(pcb,err,ret)     \
                        if((pcb)->accept != NULL) \
                        (ret = (pcb)->accept((pcb)->callback_arg,(pcb),(err)))
#define TCP_EVENT_SENT(pcb,space,ret) \
                        if((pcb)->sent != NULL) \
                        (ret = (pcb)->sent((pcb)->callback_arg,(pcb),(space)))
#define TCP_EVENT_RECV(pcb,p,err,ret) \
                        if((pcb)->recv != NULL) \
                        { ret = (pcb)->recv((pcb)->callback_arg,(pcb),(p),(err)); } else { \
                          if (p) pbuf_free(p); }
#define TCP_EVENT_CONNECTED(pcb,err,ret) \
                        if((pcb)->connected != NULL) \
                        (ret = (pcb)->connected((pcb)->callback_arg,(pcb),(err)))
#define TCP_EVENT_POLL(pcb,ret) \
                        if((pcb)->poll != NULL) \
                        (ret = (pcb)->poll((pcb)->callback_arg,(pcb)))
#define TCP_EVENT_ERR(errf,arg,err) \
                        if((errf) != NULL) \
                        (errf)((arg),(err))

#define IP_PCB struct ip_addr local_ip; \
  struct ip_addr remote_ip; \
   /* Socket options */  \
  u16_t so_options;      \
   /* Type Of Service */ \
  u8_t tos;              \
  /* Time To Live */     \
  u8_t ttl												
												
/* the TCP protocol control block */
struct tcp_pcb {
	IP_PCB;
	struct tcp_pcb *next;
	struct uip_conn *c;
	enum tcp_state state; /* TCP state */
	void *callback_arg;
  uint8_t *send_data;
  uint16_t send_data_len;
	uint16_t sent_data_index;
	uint16_t sending_data_len;
	uint8_t pollinterval;
  uint8_t flags;
  uint16_t listen_port;
	u16_t acked;
	/* idle time before KEEPALIVE is sent */
  u32_t keepalive;
  
  /* KEEPALIVE counter */
  u8_t keep_cnt;
  /* Function to be called when more send buffer space is available. */
  err_t (* sent)(void *arg, struct tcp_pcb *pcb, u16_t space);
  
  /* Function to be called when (in-sequence) data has arrived. */
  err_t (* recv)(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);

  /* Function to be called when a connection has been set up. */
  err_t (* connected)(void *arg, struct tcp_pcb *pcb, err_t err);

  /* Function to call when a listener has been connected. */
  err_t (* accept)(void *arg, struct tcp_pcb *newpcb, err_t err);

  /* Function which is called periodically. */
  err_t (* poll)(void *arg, struct tcp_pcb *pcb);

  /* Function to be called whenever a fatal error occurs. */
  void (* errf)(void *arg, err_t err);

};




#endif

