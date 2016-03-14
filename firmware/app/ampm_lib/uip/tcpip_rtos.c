


/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/ampm_list.h"
#include "tcpip.h"
#include "tcpip_rtos.h"

#include <stdio.h>
#include <string.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */
static void relisten(struct tcp_pcb *s);

LIST(tcp_pcb_list);

uint32_t tcp_ticks = 0;

void tcp_tmr(void)
{
  ++tcp_ticks;
}

u16_t tcp_sndbuf(struct tcp_pcb *pcb)
{
	return (pcb->send_data_len - pcb->sent_data_index);
}

/*---------------------------------------------------------------------------*/
static void
call_event(struct tcp_pcb *pcb, tcp_pcb_event_t event)
{
	err_t err;
  err = ERR_OK;
  if(pcb != NULL) {
    switch(event)
		{
			case TCP_PCB_CONNECTED:
				TCP_EVENT_CONNECTED(pcb, ERR_OK, err);
			break;
			case TCP_PCB_CLOSED:
				TCP_EVENT_ERR(pcb->errf,pcb,ERR_CLSD);
			break;
			case TCP_PCB_TIMEDOUT:
				TCP_EVENT_ERR(pcb->errf,pcb,ERR_ABRT);
			break;
			case TCP_PCB_ABORTED:
				TCP_EVENT_ERR(pcb->errf,pcb,ERR_ABRT);
			break;
			case TCP_PCB_DATA_SENT:
				TCP_EVENT_SENT(pcb,NULL, err);
			break;
		}
  }
}
/*---------------------------------------------------------------------------*/
static void
senddata(struct tcp_pcb *pcb)
{
  int len;
	if(pcb->send_data_len > 0 && pcb->send_data) {
		if(pcb->sent_data_index < pcb->send_data_len)
		{
			len = pcb->send_data_len - pcb->sent_data_index;
			len = MIN(len, uip_mss());
			uip_send(&pcb->send_data[pcb->sent_data_index], len);
			pcb->sending_data_len = len;
		}
		else
		{
			pcb->send_data_len = 0;
			pcb->sent_data_index = 0;
			pcb->send_data = NULL;
			pcb->sending_data_len = 0;
			call_event(pcb, TCP_PCB_DATA_SENT);
		}
  }
}
/*---------------------------------------------------------------------------*/
static void
acked(struct tcp_pcb *pcb)
{
  if(pcb->send_data_len > 0 && pcb->send_data) {
		if(pcb->sent_data_index >= pcb->send_data_len)
		{
			pcb->send_data_len = 0;
			pcb->sent_data_index = 0;
			pcb->send_data = NULL;
			pcb->sending_data_len = 0;
			call_event(pcb, TCP_PCB_DATA_SENT);
		}
		else
		{
			pcb->sent_data_index += pcb->sending_data_len;
		}
  }
}
/*---------------------------------------------------------------------------*/
static void
newdata(struct tcp_pcb *pcb)
{
	
  
  uint16_t len, copylen, bytesleft;
  uint8_t *dataptr;
	struct pbuf *recv_data;
	err_t err;
	err = ERR_OK;
	
  len = uip_datalen();
  dataptr = uip_appdata;
	if(len)
	{
		recv_data = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
		if(recv_data)
		{
			memcpy(recv_data->payload, dataptr, len);
			TCP_EVENT_RECV(pcb, recv_data, ERR_OK, err);
		}
	}
}
/*---------------------------------------------------------------------------*/
static void
relisten(struct tcp_pcb *pcb)
{
  if(pcb != NULL && pcb->listen_port != 0) {
    pcb->flags |= TCP_PCB_FLAGS_LISTENING;
  }
}
/*---------------------------------------------------------------------------*/
void tcp_pcb_appcall(void *state)
{
  struct tcp_pcb *pcb = state;
	err_t err;
	err = ERR_OK;
  if(pcb != NULL && pcb->c != NULL && pcb->c != uip_conn) {
    /* Safe-guard: this should not happen, as the incoming event relates to
     * a previous connection */
    return;
  }
  if(uip_connected()) {
    /* Check if this connection originated in a local listen
       socket. We do this by checking the state pointer - if NULL,
       this is an incoming listen connection. If so, we need to
       connect the socket to the uip_conn and call the event
       function. */
    if(pcb == NULL) {
      for(pcb = list_head(tcp_pcb_list);pcb != NULL;pcb = list_item_next(pcb)) {
				if((pcb->flags & TCP_PCB_FLAGS_LISTENING) != 0 && 
						pcb->listen_port != 0 &&
						pcb->listen_port == uip_htons(uip_conn->lport)) {
					pcb->flags &= ~TCP_PCB_FLAGS_LISTENING;
					//tcp_markconn(uip_conn, s);
					call_event(pcb, TCP_PCB_CONNECTED);
					break;
				}
      }
    } else {
      call_event(pcb, TCP_PCB_CONNECTED);
    }

    if(pcb == NULL) {
      uip_abort();
    } else {
      if(uip_newdata()) {
        newdata(pcb);
      }
      senddata(pcb);
    }
    return;
  }

  if(uip_timedout()) {
    call_event(pcb, TCP_PCB_TIMEDOUT);
    relisten(pcb);
  }

  if(uip_aborted()) {
    //tcp_markconn(uip_conn, NULL);
    call_event(pcb, TCP_PCB_ABORTED);
    relisten(pcb);

  }

  if(pcb == NULL) {
    uip_abort();
    return;
  }

  if(uip_acked()) {
    acked(pcb);
  }
  if(uip_newdata()) {
    newdata(pcb);
  }

  if(uip_rexmit() ||
     uip_newdata() ||
     uip_acked()) {
    senddata(pcb);
  } else if(uip_poll()) {
    senddata(pcb);
		TCP_EVENT_POLL(pcb, err);
  }

  if(pcb->send_data_len == 0 && pcb->flags & TCP_PCB_FLAGS_CLOSING) {
    pcb->flags &= ~TCP_PCB_FLAGS_CLOSING;
    uip_close();
    pcb->c = NULL;
    tcp_markconn(uip_conn, NULL);
    pcb->c = NULL;
    /*call_event(s, TCP_PCB_CLOSED);*/
    relisten(pcb);
  }

  if(uip_closed()) {
    tcp_markconn(uip_conn, NULL);
    pcb->c = NULL;
    call_event(pcb, TCP_PCB_CLOSED);
    relisten(pcb);
  }
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  static uint8_t inited = 0;
  if(!inited) {
    list_init(tcp_pcb_list);
    inited = 1;
  }
}

/*---------------------------------------------------------------------------*/
int
tcp_pcb_connect(struct tcp_pcb *pcb,
                   const struct ip_addr *ipaddr,
                   uint16_t port)
{
  if(pcb == NULL) {
    return -1;
  }
  if(pcb->c != NULL) {
    tcp_markconn(pcb->c, NULL);
  }
  pcb->c = tcp_connect((uip_ipaddr_t *)ipaddr, uip_htons(port), pcb);
  if(pcb->c == NULL) {
    return -1;
  } else {
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
int
tcp_pcb_listen(struct tcp_pcb *pcb,
           uint16_t port)
{
  if(pcb == NULL) {
    return -1;
  }

  pcb->listen_port = port;
  tcp_listen(uip_htons(port));
  pcb->flags |= TCP_PCB_FLAGS_LISTENING;
  return 1;
}
/*---------------------------------------------------------------------------*/
int
tcp_pcb_unlisten(struct tcp_pcb *pcb)
{
  if(pcb == NULL) {
    return -1;
  }
  tcp_unlisten(uip_htons(pcb->listen_port));
  pcb->listen_port = 0;
  pcb->flags &= ~TCP_PCB_FLAGS_LISTENING;
  return 1;
}
/*---------------------------------------------------------------------------*/
int
tcp_pcb_send(struct tcp_pcb *pcb,
                const uint8_t *data, int datalen)
{
  if(pcb == NULL) {
    return -1;
  }
	if(pcb->send_data == NULL)
	{
		pcb->send_data = (uint8_t *)data;
		pcb->send_data_len = datalen;
		pcb->sent_data_index = 0;
		pcb->sending_data_len = 0;
		return 0;
	}
  return -1;
}
/*---------------------------------------------------------------------------*/
int
tcp_pcb_send_str(struct tcp_pcb *s,
             const char *str)
{
  return tcp_pcb_send(s, (const uint8_t *)str, strlen(str));
}
/*---------------------------------------------------------------------------*/
int
tcp_pcb_close(struct tcp_pcb *s)
{
  if(s == NULL) {
    return -1;
  }

  s->flags |= TCP_PCB_FLAGS_CLOSING;
  return 1;
}
/*---------------------------------------------------------------------------*/
int
tcp_pcb_unregister(struct tcp_pcb *s)
{
  if(s == NULL) {
    return -1;
  }

  tcp_pcb_unlisten(s);
  if(s->c != NULL) {
    tcp_attach(s->c, NULL);
  }
  list_remove(tcp_pcb_list, s);
  return 1;
}

/*---------------------------------------------------------------------------*/

static err_t tcp_recv_null(void *arg, struct tcp_pcb *pcb, err_t err)
{
  return ERR_OK;
}


struct tcp_pcb *
tcp_alloc(void)
{
  struct tcp_pcb *pcb;
  u32_t iss;
  
  pcb = memp_malloc(MEMP_TCP_PCB);
  if (pcb == NULL) {
    /* Try killing oldest connection in TIME-WAIT. */
    LWIP_DEBUGF(TCP_DEBUG, ("tcp_alloc: killing off oldest TIME-WAIT connection\n"));
    //tcp_kill_timewait();
    pcb = memp_malloc(MEMP_TCP_PCB);
    if (pcb == NULL) {
      //tcp_kill_prio(prio);    
      pcb = memp_malloc(MEMP_TCP_PCB);
    }
  }
  if (pcb != NULL) {
    memset(pcb, 0, sizeof(struct tcp_pcb));
    pcb->recv = tcp_recv_null;  
  }
  return pcb;
}


void
tcp_arg(struct tcp_pcb *pcb, void *arg)
{  
  pcb->callback_arg = arg;
}
/**
 * Creates a new TCP protocol control block but doesn't place it on
 * any of the TCP PCB lists.
 *
 * @internal: Maybe there should be a idle TCP PCB list where these
 * PCBs are put on. We can then implement port reservation using
 * tcp_bind(). Currently, we lack this (BSD socket type of) feature.
 */

struct tcp_pcb *
tcp_new(void)
{
  return tcp_alloc();
}


/**
 * Used to specify the function that should be called when a TCP
 * connection receives data.
 *
 */ 
void
tcp_recv(struct tcp_pcb *pcb,
   err_t (* recv)(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err))
{
  pcb->recv = recv;
}

/**
 * Used to specify the function that should be called when TCP data
 * has been successfully delivered to the remote host.
 *
 */ 

void
tcp_sent(struct tcp_pcb *pcb,
   err_t (* sent)(void *arg, struct tcp_pcb *tpcb, u16_t len))
{
  pcb->sent = sent;
}

/**
 * Used to specify the function that should be called when a fatal error
 * has occured on the connection.
 *
 */ 
void
tcp_err(struct tcp_pcb *pcb,
   void (* errf)(void *arg, err_t err))
{
  pcb->errf = errf;
}



/**
 * Used to specify the function that should be called periodically
 * from TCP. The interval is specified in terms of the TCP coarse
 * timer interval, which is called twice a second.
 *
 */ 
void
tcp_poll(struct tcp_pcb *pcb,
   err_t (* poll)(void *arg, struct tcp_pcb *tpcb), u8_t interval)
{
  pcb->poll = poll;
  pcb->pollinterval = interval;
}


