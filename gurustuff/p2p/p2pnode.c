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

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (60 * CLOCK_SECOND)


static struct simple_udp_connection udp_conn;

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

	LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
	LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
	LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
	LOG_INFO_("\n");

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_comm_process, ev, data)
{
	static struct etimer periodic_timer;
	static unsigned count;
	static char str[32];
	static msg_pckt_t data_pckt;
	uip_ipaddr_t dest_ipaddr;

	PROCESS_BEGIN();

	/* Initialize UDP connection */
	simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
	                    UDP_SERVER_PORT, udp_rx_callback);

	etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
	while (1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

		if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
			/* Send to DAG root */
			LOG_INFO("Sending request %u to ", count);
			LOG_INFO_6ADDR(&dest_ipaddr);
			LOG_INFO_("\n");
			snprintf(str, sizeof(str), "hello %d", count);
			simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
			count++;
		} else {
			LOG_INFO("Not reachable yet\n");
		}

		/* Add some jitter */
		// etimer_set(&periodic_timer, SEND_INTERVAL
		//            - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
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

		if (nbr_list[0].node_addr != Null) {
			continue;
		} else {

			nbr = uip_ds6_nbr_head();
			if (nbr->ipaddr != NULL) {
				nbr_list[i].node_addr = nbr->ipaddr;
				nnode_init(i);
			}

			for (int i = 1; i < NEIGHBORS_LIST; i++) {
				nbr = uip_ds6_nbr_next(nbr)
				if (nbr->ipaddr != NULL) {
					nbr_list[i].node_addr = nbr->ipaddr;
					nnode_init(i);
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
