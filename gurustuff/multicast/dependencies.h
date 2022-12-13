#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H


#include <stdio.h>
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include "net/routing/routing.h"
#include "heapmem.h"

#include "data_4000B.h"
// #include "data_8000B.h"


#define NODES_SIX_FOUR			(60)
#define NODES_ONE_TWO_EIGHT		(120)
#define NODES_TWO_FIVE_SIX		(250)
#define NODES_FIVE_ONE_TWO		(505)


#define MAX_PAYLOAD_LEN 64	//check 80 bytes payload
#define MIN_ONE 		60
#define MIN_TWO 		(2 * MIN_ONE)

#define SEND_INTERVAL	(CLOCK_SECOND * MIN_TWO) /* clock ticks */
#define NUM_OF_NODES	NODES_TWO_FIVE_SIX //64 //128 //256 //512 //1024
#define QUEUE_SIZE		20

#define MIN_TEN			10
#define MIN_THIRTY		30

#define UNI_REQ_START_SEND_INTERVAL	((SEND_INTERVAL * 16) + (MIN_TEN * MIN_ONE * CLOCK_SECOND)) // 42 mins
#define UNI_REQ_SEND_INTERVAL 		(MIN_TEN * MIN_ONE * CLOCK_SECOND)

#define MIN_TWONINE		29
#define QUE_START_INTERVAL	(MIN_THIRTY * MIN_ONE * CLOCK_SECOND)	

typedef struct {
	uint8_t seq_num;
	// uint8_t chunk_num;
	uint8_t tot_chnks;
	uint8_t buf[MAX_PAYLOAD_LEN];
} packet_data;


/* missing packet request queue */
typedef struct misspckt_que_s {
	// uint8_t
	// bool *miss_pack ;
	struct misspckt_que_s *next;
	struct misspckt_que_s *prev;

	uint8_t *miss_pckt;	//number of packets missing
} misspckt_que_t;


#endif


/*
filters

(icmpv6.type == 159 ) || (ipv6.dst == ff03::fc) || udp.port == 8765




*/



