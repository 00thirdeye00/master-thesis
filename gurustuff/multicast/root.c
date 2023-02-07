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
#include "lib/queue.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6-route.h"

#include "dependencies.h"

#include <string.h>
#include <inttypes.h>


#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"
// #include "net/routing/routing.h"


// #define MAX_PAYLOAD_LEN 64
#define MCAST_SINK_UDP_PORT 3001 /* Host byte order */
// #define SEND_INTERVAL CLOCK_SECOND /* clock ticks */


// #define DATA_LEN    (strlen(seq_idd))
// #define DATA_CHNKS  (DATA_LEN / MAX_PAYLOAD_LEN)




// #define ITERATIONS DATA_CHNKS /* messages */



/* Start sending messages START_DELAY secs after we start so that routing can
 * converge */
#define START_DELAY_MINS  10
#define START_DELAY       (MIN_ONE * START_DELAY_MINS * CLOCK_SECOND)
#define CHECK_INTERVAL    (MIN_ONE * CLOCK_SECOND)

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;

static struct uip_udp_conn * mcast_conn;


typedef enum{
  TIME_RSET = 0,
  TIME_SET,
} mult_sendtim_t;

mult_sendtim_t mult_send_flag;


typedef struct ureq_mpckts_s{
  struct ureq_mpckts_s *next;
  struct ureq_mpckts_s *previous;
  uip_ipaddr_t send_addr;
  uint8_t *data;
  uint16_t datalen;
} ureq_mpckts_t;

// static ureq_mpckts_t *ureq_q;
ureq_mpckts_t *ureq_q;


typedef struct demo_struct_s {
  struct demo_struct_s *next;
  struct demo_struct_s *previous;
  unsigned short value;
} demo_struct_t;

#define DATA_STRUCTURE_DEMO_ELEMENT_COUNT 4
// static demo_struct_t elements[DATA_STRUCTURE_DEMO_ELEMENT_COUNT];
// static demo_struct_t *elements;


//"063lPLXusS0KbZcuAgXFqXIuhVFxT7PbPdA9CifI7gBC4ia4H0uQiccRRLOaj100fsffdasfas"



// static char buf[MAX_PAYLOAD_LEN];
// const char *seq_idd = "063lPLXusS0KbZcuAgXFqXIuhVFxT7PbPdA9CifI7gBC4ia4H0uQiccRRLOaj100"
//                       "1Yo8BjcgFF8aFtHLHZdRKNxliQEa8ozq38YP8dSXIwbAw2fMx46f8Xc3CmMou101"
//                       "27lBkvtUauXX1V8oF4jT4CKIlbE03ghFBS2hTcnTiZ1Go5Ti1gVWRcUdHAh9g102"
//                       "3hSAFpT2dLgVxoo2ZCHhNSE0rm2I5OBVhdklMnzBgDDW5gdpOm7389BaNgqdA103"
//                       "47oUilXORLjy7kDu86GHSNLzAJjXUxxEc7PeJfPLyC0khgSy4aYUhlFsenbK4104"
//                       "5DSZ5YS33eoOeiTze2onUl7PfPz8No6W7AHYW7S34vUADHDOcABcEkDlpxQho105"
//                       "6pM1ksIrfHSbpfCJhoqqh9w9kjITgdDrqbp9aHTzjZOScTCtzpMFACJalZNRz106"
//                       "7AY301mZjr2KD8pVrhkNfNxQyg1nmm7QJSXN4lDj0dIawEflfSVXLGShl0vxc107"
//                       "8sxK5w6msIvzeRDgaYAJ5zmMWg42S4Ap7WA1eHYepIrlwmn1TdwSEPPcMx6Tj108"
//                       "9Ilj3gU7JfgoItY5FqNQJgYy6Vw0NPdwwPcnfPmtLhH5nl55eJFwwpB6UgZan109"
//                       "10dgiEiFy8GoGIaOjXC387ihvfvaiGA2sBgh3k87i7Eb7QTNrthpZSdlIk8PE110"
//                       "11qwCORP6psr9r8BBTnJqYpuayMhQD8vuxp9aBXaMLWmBAYo2EAtB45YeXEGU111"
//                       "12BUgN9AgmDERPzcyeOYG0ExpDE5cVY8OL5pNXM43MNAt7jkJAFnSRNKw6Qq5112"
//                       "13bFRN69XbWj5t4HkcFMiVwIOLQARBNmcimSzlnhipF8bPY2P1QfsSqCfsnsu113"
//                       "14uqVbPvrWTSGIrfVveI01YwiIWOeXjeMdh78Ruq9zy9xbkNkwOLcAiXzlrF0114"
//                       "15YVCvYVKtzRSOCtCh5KPrTw0D2H5gvFFRgAH1ZgYFt6UWvv1ht65ptE7KOeN115"; /* 1024 bytes */
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
static uint8_t q_elem;



#if !NETSTACK_CONF_WITH_IPV6 || !UIP_CONF_ROUTER || !UIP_IPV6_MULTICAST || !UIP_CONF_IPV6_RPL
#error "This example can not work with the current contiki con1024figuration"
#error "Check the values of: NETSTACK_CONF_WITH_IPV6, UIP_CONF_ROUTER, UIP_CONF_IPV6_RPL"
#endif
/*---------------------------------------------------------------------------*/
PROCESS(rpl_root_process, "RPL ROOT, Multicast Sender");
PROCESS(queue_proc, "queue process");

AUTOSTART_PROCESSES(&rpl_root_process, &queue_proc);

// AUTOSTART_PROCESSES(&queue_proc);
/*---------------------------------------------------------------------------*/

QUEUE(ureq_queue);
QUEUE(demo_queue);


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
static void
queue_check_enq(void){

  demo_struct_t *elements;

  printf("=====\n");
  printf("Queue\n");

  elements = (demo_struct_t *)heapmem_alloc(sizeof(demo_struct_t));

  elements->next = NULL;
  // elements->previous = NULL;
  elements->value = 10;
  queue_enqueue(demo_queue, elements);
  printf("Enqueue: 0x%04x\n", elements->value);


  printf("Peek: 0x%04x\n",
         ((demo_struct_t *)queue_peek(demo_queue))->value);

}



static void
queue_check_deq(void){

  demo_struct_t *this;

  if(!queue_is_empty(demo_queue)){
  // for(int i = 0; i <= DATA_STRUCTURE_DEMO_ELEMENT_COUNT; i++) {
    this = queue_dequeue(demo_queue);
    printf("Dequeue: ");
    if(this == NULL) {
      printf("(queue underflow)\n");
    } else {
      printf("0x%04lx\n", (unsigned long)this->value);
      heapmem_free(this);
    }
  }

  printf("Queue is%s empty\n",
         queue_is_empty(demo_queue) ? "" : " not");
}

*/

/*---------------------------------------------------------------------------*/


/*

function: create data packet

*/


static packet_data*
create_datapckt(uint8_t chnk){

  static packet_data packet_data_send;

  packet_data_send.seq_num = chnk;
  packet_data_send.tot_chnks = DATA_CHNKS;


  // memset(buf, 0, MAX_PAYLOAD_LEN);

  memset(packet_data_send.buf, 0, sizeof(uint8_t)*MAX_PAYLOAD_LEN);


  // for(int i = 0; i < DATA_CHNKS; i++){
  // for (int i = 0, k = chnk; seq_idd[k] != '\0' && k < (chnk+MAX_PAYLOAD_LEN); i++, k++) {
  //   packet_data_send.buf[k] = seq_idd[k + (i * MAX_PAYLOAD_LEN)];
  // }

  for (int i = 0, k = (chnk * MAX_PAYLOAD_LEN); i < MAX_PAYLOAD_LEN && k < ((chnk+1)*MAX_PAYLOAD_LEN); i++, k++) {
    packet_data_send.buf[i] = seq_idd[k];
  }  

  // PRINTF(" (msg=0x%08"PRIx32")", uip_ntohl(*((uint32_t *)buf)));
  PRINTF(" (msg= %s)", packet_data_send.buf);

  PRINTF(" %lu bytes", (unsigned long)sizeof(packet_data_send.buf));
  PRINTF(" %lu bytes", (unsigned long)sizeof(packet_data_send));
  PRINTF(" %lu bytes\n", (unsigned long)sizeof(seq_id));

  return (&packet_data_send);

}


/*---------------------------------------------------------------------------*/




static void
unicast_send(uint8_t chnk, uip_ipaddr_t send_addr){
  
  static packet_data *packet_data_send_u;

  packet_data_send_u = (void *)create_datapckt(chnk);


#if WITH_SERVER_REPLY
  /* send back the same string to the client as an echo reply */
  LOG_INFO("Sending Response to Sink\n");
  // simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
  simple_udp_sendto(&udp_conn, packet_data_send_u, sizeof(packet_data), &send_addr);
#endif /* WITH_SERVER_REPLY */

}

// static struct etimer et_uni_send_data;


static void
uni_queue_deq(){

  PRINTF("Deque in Progress\n");

  ureq_mpckts_t *dq;

  // printf("queue peek datalen: %u\n", ((ureq_mpckts_t *)queue_peek(ureq_queue))->datalen);


  if(!queue_is_empty(ureq_queue)){
    PRINTF("Queue is not Empty\n");
    dq = queue_dequeue(ureq_queue);
    // if(q_elem > 0) q_elem--;
    if(dq == NULL){
      PRINTF("Queue Underflow \n");
    } else {
      q_elem--;
      PRINTF("Dequeued datalen: %u\n", dq->datalen);
      PRINTF("Dequeue data: ");
      for(int i = 0; i < dq->datalen; i++){
        PRINTF(" %u", dq->data[i]);
        // etimer_set(&et_uni_send_data, 2 * CLOCK_SECOND);
        // if(etimer_expired(&et_uni_send_data)){
          unicast_send(dq->data[i], dq->send_addr);
          // etimer_set(&et_uni_send_data, 2 * CLOCK_SECOND);
        // }
        // etimer_stop(&et_uni_send_data);
        // PRINTF("")
      }


      heapmem_free(dq->data);
      heapmem_free(dq);

      PRINTF("\n");

    }
  }
  PRINTF("Deque is Complete\n");
}


/*-------------------------------------------------------------------------------*/

/*

function name   : uni_queue_enq
Parameters      : senders address, data length, data
return          : void
description     : unicast queue enqueue, to enque missing data's seq number
                  received from nodes

*/

/*-------------------------------------------------------------------------------*/


static void
uni_queue_enq(const uip_ipaddr_t *sender_addr, uint16_t dlen, const uint8_t *data){
  PRINTF("Enqueue in Progress\n");

  // ureq_mpckts_t *ureq_q;

  ureq_q = (ureq_mpckts_t *)heapmem_alloc(sizeof(ureq_mpckts_t));

  if(ureq_q == NULL){
    PRINTF("Failed to Allocate Memory\n");
    // return 0;
  }

  ureq_q->next = NULL;

  uip_ipaddr_copy(&ureq_q->send_addr, sender_addr);

  ureq_q->data = (uint8_t *)heapmem_alloc(dlen * sizeof(uint8_t));

  memcpy(ureq_q->data, data, dlen);

  ureq_q->datalen = dlen;

  PRINTF("Printing from Queue Created: \n");
  for(int i = 0; i < ureq_q->datalen; i++){
    PRINTF(" %u", ureq_q->data[i]);
  }
  PRINTF("\nPrinting from Queue Created Complete \n");

  if(q_elem >= QUEUE_SIZE /*|| !queue_is_empty()*/){
    PRINTF(">>>>>>>> Queue is Full >>>>>>>>\n");
    // uni_queue_deq();
    return;
  } else if(q_elem >= 0 && q_elem < QUEUE_SIZE) {
    queue_enqueue(ureq_queue, ureq_q);
    q_elem++;
  }
  printf("Queue is%s Empty\n",
         queue_is_empty(ureq_queue) ? "" : " not");


  printf("Queue Peek datalen: %u\n", ((ureq_mpckts_t *)queue_peek(ureq_queue))->datalen);

  PRINTF("Enqueue is Complete\n");

  // uni_queue_deq();
  // process_start(&queue_proc, NULL);

}


/*-------------------------------------------------------------------------------*/

/*

function name   : udp_rx_callback
Parameters      : udp connection, sender address, sender port, received address,
                  receiver port, data, datalen
return          : void
description     : callback is triggered when there is a udp data sent to root
                  address and the received data is enqueued
*/

/*-------------------------------------------------------------------------------*/

static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
  
  static uint8_t i;
  // static packet_data *packet_data_send_u;
  

// typedef struct ureq_mpckts_s{
//   struct ureq_mpckts_s *next;
//   struct ureq_mpckts_s *previous;
//   uip_ipaddr_t send_addr;
//   uint8_t *data;
//   uint16_t datalen
// }ureq_mpckts_t;

  uni_queue_enq(sender_addr, datalen, data);


  // ureq_q = (ureq_mpckts_t *)heapmem_alloc(sizeof(ureq_mpckts_t));

  // if(ureq_q == NULL){
  //   PRINTF("failed to allocate memory\n");
  //   // return 0;
  // }

  // ureq_q->next = NULL;

  // uip_ipaddr_copy(&ureq_q->send_addr, sender_addr);

  // // ureq_q->data = (uint8_t *)heapmem_alloc(datalen * sizeof(uint8_t));

  // // memcpy(ureq_q->data, data, datalen);

  // ureq_q->datalen = datalen;

  // PRINTF("printing from queue created: \n");
  // // for(int i = 0; i < ureq_q->datalen; i++){
  // //   PRINTF(" %u", ureq_q->data[i]);
  // // }

  // PRINTF("\nprinting from queue created complete \n");

  // queue_enqueue(ureq_queue, ureq_q);
  // printf("Queue is%s empty\n",
  //        queue_is_empty(ureq_queue) ? "" : " not");


  // printf("queue peek datalen: %u\n", ((ureq_mpckts_t *)queue_peek(ureq_queue))->datalen);

  // uni_queue_deq();


  PRINTF("Request at Root Received:\n");
  PRINTF("datelen Received at Root: %u \n", datalen);

  // LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  PRINTF("Missing Packets Received at Root:");
  // PRINTF("%u", *data);

  for(int i = 0; i < datalen; i++){
    // PRINTF("inside for \n");
    PRINTF(" %u", data[i]);
  }
  // PRINTF("%s", (char *)data);

  PRINTF(" from ");

  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");



//   packet_data_send_u = (void *)create_datapckt(ureq_q->data[0]);


// #if WITH_SERVER_REPLY
//   /* send back the same string to the client as an echo reply */
//   LOG_INFO("Sending response to sink.\n");
//   // simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
//   simple_udp_sendto(&udp_conn, packet_data_send_u, sizeof(packet_data_send_u), sender_addr);
// #endif /* WITH_SERVER_REPLY */



  // heapmem_free(ureq_q->data);
  // heapmem_free(ureq_q);
  i++;

}


/*---------------------------------------------------------------------------*/

static void
multicast_send(void)
{

  #ifdef UNIREQ_ALLDATA
    return;
  #endif

  static uint8_t i;
  static packet_data packet_data_send;


  if (i < DATA_CHNKS) {

    packet_data_send.seq_num = seq_id;
    packet_data_send.tot_chnks = DATA_CHNKS;


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

    #ifndef UNIREQ_ALLDATA
    uip_udp_packet_send(mcast_conn, &packet_data_send, sizeof(packet_data_send));
    #endif

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

  // static ureq_mpckts_t *this;

  // queue_init(demo_queue);

  PROCESS_BEGIN();

  queue_init(ureq_queue);
  queue_init(demo_queue);

  PRINTF("Multicast Engine: '%s'\n", UIP_MCAST6.name);

  NETSTACK_ROUTING.root_start();

  prepare_mcast();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                       UDP_CLIENT_PORT, udp_rx_callback);


  // etimer_set(&et, 60 * CLOCK_SECOND);
  etimer_set(&et, START_DELAY);
  while (1) {

    // ureq_mpckts_t *this;


    // PRINTF("in while \n");
    // uni_queue_deq();

    // queue_check_enq();
    // queue_check_deq();

    // PRINTF("Routing Entries: %u\n", uip_ds6_route_num_routes());

    if((etimer_expired(&et) && (uip_ds6_route_num_routes() > NUM_OF_NODES)) || mult_send_flag == TIME_SET){
      if(!(mult_send_flag == TIME_SET)){
        etimer_set(&et, SEND_INTERVAL);
        mult_send_flag = TIME_SET;
      }
    } else {
      etimer_set(&et, CHECK_INTERVAL);
    }


    PROCESS_YIELD();

    PRINTF("Routing Entries: %u\n", uip_ds6_route_num_routes());

    if (etimer_expired(&et) && mult_send_flag == TIME_SET) {
      // if(uip_ds6_route_num_routes() == NUM_OF_NODES){ //wait until all the nodes are routed
        // uni_queue_deq();
        if (seq_id == ITERATIONS) {
          mult_send_flag = TIME_RSET;
          etimer_stop(&et);
          // etimer_set(&et, (SEND_INTERVAL));
        } else {
          // PRINTF("Routing Entries: %u\n", uip_ds6_route_num_routes());
// #if 0
          multicast_send();
// #endif
          etimer_set(&et, (SEND_INTERVAL));
        }
      // } else {
        // etimer_set(&et, (SEND_INTERVAL));
      // }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/



PROCESS_THREAD(queue_proc, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  etimer_set(&et, QUE_START_INTERVAL);
  while(1){

    // PRINTF("In Queue Process Yield\n");
    PROCESS_YIELD();
    if(etimer_expired(&et) && !queue_is_empty(ureq_queue))
    {
      PRINTF("Queue Process is Not Empty\n");
      uni_queue_deq();
    }

    etimer_set(&et, 500);
  }

  // printf("queue process\n");

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/









































