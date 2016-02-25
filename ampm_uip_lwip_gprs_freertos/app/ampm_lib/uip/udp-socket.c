
#include "udp-socket.h"

#include <string.h>


static uint8_t buf[UIP_BUFSIZE];

#define UIP_IP_BUF   ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])


/*---------------------------------------------------------------------------*/
static void
init(void)
{
  static uint8_t inited = 0;
  if(!inited) {
    inited = 1;
    process_start(&udp_socket_process, NULL);
  }
}
/*---------------------------------------------------------------------------*/
int
udp_socket_register(struct udp_socket *c,
                    void *ptr,
                    udp_socket_input_callback_t input_callback)
{
  init();

  if(c == NULL) {
    return -1;
  }
  c->ptr = ptr;
  c->input_callback = input_callback;

  c->p = PROCESS_CURRENT();
  PROCESS_CONTEXT_BEGIN(&udp_socket_process);
  c->udp_conn = udp_new(NULL, 0, c);
  PROCESS_CONTEXT_END();

  if(c->udp_conn == NULL) {
    return -1;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
int
udp_socket_close(struct udp_socket *c)
{
  if(c == NULL) {
    return -1;
  }
  if(c->udp_conn != NULL) {
    uip_udp_remove(c->udp_conn);
    return 1;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
int
udp_socket_bind(struct udp_socket *c,
                uint16_t local_port)
{
  if(c == NULL || c->udp_conn == NULL) {
    return -1;
  }
  udp_bind(c->udp_conn, UIP_HTONS(local_port));

  return 1;
}
/*---------------------------------------------------------------------------*/
int
udp_socket_connect(struct udp_socket *c,
                   uip_ipaddr_t *remote_addr,
                   uint16_t remote_port)
{
  if(c == NULL || c->udp_conn == NULL) {
    return -1;
  }

  if(remote_addr != NULL) {
    uip_ipaddr_copy(&c->udp_conn->ripaddr, remote_addr);
  }
  c->udp_conn->rport = UIP_HTONS(remote_port);
  return 1;
}
/*---------------------------------------------------------------------------*/
int
udp_socket_send(struct udp_socket *c,
                const void *data, uint16_t datalen)
{
  if(c == NULL || c->udp_conn == NULL) {
    return -1;
  }

  uip_udp_packet_send(c->udp_conn, data, datalen);
  return datalen;
}
/*---------------------------------------------------------------------------*/
int
udp_socket_sendto(struct udp_socket *c,
                  const void *data, uint16_t datalen,
                  const uip_ipaddr_t *to,
                  uint16_t port)
{
  if(c == NULL || c->udp_conn == NULL) {
    return -1;
  }

  if(c->udp_conn != NULL) {
    uip_udp_packet_sendto(c->udp_conn, data, datalen,
                          to, UIP_HTONS(port));
    return datalen;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_socket_process, ev, data)
{
  struct udp_socket *c;
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == tcpip_event) {

      /* An appstate pointer is passed to use from the IP stack
         through the 'data' pointer. We registered this appstate when
         we did the udp_new() call in udp_socket_register() as the
         struct udp_socket pointer. So we extract this
         pointer and use it when calling the reception callback. */
      c = (struct udp_socket *)data;

      /* Defensive coding: although the appstate *should* be non-null
         here, we make sure to avoid the program crashing on us. */
      if(c != NULL) {

        /* If we were called because of incoming data, we should call
           the reception callback. */
        if(uip_newdata()) {
          /* Copy the data from the uIP data buffer into our own
             buffer to avoid the uIP buffer being messed with by the
             callee. */
          memcpy(buf, uip_appdata, uip_datalen());

          /* Call the client process. We use the PROCESS_CONTEXT
             mechanism to temporarily switch process context to the
             client process. */
          if(c->input_callback != NULL) {
            PROCESS_CONTEXT_BEGIN(c->p);
            c->input_callback(c, c->ptr,
                              &(UIP_IP_BUF->srcipaddr),
                              UIP_HTONS(UIP_IP_BUF->srcport),
                              &(UIP_IP_BUF->destipaddr),
                              UIP_HTONS(UIP_IP_BUF->destport),
                              buf, uip_datalen());
            PROCESS_CONTEXT_END();
          }
        }
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
