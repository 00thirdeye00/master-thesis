
/**
 * \addtogroup p2p
 * @{
 */
/**
 * \file
 *    This file implements 'Peer to Peer' (p2p)
 *
 * \author
 *    Guru Mehar Rachaputi
 */


#include "contiki.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ipv6/multicast/uip-mcast6-route.h"
#include "net/ipv6/multicast/uip-mcast6-stats.h"
#include "net/ipv6/multicast/smrf.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/ipv6/simple-udp.h"
#if ROUTING_CONF_RPL_LITE
#include "net/routing/rpl-lite/rpl.h"
#endif /* ROUTING_CONF_RPL_LITE */
#if ROUTING_CONF_RPL_CLASSIC
#include "net/routing/rpl-classic/rpl.h"
#include "sys/ctimer.h"
#endif /* ROUTING_CONF_RPL_CLASSIC */
#include <string.h>
#include <stdio.h>

#include "sys/log.h"
#define LOG_MODULE "p2p"
#define LOG_LEVEL LOG_LEVEL_NONE

#define DEBUG DEBUG_NONE
#include "net/ipv6/uip-debug.h"

#include "p2p.h"

#define WITH_SERVER_REPLY  1
#define UDP_PORT	8765
#define UDP_PORT_2	7654
#define UDP_PORT_3	5432
#define UDP_PORT_4	5678

// static uint8_t node_upload_nbr;
// static uint8_t node_download_nbr;

static const char seq_idd[] = "063lPLXusS0KbZcuAgXFqXIuhVFxT7PbPdA9CifI7gBC4ia4H0uQiccRRLOaj100"
                      "1Yo8BjcgFF8aFtHLHZdRKNxliQEa8ozq38YP8dSXIwbAw2fMx46f8Xc3CmMou101"
                      "27lBkvtUauXX1V8oF4jT4CKIlbE03ghFBS2hTcnTiZ1Go5Ti1gVWRcUdHAh9g102"
                      "3hSAFpT2dLgVxoo2ZCHhNSE0rm2I5OBVhdklMnzBgDDW5gdpOm7389BaNgqdA103"
                      "47oUilXORLjy7kDu86GHSNLzAJjXUxxEc7PeJfPLyC0khgSy4aYUhlFsenbK4104"
                      "5DSZ5YS33eoOeiTze2onUl7PfPz8No6W7AHYW7S34vUADHDOcABcEkDlpxQho105"
                      "6pM1ksIrfHSbpfCJhoqqh9w9kjITgdDrqbp9aHTzjZOScTCtzpMFACJalZNRz106"
                      "7AY301mZjr2KD8pVrhkNfNxQyg1nmm7QJSXN4lDj0dIawEflfSVXLGShl0vxc107"
                      "8sxK5w6msIvzeRDgaYAJ5zmMWg42S4Ap7WA1eHYepIrlwmn1TdwSEPPcMx6Tj108"
                      "9Ilj3gU7JfgoItY5FqNQJgYy6Vw0NPdwwPcnfPmtLhH5nl55eJFwwpB6UgZan109"
                      "10dgiEiFy8GoGIaOjXC387ihvfvaiGA2sBgh3k87i7Eb7QTNrthpZSdlIk8PE110"
                      "11qwCORP6psr9r8BBTnJqYpuayMhQD8vuxp9aBXaMLWmBAYo2EAtB45YeXEGU111"
                      "12BUgN9AgmDERPzcyeOYG0ExpDE5cVY8OL5pNXM43MNAt7jkJAFnSRNKw6Qq5112"
                      "13bFRN69XbWj5t4HkcFMiVwIOLQARBNmcimSzlnhipF8bPY2P1QfsSqCfsnsu113"
                      "14uqVbPvrWTSGIrfVveI01YwiIWOeXjeMdh78Ruq9zy9xbkNkwOLcAiXzlrF0114"
                      "15YVCvYVKtzRSOCtCh5KPrTw0D2H5gvFFRgAH1ZgYFt6UWvv1ht65ptE7KOeN115"; 



static state_machine_download sm_download[] = {
	{IDLE_STATE,					NONE_CTRL_MSG,  		node_handshake},
	{HANDSHAKING_STATE,				NONE_CTRL_MSG,			NULL},
	{HANDSHAKED_STATE,				ACKHANDSHAKE_CTRL_MSG,	node_interest},
	{INTEREST_INFORMING_STATE,		NONE_CTRL_MSG, 			NULL}, // state=handshaked, interest=false
	{INTEREST_INFORMED_STATE,		UNCHOKE_CTRL_MSG,		node_request},
	{DOWNLOADING_STATE,				NONE_CTRL_MSG,			node_received},
	// {UPLOADING_STATE,				NULL,					NULL},
	// {LAST_COMM_STATE,				NULL,					NULL}
};


// static struct ctimer choke_timer;
// static wait_state_t node_choke_wait;

// extern struct simple_udp_connection p2p_socket;

/*---------------------------------------------------------------------------*/
// static struct simple_udp_connection udp_conn;
// // static struct simple_udp_connection udp_conn_2;
// // static struct simple_udp_connection udp_conn_3;
// // static struct simple_udp_connection udp_conn_4;

// PROCESS(node_comm_process, "UDP server");
// AUTOSTART_PROCESSES(&node_comm_process);
// /*---------------------------------------------------------------------------*/
// static void
// udp_rx_callback(struct simple_udp_connection *c,
//                 const uip_ipaddr_t *sender_addr,
//                 uint16_t sender_port,
//                 const uip_ipaddr_t *receiver_addr,
//                 uint16_t receiver_port,
//                 const uint8_t *data,
//                 uint16_t datalen)
// {
// 	LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
// 	LOG_INFO_6ADDR(sender_addr);
// 	LOG_INFO_("\n");
// #if WITH_SERVER_REPLY
// 	/* send back the same string to the client as an echo reply */
// 	LOG_INFO("Sending response.\n");
// 	simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
// #endif /* WITH_SERVER_REPLY */
// }
/*---------------------------------------------------------------------------*/



/*------------------------------------------------------------------*/
/**
 * brief: handshake time out callback
 *
 * params: neighbor
 *
 * return: void
 *
 *
 */

void callback_ack_handshake (void *ptr_a) {

	nnode_state_t *ptr = (nnode_state_t *)ptr_a;

	if (ctimer_expired(&ptr->c_timer)) {
		ctimer_stop(&ptr->c_timer);
		if (ptr->nnode_ctrlmsg == ACKHANDSHAKE_CTRL_MSG) {
			ptr->nnode_state = HANDSHAKED_STATE;
		} else {
			ptr->nnode_ctrlmsg = NONE_CTRL_MSG;
			ptr->nnode_state = IDLE_STATE;
		}
	}
}




/*------------------------------------------------------------------*/
/**
 * brief: interest informing time out callback
 *
 * params: neighbor
 *
 * return: void
 *
 *
 */

void callback_interest_informing (void *ptr_a) {

	nnode_state_t *ptr = (nnode_state_t *)ptr_a;

	LOG_DBG("FUNC: callback_interest_informing enter\n");
	
	if (ctimer_expired(&ptr->c_timer)) {
		ctimer_stop(&ptr->c_timer);

		switch (ptr->nnode_ctrlmsg) {
		case UNCHOKE_CTRL_MSG:
			ptr->nnode_state = INTEREST_INFORMED_STATE;
			break;
		case CHOKE_CTRL_MSG:
			static struct ctimer t;
			if (ctimer_expired(&t)) {
				ptr->nnode_ctrlmsg = NONE_CTRL_MSG;
				ctimer_set(&t, CLOCK_SECOND * 5, callback_interest_informing, ptr);
			}
			else
				// LOG_ERR(“INTEREST TIMER CANNOT START: NODE: (%u)”, n_idx);
				PRINTF(“INTEREST TIMER CANNOT START: NODE: %u\n”, n_idx);
		default:
			ptr->nnode_state = HANDSHAKED_STATE;
			ptr->nnode_interest = INTEREST_FALSE;
			break;
		}
	}
}






/*------------------------------------------------------------------*/
/**
 * brief: request to download timeout callback
 *
 * params: neighbor
 *
 * return: void
 *
 *
 */

void callback_request (void *ptr_a) {

	nnode_state_t *ptr = (nnode_state_t *)ptr_a;

	if (ctimer_expired(&ptr->c_timer)) {
		ctimer_stop(&ptr->c_timer);

		if (chunk_cnt[ptr->chunk_block] >= 0x01) {
			/* nothing to do */
		} else {
			ptr->failed_dlreq++;
			ptr->nnode_state = HANDSHAKED_STATE;
			ptr->nnode_interest = INTEREST_FALSE;
		}

		// FIX: for now "node_download_nbr" is reduced in the receive callback
		// if (node_download_nbr > 0 &&
		//         (chunk_cnt[ptr->chunk_requested] == true)) {
		// 	node_download_nbr--;
		// }

	}

}

/*------------------------------------------------------------------*/
/**
 * brief: create control message to send in data packet
 *
 * params: control message enum
 *
 * return: new control message
 *
 *
 */

// static void choke_timer_callback(void *ptr) {
// 	//
// 	ctimer_reset(&choke_timer);
// 	node_choke_wait = WAIT_END;

// }



/*------------------------------------------------------------------*/
/**
 * brief: create control message to send in data packet
 *
 * params: control message enum
 *
 * return: new control message
 *
 *
 */
uint8_t create_ctrl_msg(ctrl_msg_t cm_type) {
	uint8_t new_ctrl_msg = (1 << cm_type);
	return new_ctrl_msg;
}


/*------------------------------------------------------------------*/
/**
 * brief: function prepares message packet for handshake
 *
 * params: void
 *
 * return: pointer to msg_pckt_t
 *
 *
 */

void prepare_handshake(msg_pckt_t *d_pckt) {
	msg_pckt_t *pckt_msg_hs = d_pckt;

	// convert chunk_cnt from bool to uint 32bit to send in the data packet
	for (int i = 0; i < DATA_TOTAL_CHUNKS; i++) {
		// pckt_msg_hs.chunk_type.self_chunks = ((chunk_cnt[i] == true) ?	
		//                                       (pckt_msg_hs.chunk_type.self_chunks |= (1 << i)) :	
		//                                       (pckt_msg_hs.chunk_type.self_chunks));
		pckt_msg_hs->chunk_type.self_chunks |= ((chunk_cnt[i] == true) ?
		                                      (1 << i) : 0);
	}

	pckt_msg_hs->ctrl_msg = create_ctrl_msg(HANDSHAKE_CTRL_MSG);

	memset(pckt_msg_hs->data, 0, sizeof(uint8_t) * MAX_PAYLOAD_LEN);

	// return (pckt_msg_hs);
}


/*------------------------------------------------------------------*/
/**
 * brief: function prepares message packet for interest
 *
 * params: void
 *
 * return: pointer to msg_pckt_t
 *
 *
 */

void prepare_interest(msg_pckt_t *d_pckt, const uint8_t chunk) {
	msg_pckt_t *pckt_msg_in = d_pckt;


	pckt_msg_in->ctrl_msg = create_ctrl_msg(INTEREST_CTRL_MSG);

	memset(pckt_msg_in->data, 0, sizeof(uint8_t) * MAX_PAYLOAD_LEN);

	pckt_msg_in->data[0] = chunk;


	// return (pckt_msg_in);
}

/*------------------------------------------------------------------*/
/**
 * brief: function prepares message packet for choke
 *
 * params: void
 *
 * return: pointer to msg_pckt_t
 *
 *
 */

void prepare_choke(msg_pckt_t *d_pckt) {
	msg_pckt_t *pckt_msg_ch = d_pckt;
	pckt_msg_ch->chunk_type.self_chunks = 0;
	pckt_msg_ch->ctrl_msg = create_ctrl_msg(CHOKE_CTRL_MSG);
	memset(pckt_msg_ch->data, 0, sizeof(uint8_t) * MAX_PAYLOAD_LEN);

	// return (pckt_msg_ch);
}

/*------------------------------------------------------------------*/
/**
 * brief: function prepares message packet for unchoke
 *
 * params: void
 *
 * return: pointer to msg_pckt_t
 *
 *
 */

void prepare_unchoke(msg_pckt_t *d_pckt) {
	msg_pckt_t *pckt_msg_uch = d_pckt;
	pckt_msg_uch->chunk_type.self_chunks = 0;
	pckt_msg_uch->ctrl_msg = create_ctrl_msg(UNCHOKE_CTRL_MSG);
	memset(pckt_msg_uch->data, 0, sizeof(uint8_t) * MAX_PAYLOAD_LEN);

	// return (pckt_msg_uch);
}



/*------------------------------------------------------------------*/
/**
 * brief: function prepares message packet for request to download
 *
 * params: void
 *
 * return: pointer to msg_pckt_t
 *
 *
 */

void prepare_request(msg_pckt_t *d_pckt) {
	msg_pckt_t *pckt_msg_rq = d_pckt;
	pckt_msg_rq->chunk_type.self_chunks = 0;
	pckt_msg_rq->ctrl_msg = create_ctrl_msg(REQUEST_CTRL_MSG);
	memset(pckt_msg_rq->data, 0, sizeof(uint8_t) * MAX_PAYLOAD_LEN);

	//TODO: select random chunk that we do not have and request for it

	// return (pckt_msg_rq);
}

/*------------------------------------------------------------------*/
/**
 * brief: initialize function to populate each neighbor with its
 * 			default values
 *
 * params: neighbor number
 *
 * return: void
 *
 *
 */


static void
unicast_send(const msg_pckt_t *pckt, const uip_ipaddr_t *send_addr) {
	if (pckt != NULL && send_addr != NULL) {
		simple_udp_sendto(&p2p_socket, (void *)pckt, sizeof(msg_pckt_t), send_addr);
	}
}


/*------------------------------------------------------------------*/
/**
 * brief: check if the nbr address already exists in the nbr_list,
 * 			this function should help not to overwrite the existing
 * 			nbr_list in case the nbr exists already
 *
 *
 * params: neighbor address
 *
 * return: 0 or 1
 *
 *
 */

// SHouldn't the return value be opposite. 1 (true) if it exists and 0 (false) if not found
uint8_t check_nbr_exist(const uip_ipaddr_t *nbr_addr)
{
	for (int i = 0; i < NEIGHBORS_LIST; i++) {

		if (uip_ipaddr_cmp(&nbr_list[i].nnode_addr, nbr_addr)) {
			return 0;
		}
	}
	return 1;
}




/*------------------------------------------------------------------*/
/**
 * brief: select a random missing chunk and return
 *
 *
 * params: void
 *
 * return: missing chunk
 *
 */

uint8_t missing_random_chunk(void) {

	uint8_t missing_chunk;

	uint8_t i = 0;

	// while ((chunk_cnt[random_rand() % i]) != false) {	// TODO: random piece pick
	while ((chunk_cnt[missing_chunk = i++]) != false) {	// this is not random but sequential
		if (i >= DATA_TOTAL_CHUNKS)
			break;
	}

	return missing_chunk;

}


/*------------------------------------------------------------------*/
/**
 * brief: initialize function to populate each neighbor with its
 * 			default values
 *
 * params: neighbor number
 *
 * return: void
 *
 *
 */

void nnode_init(int node_i) {

	// for (int i = 0; i < NUM_OF_NEIGHBORS; i++) {
	// 	if (i == 0)
	// 		nbr[i].node_addr = uip_ds6_nbr_head();
	// 	else if (uip_ds6_nbr_next(nbr[i - 1]) != NULL)
	// 		nbr[i].node_addr = uip_ds6_nbr_next(nbr[i - 1]);


	nbr_list[node_i].nnode_state = IDLE_STATE;
	nbr_list[node_i].nnode_ctrlmsg = NONE_CTRL_MSG;
	nbr_list[node_i].nnode_interest = INTEREST_NONE;
	nbr_list[node_i].nnode_choke = CHOKE_NONE;
	nbr_list[node_i].data_chunks = 0;
	nbr_list[node_i].chunk_interested = 0xff;	// to avoid init with 0 bug, where 0 can be chunk
	nbr_list[node_i].num_upload = 0;

}


// /*------------------------------------------------------------------*/
// /**
//  * brief: change state based on the current situation
//  *
//  *
//  * params: void
//  *
//  * return: state
//  *
//  *
//  */

// void node_statechange(void) {

// }




/*------------------------------------------------------------------*/
/**
 * brief: this function, node sends handshake i.e its own info about
 * 			which pieces it possess to its neighbor nodes
 *
 * params: sender address
 *
 * return: void
 *
 *
 */

void node_handshake(const uip_ipaddr_t *n_addr, const uint8_t n_idx) {
	// Good to actually test value of n_addr. If it is NULL what should happen

	LOG_INFO("Enter: node handshake\n");

	nnode_state_t *cb_data = &nbr_list[n_idx];
	static struct ctimer *t;
	t = &nbr_list[n_idx].c_timer;

	if (!uip_ipaddr_cmp(&nbr_list[n_idx].nnode_addr, n_addr)) {
		return;
	} else {

		msg_pckt_t data_packet;
		prepare_handshake(&data_packet);

		unicast_send(&data_packet, n_addr);

		nbr_list[n_idx].nnode_state = HANDSHAKING_STATE;

		if (ctimer_expired(t))
			ctimer_set(t, CLOCK_SECOND * 3, callback_ack_handshake, cb_data);
		else
			// LOG_ERR(“HANDSHAKE TIMER CANNOT START: NODE:(%u)”, n_idx);
			PRINTF((“HANDSHAKE TIMER CANNOT START: NODE:(%u)”, n_idx));
	}

	LOG_INFO("Exit: node handshake\n");
}


/*------------------------------------------------------------------*/
/**
 * brief: this function is response to the handshake and same as
 * 			handshake, sends its own info about which pieces it
 * 			possess to node as response
 *
 * params: void
 *
 * return: void
 *
 */

void node_ack_handshake(const uip_ipaddr_t *sender_addr) {
	msg_pckt_t data_packet;

	// for (int i = 0; i < NUM_OF_NEIGHBORS; i++) {
	prepare_handshake(&data_packet);
	unicast_send(&data_packet, sender_addr);	// send packet
	// nbr[i].nnode_state = handshaking;
	// }
}




/*------------------------------------------------------------------*/
/**
 * brief: node informs to its neighbors that it is interested in
 * 			downloading a piece
 *
 * params: void
 *
 * return: void
 *
 */

// n_addr is not an ip-address it is a neighbor.
void node_interest(const uip_ipaddr_t *n_addr, const uint8_t n_idx) {
	if ((n_addr == NULL) || !uip_ipaddr_cmp(&nbr_list[n_idx].nnode_addr, n_addr))
		return;

	nnode_state_t *cb_data = &nbr_list[n_idx];
	static struct ctimer *t;
	t = &nbr_list[n_idx].c_timer;

	if (node_download_nbr < NODES_DOWNLOAD) {
		uint8_t chunk = missing_random_chunk();
		// for (int i = 0; i < NUM_OF_NEIGHBORS && nbr_list[i].nnode_addr != NULL; i++) {
		for (int i = 0; i < NUM_OF_NEIGHBORS; i++) {
			if (nbr_list[i].data_chunks & (1 << chunk)) {

				msg_pckt_t data_packet;
				prepare_interest(&data_packet, chunk);

				nbr_list[n_idx].nnode_interest = INTEREST_TRUE;
				unicast_send(&data_packet, &nbr_list[i].nnode_addr);
				nbr_list[n_idx].chunk_requested = chunk;
				nbr_list[n_idx].nnode_state = INTEREST_INFORMING_STATE;


				if (ctimer_expired(t))
					ctimer_set(t, CLOCK_SECOND * 3, callback_interest_informing, cb_data);
				else
					// LOG_ERR(“INTEREST TIMER CANNOT START: NODE: (%u)”, n_idx);
					PRINTF(“INTEREST TIMER CANNOT START: NODE: (%u)”, n_idx);

			} else {
				continue;
			}
		}
	}
}


/*------------------------------------------------------------------*/
/**
 * brief: send choke/unchoke to nbr based on i < mu
 *
 * params: void
 *
 * return: choke_state_t
 *
 */

// Note uip_ipaddr_t is a neighbor not an address. This will not work for unicast send.
choke_state_t node_choke_unchoke(const uip_ipaddr_t *sender_addr) {
	msg_pckt_t data_packet;
	choke_state_t ch_uch_state;

	if (node_upload_nbr >= NODES_UPLOAD) {
		prepare_choke(&data_packet);
		unicast_send(&data_packet, sender_addr);	// send packet
		ch_uch_state = CHOKE_FALSE;

	} else if (node_upload_nbr < NODES_UPLOAD) {
		prepare_unchoke(&data_packet);
		unicast_send(&data_packet, sender_addr);	// send packet
		ch_uch_state = CHOKE_FALSE;
	}

	return ch_uch_state;
}



/*------------------------------------------------------------------*/
/**
 * brief: choke wait for 5 seconds before sending another request
 *
 * params: void
 *
 * return: void
 *
 */

// void node_choke_wait(void) {

// 	// TODO: set timer, when timer expires
// 	// TODO: set node_choke_wait = WAIT_END



// 	nbr[i].nnode_state = HANDSHAKED;
// 	nbr[i].nnode_interest = INTEREST_FALSE;
// 	// TODO: wait function
// 	// wait(5); // wait for 5 seconds

// 	// We talked about whether your should test if the timer is already running.
// 	// If it is running you should not overwrite the timer. It should be handled as an
// 	// error case

// 	ctimer_set(&choke_timer, CLOCK_SECOND * 5, callback, NULL);

// }



/*------------------------------------------------------------------*/
/**
 * brief: request to start downloading
 *
 * params: void
 *
 * return: void
 *
 */

void node_request(const uip_ipaddr_t *n_addr, const uint8_t n_idx) {
	if ((n_addr == NULL) || !uip_ipaddr_cmp(&nbr_list[n_idx].nnode_addr, n_addr))
		return;

	nnode_state_t *cb_data = &nbr_list[n_idx];
	static struct ctimer *t;
	t = &nbr_list[n_idx].c_timer;

	if (node_download_nbr < NODES_DOWNLOAD) {

		// TODO: in leecher mode never request to more than two nodes
		msg_pckt_t data_packet;
		prepare_request(&data_packet);
		unicast_send(&data_packet, n_addr);

		nbr_list[n_idx].nnode_state = DOWNLOADING_STATE;
		node_download_nbr++;

		if (ctimer_expired(t))
			ctimer_set(t, CLOCK_SECOND * 3, callback_request, cb_data);
		else
			// LOG_ERR(“INTEREST TIMER CANNOT START: NODE: (%u)”, n_idx);
			PRINTF(“INTEREST TIMER CANNOT START: NODE: (%u)”, n_idx);
	}
	// node_received();
	// to check if the requested piece is received
	// if yes then change state and interest
}


/*------------------------------------------------------------------*/
/**
 * brief: request to start downloading
 *
 * params: void
 *
 * return: void
 *
 */

// n_addr not an IP-address

void node_received(const uip_ipaddr_t *n_addr, const uint8_t n_idx) {
	if ((n_addr == NULL) || !uip_ipaddr_cmp(&nbr_list[n_idx].nnode_addr, n_addr))
		return;

	if (chunk_cnt[nbr_list[n_idx].chunk_requested] == true) {

		node_download_nbr--;

		nbr_list[n_idx].nnode_state = HANDSHAKED_STATE;
		nbr_list[n_idx].nnode_interest = INTEREST_FALSE;
	}

	// TODO: change state to handshaked
	// TODO: change interest to false
}

/*------------------------------------------------------------------*/
/**
 * brief: node is uploading a piece to its neighbor node
 *
 * params: chunk, sender_address
 *
 * return: void
 *
 */

// send_addr not an IP it is a neighbor
void node_upload(const uint8_t chunk, const uip_ipaddr_t *sender_addr) {

	node_upload_nbr += 1;
	msg_pckt_t *data_packet = NULL;


	memset(data_packet->data, 0, sizeof(uint8_t)*MAX_PAYLOAD_LEN);

	for (int i = 0, j = 0, k = 0, l = 1; i < DATA_CHUNK_SIZE; i++) {

		data_packet->ctrl_msg = create_ctrl_msg(LAST_CTRL_MSG);

		if (j < MAX_PAYLOAD_LEN) {
			data_packet->data[j] = seq_idd[(chunk * DATA_CHUNK_SIZE) + k + j];
			j++;
		} else {
			l |= l << k / 32;
			data_packet->chunk_type.req_chunk_block |= l;
			data_packet->chunk_type.req_chunk_block <<= 8;
			data_packet->chunk_type.req_chunk_block |= chunk;
			unicast_send(data_packet, sender_addr);
			memset(data_packet->data, 0, sizeof(uint8_t)*MAX_PAYLOAD_LEN);
			k += j;
			j = 0;
		}
	}
}


/*------------------------------------------------------------------*/
/**
 * brief: this function checks if the node received all the chunks
 *
 * params: void
 *
 * return: bool
 *
 */

bool node_chunk_check(void) {
	for (int i = 0; i < DATA_TOTAL_CHUNKS; i++) {
		if (chunk_cnt[i] != true)
			return false;
		else
			continue;
	}
	return true;
}



/*------------------------------------------------------------------*/
/**
 * brief: this function returns the index of the address
 *
 * params: address
 *
 * return: int8_t
 *
 */

int8_t check_index(const uip_ipaddr_t *n_addr) {
	int i;
	// uip_ipaddr_t address = n_addr;
	// for (i = 0; i < NEIGHBORS_LIST && nbr_list[i].nnode_addr != NULL; i++) {
	for (i = 0; i < NEIGHBORS_LIST; i++) {

		// if (nbr_list[i].nnode_addr == n_addr)
		if (uip_ipaddr_cmp(&nbr_list[i].nnode_addr, n_addr))
			break;
		else
			continue;
	}

	return ((i > -1 && i < NEIGHBORS_LIST) ? i : -1);
}

/*------------------------------------------------------------------*/
/**
 * brief: print the nbr list
 *
 * params: void
 *
 * return: void
 *
 */

void nbr_list_print(void) {
	int i;
	// uip_ipaddr_t address = n_addr;
	// for (i = 0; i < NEIGHBORS_LIST && nbr_list[i].nnode_addr != NULL; i++) {
	for (i = 0; i < NEIGHBORS_LIST; i++) {
		if(&nbr_list[i].nnode_addr != NULL){
			PRINTF("node %d address			: %u\n", i, nbr_list[i].nnode_addr);
			PRINTF("node state 				: %d\n", nbr_list[i].nnode_state);
			PRINTF("node ctrl msg 			: %d\n", nbr_list[i].nnode_ctrlmsg);
			PRINTF("node interest 			: %d\n", nbr_list[i].nnode_interest);

			PRINTF("node choke 				: %d\n", nbr_list[i].nnode_choke);
			PRINTF("node chunks 			: %d\n", nbr_list[i].data_chunks);
			PRINTF("node chunk req 			: %d\n", nbr_list[i].chunk_requested);
			PRINTF("node block 				: %d\n", nbr_list[i].chunk_block);
			PRINTF("node failed req 		: %u\n", nbr_list[i].failed_dlreq);
			PRINTF("node chunk interested 	: %d\n",nbr_list[i].chunk_interested);
			PRINTF("node num upload 		: %d\n",nbr_list[i].num_upload);
		} else {
			PRINTF("nbr list exit\n");
		}

	}
}

/*------------------------------------------------------------------*/
/**
 * brief: this functions switches between systemm modes
 *
 * params: system_mode_t
 *
 * return: system_mode_t
 *
 */
system_mode_t system_mode_pp(system_mode_t system_mode) {
	system_mode_t sys_mode;
	switch (system_mode) {
	case MODE_IDLE:

		/* TODO:
		*  when part of the network is formed
		*
		*/


		// system_mode = MODE_LEECHER;
		sys_mode = MODE_LEECHER;
		break;
	case MODE_LEECHER:
		// for (int i = 0; i < NEIGHBORS_LIST; i++) {
		// 	if ((system_next_state_dl < LAST_COMM_STATE) &&
		// 	        (new_ctrl_msg_dl < LAST_CTRL_MSG) &&
		// 	        sm_download[system_next_state_dl].ctrl_msg == new_ctrl_msg_dl &&
		// 	        sm_download[system_next_state_dl].curr_state == system_next_state_dl &&
		// 	        sm_download[system_next_state_dl].sm_handler_dl != NULL) {

		// 		system_next_state_dl = (*sm_download[system_next_state_dl].sm_handler_dl)();

		// 	}
		// }

		for (int i = 0; i < NEIGHBORS_LIST; i++) {
			if ((nbr_list[i].nnode_state < LAST_COMM_STATE) &&
			        (nbr_list[i].nnode_ctrlmsg < LAST_CTRL_MSG) &&
			        sm_download[nbr_list[i].nnode_state].curr_state == nbr_list[i].nnode_state &&
			        sm_download[nbr_list[i].nnode_state].ctrl_msg == nbr_list[i].nnode_ctrlmsg &&
			        sm_download[nbr_list[i].nnode_state].sm_handler_dl != NULL) {

				(*sm_download[nbr_list[i].nnode_state].sm_handler_dl)(&nbr_list[i].nnode_addr, i);
			}


			if (sm_download[nbr_list[i].nnode_state].curr_state == HANDSHAKED_STATE) {
				if (nbr_list[i].nnode_interest == INTEREST_FALSE) {

#if 0 // we can call node_interest() function here
					node_interest(&nbr_list[i].nnode_addr, i);

#else // or we can modify the nnode_interest so state machine calls the 
					// node_interest() function in next iteration

					nbr_list[i].nnode_interest = ACKHANDSHAKE_CTRL_MSG;
#endif
				}
#if 0 // handled in the timer callback function check_interest_informing()
				else {
					// wait for 5 seconds before sending messages to same node again
					if (WAIT_FALSE == node_choke_wait()) {
						// TODO
						// can send messages again
						// node_interest(); // to send interest again
					}
				}
#endif // handled in the timer callback function check_interest_informing()

			}
#if 0 // handled in the timer callback function check_interest_informing()
			else if (sm_download[nbr_list[i].nnode_state].curr_state == INTEREST_INFORMING_STATE) {
				if (nbr_list[i].nnode_ctrlmsg == UNCHOKE) {
					nbr_list[i].nnode_state = INTEREST_INFORMED_STATE;
					nbr_list[i].nnode_ctrlmsg = NONE_CTRL_MSG; // to comply with state machine
				} else {
					nbr_list[i].nnode_state = HANDSHAKED_STATE;
					nbr_list[i].nnode_interest = INTEREST_FALSE;
				}
			}
#endif // handled in the timer callback function check_interest_informing()
		}

		/*
		*  when all the chunks are received change system mode to seeder
		*
		*/
		if (false == node_chunk_check()) {
			/* no system mode change */
			sys_mode = MODE_LEECHER;
		} else {
			sys_mode = MODE_SEEDER;	// change system mode to seeder
		}
		break;
	case MODE_SEEDER:

		/* TODO
		*  when no one is requesting for chunks
		*
		*/
		sys_mode = MODE_IDLE;
		break;
	default:

		/* TODO
		*  should never come here
		*
		*/

		break;
	}
	return sys_mode;
}


























#if 0


----------------------process_1 comm task------------start

// triggering is simulated as time until the minimum required
// network is formed
// ex: 60 of 64 nodes formed network at root so root starts
// dissemination
PROCESS_THREAD(udp_server_process, ev, data)
{
	PROCESS_BEGIN();

	/* Initialize DAG root */
	NETSTACK_ROUTING.root_start();

	/* Initialize UDP connection */
	simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
	                    UDP_CLIENT_PORT, udp_rx_callback);


	while (1) {








		// downloading
		comm_states_t system_next_state_dl = IDLE;

		// get newctlmsg from callback
		ctrl_msg_t new_ctrl_msg_dl = get_from_callback();
		if ((system_next_state_dl < LAST_COMM_STATE) && (new_ctrl_msg_dl < LAST_CTRL_MSG) &&
		        sm_download[system_next_state_dl].ctrl_msg == new_ctrl_msg_dl &&
		        sm_download[system_next_state_dl].curr_state == system_next_state_dl &&
		        sm_download[system_next_state_dl].sm_handler_dl != NULL) {

			system_next_state_dl = (*sm_download[system_next_state_dl].sm_handler_dl)();

		}



		// uploading
		ctrl_msg_t new_ctrl_msg_up = get_from_callback();
		if ((new_ctrl_msg_up < LAST_CTRL_MSG) &&
		        sm_upload[new_ctrl_msg_up].ctrl_msg == new_ctrl_msg_up) {

			(*sm_upload[new_ctrl_msg_up].sm_handler_up)();
		}


	}


	PROCESS_END();
}












-------------------- -process_1 comm task---------- -end



-------------------- -process_2 neighbor building---------- -start

// while (1) {

// 	nnode_state_t nbr_list[NEIGHBORS_LIST];
// 	for (nbr = uip_ds6_nbr_head(); nbr != NULL; nr = uip_ds6_nbr_next(nbr)) {
// 		nbr_new.node_addr = nbr;
// 	}


// }

-------------------- -process_2 neighbor building---------- -end

-------------------- -process_3 optimistic unchoking---------- -start





-------------------- -process_3 optimistic unchoking---------- -end



#endif











