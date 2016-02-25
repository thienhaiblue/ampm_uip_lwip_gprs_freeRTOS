
#ifndef __TCP_IP_H__
#define __TCP_IP_H__

#include "uipopt.h"
#include "uip.h"
#include "lwip/err.h"
#define SOCKET_NULL 0xff
void tcpip_uipcall(void);
struct uip_conn *tcp_connect(uip_ipaddr_t *ripaddr, uint16_t port,void *appstate);
struct uip_udp_conn *
udp_new(const uip_ipaddr_t *ripaddr, uint16_t port,void (*callback)(void *state),void *appstate);
void tcp_attach(struct uip_conn *conn,void *appstate);
#define tcp_markconn(conn,appstate) tcp_attach(conn, appstate)
void tcp_unlisten(uint16_t port);
void tcp_listen(uint16_t port);
#endif
