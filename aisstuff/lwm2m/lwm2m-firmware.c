/*
 * Copyright (c) 2016, SICS Swedish ICT AB
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \addtogroup lwm2m
 * @{
 *
 * Code for firmware object of lwm2m
 *
 */
#include "contiki.h"
#include "contiki-net.h"

#include "lwm2m-engine.h"
#include "lwm2m-firmware.h"
#include "coap.h"
#include <inttypes.h>
#include <string.h>

#ifdef HTTP_DOWNLOADER
#include "http-socket.h"
#define DL_URI_SCHEME "http://"
#endif

#ifdef COAP_DOWNLOADER
#include "coap-engine.h"
#include "coap-callback-api.h"
#define DL_URI_SCHEME "coap://"
#endif


/* Log configuration */
#include "coap-log.h"
#define LOG_MODULE "lwm2m-fw"
#define LOG_LEVEL  LOG_LEVEL_LWM2M

#define UPDATE_PACKAGE     0
#define UPDATE_PACKAGE_URI 1
#define UPDATE_UPDATE      2
#define UPDATE_STATE       3
#define UPDATE_RESULT      5

#define STATE_IDLE         1
#define STATE_DOWNLOADING  2
#define STATE_DOWNLOADED   3

#define RESULT_DEFAULT         0
#define RESULT_SUCCESS         1
#define RESULT_NO_STORAGE      2
#define RESULT_OUT_OF_MEM      3
#define RESULT_CONNECTION_LOST 4
#define RESULT_CRC_FAILED      5
#define RESULT_UNSUPPORTED_FW  6
#define RESULT_INVALID_URI     7


// #define PACKAGE_URI_LENGTH 128
static uint8_t state = STATE_IDLE;
static uint8_t result = RESULT_DEFAULT;
// static package_uri[PACKAGE_URI_LENGTH+1]

static lwm2m_object_instance_t reg_object;

static const lwm2m_resource_id_t resources[] =
  { WO(UPDATE_PACKAGE),
    WO(UPDATE_PACKAGE_URI),
    RO(UPDATE_STATE),
    RO(UPDATE_RESULT),
    EX(UPDATE_UPDATE)
  };

static int bytes_received = 0;
#ifdef HTTP_DOWNLOADER
  /*---------------------------------------------------------------------------*/
  static struct http_socket s;
  /*---------------------------------------------------------------------------*/
  static void
  http_callback(struct http_socket *s, void *ptr,
           http_socket_event_t e,
           const uint8_t *data, uint16_t datalen)
  {
    if(e == HTTP_SOCKET_ERR) {
      state = STATE_IDLE;
      printf("HTTP socket error\n");
    } else if(e == HTTP_SOCKET_TIMEDOUT) {
      state = STATE_IDLE;
      printf("HTTP socket error: timed out\n");
    } else if(e == HTTP_SOCKET_ABORTED) {
      state = STATE_IDLE;
      printf("HTTP socket error: aborted\n");
    } else if(e == HTTP_SOCKET_HOSTNAME_NOT_FOUND) {
      state = STATE_IDLE;
      printf("HTTP socket error: hostname not found\n");
    } else if(e == HTTP_SOCKET_CLOSED) {
      state = STATE_DOWNLOADED;
      printf("HTTP socket closed, %d bytes received\n", bytes_received);
    } else if(e == HTTP_SOCKET_DATA) {
      bytes_received += datalen;
      state = STATE_DOWNLOADING;
      result = (uint8_t)(100*bytes_received/s->header.content_length);
      printf("HTTP socket: segment: %d bytes, received: %d, content size: %d\n",
              datalen,
              bytes_received,
              (int) s->header.content_length);
    } else {
       printf("HTTP socket event ignored: %d\n", (int) e);
    }

    lwm2m_notify_object_observers(&reg_object, UPDATE_STATE);
    lwm2m_notify_object_observers(&reg_object, UPDATE_RESULT);
  }
#endif

#ifdef COAP_DOWNLOADER
  static coap_message_t request;
  static coap_endpoint_t server_ep;
  static coap_callback_request_state_t request_state;

  static void
  coap_callback(coap_callback_request_state_t *callback_state) {
    coap_request_state_t *cb_state = &callback_state->state;
    const uint8_t *chunk = NULL;
    int datalen=0;

    printf("Registration callback. Status: %d, Response: %d\n", cb_state->status, cb_state->response != NULL);
    if (cb_state->response != NULL) {
      datalen = coap_get_payload(cb_state->response, &chunk);
      bytes_received += datalen;
    }

    if(cb_state->status == COAP_REQUEST_STATUS_TIMEOUT) {
      state = STATE_IDLE;
      printf("Request timed out\n");
    } else if(cb_state->status == COAP_REQUEST_STATUS_RESPONSE) {
      printf("COAP response content size %d\n", bytes_received);
      state = STATE_DOWNLOADED;
    } else if(cb_state->status == COAP_REQUEST_STATUS_FINISHED) {
      printf("COAP - last chunk, content size %d\n", bytes_received);
      state = STATE_DOWNLOADED;
    } else if(cb_state->status == COAP_REQUEST_STATUS_MORE) {
      printf("COAP - chunk size: %d\n", datalen);
      state = STATE_DOWNLOADING;
    } else if(cb_state->status == COAP_REQUEST_STATUS_BLOCK_ERROR) {
      printf("COAP - block error\n");
      state = STATE_IDLE;
    } else {
      printf("COAP - unkown callback status\n");
    }

    lwm2m_notify_object_observers(&reg_object, UPDATE_STATE);
    lwm2m_notify_object_observers(&reg_object, UPDATE_RESULT);
  }
#endif

/*---------------------------------------------------------------------------*/

  static lwm2m_status_t
  lwm2m_dlfw(const char *dl_uri)
  {
    result = 0;
    state = STATE_DOWNLOADING;
    bytes_received = 0;
#ifdef HTTP_DOWNLOADER
    printf("HTTP init download: %s\n", dl_uri);
    http_socket_init(&s);
    http_socket_get(&s, dl_uri, 0, 0, http_callback, NULL);
    printf("HTTP socket downloading\n");
#endif
#ifdef COAP_DOWNLOADER
    coap_endpoint_parse(dl_uri, strlen(dl_uri), &server_ep);
    coap_init_message(&request, COAP_TYPE_CON, COAP_GET, 0);
    coap_set_header_uri_path(&request, "/");
    printf("COAP - init download: %s\n", dl_uri);
    LOG_INFO_COAP_EP(&server_ep);
    LOG_INFO_("\n");
    coap_send_request(&request_state, &server_ep, &request, coap_callback);
    printf("COAP - downloading\n");
#endif

    return LWM2M_STATUS_OK;
  }


/*---------------------------------------------------------------------------*/
static lwm2m_status_t
lwm2m_callback(lwm2m_object_instance_t *object,
               lwm2m_context_t *ctx)
{
  uint32_t num;
  uint8_t more;
  uint16_t size;
  uint32_t offset;

  LOG_DBG("Got request at: %d/%d/%d lv:%d\n", ctx->object_id,
          ctx->object_instance_id, ctx->resource_id, ctx->level);

  if(ctx->level == 1 || ctx->level == 2) {
    /* Should not happen - as it will be taken care of by the lwm2m engine itself. */
    return LWM2M_STATUS_ERROR;
  }

  if(ctx->operation == LWM2M_OP_READ) {
    switch(ctx->resource_id) {
    case UPDATE_STATE:
      lwm2m_object_write_int(ctx, state); /* 1 means idle */
      return LWM2M_STATUS_OK;
    case UPDATE_RESULT:
      lwm2m_object_write_int(ctx, result); /* 0 means default */
      return LWM2M_STATUS_OK;
    }
  } else if(ctx->operation == LWM2M_OP_WRITE) {

    if(LOG_DBG_ENABLED) {
      if(coap_get_header_block1(ctx->request, &num, &more, &size, &offset)) {
        LOG_DBG("CoAP BLOCK1: %"PRIu32"/%u/%u offset:%"PRIu32
                " LWM2M CTX->offset=%"PRIu32"\n",
                num, more, size, offset, ctx->offset);
      }
    }

    switch(ctx->resource_id) {
    case UPDATE_PACKAGE:
      /* The firmware is written */
      LOG_DBG("Firmware received: %"PRIu32" %d fin:%d\n", ctx->offset,
              (int)ctx->inbuf->size, lwm2m_object_is_final_incoming(ctx));
      if(lwm2m_object_is_final_incoming(ctx)) {
        state = STATE_DOWNLOADED;
      } else {
        state = STATE_DOWNLOADING;
      }
      return LWM2M_STATUS_OK;

    case UPDATE_PACKAGE_URI:
      /* The firmware URI is written */
      LOG_DBG("Firmware URI received: %"PRIu32" %d fin:%d\n", ctx->offset,
              (int)ctx->inbuf->size, lwm2m_object_is_final_incoming(ctx));
      int i;
      for(i = 0; i < ctx->inbuf->size; i++) {
        if (strncmp((const char *)&ctx->inbuf->buffer[i], DL_URI_SCHEME, strlen(DL_URI_SCHEME)) == 0) {
          break;
        }
      }
      return lwm2m_dlfw((const char *)&ctx->inbuf->buffer[i]);
    }
  } else if(ctx->operation == LWM2M_OP_EXECUTE && ctx->resource_id == UPDATE_UPDATE) {
    /* Perform the update operation */
    if(state == STATE_DOWNLOADED) {
      state = STATE_IDLE;
      bytes_received=0;
      result = 0;
      lwm2m_notify_object_observers(&reg_object, UPDATE_STATE);
            lwm2m_notify_object_observers(&reg_object, UPDATE_RESULT);
      return LWM2M_STATUS_OK;
    }
    /* Failure... */
  }
  return LWM2M_STATUS_ERROR;
}

/*---------------------------------------------------------------------------*/
void
lwm2m_firmware_init(void)
{
  reg_object.object_id = 5;
  reg_object.instance_id = 0;
  reg_object.callback = lwm2m_callback;
  reg_object.resource_ids = resources;
  reg_object.resource_count = sizeof(resources) / sizeof(lwm2m_resource_id_t);

  lwm2m_engine_add_object(&reg_object);
}
/*---------------------------------------------------------------------------*/
/** @} */
