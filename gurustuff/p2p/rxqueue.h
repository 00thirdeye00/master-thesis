/**
 * \addtogroup p2p
 * @{
 */
/**
 * \file
 *    		This file implements queue for 'Peer to Peer' (p2p)
 *
 * \author
 *    		Guru Mehar Rachaputi
 * 
 * \reviewer
 * 	  		Anders Isberg
 * 
 */

#ifndef RX_QUEUE_H_
#define RX_QUEUE_H_


#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "lib/queue.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6-route.h"
#include "heapmem.h"


#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"

#include "p2p.h"


#define QUEUE_SIZE	20	// queue size

/* events to handle upload process */
enum {
	HANDSHAKE_EVENT,
	INTEREST_EVENT,
	REQUEST_EVENT
};


/* structure for missing packets queue */
typedef struct rx_mpckts_s {
	struct rx_mpckts_s *next;		// next queue item
	struct rx_mpckts_s *previous;	// previous queue item
	uip_ipaddr_t send_addr;			// sender address
	uint16_t datalen;				// data length
	uint8_t *data;					// data
} rx_mpckts_t;


/*------------------------------------------------------------------*/
/**
 * brief: to check if the queue is empty or not
 *
 * params: void
 *
 * return: boolean
 *
 *
 */
bool is_queue_empty(void);

/*------------------------------------------------------------------*/
/**
 * brief: reset queue
 *
 * params: void
 *
 * return: void
 *
 *
 */

void queue_reset(void);

/*------------------------------------------------------------------*/
/**
 * brief: enque an element in the queue
 *
 * params: sender address, data length, data
 *
 * return: void
 *
 *
 */

void queue_enq(const uip_ipaddr_t *sender_addr, uint16_t dlen, const uint8_t *data);


/*------------------------------------------------------------------*/
/**
 * brief: dequeue an element from the queue
 *
 * params: void
 *
 * return: 0 or 1
 *
 *
 */

uint8_t queue_deq(void);

/*------------------------------------------------------------------*/
/**
 * brief: to handle upload process based on incomming events
 *
 * params: process event, process data
 *
 * return: void
 *
 *
 */

extern void upload_event_handler(process_event_t ev, const process_post_data_t *post_data);




#endif // RX_QUEUE_H_
















































































