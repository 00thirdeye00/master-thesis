#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
// #include "sys/process.h"

#include "queue.h"
#include "rxqueue.h"

#include "sys/log.h"
#define LOG_MODULE "queue"
#define LOG_LEVEL LOG_LEVEL_INFO


static uint8_t q_elem;

// static rx_mpckts_t *rx_q;


QUEUE(rx_queue);

/*------------------------------------------------------------------*/
/**
 * brief: to check if the queue is empty
 *
 * params: void
 *
 * return: bool
 *
 *
 */

bool
is_queue_empty(void) {
	return q_elem > 0 ? false : true;
}


/*------------------------------------------------------------------*/
/**
 * brief: dequeue element in the queue from rx callback
 *
 * params: void
 *
 * return: uint8_t
 *
 *
 */

uint8_t
queue_deq(void) {

	// static uint8_t recv_block_count = 0;
	static int8_t node_index = -1;

	PRINTF("Deque in Progress\n");

	rx_mpckts_t *dq;


	if (!queue_is_empty(rx_queue)) {
		PRINTF("Queue is not Empty\n");
		dq = queue_dequeue(rx_queue);
		// if(q_elem > 0) q_elem--;
		if (dq == NULL) {
			PRINTF("Queue Underflow \n");
			return 0;
		} else {
			q_elem--;
			node_index = check_index(&dq->send_addr);
			PRINTF("Dequeued datalen: %u\n", dq->datalen);
			PRINTF("Dequeue data: ");
			// for (int i = 0; i < dq->datalen; i++) {
			// 	PRINTF(" %u", dq->data[i]);
			// 	unicast_send(dq->data[i], dq->send_addr);
			// 	// PRINTF("")
			// }

			msg_pckt_t *this;

			// These are not safe constructions. I started to change them but you need to refactory the code here.
			// sender_addr and data are only available in the callback. In addition
			// if you post the data you need to have memory where you store data post_data is lost
			// once you leave the callback.
			process_post_data_t post_data;
			post_data.sender_addr = dq->send_addr;
			post_data.data = (uint8_t *)dq->data;


			this = (msg_pckt_t *)dq->data;

			// LOG_INFO("control message: %d\n", dq->ctrl_msg);


			// TODO: populate nbr node based on the ctrl_msg
			// nbr_list[node_index].node_addr = sender_addr;
			if (this->ctrl_msg == ACKHANDSHAKE_CTRL_MSG) {
				LOG_INFO("Acknowledgement received\n");
				nbr_list[node_index].nnode_ctrlmsg = this->ctrl_msg;
				nbr_list[node_index].data_chunks = this->chunk_type.self_chunks;
			}  else if (this->ctrl_msg == HANDSHAKE_CTRL_MSG) {
				LOG_INFO("Handshake received\n");
				// process_post(&node_comm_process, HANDSHAKE_EVENT, &post_data);
				upload_event_handler(HANDSHAKE_EVENT, &post_data);
			} else if (this->ctrl_msg == INTEREST_CTRL_MSG) {
				LOG_INFO("Interest received\n");
				// process_post(&node_comm_process, INTEREST_EVENT, &post_data);
				upload_event_handler(INTEREST_EVENT, &post_data);
			} else if (this->ctrl_msg == REQUEST_CTRL_MSG) {
				LOG_INFO("Request received\n");
				// process_post(&node_comm_process, REQUEST_EVENT, &post_data);
				upload_event_handler(REQUEST_EVENT, &post_data);
			}
			// else if (this->ctrl_msg == LAST_CTRL_MSG) {

			// 	// print data since it is uint8_t
			// 	LOG_INFO("Received response '%.*s' from ", datalen, (char *) this->data[0]);
			// 	recv_block_count++;

			// 	if (recv_block_count >= 4)
			// 		recv_block_count = 0;
			// }
			else {
				nbr_list[node_index].nnode_ctrlmsg = this->ctrl_msg;
			}


			heapmem_free(dq->data);
			heapmem_free(dq);

			PRINTF("\n");

			return 1;

		}
	}
	// PRINTF("Deque is Complete\n");
	return 0;
}

/*------------------------------------------------------------------*/
/**
 * brief: enqueue element in the queue from rx callback
 *
 * params: sender address, data length, data
 *
 * return: void
 *
 *
 */

void
queue_enq(const uip_ipaddr_t *sender_addr, uint16_t dlen, const uint8_t *data) {
	PRINTF("Enqueue in Progress\n");

	msg_pckt_t *this;
	this = (msg_pckt_t *)data;

	PRINTF("self chunks: %d\n", this->chunk_type.self_chunks);
	PRINTF("ctrl msg: %d\n", this->ctrl_msg);
	// PRINTF("self chunks: %d", this->self_chunks);


	rx_mpckts_t *rx_q;

	rx_q = (rx_mpckts_t *)heapmem_alloc(sizeof(rx_mpckts_t));

	if (rx_q == NULL) {
		PRINTF("Failed to Allocate Memory\n");
		// return 0;
	}

	rx_q->next = NULL;

	uip_ipaddr_copy(&rx_q->send_addr, sender_addr);

	rx_q->data = (uint8_t *)heapmem_alloc(dlen * sizeof(uint8_t));

	memcpy(rx_q->data, data, dlen);

	rx_q->datalen = dlen;

	PRINTF("Data sent from: ");
	LOG_INFO_6ADDR(sender_addr);
	PRINTF("\n");

	PRINTF("Printing from Queue Created: \n");
	for (int i = 0; i < rx_q->datalen; i++) {
		PRINTF(" %u", rx_q->data[i]);
	}
	PRINTF("\nPrinting from Queue Created Complete \n");

	if (q_elem >= QUEUE_SIZE /*|| !queue_is_empty()*/) {
		PRINTF(">>>>>>>> Queue is Full >>>>>>>>\n");
		// uni_queue_deq();
		return;
	} else if (q_elem >= 0 && q_elem < QUEUE_SIZE) {
		queue_enqueue(rx_queue, rx_q);
		q_elem++;
	}
	PRINTF("Queue is%s Empty\n",
	       queue_is_empty(rx_queue) ? "" : " not");


	PRINTF("Queue Peek datalen: %u\n", ((rx_mpckts_t *)queue_peek(rx_queue))->datalen);

	PRINTF("Enqueue is Complete\n");

	// uni_queue_deq();
	// process_start(&queue_proc, NULL);

}


















































































