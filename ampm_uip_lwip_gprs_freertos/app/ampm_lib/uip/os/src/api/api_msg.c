/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api_msg.h"
#include "lwip/memp.h"
#include "lwip/sys.h"
#include "tcpip.h"
#include "tcpip_rtos.h"
#include "tcp_ip_task.h"

#if LWIP_TCP

static void
do_connect(struct api_msg_msg *msg);
static void
do_disconnect(struct api_msg_msg *msg);
static void
do_listen(struct api_msg_msg *msg);
static void
do_accept(struct api_msg_msg *msg);
static void
do_send(struct api_msg_msg *msg);
static void
do_recv(struct api_msg_msg *msg);
static void
do_write(struct api_msg_msg *msg);
static void
do_close(struct api_msg_msg *msg);

static err_t
recv_tcp(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
 struct netconn *conn;
  u16_t len;

  (void)pcb;

  conn = arg;

  if (conn == NULL) {
    pbuf_free(p);
    return ERR_VAL;
  }

  if (conn->recvmbox != SYS_MBOX_NULL) {
        
    conn->err = err;
    if (p != NULL) {
        len = p->tot_len;
        conn->recv_avail += len;
    }
    else
        len = 0;
    /* Register event with callback */
    if (conn->callback)
        (*conn->callback)(conn, NETCONN_EVT_RCVPLUS, len);
    sys_mbox_post(conn->recvmbox, p);
  }  
  return ERR_OK;
  return ERR_OK;
}


static err_t
poll_tcp(void *arg, struct tcp_pcb *pcb)
{
  struct netconn *conn;

  (void)pcb;

  conn = arg;
  if (conn != NULL &&
     (conn->state == NETCONN_WRITE || conn->state == NETCONN_CLOSE) &&
     conn->sem != SYS_SEM_NULL) {
    sys_sem_signal(conn->sem);
  }
  return ERR_OK;
}

static err_t
sent_tcp(void *arg, struct tcp_pcb *pcb, u16_t len)
{
  struct netconn *conn;
  (void)pcb;
  conn = arg;
//  if (conn != NULL && conn->sem != SYS_SEM_NULL) {
//    sys_sem_signal(conn->sem);
//  }
	sys_mbox_post(conn->mbox, NULL);
  return ERR_OK;
}

static void
err_tcp(void *arg, err_t err)
{
  struct netconn *conn;

  conn = arg;

  conn->pcb.tcp = NULL;

  
  conn->err = err;
  if (conn->recvmbox != SYS_MBOX_NULL) {
    /* Register event with callback */
    if (conn->callback)
      (*conn->callback)(conn, NETCONN_EVT_RCVPLUS, 0);
    sys_mbox_post(conn->recvmbox, NULL);
  }
  if (conn->mbox != SYS_MBOX_NULL) {
    sys_mbox_post(conn->mbox, NULL);
  }
  if (conn->acceptmbox != SYS_MBOX_NULL) {
     /* Register event with callback */
    if (conn->callback)
      (*conn->callback)(conn, NETCONN_EVT_RCVPLUS, 0);
    sys_mbox_post(conn->acceptmbox, NULL);
  }
  if (conn->sem != SYS_SEM_NULL) {
    sys_sem_signal(conn->sem);
  }
}

static void
setup_tcp(struct netconn *conn)
{
	struct tcp_pcb *pcb;
  pcb = conn->pcb.tcp;
	tcp_arg(pcb, conn);
  tcp_recv(pcb, recv_tcp);
  tcp_sent(pcb, sent_tcp);
  tcp_poll(pcb, poll_tcp, 4);
  tcp_err(pcb, err_tcp);
}

static err_t
accept_function(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  return ERR_OK;
}
#endif /* LWIP_TCP */

static void
do_newconn(struct api_msg_msg *msg)
{
  if(msg->conn->pcb.tcp != NULL) {
   /* This "new" connection already has a PCB allocated. */
   /* Is this an error condition? Should it be deleted? 
      We currently just are happy and return. */
     sys_mbox_post(msg->conn->mbox, NULL);
     return;
   }
   msg->conn->err = ERR_OK;

   /* Allocate a PCB for this connection */
   switch(msg->conn->type) {
#if LWIP_TCP
   case NETCONN_TCP:
      msg->conn->pcb.tcp = tcp_new();
      if(msg->conn->pcb.tcp == NULL) {
         msg->conn->err = ERR_MEM;
         break;
      }
      setup_tcp(msg->conn);
      break;
#endif
   }
  sys_mbox_post(msg->conn->mbox, NULL);
}


static void
do_delconn(struct api_msg_msg *msg)
{
  
  if (msg->conn->mbox != SYS_MBOX_NULL) {
    sys_mbox_post(msg->conn->mbox, NULL);
  }
}

static void
do_bind(struct api_msg_msg *msg)
{
  sys_mbox_post(msg->conn->mbox, NULL);
}
#if LWIP_TCP

static err_t
do_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
	struct netconn *conn;

  (void)pcb;

  conn = arg;

  if (conn == NULL) {
    return ERR_VAL;
  }
  
  conn->err = err;
  if (conn->type == NETCONN_TCP && err == ERR_OK) {
    setup_tcp(conn);
  }    
  sys_mbox_post(conn->mbox, NULL);
  return ERR_OK;
}
#endif  

static void
do_connect(struct api_msg_msg *msg)
{
	if (msg->conn->pcb.tcp == NULL) {
    switch (msg->conn->type) {
#if LWIP_TCP      
    case NETCONN_TCP:
      msg->conn->pcb.tcp = tcp_new();      
      if (msg->conn->pcb.tcp == NULL) {
				msg->conn->err = ERR_MEM;
				sys_mbox_post(msg->conn->mbox, NULL);
				return;
      }
#endif
    default:
      break;
    }
  }
  switch (msg->conn->type) {
#if LWIP_TCP      
  case NETCONN_TCP:
    setup_tcp(msg->conn);
		msg->conn->pcb.tcp->connected = do_connected;
		tcp_pcb_connect(msg->conn->pcb.tcp,msg->msg.bc.ipaddr,msg->msg.bc.port);
#endif

  default:
    break;
  }
}

static void
do_disconnect(struct api_msg_msg *msg)
{
  sys_mbox_post(msg->conn->mbox, NULL);
}


static void
do_listen(struct api_msg_msg *msg)
{
  sys_mbox_post(msg->conn->mbox, NULL);
}

static void
do_accept(struct api_msg_msg *msg)
{
  if (msg->conn->pcb.tcp != NULL) {
    switch (msg->conn->type) {
    case NETCONN_TCP:
      break;
				default:
		break;
    }
  }
}

static void
do_send(struct api_msg_msg *msg)
{
  if (msg->conn->pcb.tcp != NULL) {
    switch (msg->conn->type) {
    case NETCONN_TCP:
      break;
		default:
			break;
    }
  }
  sys_mbox_post(msg->conn->mbox, NULL);
}

static void
do_recv(struct api_msg_msg *msg)
{
  sys_mbox_post(msg->conn->mbox, NULL);
}

static void
do_write(struct api_msg_msg *msg)
{
  err_t err;
  if (msg->conn->pcb.tcp != NULL) {
    switch (msg->conn->type) {
#if LWIP_TCP 
    case NETCONN_TCP:      
      if( tcp_pcb_send(msg->conn->pcb.tcp, msg->msg.w.dataptr,
                      msg->msg.w.len) == msg->msg.w.len)
			{
				err = ERR_BUF;
			}
			break;
#endif
    default:
      break;
    }
  }
}

static void
do_close(struct api_msg_msg *msg)
{
  sys_mbox_post(msg->conn->mbox, NULL);
}

typedef void (* api_msg_decode)(struct api_msg_msg *msg);
static api_msg_decode decode[API_MSG_MAX] = {
  do_newconn,
  do_delconn,
  do_bind,
  do_connect,
  do_disconnect,
  do_listen,
  do_accept,
  do_send,
  do_recv,
  do_write,
  do_close
  };
void
api_msg_input(struct api_msg *msg)
{  
  decode[msg->type](&(msg->msg));
}

void
api_msg_post(struct api_msg *msg)
{
  tcpip_apimsg(msg);
}



