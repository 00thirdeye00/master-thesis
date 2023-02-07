

#ifndef P2P_H_
#define P2P_H_


#include "contiki.h"

#include <stdint.h>

/*---------------------------------------------------------------------------*/
/* Configuration */
/*---------------------------------------------------------------------------*/

#define NODES_MAX 			4 				// M is the max number of neighbor nodes a node 
// can comm at a time
#define NODES_MAX_DL_UL		2 // max uploading nodes = 2 and max downloading nodes = 2

#define NUM_OF_NEIGHBORS	NODES_MAX // num of nodes to comm at at time from the ref paper
#define NEIGHBORS_LIST		10 // num of neighbors in the list
#define NODES_UPLOAD		2 // in the leecher mode
#define NODES_DOWNLOAD		2 // in the leecher mode


#define TOTAL_DATA_S		4096	// 4kb of data
#define NUM_CHUNKS_X		32		// constant number of chunks
#define NUM_OF_BLOCKS		4 		// each chunk split into blocks of 4


#define DATA_IN_BYTES 		TOTAL_DATA_S
#define DATA_TOTAL_CHUNKS	NUM_CHUNKS_X
#define DATA_CHUNK_SIZE		( TOTAL_DATA_S / NUM_CHUNKS_X )
#define BLOCKS_OF_CHUNK		( DATA_CHUNK_SIZE / NUM_OF_BLOCKS )

/*--------------------------------------------*/

// node state
typedef enum {
	NONE,
	IDLE,		// node state at startup
	LEECHER, 	// node doesn't have all pieces
	SEEDER,		// node has all pieces
} node_state_t;

// node download state
typedef enum {
	NONE,
	DOWNLOADING_PROGRESS,
	DOWNLOADING_COMPLETE,
	LAST_DOWNLOADING_STATE,
} node_dlstate_t;

// control message for communication
typedef enum {
	NONE_CTRL_MSG,
	HANDSHAKE_CTRL_MSG,
	ACKHANDSHAKE_CTRL_MSG,
	INTEREST_CTRL_MSG,
	CHOKE_CTRL_MSG,
	UNCHOKE_CTRL_MSG,
	REQUEST_CTRL_MSG,
	LAST_CTRL_MSG,
} ctrl_msg_t;


// communication state of a node
typedef enum {
	IDLE_STATE,
	HANDSHAKING_STATE,
	HANDSHAKED_STATE,
	INTEREST_INFORMING_STATE,
	INTEREST_INFORMING_C_STATE,
	INTEREST_INFORMING_UC_STATE,
	INTEREST_INFORMED_STATE,
	DOWNLOADING_STATE,
	UPLOADING_STATE,
	LAST_COMM_STATE,
} comm_states_t;

// interest state of a node
typedef enum {
	NONE,
	INTEREST_FALSE,
	INTEREST_TRUE,
} interest_state_t;

// choke or unchoke state of a node
typedef enum {
	NONE,
	UNCHOKE,
	CHOKE,
} choke_state_t;


// struct for each nbr node
typedef struct {
	uip_ds6_nbr_t *node_addr;			// neighbor address
	comm_states_t nnode_state;			// neighbor node state
	interest_state_t nnode_interest;	// neighbor node interest state
	choke_state_t nnode_choke;			// neighbor node choke state
	uint8_t *data_chunks;				// neighbor nodes data chunks
	// ctrl_msg_t nnode_interest;
	// ctrl_msg_t nnode_choke;
	uint8 numUL;
} nnode_state_t;

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
bit 7 -> UNDEFINED
*/

// message packet
typedef struct {
	uint8 ctrl_msg;	//
	uint8 *data;	// 32 bytes in a block
} msg_pckt_t;

/*--------------------------------------------*/


#endif
































