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
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "p2p.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

// #define WITH_SERVER_REPLY  1
// #define UDP_CLIENT_PORT	8765
// #define UDP_SERVER_PORT	5678
#define P2P_PORT 5432

#define SEND_INTERVAL		  (60 * CLOCK_SECOND)


static struct process_post_comm post_comm_process;

static struct simple_udp_connection p2p_socket;
// process_event_t node_comm_upload_event;
enum {
	HANDSHAKE_EVENT,
	INTEREST_EVENT,
	REQUEST_EVENT
};


/*---------------------------------------------------------------------------*/
PROCESS(node_comm_process, "UDP client");
PROCESS(nbr_construction_process, "Neighbor construction");
AUTOSTART_PROCESSES(&node_comm_process, &nbr_construction_process);
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
	static int8_t node_idx = -1;
	static uint8_t recv_block_count = 0;

	node_idx = check_index(sender_addr);

	if ((0 > node_idx) && (node_idx >= NEIGHBORS_LIST)) {
		LOG_ERROR("Node doesn't exists in the node list");
	} else {
		msg_pckt_t *this;
		process_post_data_t *post_data;

		post_data->sender_addr;
		post_data->data = data;

		this = (msg_pckt_t *)data;

		// TODO: populate nbr node based on the ctrl_msg
		// nbr_list[node_idx].node_addr = sender_addr;
		if (this->ctrl_msg == ACKHANDSHAKE_CTRL_MSG) {
			nbr_list[node_idx].nnode_ctrlmsg = this->ctrl_msg;
			nbr_list[node_idx].data_chunks = this->self_chunks;
		} else if (this->ctrl_msg == HANDSHAKE_CTRL_MSG) {
			process_post(&node_comm_process, HANDSHAKE_EVENT, post_data);
		} else if (this->ctrl_msg == INTEREST_CTRL_MSG) {
			process_post(&node_comm_process, INTEREST_EVENT, post_data);
		} else if (this->ctrl_msg == REQUEST_CTRL_MSG) {
			process_post(&node_comm_process, REQUEST_EVENT, post_data);
		} else if (this->ctrl_msg == LAST_CTRL_MSG) {

			LOG_INFO("Received response '%.*s' from ", datalen, (char *) this->data);
			recv_block_count++;

			if (recv_block_count >= 4)
				recv_block_count = 0;
		}
		// else if (this->ctrl_msg == UNCHOKE_CTRL_MSG) {
		// 	nbr_list[node_idx].nnode_ctrlmsg = this->ctrl_msg;
		// } else if (this->ctrl_msg == CHOKE_CTRL_MSG) {
		// 	// TODO: set timer for 5 seconds
		// 	nbr_list[node_idx].nnode_ctrlmsg = this->ctrl_msg;
		// }
		else {
			nbr_list[node_idx].nnode_ctrlmsg = this->ctrl_msg;
		}



	}



	// LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
	LOG_INFO("In node callback\n");
	LOG_INFO("Received message from: ");
	LOG_INFO_6ADDR(sender_addr);
// #if LLSEC802154_CONF_ENABLED
	// LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
// #endif
	LOG_INFO_("\n");

}
/*---------------------------------------------------------------------------*/
static void
upload_event_handler(process_event_t ev, const process_post_data_t *post_data)
{

	int8_t node_idx = -1;
	node_idx = check_index(post_data->sender_addr);
	if (-1 == node_idx) {
		LOG_ERROR("Node doesn't exists in the node list");
		return;
	} else {
		switch (ev) {
		case HANDSHAKE_EVENT:
			node_ack_handshake(post_data->sender_addr);
			break;
		case INTEREST_EVENT:
			nbr_list[node_idx].chunk_interested = post_data->data[0];
			nbr_list[node_idx].nnode_choke = node_choke_unchoke(post_data->sender_addr);
			break;
		case REQUEST_EVENT:
			nbr_list[node_idx].nnode_state = UPLOADING_STATE;
			node_upload(nbr_list[node_idx].chunk_interested, post_data->sender_addr);
			nbr_list[node_idx].num_upload += 4;
			break;
		}
	}
}
/*---------------------------------------------------------------------------*/
// static uint8_t
// check_nbr_exist(const uip_ds6_nbr_t *nbr_addr)
// {
// 	for (int i = 0; i < NEIGHBORS_LIST; i++) {
// 		if (nbr_list[i].node_addr == nbr_addr) {
// 			return 0;
// 		}
// 	}
// 	return 1;
// }


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_comm_process, ev, data)
{
	static struct etimer periodic_timer;
	static unsigned count;
	static char str[32];
	static msg_pckt_t data_pckt;
	static uip_ipaddr_t dest_ipaddr;

	PROCESS_BEGIN();

	system_mode_t system_mode = MODE_IDLE;

	/* Initialize UDP connection */
	simple_udp_register(&p2p_socket, P2P_PORT, NULL,
	                    P2P_PORT, udp_rx_callback);

	etimer_set(&et, random_rand() % SEND_INTERVAL);
	while (1) {
		// PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer) ||
		//                          ev == node_comm_upload_event);
		// // TODO: line75 ||

		if ((etimer_expired(&et) && (uip_ds6_route_num_routes() > NUM_OF_NODES)) ||
		        ev == node_comm_upload_event) {

			system_mode = system_mode_pp(system_mode);

			upload_event_handler(ev, post_data);

			etimer_set(&et, CLOCK_SECOND);
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nbr_construction_process, ev, data)
{
	// static unsigned i = 0;
	static uip_ds6_nbr_t *nbr;

	PROCESS_BEGIN();


	// etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
	while (1) {
		// PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

		if (nbr_list[0].nnode_addr != Null) {
			continue;
		} else {

			nbr = uip_ds6_nbr_head();
			if (check_nbr_exist(nbr->ipaddr)) {
				if (nbr->ipaddr != NULL && nbr_list[i].nnode_addr != nbr->ipaddr) {
					nbr_list[i].nnode_addr = nbr->ipaddr;
					nnode_init(i);
				}
			}

			for (int i = 1; i < NEIGHBORS_LIST; i++) {
				nbr = uip_ds6_nbr_next(nbr)
				if (check_nbr_exist(nbr->ipaddr)) {
					if (nbr->ipaddr != NULL && nbr_list[i].nnode_addr != nbr->ipaddr) {
						nbr_list[i].nnode_addr = nbr->ipaddr;
						nnode_init(i);
					}
				}
			}
		}


		/* Add some jitter */
		// etimer_set(&periodic_timer, SEND_INTERVAL
		// - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
