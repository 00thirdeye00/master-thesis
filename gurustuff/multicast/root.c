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
 *         This node is part of the RPL multicast example. It is an RPL root
 *         and sends a multicast message periodically. For the example to work,
 *         we need one of those nodes.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/multicast/uip-mcast6.h"

#include "dependencies.h"

#include <string.h>
#include <inttypes.h>

#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"
// #include "net/routing/routing.h"


// #define MAX_PAYLOAD_LEN 64
#define MCAST_SINK_UDP_PORT 3001 /* Host byte order */
// #define SEND_INTERVAL CLOCK_SECOND /* clock ticks */


#define DATA_LEN    (strlen(seq_idd))
#define DATA_CHNKS  (DATA_LEN / MAX_PAYLOAD_LEN)




#define ITERATIONS DATA_CHNKS /* messages */



/* Start sending messages START_DELAY secs after we start so that routing can
 * converge */
#define START_DELAY_MIN 10
#define START_DELAY (60 * START_DELAY_MIN)

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;

static struct uip_udp_conn * mcast_conn;


// static char buf[MAX_PAYLOAD_LEN];
const char *seq_idd = "063lPLXusS0KbZcuAgXFqXIuhVFxT7PbPdA9CifI7gBC4ia4H0uQiccRRLOaj100"
                      "1Yo8BjcgFF8aFtHLHZdRKNxliQEa8ozq38YP8dSXIwbAw2fMx46f8Xc3CmMou101"
                      "27lBkvtUauXX1V8oF4jT4CKIlbE03ghFBS2hTcnTiZ1Go5Ti1gVWRcUdHAh9g102"
                      "3hSAFpT2dLgVxoo2ZCHhNSE0rm2I5OBVhdklMnzBgDDW5gdpOm7389BaNgqdA103"
                      "47oUilXORLjy7kDu86GHSNLzAJjXUxxEc7PeJfPLyC0khgSy4aYUhlFsenbK4104"
                      "5DSZ5YS33eoOeiTze2onUl7PfPz8No6W7AHYW7S34vUADHDOcABcEkDlpxQho105"
                      "6pM1ksIrfHSbpfCJhoqqh9w9kjITgdDrqbp9aHTzjZOScTCtzpMFACJalZNRz106"
                      "7AY301mZjr2KD8pVrhkNfNxQyg1nmm7QJSXN4lDj0dIawEflfSVXLGShl0vxc107"
                      "8sxK5w6msIvzeRDgaYAJ5zmMWg42S4Ap7WA1eHYepIrlwmn1TdwSEPPcMx6Tj108"
                      "9Ilj3gU7JfgoItY5FqNQJgYy6Vw0NPdwwPcnfPmtLhH5nl55eJFwwpB6UgZan109"
                      "10dgiEiFy8GoGIaOjXC387ihvfvaiGA2sBgh3k87i7Eb7QTNrthpZSdlIk8PE110"
                      "11qwCORP6psr9r8BBTnJqYpuayMhQD8vuxp9aBXaMLWmBAYo2EAtB45YeXEGU111"
                      "12BUgN9AgmDERPzcyeOYG0ExpDE5cVY8OL5pNXM43MNAt7jkJAFnSRNKw6Qq5112"
                      "13bFRN69XbWj5t4HkcFMiVwIOLQARBNmcimSzlnhipF8bPY2P1QfsSqCfsnsu113"
                      "14uqVbPvrWTSGIrfVveI01YwiIWOeXjeMdh78Ruq9zy9xbkNkwOLcAiXzlrF0114"
                      "15YVCvYVKtzRSOCtCh5KPrTw0D2H5gvFFRgAH1ZgYFt6UWvv1ht65ptE7KOeN115"; /* 1024 bytes */
                      // "063lPLXusS0KbZcuAgXFqXIuhVFxT7PbPdA9CifI7gBC4ia4H0uQiccRRLOaj200"
                      // "1Yo8BjcgFF8aFtHLHZdRKNxliQEa8ozq38YP8dSXIwbAw2fMx46f8Xc3CmMou201"
                      // "27lBkvtUauXX1V8oF4jT4CKIlbE03ghFBS2hTcnTiZ1Go5Ti1gVWRcUdHAh9g202"
                      // "3hSAFpT2dLgVxoo2ZCHhNSE0rm2I5OBVhdklMnzBgDDW5gdpOm7389BaNgqdA203"
                      // "47oUilXORLjy7kDu86GHSNLzAJjXUxxEc7PeJfPLyC0khgSy4aYUhlFsenbK4204"
                      // "5DSZ5YS33eoOeiTze2onUl7PfPz8No6W7AHYW7S34vUADHDOcABcEkDlpxQho205"
                      // "6pM1ksIrfHSbpfCJhoqqh9w9kjITgdDrqbp9aHTzjZOScTCtzpMFACJalZNRz206"
                      // "7AY301mZjr2KD8pVrhkNfNxQyg1nmm7QJSXN4lDj0dIawEflfSVXLGShl0vxc207"
                      // "8sxK5w6msIvzeRDgaYAJ5zmMWg42S4Ap7WA1eHYepIrlwmn1TdwSEPPcMx6Tj208"
                      // "9Ilj3gU7JfgoItY5FqNQJgYy6Vw0NPdwwPcnfPmtLhH5nl55eJFwwpB6UgZan209"
                      // "10dgiEiFy8GoGIaOjXC387ihvfvaiGA2sBgh3k87i7Eb7QTNrthpZSdlIk8PE210"
                      // "11qwCORP6psr9r8BBTnJqYpuayMhQD8vuxp9aBXaMLWmBAYo2EAtB45YeXEGU211"
                      // "12BUgN9AgmDERPzcyeOYG0ExpDE5cVY8OL5pNXM43MNAt7jkJAFnSRNKw6Qq5212"
                      // "13bFRN69XbWj5t4HkcFMiVwIOLQARBNmcimSzlnhipF8bPY2P1QfsSqCfsnsu213"
                      // "14uqVbPvrWTSGIrfVveI01YwiIWOeXjeMdh78Ruq9zy9xbkNkwOLcAiXzlrF0214"
                      // "15YVCvYVKtzRSOCtCh5KPrTw0D2H5gvFFRgAH1ZgYFt6UWvv1ht65ptE7KOeN215" /* 2048 bytes */
                      // "063lPLXusS0KbZcuAgXFqXIuhVFxT7PbPdA9CifI7gBC4ia4H0uQiccRRLOaj300"
                      // "1Yo8BjcgFF8aFtHLHZdRKNxliQEa8ozq38YP8dSXIwbAw2fMx46f8Xc3CmMou301"
                      // "27lBkvtUauXX1V8oF4jT4CKIlbE03ghFBS2hTcnTiZ1Go5Ti1gVWRcUdHAh9g302"
                      // "3hSAFpT2dLgVxoo2ZCHhNSE0rm2I5OBVhdklMnzBgDDW5gdpOm7389BaNgqdA303"
                      // "47oUilXORLjy7kDu86GHSNLzAJjXUxxEc7PeJfPLyC0khgSy4aYUhlFsenbK4304"
                      // "5DSZ5YS33eoOeiTze2onUl7PfPz8No6W7AHYW7S34vUADHDOcABcEkDlpxQho305"
                      // "6pM1ksIrfHSbpfCJhoqqh9w9kjITgdDrqbp9aHTzjZOScTCtzpMFACJalZNRz306"
                      // "7AY301mZjr2KD8pVrhkNfNxQyg1nmm7QJSXN4lDj0dIawEflfSVXLGShl0vxc307"
                      // "8sxK5w6msIvzeRDgaYAJ5zmMWg42S4Ap7WA1eHYepIrlwmn1TdwSEPPcMx6Tj308"
                      // "9Ilj3gU7JfgoItY5FqNQJgYy6Vw0NPdwwPcnfPmtLhH5nl55eJFwwpB6UgZan309"
                      // "10dgiEiFy8GoGIaOjXC387ihvfvaiGA2sBgh3k87i7Eb7QTNrthpZSdlIk8PE310"
                      // "11qwCORP6psr9r8BBTnJqYpuayMhQD8vuxp9aBXaMLWmBAYo2EAtB45YeXEGU311"
                      // "12BUgN9AgmDERPzcyeOYG0ExpDE5cVY8OL5pNXM43MNAt7jkJAFnSRNKw6Qq5312"
                      // "13bFRN69XbWj5t4HkcFMiVwIOLQARBNmcimSzlnhipF8bPY2P1QfsSqCfsnsu313"
                      // "14uqVbPvrWTSGIrfVveI01YwiIWOeXjeMdh78Ruq9zy9xbkNkwOLcAiXzlrF0314"
                      // "15YVCvYVKtzRSOCtCh5KPrTw0D2H5gvFFRgAH1ZgYFt6UWvv1ht65ptE7KOeN315"; /* 3072 bytes */
// "163lPLXusS0KbZcuAgXFqXIuhVFxT7PbPdA9CifI7gBC4ia4H0uQiccRRLOajC16";



// static uint32_t seq_id;
static uint8_t seq_id;


#if !NETSTACK_CONF_WITH_IPV6 || !UIP_CONF_ROUTER || !UIP_IPV6_MULTICAST || !UIP_CONF_IPV6_RPL
#error "This example can not work with the current contiki con1024figuration"
#error "Check the values of: NETSTACK_CONF_WITH_IPV6, UIP_CONF_ROUTER, UIP_CONF_IPV6_RPL"
#endif
/*---------------------------------------------------------------------------*/
PROCESS(rpl_root_process, "RPL ROOT, Multicast Sender");
AUTOSTART_PROCESSES(&rpl_root_process);
/*---------------------------------------------------------------------------*/

/*

function: unicast reception n/a
brief: prepare missing packet data using sequence/packet num

*/

// static void
// prepare_ucast(uint8_t pckt_num) {


//   PRINTF("function to prepare ucast reception and response");

//   //start_byte = seq_idd[MAX_PAYLOAD_LEN * pckt_num]
//   //end_byte = seq_idd[MAX_PAYLOAD_LEN * pckt_num] + MAX_PAYLOAD_LEN
// }







/*---------------------------------------------------------------------------*/


/*

function: unicast request receive and response

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
  PRINTF("unicast request at root received: \n");
  PRINTF("date len received at root: %u \n", datalen);

  // LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  PRINTF("missing packets received: ");
  for(int i = 0; data[i] != '\0'; i++){
    PRINTF("%u", data[i]);
  }
  // PRINTF("%s", (char *)data);

  PRINTF(" from ");

  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
#if WITH_SERVER_REPLY
  /* send back the same string to the client as an echo reply */
  // LOG_INFO("Sending response.\n");
  // simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
#endif /* WITH_SERVER_REPLY */
}


/*---------------------------------------------------------------------------*/

static void
multicast_send(void)
{
  // // uint8_t id;
  // uint8_t len;  //length of the data
  // uint8_t chks; //number of chunks
  // // uint8_t pkts; //number of packets

  // len = strlen(seq_idd);

  // chks = len/MAX_PAYLOAD_LEN;



  static uint8_t i;
  static packet_data packet_data_send;


  if (i < DATA_CHNKS) {

    packet_data_send.seq_num = seq_id;
    packet_data_send.tot_chnks = DATA_CHNKS;




    // memset(buf, 0, MAX_PAYLOAD_LEN);

    memset(packet_data_send.buf, 0, sizeof(uint8_t)*MAX_PAYLOAD_LEN);


    // for(int i = 0; i < DATA_CHNKS; i++){
    for (int k = 0; seq_idd[k] != '\0' && k < MAX_PAYLOAD_LEN; k++) {
      packet_data_send.buf[k] = seq_idd[k + (i * MAX_PAYLOAD_LEN)];
    }

    PRINTF("Send to: ");
    PRINT6ADDR(&mcast_conn->ripaddr);
    PRINTF(" Local Port %u,", uip_ntohs(mcast_conn->lport));
    PRINTF(" Remote Port %u,", uip_ntohs(mcast_conn->rport));
    // PRINTF(" (msg=0x%08"PRIx32")", uip_ntohl(*((uint32_t *)buf)));
    PRINTF(" (msg= %s)", packet_data_send.buf);

    PRINTF(" %lu bytes", (unsigned long)sizeof(packet_data_send.buf));
    PRINTF(" %lu bytes", (unsigned long)sizeof(packet_data_send));
    PRINTF(" %lu bytes\n", (unsigned long)sizeof(seq_id));

    seq_id++;
    uip_udp_packet_send(mcast_conn, &packet_data_send, sizeof(packet_data_send));

    i++;

  }
  else {
    i = 0;
  }

}
/*---------------------------------------------------------------------------*/
static void
prepare_mcast(void)
{
  uip_ipaddr_t ipaddr;

#if UIP_MCAST6_CONF_ENGINE == UIP_MCAST6_ENGINE_MPL
  /*
   * MPL defines a well-known MPL domain, MPL_ALL_FORWARDERS, which
   *  MPL nodes are automatically members of. Send to that domain.
   */
  uip_ip6addr(&ipaddr, 0xFF03, 0, 0, 0, 0, 0, 0, 0xFC);
#else
  /*
   * IPHC will use stateless multicast compression for this destination
   * (M=1, DAC=0), with 32 inline bits (1E 89 AB CD)
   */
  uip_ip6addr(&ipaddr, 0xFF1E, 0, 0, 0, 0, 0, 0x89, 0xABCD);
#endif
  mcast_conn = udp_new(&ipaddr, UIP_HTONS(MCAST_SINK_UDP_PORT), NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rpl_root_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  PRINTF("Multicast Engine: '%s'\n", UIP_MCAST6.name);

  NETSTACK_ROUTING.root_start();

  prepare_mcast();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                       UDP_CLIENT_PORT, udp_rx_callback);


  etimer_set(&et, START_DELAY * CLOCK_SECOND);
  while (1) {
    PROCESS_YIELD();
    if (etimer_expired(&et)) {
      if (seq_id == ITERATIONS) {
        etimer_stop(&et);
      } else {
        multicast_send();
        etimer_set(&et, (SEND_INTERVAL * 60));
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
