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
 *          Project specific configuration defines for the RPl multicast
 *          example.
 *
 * \author
 *          Guru Mehar Rachaputi
 *          Anders Isberg
 *         
 * \reviewer
 *          Anders Isberg
 * 
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#include "net/ipv6/multicast/uip-mcast6-engines.h"

// Log level config
#ifdef LOG_INFO
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#else
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#endif

/* Length of various slot frames */
#define ORCHESTRA_CONF_EBSF_PERIOD          397
#define ORCHESTRA_CONF_UNICAST_PERIOD       17
#define ORCHESTRA_CONF_COMMON_SHARED_PERIOD 23

/* For Imin: Use 16 over CSMA, 64 over Contiki MAC */
#define ROLL_TM_CONF_IMIN_1         64
#define MPL_CONF_DATA_MESSAGE_IMIN  64
#define MPL_CONF_CONTROL_MESSAGE_IMIN  64

/* Size of the multicast routing table */
#define UIP_MCAST6_ROUTE_CONF_ROUTES 1
/* Disable TCP */
#define UIP_CONF_TCP 0

// Enable Fragementation
#define SICSLOWPAN_CONF_FRAG 1

/**
 * Timeout for packet reassembly at the 6lowpan layer
 * (should be < 60s)
 */
#define SICSLOWPAN_CONF_MAXAGE 60

// Size of send queue, default is 8
#define QUEUEBUF_CONF_NUM 64


/* Code/RAM footprint savings so that things will fit on our device */
#ifndef NETSTACK_MAX_ROUTE_ENTRIES
#define NETSTACK_MAX_ROUTE_ENTRIES   512
#endif

/* Number of entries in the neighbors table */
#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 32
#endif

/* To use heap module we need to explicitly set this 
macro since by default it is only 1 bytes */
#define HEAPMEM_CONF_ARENA_SIZE 4096


#endif /* PROJECT_CONF_H_ */
