/*
 * Copyright (c) 2010, Loughborough University - Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         This node is part of the RPL multicast example. It is a node that
 *         joins a multicast group and listens for messages. It also knows how
 *         to forward messages down the tree.
 *         For the example to work, we need one or more of those nodes.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"
#include "net/ipv6/uip-ds6-route.h"

#include "sys/node-id.h"

#include "dependencies.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"

#define MCAST_SINK_UDP_PORT 3001 /* Host byte order */

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;

static struct uip_udp_conn *sink_conn;
static uint16_t count;
static uint16_t count_u;

static bool recv_cnt[1024];     // static bool recv_count[1024] = {};
static uint8_t miss_pckt[1024]; // static int for missing packet reqst
static uint8_t prev_packet;
static uint8_t total_chunks;


typedef enum {
  STATE_IDLE = 0,
  STATE_DOWNLOADING,
  STATE_RECOVERY,
  STATE_COMPLETED
} system_state_t;

typedef enum {
  DATA_RSTFLAG = 0,
  DATA_RECEIVED,
  DATA_MISSING
} missing_data_t;

typedef enum{
  TIME_RSET = 0,
  TIME_SET,
  TIME_EXPD
} mult_recvtim_t;

system_state_t system_state_flag;
missing_data_t uni_req_flag;
mult_recvtim_t mult_recv_flag;

#if !NETSTACK_CONF_WITH_IPV6 || !UIP_CONF_ROUTER || !UIP_IPV6_MULTICAST || !UIP_CONF_IPV6_RPL
#error "This example can not work with the current contiki configuration"
#error "Check the values of: NETSTACK_CONF_WITH_IPV6, UIP_CONF_ROUTER, UIP_CONF_IPV6_RPL"
#endif

static void recv_data_check(uint8_t chnks);

/*---------------------------------------------------------------------------*/
PROCESS(mcast_sink_process, "Multicast Sink");
AUTOSTART_PROCESSES(&mcast_sink_process);
/*---------------------------------------------------------------------------*/


/*

function name   : uni_pckt_req
Parameters      : missing chunk count
return          : void
description     : send request for missing packets

*/

static void
uni_pckt_req(uint8_t mcnt)
{
  uni_req_flag = DATA_RSTFLAG;
  uip_ipaddr_t dest_ipaddr;

  uint8_t *mpckt;

  mpckt = (uint8_t *)heapmem_alloc(mcnt * sizeof(uint8_t));

  memset(mpckt, 0, sizeof(uint8_t)*mcnt);

  PRINTF("Missing Packets: ");

  /*  */
  for(int i = 0; i < mcnt; i++){
    mpckt[i] = miss_pckt[i];
    PRINTF(" %u", mpckt[i]);
  }
  
  PRINTF("\n");

  if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
    /* Send to DAG root */
    LOG_INFO("Sending request %u to ", count_u);
    LOG_INFO_6ADDR(&dest_ipaddr);
    LOG_INFO_("\n");
    PRINTF("Sending Missing Packets to Root\n");    
    simple_udp_sendto(&udp_conn, mpckt, mcnt, &dest_ipaddr);
    count_u++;
  } else {
    LOG_INFO("Root not reachable yet\n");
  }
}


/*---------------------------------------------------------------------------*/


/*

function name   : udp_rx_callback
Parameters      : structure of udp connection, sender address, sender port,
                  receiver address, receiver port, data, data length
return          : void
description     : receive callback

*/

static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{

  PRINTF("Received Response for Missing Packets Requested:\n");

  packet_data *this;

  this = (packet_data *)data;


  PRINTF("seq_num Received from Root: %u\n", this->seq_num);
  PRINTF("datalen Received from Root: %u\n", datalen);
  PRINTF(" (msg= %s\n)", this->buf);

  PRINTF("seq_num: ");

  for(int i = 0; i < 1; i++){     // i < datalen corrected to 1
    if(recv_cnt[this->seq_num] != true){ //-|
      recv_cnt[this->seq_num] = true;    //-|- uncomment when data sent from root
    }                              //-|
    PRINTF(" %u", this->seq_num);
  }

  PRINTF(" from ");

  LOG_INFO_6ADDR(sender_addr);

#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");

  recv_data_check(total_chunks);

}


/*---------------------------------------------------------------------------*/

/*

function name   : recv_data_check
Parameters      : total chunks
return          : void
description     : check if all the chunks are received

*/

static void
recv_data_check(uint8_t chnks)
{

  uint8_t cnt = 0; // missing chunk counter

  memset(miss_pckt, 0, sizeof(uint8_t)*chnks);
  
  PRINTF("Missing Packet:");

  #ifdef UNIREQ_ALLDATA
  chnks = DATA_CHNKS;
  #endif

  /* check for missing chunks */
  for (int i = 0; i < chnks; i++) { 
    if (recv_cnt[i] != true) {
      miss_pckt[cnt++] = i; 
      uni_req_flag = DATA_MISSING;  // data missing flag is set
      PRINTF(" %u", i);
    }
  }
  miss_pckt[cnt] = '\0';
  PRINTF("\n");

  PRINTF("Missing Packets from miss_packt:");
  for(int i = 0; miss_pckt[i] != '\0'; i++){
    PRINTF(" %u", miss_pckt[i]);
  }
  PRINTF("\n");

  (cnt > 0) ? (PRINTF(" --> cnt: %u\n", cnt)) : (PRINTF("All Packets Received\n"), system_state_flag = STATE_COMPLETED);

    if (uni_req_flag == DATA_MISSING && mult_recv_flag == TIME_EXPD && cnt > 0) { //if the data missing flag is set
      mult_recv_flag = TIME_RSET;
      PRINTF("Data Missing Flag Checked \n");
      uni_pckt_req(cnt);
    }

  return;

}


/*---------------------------------------------------------------------------*/
/*

function name   : tcpip_handler
Parameters      : void
return          : void
description     : handle incomming data packets

*/


static void
tcpip_handler(void)
{

PRINTF("inside tcpip handler\n");

  if (uip_newdata()) {

    PRINTF("inside tcpip handler new data\n");

    packet_data *packet_data_recv = (packet_data *)uip_appdata;

    prev_packet = packet_data_recv->seq_num;

    packet_data_recv->buf[MAX_PAYLOAD_LEN] = '\0';

    if(recv_cnt[packet_data_recv->seq_num] != true) {
      recv_cnt[packet_data_recv->seq_num] = true;
      if (recv_cnt[packet_data_recv->seq_num]) {
        PRINTF("seq num %u is set to %u\n", packet_data_recv->seq_num,
             recv_cnt[packet_data_recv->seq_num]);
      }
    } else {
      PRINTF("duplicate %u packet received \n", packet_data_recv->seq_num);
    }

    count++;

    PRINTF("%lu bytes\n", (unsigned long)sizeof(packet_data));

    PRINTF("seq num: %u\n", packet_data_recv->seq_num);
    total_chunks = packet_data_recv->tot_chnks;
    PRINTF("total chunks: %u\n", packet_data_recv->tot_chnks);
    PRINTF("In: %s, total %u\n", packet_data_recv->buf, count);
  }

  return;
}



/*---------------------------------------------------------------------------*/
#if UIP_MCAST6_CONF_ENGINE != UIP_MCAST6_ENGINE_MPL
static uip_ds6_maddr_t *
join_mcast_group(void)
{
  uip_ipaddr_t addr;
  uip_ds6_maddr_t *rv;
  const uip_ipaddr_t *default_prefix = uip_ds6_default_prefix();

  /* First, set our v6 global */
  uip_ip6addr_copy(&addr, default_prefix);
  uip_ds6_set_addr_iid(&addr, &uip_lladdr);
  uip_ds6_addr_add(&addr, 0, ADDR_AUTOCONF);

  /*
   * IPHC will use stateless multicast compression for this destination
   * (M=1, DAC=0), with 32 inline bits (1E 89 AB CD)
   */
  uip_ip6addr(&addr, 0xFF1E, 0, 0, 0, 0, 0, 0x89, 0xABCD);
  rv = uip_ds6_maddr_add(&addr);

  if (rv) {
    PRINTF("Joined multicast group ");
    PRINT6ADDR(&uip_ds6_maddr_lookup(&addr)->ipaddr);
    PRINTF("\n");
  }
  return rv;
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mcast_sink_process, ev, data)
{

  /* unicast connection */
  static struct etimer periodic_timer;

  system_state_flag = STATE_IDLE;

  PROCESS_BEGIN();

  // /* Initialize UDP connection for unicast */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  PRINTF("Multicast Engine: '%s'\n", UIP_MCAST6.name);

  /*
   * MPL nodes are automatically configured to subscribe to the ALL_MPL_FORWARDERS
   *  well-known address, so this isn't needed.
   */
#if UIP_MCAST6_CONF_ENGINE != UIP_MCAST6_ENGINE_MPL
  if (join_mcast_group() == NULL) {
    PRINTF("Failed to join multicast group\n");
    PROCESS_EXIT();
  }
#endif

  count = 0;

  sink_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(sink_conn, UIP_HTONS(MCAST_SINK_UDP_PORT));

  PRINTF("Listening: ");
  PRINT6ADDR(&sink_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
         UIP_HTONS(sink_conn->lport), UIP_HTONS(sink_conn->rport));


  // RANDOM_RAND_MAX is 65535U, so range is 0 - 65535

  etimer_set(&periodic_timer, UNI_REQ_START_SEND_INTERVAL + 
  ((random_rand() % node_id) + 
  (random_rand() % NUM_OF_NODES)) * CLOCK_SECOND); //30 to 15 ((random_rand() % node_id) + 1) * CLOCK_SECOND)
  mult_recv_flag = TIME_RSET;

  while (1) {

    if(etimer_expired(&periodic_timer)){

      mult_recv_flag = TIME_EXPD;
      PRINTF("SINK: Node ID: %d\n", node_id);
      PRINTF("SINK: 10 mins unicast request timer expired\n");

      if(system_state_flag != STATE_COMPLETED){
        recv_data_check(total_chunks);
        etimer_set(&periodic_timer, ((random_rand() % node_id) + (random_rand() % NUM_OF_NODES)) * CLOCK_SECOND); //UNI_REQ_SEND_INTERVAL or (random_rand() % 256)
      } else {
        etimer_stop(&periodic_timer);
      }
    }

    PROCESS_YIELD();
    if (ev == tcpip_event) {
      PRINTF("calling tcpip handler\n");
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/








