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
#include "sys/node-id.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "p2p.h"
#include "rxqueue.h"

#include "sys/log.h"
#define LOG_MODULE "p2p"
#define LOG_LEVEL LOG_LEVEL_INFO

// #define WITH_SERVER_REPLY  1
// #define UDP_CLIENT_PORT	8765
// #define UDP_SERVER_PORT	5678
#define P2P_PORT 5432

#define SEND_INTERVAL		  (60 * CLOCK_SECOND)


// static struct process_post_comm post_comm_process;

// TODO: check the socket
struct simple_udp_connection p2p_socket;
// process_event_t node_comm_upload_event;
// enum {
// 	HANDSHAKE_EVENT,
// 	INTEREST_EVENT,
// 	REQUEST_EVENT
// };


/*---------------------------------------------------------------------------*/
PROCESS_NAME(p2p_content_distribution);
PROCESS(node_comm_process, "UDP client");
PROCESS(nbr_construction_process, "Neighbor construction");
AUTOSTART_PROCESSES(&node_comm_process, &nbr_construction_process);
/*---------------------------------------------------------------------------*/

// As you say I think you should have this in p2p.c. p2p.[ch] provides the api for a main process
// that would like to use the p2p implementation. Internal data structures should not be exposed to the
// main program only the api. Understand that the process post needs to know where to post data.
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

	node_idx = check_index(sender_addr);

	if ((0 > node_idx) && (node_idx >= NEIGHBORS_LIST)) {
		LOG_ERR("Node doesn't exists in the node list");
		// PRINTF("Node doesn't exists in the node list");
	} else {

		msg_pckt_t *this;

		this = (msg_pckt_t *)data;

		if (this->ctrl_msg == LAST_CTRL_MSG) {

			uint8_t chunk_num;
			uint8_t block_num;

			chunk_num = (this->chunk_type.req_chunk_block & 0x00ff);
			block_num = (this->chunk_type.req_chunk_block & 0x0f00) >> 8;



			// print data since it is uint8_t
			LOG_INFO("Received response '%.*s' from ", 32, (char *) this->data);

			if ((chunk_num == nbr_list[node_idx].chunk_requested) &&
			        !(nbr_list[node_idx].chunk_block > block_num) &&
			        (nbr_list[node_idx].chunk_block < 0x0f)) {

				nbr_list[node_idx].chunk_block |= block_num;

				if (nbr_list[node_idx].chunk_block == 0x0f) {
					chunk_cnt[nbr_list[node_idx].chunk_requested] = true;
					node_download_nbr--;
					nbr_list[node_idx].nnode_state = HANDSHAKED_STATE;
					nbr_list[node_idx].nnode_interest = INTEREST_FALSE;
				}
			}

		} else {
			queue_enq(sender_addr, datalen, data);
		}



		// msg_pckt_t *this;

		// // These are not safe constructions. I started to change them but you need to refactory the code here.
		// // sender_addr and data are only available in the callback. In addition
		// // if you post the data you need to have memory where you store data post_data is lost
		// // once you leave the callback.
		// process_post_data_t post_data;
		// post_data.sender_addr = sender_addr;
		// post_data.data = (uint8_t *)data; //


		// this = (msg_pckt_t *)data;

		// // TODO: populate nbr node based on the ctrl_msg
		// // nbr_list[node_idx].node_addr = sender_addr;
		// if (this->ctrl_msg == ACKHANDSHAKE_CTRL_MSG) {
		// 	nbr_list[node_idx].nnode_ctrlmsg = this->ctrl_msg;
		// 	nbr_list[node_idx].data_chunks = this->self_chunks;
		// } else if (this->ctrl_msg == HANDSHAKE_CTRL_MSG) {
		// 	process_post(&node_comm_process, HANDSHAKE_EVENT, &post_data);
		// } else if (this->ctrl_msg == INTEREST_CTRL_MSG) {
		// 	process_post(&node_comm_process, INTEREST_EVENT, &post_data);
		// } else if (this->ctrl_msg == REQUEST_CTRL_MSG) {
		// 	process_post(&node_comm_process, REQUEST_EVENT, &post_data);
		// } else if (this->ctrl_msg == LAST_CTRL_MSG) {

		// 	LOG_INFO("Received response '%.*s' from ", datalen, (char *) this->data);
		// 	recv_block_count++;

		// 	if (recv_block_count >= 4)
		// 		recv_block_count = 0;
		// }
		// // else if (this->ctrl_msg == UNCHOKE_CTRL_MSG) {
		// // 	nbr_list[node_idx].nnode_ctrlmsg = this->ctrl_msg;
		// // } else if (this->ctrl_msg == CHOKE_CTRL_MSG) {
		// // 	// TODO: set timer for 5 seconds
		// // 	nbr_list[node_idx].nnode_ctrlmsg = this->ctrl_msg;
		// // }
		// else {
		// 	nbr_list[node_idx].nnode_ctrlmsg = this->ctrl_msg;
		// }

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
void
upload_event_handler(process_event_t ev, const process_post_data_t *post_data)
{

	static int8_t node_idx = -1;
	node_idx = check_index(&post_data->sender_addr);
	if (-1 == node_idx) {
		// LOG_ERR("NODE DOES NOT EXIST IN NODE LIST\n");
		PRINTF("NODE DOES NOT EXIST IN NODE LIST\n");
		return;
	} else {
		switch (ev) {
		case HANDSHAKE_EVENT:
			node_ack_handshake(&post_data->sender_addr);
			break;
		case INTEREST_EVENT:
			nbr_list[node_idx].chunk_interested = post_data->data[0];
			nbr_list[node_idx].nnode_choke = node_choke_unchoke(&post_data->sender_addr);
			break;
		case REQUEST_EVENT:
			nbr_list[node_idx].nnode_state = UPLOADING_STATE;
			node_upload(nbr_list[node_idx].chunk_interested, &post_data->sender_addr);
			nbr_list[node_idx].num_upload += 4;
			break;
		}
	}
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_comm_process, ev, data)
{
	static struct etimer periodic_timer;
	// static unsigned count;
	// static char str[32];
	// static msg_pckt_t data_pckt;
	// static uip_ipaddr_t dest_ipaddr;
	chunk_cnt[DATA_TOTAL_CHUNKS] = 0;


	static int is_coordinator;

  	PROCESS_BEGIN();

  	is_coordinator = 0;

#if CONTIKI_TARGET_COOJA || CONTIKI_TARGET_Z1
  is_coordinator = (node_id == 1);
#endif

	if(is_coordinator) {
	NETSTACK_ROUTING.root_start();
	}

	// In threads and processes variables need to be static
	static system_mode_t system_mode = MODE_IDLE;

	LOG_ERR("MAIN PROCESS\n");

	/* Initialize UDP connection */
	simple_udp_register(&p2p_socket, P2P_PORT, NULL,
	                    P2P_PORT, udp_rx_callback);

	// etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
	etimer_set(&periodic_timer, 60 * SEND_INTERVAL);

	while (1) {

		LOG_INFO("Enter: In while\n");

		// You never sleep. This will not work.
		#if (UIP_MAX_ROUTES != 0)
        PRINTF("Routing entries: %u\n", uip_ds6_route_num_routes());
      	#endif
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer) ||
		                         !is_queue_empty());


		// PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

		LOG_INFO("Enter: In while after clock\n");

		if (!is_queue_empty()) {
			queue_deq();
		}

		if ((etimer_expired(&periodic_timer) && (uip_ds6_route_num_routes() > NUM_OF_NODES))) {

			LOG_INFO("In main while:");

			// Here you are passing system mode as a value not pointer. If it is an integer or similar
			// It is fine. If it is a complex type you should use pointers.
			system_mode = system_mode_pp(system_mode);

			// upload_event_handler(ev, post_data);

			etimer_set(&periodic_timer, 60 * CLOCK_SECOND);
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nbr_construction_process, ev, data)
{
	static struct etimer periodic_timer;
	static uint8_t i = 0;
	// i is used below,
	static uip_ds6_nbr_t *nbr;

	PROCESS_BEGIN();

	etimer_set(&periodic_timer, 80 * SEND_INTERVAL);
	// etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);

	LOG_INFO("Enter: nbr construction process\n");

	while (1) {


		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
		// This process does not sleep. It should sleep until an en event occur or timer expire.
		// Here you are adding new nb but how do you make sure that nbr_list and uip_ds6_nbr are consistent.
		// For example if a nb goes away should the state be maintained and should timers be cleared, i.e.
		// is there not a need for add, delete reset functions. Now it is only add.


		LOG_INFO("Enter: nbr construction process in while after clock\n");


		if (&nbr_list[i].nnode_addr != NULL) {
			// I don't understand the construction. If nnode_addr != NULL, nothing is done?
			// Should [0] be [i]? nbr_list[0].nnode_addr is not a pointer so you can't compare with NULL
			LOG_INFO("nbr continue\n");
			continue;
		} else {

			nbr = uip_ds6_nbr_head();
			if (nbr != NULL && check_nbr_exist(&nbr->ipaddr)) {
				if (uip_ipaddr_cmp(&nbr_list[i].nnode_addr, &nbr->ipaddr)) {
					uip_ipaddr_copy(&nbr_list[i].nnode_addr, &nbr->ipaddr);
					nnode_init(i);
				}
			}

			for (int i = 1; i < NEIGHBORS_LIST; i++) {
				nbr = uip_ds6_nbr_next(nbr);
				if (nbr != NULL && check_nbr_exist(&nbr->ipaddr)) {
					if (uip_ipaddr_cmp(&nbr_list[i].nnode_addr, &nbr->ipaddr)) {
						uip_ipaddr_copy(&nbr_list[i].nnode_addr, &nbr->ipaddr);
						nnode_init(i);
					}
				}
			}
		}

		/* Add some jitter */
		// etimer_set(&periodic_timer, (random_rand() % (1 * CLOCK_SECOND)));

		nbr_list_print();

		etimer_set(&periodic_timer, (80 * CLOCK_SECOND));

	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
