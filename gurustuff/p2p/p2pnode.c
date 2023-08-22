/**
 * \addtogroup p2p
 * @{
 */
/**
 * \file
 *    		This file implements 'Peer to Peer' (p2p)
 *
 * \author
 *    		Guru Mehar Rachaputi
 * 
 * \reviewer
 * 	  		Anders Isberg
 * 
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

#define P2P_PORT 5432

#define SEND_INTERVAL		  	(60 * CLOCK_SECOND)
#define QUE_START_INTERVAL	SEND_INTERVAL
#define DELAY_CYCLE_COUNT		20 //(60 / PROCESS_WAIT_TIME_DEFAULT)

// #define TESTING // For log prints

bool chunk_cnt[DATA_TOTAL_CHUNKS];
bool missing_chunk_req[DATA_TOTAL_CHUNKS];

struct simple_udp_connection p2p_socket;

/*---------------------------------------------------------------------------*/
PROCESS_NAME(p2p_content_distribution);
PROCESS(node_comm_process, "UDP client");
PROCESS(queue_proc, "queue process");
AUTOSTART_PROCESSES(&node_comm_process, &queue_proc);

/*---------------------------------------------------------------------------*/

/******function declarations*******/

void nbr_construction(const uip_ipaddr_t *ipaddr);



/*---------------------------------------------------------------------------*/
static bool
node_data_check(void){
	LOG_INFO("node data check\n");
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
static uint8_t
nbr_count(void){

	static uint8_t nbr_count;
	nbr_count = 0;
	for(int i = 0; i < NEIGHBORS_LIST && 
		!uip_is_addr_unspecified(&nbr_list[i].nnode_addr);
		i++){
		nbr_count += 1;
	}
	return nbr_count;
}

/*---------------------------------------------------------------------------*/
static bool
is_nbr_handhaked(bool ret_type) {
	static uint8_t nbr_hs_count;

	LOG_INFO("is nbr handshake check\n");

	nbr_hs_count = 0;

	for(int i = 0; i < NEIGHBORS_LIST &&
		!uip_is_addr_unspecified(&nbr_list[i].nnode_addr);
		i++){

		if(nbr_list[i].nnode_state >= HANDSHAKED_STATE &&
			nbr_list[i].nnode_rank <= my_rank &&
			my_rank < RANK_1){
			nnode_reset_lowrank(i);
		}

		if(nbr_list[i].nnode_state == HANDSHAKING_STATE ||
			nbr_list[i].nnode_state == IDLE_STATE) {
			LOG_INFO("nbr handshaking progress\n");
			nbr_hs_count++;
		}
	}
	if(nbr_hs_count > nbr_count()/2){
		return (ret_type)? false : nbr_hs_count;
	}

	return (ret_type)? true : nbr_hs_count;
}

/*---------------------------------------------------------------------------*/

static bool
is_nbr_rank_low(void){

	static uint8_t nbr_rank_low_num;
	
	nbr_rank_low_num = 0;

	for(int i = 0; i < NEIGHBORS_LIST && 
		!uip_is_addr_unspecified(&nbr_list[i].nnode_addr) &&
		nbr_list[i].nnode_rank <= my_rank;
		i++){
		nbr_rank_low_num += 1;
	}

	if(nbr_rank_low_num == nbr_count())
		return true;

	return false;
}

/*---------------------------------------------------------------------------*/

static bool
is_nbr_rank_high(void) {
	static bool rank_high_flag;
	static uint8_t rank_high_count;

	LOG_INFO("is nbr rank high check\n");

	rank_high_flag = false;
	rank_high_count = 0;
	// rank_high_freq = 0;
	for(int i = 0; i < NEIGHBORS_LIST && 
		!uip_is_addr_unspecified(&nbr_list[i].nnode_addr);
		i++){

		if(nbr_list[i].nnode_state >= HANDSHAKED_STATE){
			
			if(nbr_list[i].nnode_rank > my_rank && 
				nbr_list[i].failed_dlreq <= 2)
				rank_high_count += 1;
		}

		LOG_INFO("node id: %d\n", i);
		LOG_INFO("node state: %d\n", nbr_list[i].nnode_state);
		LOG_INFO("node rank: %d\n", nbr_list[i].nnode_rank);
	}

	if(rank_high_count >= 1)
		rank_high_flag = true;
	else
		rank_high_flag = false;


	nbr_list_print();
	return rank_high_flag;
}

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

	node_idx = check_index(sender_addr);

	LOG_INFO("rx call back node idx: %d", node_idx);

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

					chunk_num = 0;
					block_num = 0;

					#ifdef TESTING
						nbr_list_print();	// tesing
						node_data_check(); // testing
					#endif

				}
			}
		} else {
			queue_enq(sender_addr, datalen, data);
		}
	}

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
		chunk_cnt[i] = chunk_init;
	}
}

/*---------------------------------------------------------------------------*/
// init function
static void
node_chunk_req(void){
	for(int i = 0; i < DATA_TOTAL_CHUNKS; i++){
		missing_chunk_req[i] = false;
	}
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_comm_process, ev, data)
{
	static struct etimer periodic_timer;
	static uint8_t is_coordinator;
	static system_mode_t system_mode = MODE_IDLE;
	static uint8_t delay_cycle = 0;
	static uint32_t process_timer = PROCESS_WAIT_TIME_DEFAULT;

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
	NETSTACK_MAC.on();

	node_chunk_req();	// init missing data


	LOG_INFO("MAIN PROCESS\n");

	/* Initialize UDP connection */
	simple_udp_register(&p2p_socket, P2P_PORT, NULL,
	                    P2P_PORT, udp_rx_callback);

	etimer_set(&periodic_timer, 10 * SEND_INTERVAL); // 120 to 60 minutes

	while (1) {

		LOG_INFO("Enter: In while\n");

		#if (UIP_MAX_ROUTES != 0)
        PRINTF("Routing entries: %u\n", uip_ds6_route_num_routes());
    #endif
		PROCESS_WAIT_EVENT_UNTIL(/*uip_ds6_route_num_routes() > NUM_OF_NODES && */
														/*(ev == queue_event ||*/
														!is_queue_empty() || 
														etimer_expired(&periodic_timer));

		LOG_INFO("Routing entries: %u\n", uip_ds6_route_num_routes());
		LOG_INFO("Enter: In while after clock\n");

		if (etimer_expired(&periodic_timer)) {

			LOG_INFO("In main while:\n");

			node_my_rank();	// set my_rank variable

				if(system_mode != MODE_SEEDER && (is_coordinator || node_data_check() == true)) // test for print only once
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
			}

			if(!is_coordinator && my_rank < RANK_1 && is_nbr_handhaked(true) == true && 
				(is_nbr_rank_low() == true || is_nbr_rank_high() == false) &&
				delay_cycle <= DELAY_CYCLE_COUNT) {
				delay_cycle += 1;
				LOG_INFO("is nbr rank high : %d\n", is_nbr_rank_high());
				LOG_INFO("delay cycle : %d\n", delay_cycle);
				if(delay_cycle >= DELAY_CYCLE_COUNT){
					delay_cycle = 0;
					process_timer = PROCESS_WAIT_TIME_DEFAULT;
					nnode_reset_all();
					nbr_list_print();
				}
			} else {
				delay_cycle = 0;
				process_timer = PROCESS_WAIT_TIME_DEFAULT;
			}

			etimer_set(&periodic_timer, process_timer * 60 * CLOCK_SECOND); // 30 to 15 minutes
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(queue_proc, ev, data)
{
  static struct etimer et_queue;

  PROCESS_BEGIN();

  etimer_set(&et_queue, 10 * QUE_START_INTERVAL);
  while(1){

    PRINTF("In Queue Process Yield\n");
    PROCESS_YIELD();
    if(etimer_expired(&et_queue) && !is_queue_empty())
    {
      PRINTF("Queue Process is Not Empty\n");
      queue_deq();
    }

    etimer_set(&et_queue, 20 * CLOCK_SECOND);
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












































