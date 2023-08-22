/**
 * \addtogroup p2p
 * @{
 */
/**
 * \file
 *    		This file implements 'Peer to Peer' (p2p)
 *
 * \author
 *    		Guru Mehar Rachaputi
 * 
 * \reviewer
 * 	  		Anders Isberg
 * 
 */

#ifndef P2P_H_
#define P2P_H_


#include "contiki.h"

#include <stdint.h>
#include "sys/ctimer.h"

/*---------------------------------------------------------------------------*/
/* Configuration */
/*---------------------------------------------------------------------------*/

// // #define BYTES_1024 //BYTES_1024
// // #define BYTES_4000 //BYTES_4000
// #define BYTES_4096	//BYTES_4096
// // #define BYTES_4000 //BYTES_8000

// #ifdef BYTES_1024
// #include "data_1024B.h"
// #endif

// #ifdef BYTES_4000
// #include "data_4000B.h"
// #endif

// #ifdef BYTES_4096
// #include "data_4096B.h"
// #endif

// #ifdef BYTES_8000
// #include "data_8000B.h"
// #endif


#define NODES_MAX_NBRS 		10 // M is the max number of neighbor nodes a node can have
#define NODES_MAX_COMM		4 // num of nodes to comm at at time from the ref paper
// can comm at a time
#define NODES_MAX_DL_UL		2 // max uploading nodes = 2 and max downloading nodes = 2

// #define NUM_OF_NEIGHBORS	NODES_MAX // num of nodes to comm at at time from the ref paper
#define NEIGHBORS_LIST		NODES_MAX_NBRS // num of neighbors in the list
#define NODES_UPLOAD		(NODES_MAX_COMM / 2) // in the leecher mode
#define NODES_DOWNLOAD		(NODES_MAX_COMM / 2) // in the leecher mode


#define TOTAL_DATA_S		(sizeof(seq_idd))	// 4kb of data
#define NUM_CHUNKS_X		32					// constant number of chunks
#define NUM_OF_BLOCKS		4 					// each chunk split into blocks of 4


#define DATA_IN_BYTES 		TOTAL_DATA_S
#define DATA_TOTAL_CHUNKS	NUM_CHUNKS_X
#define DATA_CHUNK_SIZE		(DATA_IN_BYTES / DATA_TOTAL_CHUNKS)
#define DATA_CHUNK_ONE		(DATA_CHUNK_SIZE / NUM_OF_BLOCKS)


#define MAX_PAYLOAD_LEN 	DATA_CHUNK_ONE	// block or 32 bytes of payload in each packet
// #define MAX_PAYLOAD_LEN		32	/* for testing */ //block or 32 bytes of payload in each packet


#define NUM_OF_NODES		16 	// no. of nodes in network

#define PROCESS_WAIT_TIME_DEFAULT			2	// set 30 minutes timer for process
#define PROCESS_WAIT_TIME_NO_CHUNKS 		60	// wait 30 minutes if the nbr has no chunks
#define PROCESS_WAIT_TIME_NBR_RANK_LOW 		(6 * PROCESS_WAIT_TIME_NO_CHUNKS) // if all nbr's rank less/equal to my_rank
#define PROCESS_WAIT_TIME_PARTIAL_CHUNKS 	PROCESS_WAIT_TIME_DEFAULT	// wait 15 minutes if the nbr has some chunks
#define PROCESS_WAIT_TIME_SEEDER			200 // set 200 minutes timer for process when its seeder			

typedef enum{
	RANK_0 = 0,
	RANK_1 = 4, // highest rank
	RANK_2 = 3,
	RANK_3 = 2,
	RANK_4 = 1,	// lowest rank
} node_rank_t;



/*--------------------------------------------*/

// node state
typedef enum {
	MODE_NONE,
	MODE_IDLE,		// node state at startup
	MODE_LEECHER, 	// node doesn't have all pieces
	MODE_SEEDER,	// node has all pieces
} system_mode_t;

// node download state
typedef enum {
	DOWNLOADING_NONE,
	DOWNLOADING_PROGRESS,
	DOWNLOADING_COMPLETE,
	LAST_DOWNLOADING_STATE,
} node_dlstate_t;

// control message for communication
typedef enum {
	NONE_CTRL_MSG = 0,
	HANDSHAKE_CTRL_MSG = 1,
	ACKHANDSHAKE_CTRL_MSG = 2,
	INTEREST_CTRL_MSG = 3,
	CHOKE_CTRL_MSG = 4,
	UNCHOKE_CTRL_MSG = 5,
	REQUEST_CTRL_MSG =6,
	LAST_CTRL_MSG = 7,
} ctrl_msg_t;


// communication state of a node
typedef enum {
	IDLE_STATE = 0,
	HANDSHAKING_STATE = 1,
	HANDSHAKED_STATE = 2,
	INTEREST_INFORMING_STATE = 3,
	// INTEREST_INFORMING_C_STATE,
	// INTEREST_INFORMING_UC_STATE,
	INTEREST_INFORMED_STATE = 4,
	DOWNLOADING_STATE = 5,
	UPLOADING_STATE = 6,
	LAST_COMM_STATE = 7,
} comm_states_t;

// interest state of a node
typedef enum {
	INTEREST_NONE,
	INTEREST_FALSE,
	INTEREST_TRUE,
} interest_state_t;

// choke or unchoke state of a node
typedef enum {
	CHOKE_NONE,
	CHOKE_FALSE,
	CHOKE_TRUE,
} choke_state_t;

typedef enum {
	WAIT_END,
	WAIT_START,
} wait_state_t;


#pragma pack(push, 1)

typedef struct {
	uip_ipaddr_t nnode_addr;			// neighbor address
	comm_states_t nnode_state;			// neighbor node state
	ctrl_msg_t nnode_ctrlmsg;			// neighbor node ctrl msg received
	interest_state_t nnode_interest;	// neighbor node interest state
	choke_state_t nnode_choke;			// neighbor node choke state
	struct ctimer c_timer;				// ctimer for this node
	node_rank_t nnode_rank;				// nbr node rank
	uint32_t data_chunks;				// neighbor nodes data chunks
	uint8_t chunk_requested;			// chunk requested to nbr
	uint8_t	chunk_block;				// block number of the chunk
	uint8_t failed_dlreq;				// failed download request attemp
	uint8_t chunk_interested;			// nbr is interested in chunk
	// ctrl_msg_t nnode_interest;
	// ctrl_msg_t nnode_choke;
	uint8_t num_upload;					// number of blocks upload
} nnode_state_t;

// See comment in udp callback
typedef struct  {
	uip_ipaddr_t sender_addr;
	uint8_t *data;
} process_post_data_t;

/*--------------------------------------------*/

//ctrl and data msg frame

/*
//ctrl_state below
bit 0 -> HANDSHAKE
bit 1 -> ACKHANDSHAKE
bit 2 -> INTEREST
bit 3 -> CHOKE
bit 4 -> UNCHOKE
bit 5 -> REQUEST
bit 6 -> UNDEFINED
bit 7 -> DATAPACKET
*/

// message packet
typedef struct {
	union {
		uint32_t self_chunks;	// data chunks a node has
		// uint8_t	req_chunk;	// requested chunk
		// uint8_t	req_cblock;	// requested chunks block number
		uint16_t req_chunk_block;	// requested chunk and block number
	} chunk_type;
	uint8_t ctrl_msg;		// control message
	uint8_t data[32];		// 32 bytes in a block
} msg_pckt_t;

#pragma pack(pop)

/* state machine handler function pointer */
typedef void (*downloading_state_handler)(const uip_ipaddr_t *n_addr, const uint8_t node_idx);
typedef void (*uploading_state_handler)(const uip_ipaddr_t *n_addr, const uint8_t node_idx);

/* state machine - download */
typedef struct {
	comm_states_t curr_state;					// current state
	ctrl_msg_t ctrl_msg;						// control message
	downloading_state_handler sm_handler_dl;	// handler function sm_handler_dl returns next state
} state_machine_download;

/* state machine - upload */
typedef struct {
	ctrl_msg_t ctrl_msg;
	uploading_state_handler sm_handler_dl;
} state_machine_upload;

/* p2p_socket defined in p2pnode.c */
extern struct simple_udp_connection p2p_socket;

/*--------------------------------------------*/

extern uint8_t my_rank;			// my rank
extern uint8_t node_upload_nbr;	// to keep track of neighbors this node uploading to
extern uint8_t node_download_nbr; // to keep track of neighbors this node downloading from	

extern bool chunk_cnt[DATA_TOTAL_CHUNKS]; // total number of chunks this node contains
extern bool missing_chunk_req[DATA_TOTAL_CHUNKS]; // number of chunks requested

extern nnode_state_t nbr_list[NEIGHBORS_LIST]; // total number of neighbors for this node


/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/**
 * brief: function to prepare handshake
 *
 * params: pointer to message packet, control message
 *
 * return: void
 *
 */
void prepare_handshake(msg_pckt_t *d_pckt, ctrl_msg_t hs_ack_hs);

/*------------------------------------------------------------------*/
/**
 * brief: function to prepare interest
 *
 * params: pointer to message packet, chunk number
 * 			
 * return: void
 *
 */
void prepare_interest(msg_pckt_t *d_pckt, const uint8_t chunk);

/**
 * brief: function to prepare request
 *
 * params: pointer to message packet
 *
 * return: void
 *
 */
void prepare_request(msg_pckt_t *d_pckt);

/**
 * brief: function to check if the neighbor exists
 *
 * params: pointer to neighbor address
 *
 * return: 1 if neighbor exists, 0 if neighbor doesn't exists
 *
 */
uint8_t check_nbr_exist(const uip_ipaddr_t *nbr_addr);

/**
 * brief: function to return missing random chunk
 *
 * params: void
 *
 * return: chunk number
 *
 */
uint8_t missing_random_chunk(void);

/**
 * brief: function to initialize neighbor nodes
 *
 * params: neighbor number
 *
 * return: void
 *
 */
void nnode_init(int node_i);

/**
 * brief: function to reset to initial setting of neighbor node
 *
 * params: neighbor number
 *
 * return: void
 *
 */
// void nnode_reset_init(uint8_t node_i);

/**
 * brief: function to reset neighbor nodes
 *
 * params: void
 *
 * return: void
 *
 */
void nnode_reset_all(void);


/**
 * brief: function to reset neighbor node
 *
 * params: nbr_idx
 *
 * return: void
 *
 */
void nnode_reset_lowrank(uint8_t nbr_idx);


/**
 * brief: check node index in the receive callback
 *
 * params: pointer to neighbor address
 *
 * return: neighbor index
 *
 */
int8_t check_index(const uip_ipaddr_t *n_addr);

/**
 * brief: change state based on the current situation
 *
 * params: void
 *
 * return: void
 *
 */
void node_statechange(void);

/**
 * brief: function to initiate handshake
 *
 * params: pointer to neighbor address, neighbor index
 *
 * return: void
 *
 */
void node_handshake(const uip_ipaddr_t *n_addr, const uint8_t node_idx);		// set HANDSHAKING_STATE with that neighbor

/**
 * brief: function to acknowledge received handshake
 *
 * params: pointer to address the handshake received from
 *
 * return: void
 *
 */
void node_ack_handshake(const uip_ipaddr_t *sender_addr);	// set HANDSHAKED_STATE with that neighbor

/**
 * brief: function to initialize interest
 *
 * params: pointer to neighbor address, neighbor index
 *
 * return: void
 *
 */
void node_interest(const uip_ipaddr_t *n_addr, const uint8_t node_idx);		// set INTEREST_INFORMING with that neighbor
// void node_choke_wait(void);			// wait for 5 seconds

/**
 * brief: function to handle choke/unchoke
 *
 * params: pointer to address choke/unchoke received from
 *
 * return: choke_state_t
 *
 */
choke_state_t node_choke_unchoke(const uip_ipaddr_t *sender_addr);	// refer point 2

/**
 * brief: function to request for chunk(ready to download and set to download state)
 *
 * params: pointer to address choke/unchoke received from
 *
 * return: void
 *
 */
void node_request(const uip_ipaddr_t *n_addr, const uint8_t node_idx);			// set DOWNLOADING_STATE with that neighbor

/**
 * brief: function to check if the chunk is received
 *
 * params: pointer to neighbor address, neighbor index
 *
 * return: void
 *
 */
void node_received(const uip_ipaddr_t *n_addr, const uint8_t n_idx);		//

/**
 * brief: function to upload chunk
 *
 * params: chunk, pointer to address of node
 *
 * return: void
 *
 */
void node_upload(const uint8_t chunk, const uip_ipaddr_t *sender_addr);			// set UPLOADING_STATE with that neighbor

// /**
//  * brief: function to check and node rank and return node rank
//  *
//  * params: void
//  *
//  * return: bool
//  *
//  */
// node_rank_t node_rank_check(void);

/**
 * brief: function to return node rank
 *
 * params: void
 *
 * return: bool
 *
 */
node_rank_t node_my_rank(void);

/**
 * brief: function to check if all the chunks are received
 *
 * params: void
 *
 * return: bool
 *
 */
bool node_chunk_check(void);	//


// uip_ipaddr_t *nbr_list_nnode_addr(uint8_t node_idx);

/**
 * brief: function to print neighbor list with its states
 *
 * params: void
 *
 * return: void
 *
 */
void nbr_list_print(void);

/**
 * brief: function to switch between system modes
 *
 * params: system mode
 *
 * return: system mode
 *
 */
system_mode_t system_mode_pp(system_mode_t sys_mode);




/*------------------------------------------------------------------*/


#endif //P2P_H_






























