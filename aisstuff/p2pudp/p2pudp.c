/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 */
/**
 * \file
 *         Simple P2P client

 *
 * \author Anders Isberg <anders.isberg@sony.com>
 */
#include "contiki.h"
#include "contiki-net.h"
#include "sys/log.h"
#include "sys/etimer.h"
#include "sys/node-id.h"
#include "lib/random.h"
#include "net/linkaddr.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include <stdio.h>
#include "p2pudp.h"


#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"

/* Log configuration */
#define LOG_MODULE "P2PDUP"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Socket for p2p communication */
static struct simple_udp_connection p2p_socket;

/*---------------------------------------------------------------------------*/

/* Get neighbor helper function*/
static uip_ipaddr_t*
get_neighbor(void) {
  static uip_ds6_nbr_t *nbr = NULL;

  if (nbr == NULL) {
     nbr = uip_ds6_nbr_head();  
  } else {
     nbr = uip_ds6_nbr_next(nbr);
     if (nbr == NULL) {
        nbr = uip_ds6_nbr_head();  
     }
  }

  return (nbr != NULL)? &nbr->ipaddr: (uip_ipaddr_t*) NULL;
}

/*---------------------------------------------------------------------------*/

/*
 * UDP RX Callback
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
  const struct p2p_data_t *p2p = (const struct p2p_data_t *) data;
  static struct p2p_data_t p2p_resp;

  if(datalen > 0  && datalen == sizeof(struct p2p_data_t)) {
    switch (p2p->type) {
        case P2P_REQ:
            LOG_INFO("Received Request: %s, Seq Number: %u, From: ", p2p->data, p2p->seq_no);    
            LOG_INFO_6ADDR(sender_addr);
            LOG_INFO_("\n");

            /* Send response to sender of request */
            p2p_resp.type = P2P_RESP;
            p2p_resp.seq_no = p2p->seq_no;
            memcpy((void *)p2p_resp.data, "OK", MIN(sizeof(p2p_resp.data), strlen("OK")));
            simple_udp_sendto(&p2p_socket, &p2p_resp, sizeof(struct p2p_data_t), sender_addr);
            break;

        case P2P_RESP:
            LOG_INFO("Received Resp: %s, Seq Number: %u, From: ", p2p->data, p2p->seq_no);    
            LOG_INFO_6ADDR(sender_addr);
            LOG_INFO_("\n");
            break;           
    }    
  } else { // Unknown data
    LOG_INFO("Received unknown data: Len (%u bytes), From: ", datalen);
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
  }
}


/*---------------------------------------------------------------------------*/

#define P2P_SEND_INTERVAL (10*CLOCK_SECOND) // 10 seconds
#define P2P_PORT 32929

/*---------------------------------------------------------------------------*/
PROCESS(p2p_process, "P2P Module");
AUTOSTART_PROCESSES(&p2p_process);
/*---------------------------------------------------------------------------*/
#define SEND_INTERVAL 30
PROCESS_THREAD(p2p_process, ev, data)
{
  static struct p2p_data_t  p2p_req;
  static struct etimer et;
  static uip_ipaddr_t *dest_addr;

  PROCESS_BEGIN();

  /* Clear seq_no */
  p2p_req.seq_no = 0;
  
  /* Initialize NETSTACK */
  if(node_id == 1) {
    NETSTACK_ROUTING.root_start();
  }

  NETSTACK_MAC.on();
  

  /* Initialize UDP socket */
  simple_udp_register(&p2p_socket, P2P_PORT, NULL, P2P_PORT, udp_rx_callback);

/* Set event timer */
  etimer_set(&et, P2P_SEND_INTERVAL-CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND))); 

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if(NETSTACK_ROUTING.node_is_reachable()) {
        dest_addr = get_neighbor();
        if (dest_addr != NULL) {
            p2p_req.type = P2P_REQ;
            p2p_req.seq_no = (p2p_req.seq_no<255)?  p2p_req.seq_no+1: 0;
            memcpy((void *)p2p_req.data, "Hello World", MIN(sizeof(p2p_req.data), strlen("Hello World")));
            simple_udp_sendto(&p2p_socket, &p2p_req, sizeof(struct p2p_data_t), dest_addr);
        }
    }

    // Wakeup every P2P_SEND_INTERVAL with some jitter
    etimer_set(&et, P2P_SEND_INTERVAL-CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND))); 
  }

  PROCESS_END();
}