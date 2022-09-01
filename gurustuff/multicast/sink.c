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

static struct uip_udp_conn *sink_conn;
static uint16_t count;

static bool recv_cnt[1024];  // static bool recv_count[1024] = {};
static uint8_t prev_packet;


#if !NETSTACK_CONF_WITH_IPV6 || !UIP_CONF_ROUTER || !UIP_IPV6_MULTICAST || !UIP_CONF_IPV6_RPL
#error "This example can not work with the current contiki configuration"
#error "Check the values of: NETSTACK_CONF_WITH_IPV6, UIP_CONF_ROUTER, UIP_CONF_IPV6_RPL"
#endif
/*---------------------------------------------------------------------------*/
PROCESS(mcast_sink_process, "Multicast Sink");
AUTOSTART_PROCESSES(&mcast_sink_process);
/*---------------------------------------------------------------------------*/

/*  
  funtion: unicast reception

*/








/*---------------------------------------------------------------------------*/
/*  
  funtion: to send unicast request for missing data
  
*/

// static void
// uni_pckt_req(uin8_t pck_num)
// {

// }


/*---------------------------------------------------------------------------*/



/*

function: check if all the packets are received

*/


static void
recv_data_check(uint8_t chnks)
{

  // if(recv_cnt[chnks] == true){
    PRINTF("Missing Packet:");

    for(int i = 0; i < chnks; i++){
      // PRINTF("Missing Packet:");
      if(recv_cnt[i] != true){
        // uni_pckt_req(i);
        PRINTF(" %u", i);
      }

    }
    PRINTF("\n");
    #if (UIP_MAX_ROUTES != 0)
      PRINTF("Routing entries: %u\n", uip_ds6_route_num_routes());
    #endif

  // }

  return;

}






/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{

  if(uip_newdata()) {

    packet_data *packet_data_recv = (packet_data *)uip_appdata;

    // PRINTF("test seq num: %u\n", packet_data_recv->seq_num);

    prev_packet = packet_data_recv->seq_num;

    recv_cnt[packet_data_recv->seq_num] = true;

    packet_data_recv->buf[MAX_PAYLOAD_LEN] = '\0';

    if(recv_cnt[packet_data_recv->seq_num]){
      PRINTF("seq num %u is set to %u\n", packet_data_recv->seq_num, 
        recv_cnt[packet_data_recv->seq_num]);
    }



    count++;
    // PRINTF("In: [0x%08lx], TTL %u, total %u\n",
    //     (unsigned long)uip_ntohl((unsigned long) *((uint32_t *)(uip_appdata))),
    //     UIP_IP_BUF->ttl, count);

    // PRINTF("In: %s, total %u\n", (char *)uip_appdata, count);

    PRINTF("%lu bytes\n", (unsigned long)sizeof(packet_data));

    PRINTF("seq num: %u\n", packet_data_recv->seq_num);
    PRINTF("total chunks: %u\n", packet_data_recv->tot_chnks);
    PRINTF("In: %s, total %u\n", packet_data_recv->buf, count);

    recv_data_check(packet_data_recv->tot_chnks);

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
  uip_ip6addr(&addr, 0xFF1E,0,0,0,0,0,0x89,0xABCD);
  rv = uip_ds6_maddr_add(&addr);

  if(rv) {
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
  PROCESS_BEGIN();


  for(int i = 0; i < 1024; i++){
    recv_cnt[i] = false;
  }

  PRINTF("Multicast Engine: '%s'\n", UIP_MCAST6.name);

  /*
   * MPL nodes are automatically configured to subscribe to the ALL_MPL_FORWARDERS
   *  well-known address, so this isn't needed.
   */
#if UIP_MCAST6_CONF_ENGINE != UIP_MCAST6_ENGINE_MPL
  if(join_mcast_group() == NULL) {
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

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
