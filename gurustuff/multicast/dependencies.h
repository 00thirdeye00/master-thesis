/**
 * \addtogroup multicast
 * @{
 */
/**
 * \file
 *    		Dependencies file for multicast
 *
 * \author
 *    		Guru Mehar Rachaputi
 * 
 * \reviewer
 * 	  		Anders Isberg
 * 
 */
#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H


#include <stdio.h>
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include "net/routing/routing.h"
#include "heapmem.h"

// #define UNICAST_ALLDATA
// #undef	UNIREQ_ALLDATA

#ifdef UNICAST_ALLDATA
#define UNIREQ_ALLDATA	1
// #else
// #undef	UNIREQ_ALLDATA
#endif	

/* define BYTES_1024 for 1kB, BYTES_4000 for 4kB, BYTES_8000 for 8kB of total data */

// #define BYTES_1024 //BYTES_1024
#define BYTES_4000 //BYTES_4000
// #define BYTES_8000 //BYTES_8000

#ifdef BYTES_1024
#include "data_1024B.h"
#endif

#ifdef BYTES_4000
#include "data_4000B.h"
#endif

#ifdef BYTES_8000
#include "data_8000B.h"
#endif


#define DATA_LEN    (strlen(seq_idd))			 // total data length	
#define DATA_CHNKS  (DATA_LEN / MAX_PAYLOAD_LEN) // total data chunks

#define ITERATIONS DATA_CHNKS // total data chunks


#define MAX_PAYLOAD_LEN	32	// set to 32 to compare with p2p (set to 64 for MPL/SMRF/Unicast)

#define MIN_ONE 		60				// one minute
#define MIN_TWO 		(2 * MIN_ONE)	// two minutes
#define MIN_THREE 		(3 * MIN_ONE)	// three minutes
#define MIN_FIVE 		(5 * MIN_ONE)	// five minutes

#define NODES_THIRTY_ONE		(28)	// atleast network formed of 28 nodes for 32 node topology
#define NODES_SIX_FOUR			(60)	// atleast network formed of 60 nodes for 32 node topology
#define NODES_ONE_TWO_EIGHT		(120)	// atleast network formed of 120 nodes for 32 node topology
#define NODES_TWO_FIVE_SIX		(250)	// atleast network formed of 256 nodes for 32 node topology
#define NODES_FIVE_ONE_TWO		(505)	// atleast network formed of 505 nodes for 32 node topology

#define NUM_OF_NODES	NODES_THIRTY_ONE //64 //128 //256 //512 //1024
#define SEND_INTERVAL	(CLOCK_SECOND * MIN_FIVE) /* clock ticks */ //MIN_FIVE for multicast

#define QUEUE_SIZE		20 // queue size

#define MIN_TEN			10 // 10 minutes
#define MIN_THIRTY		30 // 30 minutes

// unicast request start send interval
#define UNI_REQ_START_SEND_INTERVAL	((SEND_INTERVAL * ITERATIONS) + (MIN_TEN * MIN_ONE * CLOCK_SECOND))
// unicast request send interval
#define UNI_REQ_SEND_INTERVAL 		(MIN_TEN * MIN_ONE * CLOCK_SECOND)

// queue process start interval
#define QUE_START_INTERVAL	(MIN_THIRTY * MIN_ONE * CLOCK_SECOND)	

/* data packet structure */
typedef struct {
	uint8_t seq_num;				// sequence number
	uint8_t tot_chnks;				// total number of chunks
	uint8_t buf[MAX_PAYLOAD_LEN];	// buffer with payload length
} packet_data;


/* missing packet request queue */
typedef struct misspckt_que_s {
	struct misspckt_que_s *next;
	struct misspckt_que_s *prev;	
	uint8_t *miss_pckt;				// number of packets missing
} misspckt_que_t;


#endif


/*
filters

(icmpv6.type == 159 ) || (ipv6.dst == ff03::fc) || udp.port == 8765




*/


// total data of 1024 bytes captured with 3 mins of SEND_INTERVAL
// total data of 4000 bytes captured with 5 mins of SEND_INTERVAL 
	// and data packet of 78 bytes (total packet = 80 bytes)


// for 64 bytes data packet the payload is 66 bytes
























































