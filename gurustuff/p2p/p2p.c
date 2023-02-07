
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


static struct simple_udp_connection udp_conn;
// static struct simple_udp_connection udp_conn_2;
// static struct simple_udp_connection udp_conn_3;
// static struct simple_udp_connection udp_conn_4;

PROCESS(comm_process, "UDP server");
AUTOSTART_PROCESSES(&comm_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
	LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
	LOG_INFO_6ADDR(sender_addr);
	LOG_INFO_("\n");
#if WITH_SERVER_REPLY
	/* send back the same string to the client as an echo reply */
	LOG_INFO("Sending response.\n");
	simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
#endif /* WITH_SERVER_REPLY */
}
/*---------------------------------------------------------------------------*/








static nnode_state_t nbr[NUM_OF_NEIGHBORS];

static bool chunks[512];



//--------------------------------------------

static void node_handshake(void);		// set HANDSHAKING_STATE with that neighbor
static void node_ack_handshake(void);	// set HANDSHAKED_STATE with that neighbor
static void node_interest(void);		// set INTEREST_INFORMING with that neighbor
static void node_choke_wait();			// wait for 5 seconds
static void node_choke_unchoke(void);	// refer point 2
static void node_request(void);			// set DOWNLOADING_STATE with that neighbor
static void node_upload(void);			// set UPLOADING_STATE with that neighbor












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

static msg_pckt_t *prepare_handshake(void) {
	msg_pckt_t hs_packet;
	hs_packet.comm_msg = (1 << 0);
	for (int i = 0; i < 512; i++)
		hs_packet.data[i] = chunks[i];
	return hs_packet;
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

static msg_pckt_t *prepare_request(void) {
	msg_pckt_t rqst_packet;
	rqst_packet.comm_msg = (1 << 4);
	rqst_packet.data = NULL;
}


/*------------------------------------------------------------------*/
/**
 * brief: initialize function to populate each neighbor with its
 * 			default values
 *
 * params: void
 *
 * return: void
 *
 *
 */

static void nnode_init(void) {

	for (int i = 0; i < NUM_OF_NEIGHBORS; i++) {
		if (i == 0)
			nbr[i].node_addr = uip_ds6_nbr_head();
		else if (uip_ds6_nbr_next(nbr[i - 1]) != NULL)
			nbr[i].node_addr = uip_ds6_nbr_next(nbr[i - 1]);

		nbr[i].nnode_state = IDLE;
		nbr[i].nnode_interest = NONE;
		nbr[i].nnode_choke = NONE;
		nbr[i].numUL = 0;
	}

}



// handshake();
// ackhandshake();
// interest();
// wait();
// choke();
// unchoke();
// request();
// upload();








// node i send/request functions




/*------------------------------------------------------------------*/
/**
 * brief: this function, node sends its own info about which pieces
 * 			it possess to its neighbor nodes
 *
 * params: void
 *
 * return: nextstate
 *
 *
 */

static comm_states_t node_handshake(void) {
	msg_pckt_t *data_packet;

	for (int i = 0; i < NUM_OF_NEIGHBORS; i++) {
		data_packet = prepare_handshake();
		send(&data_packet);	// send packet
		nbr[i].nnode_state = handshaking;
	}
}


/*------------------------------------------------------------------*/
/**
 * brief: this function is response to the handshake and same as
 * 			handshake, sends its own info about which pieces it
 * 			possess the node as response
 *
 * params: void
 *
 * return: nextstate
 *
 */

static comm_states_t node_ack_handshake(void) {
	msg_pckt_t *data_packet;

	for (int i = 0; i < NUM_OF_NEIGHBORS; i++) {
		data_packet = prepare_handshake();
		send(&data_packet);	// send packet
		// nbr[i].nnode_state = handshaking;
	}
}




/*------------------------------------------------------------------*/
/**
 * brief: node informs to its neighbors that it is interested in
 * 			downloading a piece
 *
 * params: void
 *
 * return: nextstate
 *
 */

static comm_states_t node_interest(void) {
	if (i < NODES_DOWNLOAD) {
		chunk = random(chunk);
		for (int i = 0; i < NUM_OF_NEIGHBORS; i++) {
			for (int j = 0; j < sizeof(data_chunks) / sizeof(data_chunks[0]); j++) {
				if (nbr[i].data_chunks[j] != chunk) {
					continue;
				} else {
					// send request to nbr[i].node_addr
					send(request to nbr[i].node_addr);
					nbr[i].nnode_interest = INTEREST_TRUE;
				}
			}
		}
	}
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

static comm_states_t node_choke_wait() {
	nbr[i].nnode_state = HANDSHAKED;
	nbr[i].nnode_interest = INTEREST_FALSE;
	wait(5); // wait for 5 seconds
}


// static void node_choke_unchoke() {

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

static comm_states_t node_request(void) {
	msg_pckt_t *data_packet;
	data_packet = prepare_request();
	send(data_packet);
}



/*------------------------------------------------------------------*/
/**
 * brief: node is uploading a piece to its neighbor node
 *
 * params: void
 *
 * return: void
 *
 */

static comm_states_t node_upload(void) {
	nbr[i].comm_states_t = UPLOADING;
	int chunk_block_size = chunk / NUM_OF_BLOCKS;
	uint8_t *each_block;

	// each_block = (uint8_t *)malloc(chunk_block_size * sizeof(uint8_t));

	for (int i = 0; i < chunk_piece_size; i++) {
		each_block[i] = data[i];
		send(&each_block);
	}

	free(each_block);
}



// while (1) {






// 	switch (case) {
// 	case == ack_handshake:
// 		nbr.nnode_state = HANDSHAKED;
// 		node_interest();
// 		break;
// 	case == choke/unchoke:
// 		nbr.nnode_choke = UNCHOKE;
// 		if (unchoke)
// 			node_request();
// 		break;
// 	default:
// 		break;
// 	}






// downloading mode state machine:


typedef comm_states_t (*downloading_state_handler)(void);

typedef struct {
	comm_states_t curr_state;					// current state
	ctrl_msg_t ctrl_msg;						// control message
	downloading_state_handler sm_handler_dl;	// handler function sm_handler_dl returns next state
} state_machine_download;

state_machine_download sm_download[] {
// curr_state, ctrl_msg, sm_handler_dl
	{IDLE_STATE,					NONE_CTRL_MSG,  		node_handshake},
	{HANDSHAKING_STATE,				ACKHANDSHAKE_CTRL_MSG,	node_interest},
	{NULL,							NULL,					NULL},
	{INTEREST_INFORMING_STATE,			NULL,					NULL},
	{INTEREST_INFORMING_C_STATE,	CHOKE_CTRL_MSG, 		node_choke_wait}, // state=handshaked, interest=false
	{INTEREST_INFORMING_UC_STATE,	UNCHOKE_CTRL_MSG, 		node_request}, // state=interest_informed
	{NULL,							NULL,					NULL},
	{NULL,							NULL,					NULL},
	{NULL,							NULL,					NULL},
	{NULL,							NULL,					NULL}
};


// ----------------





// uploading mode state machine:


typedef comm_states_t (*uploading_state_handler)(void);

// ----------------check
// typedef struct {
// 	comm_states_t curr_state;
// 	comm_states_t new_state;
// 	uploading_state_handler sm_handler_dl;
// } state_machine_upload;


// state_machine_upload sm_up[]{
// 	{handshake, handshaked, ackhandshake},
// 	{handshaked, interest, choke_unchoke},
// 	{interest, choke, unchoke},
// 	{interest, request, upload}
// }

// ----------------more feasible


// ctrl_msg, function




typedef struct {
	ctrl_msg_t ctrl_msg;
	uploading_state_handler sm_handler_dl;
} state_machine_upload;


state_machine_upload sm_upload[] {
	//ctrl_msg, sm_handler_up
	{NULL,					NULL},
	{HANDSHAKE_CTRL_MSG,	node_ack_handshake},
	{INTEREST_CTRL_MSG,		node_choke_unchoke},
	{REQUEST_CTRL_MSG,		node_upload},
	{NULL,					NULL},
	{NULL,					NULL},
	{NULL,					NULL},
	{NULL,					NULL}
}

// ----------------









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





















































































