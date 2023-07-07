/**
 * \addtogroup p2p
 * @{
 */
/**
 * \file
 *    This file implements queue for 'Peer to Peer' (p2p)
 *
 * \author
 *    Guru Mehar Rachaputi
 * 
 * \reviewer
 * 	  Anders Isberg
 */

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


// node_rank_t
// nnode_rank_set(uint32_t nnode_self_chunks){
	
// 	node_rank_t nnode_rank;
// 	if(nnode_self_chunks > 0 && nnode_self_chunks <= 0xff)
// 		nnode_rank = RANK_4;
// 	else if((nnode_self_chunks >= (0xff + 1))  && nnode_self_chunks <= 0xffff)
// 		nnode_rank = RANK_3;
// 	else if((nnode_self_chunks >= (0xffff + 1)) && nnode_self_chunks <= 0xfffffffe)
// 		nnode_rank = RANK_2;
// 	else if(nnode_self_chunks > 0xfffffffe)
// 		nnode_rank = RANK_1;
// 	else
// 		nnode_rank = RANK_0;

// 	return nnode_rank;
// }


node_rank_t
nnode_rank_set(uint32_t nnode_self_chunks){
	
	node_rank_t nnode_rank;
	if(nnode_self_chunks >= 0xfffffffe)
		nnode_rank = RANK_1;
	else if(nnode_self_chunks >= 0xfffffff0)
		nnode_rank = RANK_2;
	else if(nnode_self_chunks >= 0xffff)
		nnode_rank = RANK_3;
	else if(nnode_self_chunks > 0xff)
		nnode_rank = RANK_4;
	else
		nnode_rank = RANK_0;

	return nnode_rank;
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
void
queue_reset(void){

	rx_mpckts_t *q_rst;

	LOG_INFO("queue reset\n");
	while(!queue_is_empty(rx_queue)){

		q_rst = queue_dequeue(rx_queue);

		if(q_rst != NULL){
			heapmem_free(q_rst->data);
			heapmem_free(q_rst);
		}
	}
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

	LOG_INFO("Deque in Progress\n");

	rx_mpckts_t *dq;


	if (!queue_is_empty(rx_queue)) {
		LOG_INFO("Queue is not Empty\n");
		dq = queue_dequeue(rx_queue);
		if (dq == NULL) {
			PRINTF("Queue Underflow \n");
			return 0;
		} else {
			q_elem--;
			node_index = check_index(&dq->send_addr);
			LOG_INFO("Dequeued datalen: %u\n", dq->datalen);
			LOG_INFO("Dequeue data: \n");
			for (int i = 0; i < dq->datalen; i++) {
				PRINTF(" %u", dq->data[i]);
			}

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

			LOG_INFO("\ncontrol message received dequed:  %d\n", this->ctrl_msg);
			LOG_INFO("post data: %d\n", post_data.data[0]);
			LOG_INFO("data interest: %d\n", this->data[0]);

			// TODO: populate nbr node based on the ctrl_msg
			// nbr_list[node_index].node_addr = sender_addr;
			if (this->ctrl_msg == ACKHANDSHAKE_CTRL_MSG) {
				LOG_INFO("Acknowledgement received\n");

				/* check the nnode rank is greater than my_rank,
					-> if nnode_rank > my_rank then state = handshaked
					-> else set nnode_rank and state = idle
				 */
				nbr_list[node_index].nnode_rank = nnode_rank_set(this->chunk_type.self_chunks);
				// if(nbr_list[node_index].nnode_rank > my_rank) {
				nbr_list[node_index].nnode_state = HANDSHAKED_STATE;
				nbr_list[node_index].nnode_ctrlmsg = this->ctrl_msg;
				// }
				LOG_INFO("self chunks 0x%x\n", this->chunk_type.self_chunks);
				nbr_list[node_index].data_chunks = this->chunk_type.self_chunks;
			} else if (this->ctrl_msg == HANDSHAKE_CTRL_MSG) {
				LOG_INFO("Handshake received\n");
				upload_event_handler(HANDSHAKE_EVENT, &post_data);
			} else if (this->ctrl_msg == INTEREST_CTRL_MSG) {
				LOG_INFO("Interest received\n");
				upload_event_handler(INTEREST_EVENT, &post_data);
			} else if (this->ctrl_msg == UNCHOKE_CTRL_MSG) {
				LOG_INFO("Unchoke received\n");
				nbr_list[node_index].nnode_ctrlmsg = this->ctrl_msg;
				nbr_list[node_index].nnode_state = INTEREST_INFORMED_STATE;
			} else if (this->ctrl_msg == REQUEST_CTRL_MSG) {
				LOG_INFO("Request received\n");
				upload_event_handler(REQUEST_EVENT, &post_data);
			} else {
				nbr_list[node_index].nnode_ctrlmsg = this->ctrl_msg;
			}


			heapmem_free(dq->data);
			heapmem_free(dq);

			node_index = -1;
			LOG_INFO("\n");

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
	LOG_INFO("Enqueue in Progress\n");

	msg_pckt_t *this;
	rx_mpckts_t *rx_q;

	this = (msg_pckt_t *)data;
	LOG_INFO("self chunks: %d\n", this->chunk_type.self_chunks);
	LOG_INFO("ctrl msg: %d\n", this->ctrl_msg);
	// PRINTF("self chunks: %d", this->self_chunks);

	rx_q = (rx_mpckts_t *)heapmem_alloc(sizeof(rx_mpckts_t));

	if (rx_q == NULL) {
		LOG_INFO("Failed to Allocate Memory\n");
		return;
	}

	rx_q->next = NULL;

	uip_ipaddr_copy(&rx_q->send_addr, sender_addr);

	rx_q->data = (uint8_t *)heapmem_alloc(dlen * sizeof(uint8_t));

	if (rx_q->data == NULL) {
		if(rx_q != NULL){
			heapmem_free(rx_q);
		}
		LOG_INFO("Failed to Allocate Memory\n");
		return;
	}

	memcpy(rx_q->data, data, dlen);

	rx_q->datalen = dlen;

	LOG_INFO("Data received from: ");
	LOG_INFO_6ADDR(sender_addr);
	LOG_INFO("\n");

	LOG_INFO("Printing from Queue Created: \n");

	msg_pckt_t *rq;
	rq = (msg_pckt_t *) rx_q->data;

	LOG_INFO("self chunks: %x\n", rq->chunk_type.self_chunks);
	LOG_INFO("ctrl msg enq: %d\n", rq->ctrl_msg);
	LOG_INFO("data msg enq: %d\n", rq->data[0]);

	// for (int i = 0; i < rx_q->datalen; i++) {
	// 	PRINTF(" %u", rx_q->data[i]);
	// }
	PRINTF("\n");
	LOG_INFO("\nPrinting from Queue Created Complete \n");

	if (q_elem >= QUEUE_SIZE /*|| !queue_is_empty()*/) {
		LOG_INFO(">>>>>>>> Queue is Full >>>>>>>>\n");
		// uni_queue_deq();
		return;
	} else if (q_elem >= 0 && q_elem < QUEUE_SIZE) {
		queue_enqueue(rx_queue, rx_q);
		q_elem++;
	}
	LOG_INFO("Queue is%s Empty\n",
	       queue_is_empty(rx_queue) ? "" : " not");


	LOG_INFO("Queue Peek datalen: %u\n", ((rx_mpckts_t *)queue_peek(rx_queue))->datalen);

	LOG_INFO("Enqueue is Complete\n");

	// uni_queue_deq();
	// process_start(&queue_proc, NULL);

}


















































































