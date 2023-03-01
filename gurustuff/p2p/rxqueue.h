

#ifndef RX_QUEUE_H_
#define RX_QUEUE_H_


#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "lib/queue.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6-route.h"


#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"



#define QUEUE_SIZE		20



typedef struct rx_mpckts_s {
	struct rx_mpckts_s *next;
	struct rx_mpckts_s *previous;
	uip_ipaddr_t send_addr;
	uint16_t datalen;
	uint8_t *data;
} rx_mpckts_t;

rx_mpckts_t *rx_q;


bool is_queue_empty(void)

void queue_enq(const uip_ipaddr_t *sender_addr, uint16_t dlen, const uint8_t *data);

void queue_deq(void);











#endif // RX_QUEUE_H_















































































