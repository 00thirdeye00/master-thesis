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
#include "dev/leds.h"
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


// TODO: check the socket
struct simple_udp_connection p2p_socket;
// main process timer
uint8_t process_timer = PROCESS_WAIT_TIME_DEFAULT;

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
bool
node_data_check(void){
	for(int i = 0; i < DATA_TOTAL_CHUNKS; i++) {
		// PRINTF("Data chunk %d:%d\n", i, chunk_cnt[i]);
		if(chunk_cnt[i] == 1){
			PRINTF("Data chunk %d:%d\n", i, chunk_cnt[i]);
			continue;
		} else
				return false;
	}
	LOG_INFO("node data checked\n");
	return true;
}


/*---------------------------------------------------------------------------*/
// void 
// nbr_list_init(void){
// 	for(int nbr_idx = 0; nbr_idx < NEIGHBORS_LIST; nbr_idx++){
// 		nbr_list[nbr_idx].nnode_addr = NULL;
// 		// uip_ipaddr_copy(&nbr_list[nbr_idx].nnode_addr, NULL);
// 	}
// }


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

			LOG_INFO("Received message from: ");
			LOG_INFO_6ADDR(sender_addr);
			LOG_INFO_("\n");
			PRINTF("chunk number: %d\n", chunk_num);
			PRINTF("block number: %d\n", block_num);
			PRINTF("block number check: %d\n", (this->chunk_type.req_chunk_block & 0x0f00));
			PRINTF("ctrl message: %d\n", this->ctrl_msg);
			LOG_INFO("data received: '%.*s' \n", 32, (char *)this->data);

			if(chunk_cnt[chunk_num] == true) {
				LOG_INFO("DUPLICATE: chunk duplicate received\n");
				if(node_download_nbr > 0)
					node_download_nbr--;
				// (node_download_nbr > 0) ? node_download_nbr-- : LOG_INFO("node dlnbr: %d\n", node_download_nbr);
				nbr_list[node_idx].nnode_state = HANDSHAKED_STATE;
				nbr_list[node_idx].nnode_interest = INTEREST_FALSE;
				return;
			}

			if ((chunk_num == nbr_list[node_idx].chunk_requested) &&
			        !(nbr_list[node_idx].chunk_block > block_num) &&
			        (nbr_list[node_idx].chunk_block <= 0x0f)) {

				nbr_list[node_idx].chunk_block |= block_num;

				if(nbr_list[node_idx].chunk_block == 0x0f) {

					LOG_INFO("chunk block: %d\n", nbr_list[node_idx].chunk_block);
					LOG_INFO("chunk requested: %d\n", chunk_cnt[nbr_list[node_idx].chunk_requested]);

					chunk_cnt[nbr_list[node_idx].chunk_requested] = true;
					node_download_nbr -= (node_download_nbr > 0) ? 1 : 0;
					// nbr_list[node_idx].nnode_ctrlmsg = ACKHANDSHAKE_CTRL_MSG;
					// nbr_list[node_idx].nnode_state = HANDSHAKED_STATE;
					// nbr_list[node_idx].nnode_interest = INTEREST_FALSE;

					chunk_num = 0;
					block_num = 0;

					nbr_list_print();	// tesing
					node_data_check(); // testing

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

	msg_pckt_t *this;
	this = (msg_pckt_t *) post_data->data;

	static int8_t node_idx = -1;
	node_idx = check_index(&post_data->sender_addr);
	if (-1 == node_idx) {
		LOG_ERR("NODE DOES NOT EXIST IN NODE LIST\n");
		return;
	} else {
		switch (ev) {
		case HANDSHAKE_EVENT:
			LOG_INFO("handshake event handler to ack handshake\n");
			node_ack_handshake(&post_data->sender_addr);
			break;
		case INTEREST_EVENT:
			LOG_INFO("interest event handler to choke/unchoke\n");
			LOG_INFO_6ADDR(&post_data->sender_addr);
			LOG_INFO("control message:  %d\n", this->ctrl_msg);
			LOG_INFO("chunk interested in: %d\n", this->data[0]);
			nbr_list[node_idx].chunk_interested = this->data[0];
			nbr_list[node_idx].nnode_choke = node_choke_unchoke(&post_data->sender_addr);
			break;
		case REQUEST_EVENT:
			LOG_INFO("request event handler to upload\n");
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

	// node_data_check();


	LOG_ERR("MAIN PROCESS\n");

	// init nbr_list addresses to null
	// nbr_list_init();

	/* Initialize UDP connection */
	simple_udp_register(&p2p_socket, P2P_PORT, NULL,
	                    P2P_PORT, udp_rx_callback);

	// etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
	etimer_set(&periodic_timer, 60 * SEND_INTERVAL); // 120 to 60 minutes

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

			// process_timer = PROCESS_WAIT_TIME_DEFAULT;

			if(is_coordinator || node_data_check() == true)
				system_mode = system_mode_pp(MODE_SEEDER);
			else
				system_mode = system_mode_pp(system_mode);


			if(system_mode == MODE_IDLE){
				leds_off(LEDS_ALL);
				leds_single_on(0x02);	// red led
			} else if(system_mode == MODE_LEECHER){
				leds_off(LEDS_ALL);
				leds_single_on(0x01);	// blue led
			} else if(system_mode == MODE_SEEDER){
				leds_off(LEDS_ALL);
				leds_single_on(0x00);	// green led
			} else{
				leds_on(LEDS_ALL);
				// leds_single_on(0x00);
			}


			etimer_set(&periodic_timer, process_timer * 60 * CLOCK_SECOND); // 30 to 15 minutes
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

}


/*---------------------------------------------------------------------------*/


/*--------------------------------notes--------------------------------------*/

/*
	kill crashed applications graphically in linux

 	in a terminal, run
 $xkill
	

*/



/* gradle run */

/*
	
	$./gradlew run --scan

	$./gradlew run --stacktrace

	$./gradlew run --info

*/


/* debugging */
/*

make -j$(CPUS) p2pnode.cooja TARGET=cooja

- to run cooja 
	$ ./gradlew run --stacktrace

- to execute java application with unlimited coredump memory
	$ ulimit -c unlimited

- for apport log 
	-> cat /var/log/apport.log

- for core dump
	-> cd /var/crash/

- for javatoolchains
	-> $ gradle javaToolchains --scan

- if program is not installed package
	-> create file named "~/.config/apport/settings" with content
		[main]
		unpackaged=true

- /var/crash file *.1000.crash cannot be read by gdb. to make it 
	readable by gdb
	-> apport-unpack <location_of_report> <target_directory>
	$ apport-unpack _home_sony_.gradle_jdks_eclipse_adoptium-17-amd64-linux_jdk-17.0.7+7_bin_java.1000.crash ~/thesis/osnew/contiki-ng/tools/cooja/coredump


- to debug
 gdb -c '/home/sony/thesis/osnew/contiki-ng/tools/cooja/coredump/CoreDump' '/home/sony/thesis/osnew/contiki-ng/examples/p2p/build/cooja/mtype488261898.cooja' 


*/












































