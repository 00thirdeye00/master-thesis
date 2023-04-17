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
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_



/* Set to enable TSCH security */
#ifndef WITH_SECURITY
#define WITH_SECURITY 0
#endif /* WITH_SECURITY */

/*******************************************************/
/******************* Configure TSCH ********************/
/*******************************************************/

/* IEEE802.15.4 PANID */
#define IEEE802154_CONF_PANID 0x81a5

/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
#define TSCH_CONF_AUTOSTART 0
/* Set number of frequences, few frequences faster joining */
#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE TSCH_HOPPING_SEQUENCE_2_2


#if WITH_SECURITY
/* Enable security */
#define LLSEC802154_CONF_ENABLED 1
#endif /* WITH_SECURITY */

/*******************************************************/
/************* Orchestra conf **************/
/*******************************************************/

// ORCHESTRA scheduling,
// low value gives fast response high power consumption
// high value gives high latenct, low power consmption
// Tune based on needs. Config should be a prime number
// that is mutually exlusive
#ifdef ORCHESTRA_CONF_RULES
#define ORCHESTRA_CONF_EBSF_PERIOD 397 // Default 397
#define ORCHESTRA_CONF_COMMON_SHARED_PERIOD 31// Default 31
#define ORCHESTRA_CONF_UNICAST_PERIOD 17 // Default 17, Alice 43
#endif



// UIP Config

#define QUEUEBUF_CONF_NUM 32 // default 8 ()
#define UIP_CONF_BUFFER_SIZE 1280 // Default 1280
#define NBR_TABLE_CONF_MAX_NEIGHBORS 16 //Default 16


// Number of route entries
#define NETSTACK_MAX_ROUTE_ENTRIES 255

/* Logging LOG_LEVEL_INFO LOG_LEVEL_NONE*/
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_ORCHESTRA                   LOG_LEVEL_INFO
#define TSCH_LOG_CONF_PER_SLOT                     0

#endif /* PROJECT_CONF_H_ */