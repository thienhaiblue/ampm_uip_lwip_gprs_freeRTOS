#ifndef __UIP_CONF_H__
#define __UIP_CONF_H__

#include <stdint.h>



typedef uint8_t u8_t;
typedef int8_t   s8_t;
typedef uint16_t u16_t;
typedef int16_t s16_t;
typedef uint32_t u32_t;
typedef int32_t s32_t;
typedef uint32_t uip_stats_t;
typedef u32_t mem_ptr_t;
typedef int sys_prot_t;


extern void UIP_TCPCallback(void);
extern void UIP_UDPCallback(void);



#define NO_SYS                  0
#define MEMP_STATS	0
#define LWIP_NOASSERT						1
#define PBUF_STATS							0
/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                (2*1024)

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           10

#define CCIF

#define UIP_CONF_MAX_CONNECTIONS	5

#define UIP_CONF_UDP_CONNS			5

#define UIP_CONF_MAX_LISTENPORTS	1

#define UIP_CONF_BUFFER_SIZE		1514

#define UIP_CONF_BYTE_ORDER			LITTLE_ENDIAN

#define UIP_CONF_LOGGING			1

#define UIP_CONF_UDP				1

#define UIP_CONF_UDP_CHECKSUMS		1

#define UIP_CONF_STATISTICS			0

#define UIP_CONF_LLH_LEN			4


// NTP client
#define NTP_TZ   +7

#define NTP_REQ_CYCLE 600

#define NTP_REPEAT  4

#define BYTE_ORDER	UIP_CONF_BYTE_ORDER

#define LWIP_PROVIDE_ERRNO 1

/* Debugging options all default to off */

#ifndef DBG_TYPES_ON
#define DBG_TYPES_ON                    0
#endif

#ifndef ETHARP_DEBUG
#define ETHARP_DEBUG                    DBG_OFF
#endif

#ifndef NETIF_DEBUG
#define NETIF_DEBUG                     DBG_OFF
#endif

#ifndef PBUF_DEBUG
#define PBUF_DEBUG                      DBG_OFF
#endif

#ifndef API_LIB_DEBUG
#define API_LIB_DEBUG                   DBG_OFF
#endif

#ifndef API_MSG_DEBUG
#define API_MSG_DEBUG                   DBG_OFF
#endif

#ifndef SOCKETS_DEBUG
#define SOCKETS_DEBUG                   DBG_OFF
#endif

#ifndef ICMP_DEBUG
#define ICMP_DEBUG                      DBG_OFF
#endif

#ifndef INET_DEBUG
#define INET_DEBUG                      DBG_OFF
#endif

#ifndef IP_DEBUG
#define IP_DEBUG                        DBG_OFF
#endif

#ifndef IP_REASS_DEBUG
#define IP_REASS_DEBUG                  DBG_OFF
#endif

#ifndef RAW_DEBUG
#define RAW_DEBUG                       DBG_OFF
#endif

#ifndef MEM_DEBUG
#define MEM_DEBUG                       DBG_OFF
#endif

#ifndef MEMP_DEBUG
#define MEMP_DEBUG                      DBG_OFF
#endif

#ifndef SYS_DEBUG
#define SYS_DEBUG                       DBG_OFF
#endif

#ifndef TCP_DEBUG
#define TCP_DEBUG                       DBG_OFF
#endif

#ifndef TCP_INPUT_DEBUG
#define TCP_INPUT_DEBUG                 DBG_OFF
#endif

#ifndef TCP_FR_DEBUG
#define TCP_FR_DEBUG                    DBG_OFF
#endif

#ifndef TCP_RTO_DEBUG
#define TCP_RTO_DEBUG                   DBG_OFF
#endif

#ifndef TCP_REXMIT_DEBUG
#define TCP_REXMIT_DEBUG                DBG_OFF
#endif

#ifndef TCP_CWND_DEBUG
#define TCP_CWND_DEBUG                  DBG_OFF
#endif

#ifndef TCP_WND_DEBUG
#define TCP_WND_DEBUG                   DBG_OFF
#endif

#ifndef TCP_OUTPUT_DEBUG
#define TCP_OUTPUT_DEBUG                DBG_OFF
#endif

#ifndef TCP_RST_DEBUG
#define TCP_RST_DEBUG                   DBG_OFF
#endif

#ifndef TCP_QLEN_DEBUG
#define TCP_QLEN_DEBUG                  DBG_OFF
#endif

#ifndef UDP_DEBUG
#define UDP_DEBUG                       DBG_OFF
#endif

#ifndef TCPIP_DEBUG
#define TCPIP_DEBUG                     DBG_OFF
#endif

#ifndef PPP_DEBUG 
#define PPP_DEBUG                       DBG_OFF
#endif

#ifndef SLIP_DEBUG 
#define SLIP_DEBUG                      DBG_OFF
#endif

#ifndef DHCP_DEBUG 
#define DHCP_DEBUG                      DBG_OFF
#endif


#ifndef DBG_MIN_LEVEL
#define DBG_MIN_LEVEL                   DBG_LEVEL_OFF
#endif


#endif /* __UIP_CONF_H__ */

/** @} */
/** @} */
