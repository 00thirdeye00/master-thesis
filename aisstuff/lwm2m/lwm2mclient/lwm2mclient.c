/*
 * Copyright (c) 2015, Yanzi Networks AB.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *      OMA LWM2M and IPSO Objects example.
 * \author
 *      Joakim Eriksson, joakime@sics.se
 *      Niclas Finne, nfi@sics.se
 *      Carlos Gonzalo Peces, carlosgp143@gmail.com
 */

#include "contiki.h"
#include "dev/leds.h"
#include "services/lwm2m/lwm2m-engine.h"
#include "services/lwm2m/lwm2m-rd-client.h"
#include "services/lwm2m/lwm2m-device.h"
#include "services/lwm2m/lwm2m-server.h"
#include "services/lwm2m/lwm2m-firmware.h"

#define DEBUG DEBUG_NONE
#include "net/ipv6/uip-debug.h"

/* Define this macro to non-zero to register via a bootstrap server */
#ifndef REGISTER_WITH_LWM2M_BOOTSTRAP_SERVER
#define REGISTER_WITH_LWM2M_BOOTSTRAP_SERVER 0
#endif

#if REGISTER_WITH_LWM2M_BOOTSTRAP_SERVER
#define SERVER_TYPE LWM2M_RD_CLIENT_BOOTSTRAP_SERVER
#else
#define SERVER_TYPE LWM2M_RD_CLIENT_LWM2M_SERVER
#endif

#ifndef LWM2M_SERVER_ADDRESS
#define LWM2M_SERVER_ADDRESS "coap://[fd00::1]"
#endif

static lwm2m_session_info_t session_info;

/* Define this macro to register with a second LWM2M server */
#ifdef LWM2M_SERVER_ADDRESS_SECOND
static lwm2m_session_info_t session_info_second;
#endif

PROCESS(lwm2m_objects, "LWM2M object example");
AUTOSTART_PROCESSES(&lwm2m_objects);
/*---------------------------------------------------------------------------*/
static void
setup_lwm2m_servers(void)
{
#ifdef LWM2M_SERVER_ADDRESS
  coap_endpoint_t server_ep;
  if(coap_endpoint_parse(LWM2M_SERVER_ADDRESS, strlen(LWM2M_SERVER_ADDRESS),
                         &server_ep) != 0) {
    lwm2m_rd_client_register_with_server(&session_info, &server_ep, SERVER_TYPE);
  }
#endif /* LWM2M_SERVER_ADDRESS */

#ifdef LWM2M_SERVER_ADDRESS_SECOND
  coap_endpoint_t server_ep_second;
  if(coap_endpoint_parse(LWM2M_SERVER_ADDRESS_SECOND, strlen(LWM2M_SERVER_ADDRESS_SECOND),
                         &server_ep_second) != 0) {
    lwm2m_rd_client_register_with_server(&session_info_second, &server_ep_second, SERVER_TYPE);
  }
#endif /* LWM2M_SERVER_ADDRESS_SECOND */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(lwm2m_objects, ev, data)
{
  static struct etimer periodic;
  PROCESS_BEGIN();
  PROCESS_PAUSE();

    /* Initialize the OMA LWM2M engine */
  lwm2m_engine_init();

  /* Register default LWM2M objects */
  lwm2m_device_init();
  lwm2m_server_init();
  lwm2m_firmware_init();

  setup_lwm2m_servers();
  /* Tick loop each 5 seconds */
  etimer_set(&periodic, CLOCK_SECOND * 5);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_TIMER && etimer_expired(&periodic)) {
      etimer_reset(&periodic);
    }
  }
  PROCESS_END();
}
