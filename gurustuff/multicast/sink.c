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

static bool recv_cnt[1024];  // static bool recv_count[1024] = {};
static uint8_t miss_pckt[1024]; //static int for missing packet reqst
// static uint8_t *miss_pckt; //static int for missing packet reqst
static uint8_t prev_packet;
static uint8_t total_chunks;


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
  funtion: to send unicast request for missing packets

*/

static void
uni_pckt_req(uint8_t mcnt)
{
  uni_req_flag = DATA_RSTFLAG;
  uip_ipaddr_t dest_ipaddr;

  PRINTF("missing packet request to send from here\n");

  // if (uni_req_flag != DATA_RECEIVED) {
  //   uip_udp_packet_sendto(&sink_conn, pack_to_req, strlen(pack_to_req),
  //                         &sink_conn->ripaddr, UIP_HTONS(sink_conn->rport));

  // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  // uint8_t mpckt[mcnt];
  // uint8_t mpckt[1024];
  uint8_t *mpckt;
  // char *mpckt;

  mpckt = (uint8_t *)heapmem_alloc(mcnt * sizeof(uint8_t));

  // memset(&mpckt, 0, sizeof(uint8_t)*1024);
  memset(mpckt, 0, sizeof(uint8_t)*mcnt);

  PRINTF("missing packets from sink: ");

  // uint8_t i = 0;
  for(int i = 0; i < mcnt; i++){
    // PRINTF("%u", miss_pckt[i]);
    mpckt[i] = miss_pckt[i];
    // PRINTF("from miss_pckt: %u", miss_pckt[i]);
    // PRINTF("from mpckt: %u", mpckt[i]);
    PRINTF(" %u", mpckt[i]);
  }
  // mpckt[i] = '\0';
  
  PRINTF("\n");
  PRINTF("missing packets in sink packed\n");


  // mpckt = "hello";


  if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
    /* Send to DAG root */
    LOG_INFO("Sending request %u to ", count_u);
    LOG_INFO_6ADDR(&dest_ipaddr);
    LOG_INFO_("\n");
    // snprintf(str, sizeof(str), "hello %d", count);
    PRINTF("sending missing packets to root \n");    
    simple_udp_sendto(&udp_conn, mpckt, mcnt, &dest_ipaddr);
    count_u++;
  } else {
    LOG_INFO("Not reachable yet\n");
  }

// }
}


/*---------------------------------------------------------------------------*/

/*
  funtion: unicast reception

*/


//   //settimer for (no_of_packets * packet_delay)


static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{

  PRINTF("received response for unicast request: \n");
  // LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);

  PRINTF("received response for missing packets requested: ");

  packet_data *this;

  this = (packet_data *)data;


  PRINTF("seq_num received from unic: %u \n", this->seq_num);
  PRINTF("datalen received from unic: %u \n", datalen);
  PRINTF(" (msg= %s)", this->buf);


  // PRINTF("data ptr check s %u \n", *data);
  // PRINTF("data ptr check s %u \n", *(data+1));
  // PRINTF("data ptr check s %u \n", *(data+2));
  // PRINTF("data ptr check s %u \n", *(data+3));
  // PRINTF("data ptr check s %u \n", *(data+4));
  // PRINTF("data ptr check s %u \n", *(data+5));
  // PRINTF("data ptr check s %u \n", *(data+6));
  // PRINTF("data ptr check s %u \n", *(data+7));
  // PRINTF("data ptr check s %u \n", *(data+8));

  for(int i = 0; i < datalen; i++){
    // PRINTF("inside unicast sink for\n");
    if(recv_cnt[this->seq_num] != true){ //-|
      // PRINTF("unicast response received for %u packet \n", data[i]);
      recv_cnt[this->seq_num] = true;    //-|- uncomment when data sent from root
    }                              //-|
    PRINTF(" %u", this->seq_num);
  }

  PRINTF(" from ");

  LOG_INFO_6ADDR(sender_addr);
  // LOG_INFO_("\n");

#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");

  recv_data_check(total_chunks);

}


/*---------------------------------------------------------------------------*/

/*

function: check if all the packets are received

*/


static void
recv_data_check(uint8_t chnks)
{

  // static struct timer periodic_timer;
  uint8_t cnt = 0;

  // timer_set(&periodic_timer, 30 * 60 * SEND_INTERVAL);

  // miss_pckt = heapmem_alloc(chnks);
  memset(miss_pckt, 0, sizeof(uint8_t)*chnks);
  
  // memset(miss_pckt, 0, sizeof(uint8_t)*1024);
  // memset(&mpckt, 0, sizeof(uint8_t)*1024);




  // if(recv_cnt[chnks] == true){
  PRINTF("Missing Packet:");

  for (int i = 0; i < chnks; i++) {
    // PRINTF("Missing Packet:");
    if (recv_cnt[i] != true) {
      miss_pckt[cnt++] = i;
      uni_req_flag = DATA_MISSING;  //data missing flag is set
      // uni_pckt_req(i);
      PRINTF(" %u", i);
    }
  }
  miss_pckt[cnt] = '\0';
  PRINTF("\n");

  PRINTF("missing packets from miss_packt ");
  for(int i = 0; miss_pckt[i] != '\0'; i++){
    PRINTF(" %u", miss_pckt[i]);
  }
  PRINTF("\n");


  PRINTF(" --> cnt: %u\n", cnt);

  // #if (UIP_MAX_ROUTES != 0)
  //   PRINTF("Routing entries: %u\n", uip_ds6_route_num_routes());
  // #endif

  // }

  // if(timer_expired(&periodic_timer)){
    // PRINTF("recv timer expired \n");
    if (uni_req_flag == DATA_MISSING && mult_recv_flag == TIME_EXPD && cnt > 0) { //if the data missing flag is set
      mult_recv_flag = TIME_RSET;
      PRINTF("data missing flag checked \n");
      uni_pckt_req(cnt);
      // timer_reset(&periodic_timer);
      // heapmem_free(miss_pckt);
    }
  // }

  return;

}






/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{

  // static struct stimer periodic_timer;

  // stimer_set(&periodic_timer, 30 * 60 * SEND_INTERVAL);
  // static uint8_t total_chunks;

  if (uip_newdata()) {

    packet_data *packet_data_recv = (packet_data *)uip_appdata;

    // PRINTF("test seq num: %u\n", packet_data_recv->seq_num);

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
      // return;
    }


    // packet_data_recv->buf[MAX_PAYLOAD_LEN] = '\0';

    // if (recv_cnt[packet_data_recv->seq_num]) {
    //   PRINTF("seq num %u is set to %u\n", packet_data_recv->seq_num,
    //          recv_cnt[packet_data_recv->seq_num]);
    // }



    count++;
    // PRINTF("In: [0x%08lx], TTL %u, total %u\n",
    //     (unsigned long)uip_ntohl((unsigned long) *((uint32_t *)(uip_appdata))),
    //     UIP_IP_BUF->ttl, count);

    // PRINTF("In: %s, total %u\n", (char *)uip_appdata, count);

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
  // static struct stimer periodic_timer;
  // static unsigned count;
  // static char str[32];
  // uip_ipaddr_t dest_ipaddr;
  /* unicast connection */

  PROCESS_BEGIN();

  // /* Initialize UDP connection for unicast */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);


  // for (int i = 0; i < 1024; i++) {
  //   recv_cnt[i] = false;
  // }

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

  etimer_set(&periodic_timer, 30 * 60 * SEND_INTERVAL);
  mult_recv_flag = TIME_RSET;

  while (1) {

    if(etimer_expired(&periodic_timer)){

      mult_recv_flag = TIME_EXPD;
      PRINTF("10 mins timer expired\n");
      recv_data_check(total_chunks);
      etimer_set(&periodic_timer, 10 * 60 * SEND_INTERVAL);

    }

    PROCESS_YIELD();
    if (ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
