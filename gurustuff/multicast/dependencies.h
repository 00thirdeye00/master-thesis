#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H


#include <stdio.h>
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include "net/routing/routing.h"

#define MAX_PAYLOAD_LEN 64



typedef struct {
	uint8_t seq_num;
	// uint8_t chunk_num;
	uint8_t tot_chnks;
	uint8_t buf[MAX_PAYLOAD_LEN];
}packet_data;




#endif