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
#define LOG_MODULE "p2pnode"
#define LOG_LEVEL LOG_LEVEL_INFO

// #define WITH_SERVER_REPLY  1
// #define UDP_CLIENT_PORT	8765
// #define UDP_SERVER_PORT	5678
#define P2P_PORT 5432

#define SEND_INTERVAL		  (60 * CLOCK_SECOND)

bool chunk_cnt[DATA_TOTAL_CHUNKS];


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
// PROCESS(nbr_construction_process, "Neighbor construction");
// AUTOSTART_PROCESSES(&node_comm_process, &nbr_construction_process);
AUTOSTART_PROCESSES(&node_comm_process);

/*---------------------------------------------------------------------------*/

/******function declarations*******/

void nbr_construction(const uip_ipaddr_t *ipaddr);






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
	} else {

		msg_pckt_t *this;

		this = (msg_pckt_t *)data;

		LOG_INFO("ctrl msg callback: %d\n", this->ctrl_msg);

		if (this->ctrl_msg == LAST_CTRL_MSG) {

			static uint8_t chunk_num;
			static uint8_t block_num;

			chunk_num = (this->chunk_type.req_chunk_block & 0x00ff);
			block_num = (this->chunk_type.req_chunk_block & 0x0f00) >> 8;

			// print data since it is uint8_t
			LOG_INFO("Received response '%.*s' from ", datalen, (char *) this->data);
			// LOG_INFO("Received response '%.*s' from ", 16, (char *) this->data);

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
	}

	// LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
	LOG_INFO("In node callback\n");
	LOG_INFO("Received message from: ");
	LOG_INFO_6ADDR(sender_addr);
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
		LOG_ERR("NODE DOES NOT EXIST IN NODE LIST\n");
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
static void
node_coordinator_data(uint8_t chunk_init){
	for(int i = 0; i < DATA_TOTAL_CHUNKS; i++) {
		// PRINTF("Data chunk %d:%d\n", i, chunk_cnt[i]);
		chunk_cnt[i] = chunk_init;
	}
}

/*---------------------------------------------------------------------------*/
static void
node_data_check(void){
	for(int i = 0; i < DATA_TOTAL_CHUNKS; i++) {
		// PRINTF("Data chunk %d:%d\n", i, chunk_cnt[i]);
		if(chunk_cnt[i] == 0)
			continue;
		else
			return;
	}
	LOG_INFO("node data checked\n");
}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_comm_process, ev, data)
{
	static struct etimer periodic_timer;
	static uint8_t is_coordinator;
	// In threads and processes variables need to be static
	static system_mode_t system_mode = MODE_IDLE;

  	PROCESS_BEGIN();

  	is_coordinator = 0;

#if CONTIKI_TARGET_COOJA || CONTIKI_TARGET_Z1
  is_coordinator = (node_id == 1);
#endif

	node_coordinator_data(0);

	if(is_coordinator) {
		NETSTACK_ROUTING.root_start();
		node_coordinator_data(1);
	}

	node_data_check();


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

		LOG_INFO("Enter: In while after clock\n");

		if (!is_queue_empty()) {
			queue_deq();
			nbr_list_print();
		}

		// if ((etimer_expired(&periodic_timer) && (uip_ds6_route_num_routes() > NUM_OF_NODES))) {
		if (etimer_expired(&periodic_timer)) {
			LOG_INFO("In main while:\n");

			// Here you are passing system mode as a value not pointer. If it is an integer or similar
			// It is fine. If it is a complex type you should use pointers.

			// if(!is_coordinator){
				system_mode = system_mode_pp(system_mode);
			// }

			etimer_set(&periodic_timer, 30 * 60 * CLOCK_SECOND);
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/


void nbr_construction(const uip_ipaddr_t *ipaddr) {

	static uint8_t nbr_index = 0;

	if (ipaddr != NULL) {
		if(check_nbr_exist(ipaddr)) {
			LOG_INFO("neighbor exists\n");
		} else if (nbr_index >= NEIGHBORS_LIST) {
			LOG_ERR("nbr list full\n");
		} else {
			uip_ipaddr_copy(&nbr_list[nbr_index].nnode_addr, ipaddr);
			LOG_INFO("new neighbor added to list\n");
			LOG_INFO_6ADDR(&nbr_list[nbr_index].nnode_addr);
			LOG_INFO("\n");
			nnode_init(nbr_index);
			nbr_index++;
		}
	} else {
		LOG_ERR("ipaddr empty\n");
	}

	nbr_list_print();



	/******** old neighbor construction *********/
	// if (ipaddr != NULL && !check_nbr_exist(ipaddr) && nbr_index < NEIGHBORS_LIST) {
	// 	LOG_INFO("check nbr exists %d\n", check_nbr_exist(ipaddr));
	// 	if (!uip_ipaddr_cmp(&nbr_list[nbr_index].nnode_addr, ipaddr)) {
	// 		LOG_INFO("nbr construction: \n");
	// 		// LOG_INFO_6ADDR(&nbr_list[nbr_index].nnode_addr);
	// 		// LOG_INFO("added \n");
	// 		uip_ipaddr_copy(&nbr_list[nbr_index].nnode_addr, ipaddr);
	// 		LOG_INFO("new neighbor added:\n");
	// 		LOG_INFO_6ADDR(&nbr_list[nbr_index].nnode_addr);
	// 		LOG_INFO("\n");
	// 		nnode_init(nbr_index);
	// 		nbr_index++;
	// 	}
	// } else {
	// 	if(nbr_index >= NEIGHBORS_LIST){
	// 		LOG_ERR("nbr list full\n");
	// 	} else if(check_nbr_exist(ipaddr)){
	// 		LOG_ERR("nbr exists\n");
	// 	}
	// }

	// nbr_list_print();

}


/*---------------------------------------------------------------------------*/





// PROCESS_THREAD(nbr_construction_process, ev, data)
// {
// 	static struct etimer periodic_timer;
// 	static uint8_t i;
// 	static uip_ds6_nbr_t *nbr;

// 	PROCESS_BEGIN();

// 	etimer_set(&periodic_timer, 10 * SEND_INTERVAL);

// 	LOG_INFO("Enter: nbr construction process\n");

// 	while (1) {

// 		i = 0;

// 		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
// 		// This process does not sleep. It should sleep until an en event occur or timer expire.
// 		// Here you are adding new nb but how do you make sure that nbr_list and uip_ds6_nbr are consistent.
// 		// For example if a nb goes away should the state be maintained and should timers be cleared, i.e.
// 		// is there not a need for add, delete reset functions. Now it is only add.


// 		LOG_INFO("Enter: nbr construction process in while after clock\n");

// 		if (!uip_is_addr_unspecified(&nbr_list[i].nnode_addr)) {
// 			LOG_INFO("nbr continue\n");
// 			LOG_INFO("Address specified node %d: ", i);
// 			LOG_INFO_6ADDR(&nbr_list[i].nnode_addr);
// 			PRINTF("\n");
// 			i++;
// 			// continue;
// 		} else {

// 			LOG_INFO("nbr else\n");

// 			nbr = uip_ds6_nbr_head();

// 			for (; i < NEIGHBORS_LIST; i++) {
// 				// nbr = uip_ds6_nbr_next(nbr);
// 				if (nbr != NULL && check_nbr_exist(&nbr->ipaddr)) {
// 					if (!uip_ipaddr_cmp(&nbr_list[i].nnode_addr, &nbr->ipaddr)) {
// 						uip_ipaddr_copy(&nbr_list[i].nnode_addr, &nbr->ipaddr);
// 						nnode_init(i);
// 					}
// 				}
// 				nbr = uip_ds6_nbr_next(nbr);
// 			}
// 		}

// 		LOG_INFO("check\n");

// 		nbr_list_print();

// 		etimer_set(&periodic_timer, (10 * 60 * CLOCK_SECOND));

// 	}


// 	PROCESS_END();
// }
/*---------------------------------------------------------------------------*/
