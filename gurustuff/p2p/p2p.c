
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

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define DEBUG DEBUG_NONE
#include "net/ipv6/uip-debug.h"

#include "p2p.h"

#define WITH_SERVER_REPLY  1
#define UDP_PORT	8765
#define UDP_PORT_2	7654
#define UDP_PORT_3	5432
#define UDP_PORT_4	5678


static struct ctimer choke_timer;
static wait_state_t node_choke_wait;

extern struct simple_udp_connection p2p_socket;

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
 * brief: create control message to send in data packet
 *
 * params: control message enum
 *
 * return: new control message
 *
 *
 */

static void choke_timer_callback(void *ptr) {
	//
	ctimer_reset(&choke_timer);
	node_choke_wait = WAIT_END;

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
msg_pckt_t* prepare_handshake(void) {
	msg_pckt_t pckt_msg_hs;

	// convert chunk_cnt from bool to uint 32bit to send in the data packet
	for (int i = 0; i < DATA_TOTAL_CHUNKS; i++) {
		pckt_msg_hs.self_chunks = (chunk_cnt[i] ?
		                           (pckt_msg_hs.self_chunks |= (1 << i)) :
		                           (pckt_msg_hs.self_chunks));
	}

	pckt_msg_hs.ctrl_msg = create_ctrl_msg(HANDSHAKE_CTRL_MSG);

	memset(pckt_msg_hs.data, 0, sizeof(uint8_t) * MAX_PAYLOAD_LEN);

	return (&pckt_msg_hs);
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

msg_pckt_t* prepare_interest(const uint8_t chunk) {
	msg_pckt_t pckt_msg_in;


	pckt_msg_in.ctrl_msg = create_ctrl_msg(INTEREST_CTRL_MSG);

	memset(pckt_msg_in.data, 0, sizeof(uint8_t) * MAX_PAYLOAD_LEN);

	pckt_msg_in.data[0] = chunk;


	return (&pckt_msg_in);
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

msg_pckt_t* prepare_choke(void) {
	msg_pckt_t pckt_msg_ch;
	pckt_msg_ch.self_chunks = 0;
	pckt_msg_ch.ctrl_msg = create_ctrl_msg(CHOKE_CTRL_MSG);
	memset(pckt_msg_ch.data, 0, sizeof(uint8_t) * MAX_PAYLOAD_LEN);

	return pckt_msg_ch;
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

msg_pckt_t* prepare_unchoke(void) {
	msg_pckt_t pckt_msg_uch;
	pckt_msg_uch.self_chunks = 0;
	pckt_msg_uch.ctrl_msg = create_ctrl_msg(UNCHOKE_CTRL_MSG);
	memset(pckt_msg_uch.data, 0, sizeof(uint8_t) * MAX_PAYLOAD_LEN);

	return pckt_msg_uch;
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

msg_pckt_t* prepare_request(void) {
	msg_pckt_t pckt_msg_rq;
	pckt_msg_rq.self_chunks = 0;
	pckt_msg_rq.ctrl_msg = create_ctrl_msg(REQUEST_CTRL_MSG);
	memset(pckt_msg_rq.data, 0, sizeof(uint8_t) * MAX_PAYLOAD_LEN);

	//TODO: select random chunk that we do not have and request for it

	return pckt_msg_rq;
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
// Pass addresses as pointers. 
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
uint8_t check_nbr_exist(const uip_ds6_nbr_t *nbr_addr)
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
 * brief: this function, node sends its own info about which pieces
 * 			it possess to its neighbor nodes
 *
 * params: sender address
 *
 * return: void
 *
 *
 */

void node_handshake(const uip_ds6_nbr_t *n_addr, const uint8_t n_idx) {
	// Good to actually test value of n_addr. If it is NULL what should happen

	if (!uip_ipaddr_cmp(&nbr_list[i].nnode_addr, n_addr)) {
		return;
	} else {

		msg_pckt_t *data_packet;
		data_packet = prepare_handshake();

		unicast_send(data_packet, n_addr);

		nbr_list[i].nnode_state = HANDSHAKING_STATE;
	}

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

void node_ack_handshake(uip_ds6_nbr_t *sender_addr) {
	msg_pckt_t *data_packet;

	// for (int i = 0; i < NUM_OF_NEIGHBORS; i++) {
	data_packet = prepare_handshake();
	unicast_send(data_packet, sender_addr);	// send packet
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
void node_interest(const uip_ds6_nbr_t *n_addr, const uint8_t n_idx) {
	if ((n_addr == NULL) || !uip_ipaddr_cmp(&nbr_list[n_idx].nnode_addr, n_addr))
		return;

	if (node_download_nbr < NODES_DOWNLOAD) {
		//TODO: random chunk select implementation
		uint8_t chunk = missing_random_chunk();
		for (int i = 0; i < NUM_OF_NEIGHBORS && nbr_list[i].nnode_addr != NULL; i++) {
			if (nbr_list[i].data_chunks & (1 << chunk)) {

				msg_pckt_t *data_packet;
				data_packet = prepare_interest(chunk);

				nbr_list[n_idx].nnode_interest = INTEREST_TRUE;
				unicast_send(data_packet, &nbr_list[i].nnode_addr);
				nbr_list[n_idx].chunk_requested = chunk;
				nbr_list[n_idx].nnode_state = INTEREST_INFORMING_STATE;
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

// Note uip_ds6_nbr_t is a neighbor not an address. This will not work for unicast send.
choke_state_t node_choke_unchoke(const uip_ds6_nbr_t *sender_addr) {
	msg_pckt_t *data_packet;
	choke_state_t ch_uch_state;

	if (node_upload_nbr >= NODES_UPLOAD) {
		data_packet = prepare_choke();
		unicast_send(data_packet, sender_addr);	// send packet
		ch_uch_state = CHOKE_FALSE;

	} else if (node_upload_nbr < NODES_UPLOAD) {
		data_packet = prepare_unchoke();
		unicast_send(data_packet, sender_addr);	// send packet
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

comm_states_t node_choke_wait() {

	// TODO: set timer, when timer expires
	// TODO: set node_choke_wait = WAIT_END



	nbr[i].nnode_state = HANDSHAKED;
	nbr[i].nnode_interest = INTEREST_FALSE;
	// TODO: wait function
	// wait(5); // wait for 5 seconds
	
	// We talked about whether your should test if the timer is already running. 
	// If it is running you should not overwrite the timer. It should be handled as an
	// error case

	ctimer_set(&choke_timer, CLOCK_SECOND * 5, callback, NULL);

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

comm_states_t node_request(const uip_ds6_nbr_t *n_addr, const uint8_t n_idx) {
	if ((n_addr == NULL) || !uip_ipaddr_cmp(&nbr_list[n_idx].nnode_addr, n_addr))
		return;
	// TODO: in leecher mode never request to more than two nodes
	msg_pckt_t *data_packet;
	data_packet = prepare_request();
	unicast_send(data_packet, n_addr);

	nbr_list[n_idx].nnode_state = DOWNLOADING_STATE;
	node_download_nbr++;

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
void node_received(const uip_ds6_nbr_t *n_addr, const uint8_t n_idx) {
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
void node_upload(const uint8_t chunk, const uip_ds6_nbr_t *sender_addr) {

	node_upload_nbr += 1;
	msg_pckt_t *data_packet;


	memset(data_packet.data, 0, sizeof(uint8_t)*MAX_PAYLOAD_LEN);

	for (int i = 0, j = 0, k = 0; i < DATA_CHUNK_SIZE; i++) {

		data_packet.ctrl_msg = create_ctrl_msg(LAST_CTRL_MSG);

		if (j < MAX_PAYLOAD_LEN) {
			data_packet.data[j] = seq_idd[(chunk * DATA_CHUNK_SIZE) + k + j++];
		} else {
			unicast_send(data_packet, sender_addr);
			memset(data_packet.data, 0, sizeof(uint8_t)*MAX_PAYLOAD_LEN);
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
 * return: uint8_t
 *
 */

int8_t check_index(const uip_ds6_nbr_t *n_addr) {
	for (int i = 0; i < NEIGHBORS_LIST && nbr_list[i].nnode_addr != NULL; i++) {
		if (nbr_list[i].nnode_addr == n_addr)
			break;
		else
			continue;
	}

	return (i > -1 && i < NEIGHBORS_LIST) ? i : -1;
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
system_mode_t system_mode_pp(system_mode) {
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

				(*sm_download[nbr_list[i].nnode_state].sm_handler_dl)(nbr_list[i].nnode_addr, i);
			}


			if (sm_download[nbr_list[i].nnode_state].curr_state == HANDSHAKED_STATE) {
				if (nbr_list[i].nnode_interest) {
					// TODO
					// state -> interest informed
				} else {
					// wait for 5 seconds before sending messages to same node again
					if (WAIT_FALSE == node_choke_wait()) {
						// TODO
						// can send messages again
						// node_interest(); // to send interest again
					}
				}

			} else if (sm_download[nbr_list[i].nnode_state].curr_state == INTEREST_INFORMING_STATE) {
				if (nbr_list[i].nnode_ctrlmsg == UNCHOKE) {
					nbr_list[i].nnode_state = INTEREST_INFORMED_STATE;
					nbr_list[i].nnode_ctrlmsg = NONE_CTRL_MSG; // to comply with state machine
				} else {
					nbr_list[i].nnode_state = HANDSHAKED_STATE;
					nbr_list[i].nnode_interest = INTEREST_FALSE;
				}
			}
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

















































































