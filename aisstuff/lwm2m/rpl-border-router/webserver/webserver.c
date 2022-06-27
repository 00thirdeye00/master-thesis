/*
 * Copyright (c) 2017, RISE SICS
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
 *
 */

#include "contiki.h"
#include "sys/log.h"
#include "net/routing/routing.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/ipv6/uip-sr.h"

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
static const char *data1kb = "0123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789"
                             "1123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789"
                             "2123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789"
                             "3123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789"
                             "4123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789"
                             "5123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789"
                             "6123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789"
                             "7123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789"
                             "8123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789"
                             "9123456789112345678921234567893123456789412345678951234567896123456789712345678981234567899123456789";

const char http_content_length_1k[] = "Content-Length: 1000\r\n";
const char http_content_length_100k[] = "Content-Length: 100000\r\n";
const char http_end_of_headers[]= "\r\n";

/* Use simple webserver with only one page for minimum footprint.
 * Multiple connections can result in interleaved tcp segments since
 * a single static buffer is used for all segments.
 */
#include "httpd-simple.h"

static
PT_THREAD(generate_content1k(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
  SEND_STRING(&s->sout, http_content_length_1k);
  SEND_STRING(&s->sout, http_end_of_headers);
  SEND_STRING(&s->sout, data1kb);
  PSOCK_END(&s->sout);
}

static
PT_THREAD(generate_content100k(struct httpd_state *s))
{
  static uint8_t i;
  PSOCK_BEGIN(&s->sout);
  SEND_STRING(&s->sout, http_content_length_100k);
  SEND_STRING(&s->sout, http_end_of_headers);
  for (i=0; i<100; i++) {
    SEND_STRING(&s->sout, data1kb);
  }
  PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
PROCESS(webserver_nogui_process, "Web server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  if (strncmp(name, "100k", WEBSERVER_CONF_CFS_PATHLEN) == 0){
    return generate_content100k;
  }

  return generate_content1k;
}
/*---------------------------------------------------------------------------*/
