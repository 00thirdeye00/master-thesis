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
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define COAP_DOWNLOADER 1


// LOG_LEVEL_NONE LOG_LEVEL_ERR LOG_LEVEL_WARN LOG_LEVEL_INFO LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_LWM2M               LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_COAP                LOG_LEVEL_NONE

#ifdef BOARD_STRING
#define LWM2M_DEVICE_MODEL_NUMBER BOARD_STRING
#elif defined(CONTIKI_TARGET_WISMOTE)
#include "dev/watchdog.h"
#define LWM2M_DEVICE_MODEL_NUMBER "LWM2M_DEVICE_MODEL_NUMBER"
#define LWM2M_DEVICE_MANUFACTURER "LWM2M_DEVICE_MANUFACTURER"
#define LWM2M_DEVICE_SERIAL_NO    "LWM2M_DEVICE_SERIAL_NO"
#define PLATFORM_REBOOT watchdog_reboot
#endif

#define LWM2M_DEFAULT_CLIENT_LIFETIME 3600 // Update will sent every LWM2M_DEFAULT_CLIENT_LIFETIME/2 s

/* Increase rpl-border-router IP-buffer when using more than 64. */
#define COAP_MAX_CHUNK_SIZE            1024

/* Multiplies with chunk size, be aware of memodery constraints. */
#define COAP_MAX_OPEN_TRANSACTIONS     4

/* Filtering .well-known/core per query can be disabled to save space. */
#define COAP_LINK_FORMAT_FILTERING     0
#define COAP_PROXY_OPTION_PROCESSING   0

/* Enable client-side support for COAP observe */
#define COAP_OBSERVE_CLIENT 1

/* Definitions to enable Queue Mode, include the dynamic adaptation and change the default parameters  */
/* #define LWM2M_QUEUE_MODE_CONF_ENABLED 1
   #define LWM2M_QUEUE_MODE_CONF_INCLUDE_DYNAMIC_ADAPTATION 1
   #define LWM2M_QUEUE_MODE_CONF_DEFAULT_CLIENT_AWAKE_TIME 2000
   #define LWM2M_QUEUE_MODE_CONF_DEFAULT_CLIENT_SLEEP_TIME 10000
   #define LWM2M_QUEUE_MODE_CONF_DEFAULT_DYNAMIC_ADAPTATION_FLAG 0
   #define LWM2M_QUEUE_MODE_OBJECT_CONF_ENABLED 1 */


#define QUEUEBUF_CONF_NUM 64
#define UIP_CONF_TCP 1
#define UIP_CONF_MAX_ROUTES 256
#define NBR_TABLE_CONF_MAX_NEIGHBORS 128

#endif /* PROJECT_CONF_H_ */
