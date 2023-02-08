

#ifndef P2P_H_
#define P2P_H_


#include "contiki.h"

#include <stdint.h>

/*---------------------------------------------------------------------------*/
/* Configuration */
/*---------------------------------------------------------------------------*/

// #define BYTES_1024 //BYTES_1024
// #define BYTES_4000 //BYTES_4000
#define BYTES_4000 //BYTES_8000

#ifdef BYTES_1024
#include "data_1024B.h"
#endif

#ifdef BYTES_4000
#include "data_4000B.h"
#endif

#ifdef BYTES_8000
#include "data_8000B.h"
#endif


#define NODES_MAX 			4 				// M is the max number of neighbor nodes a node 
// can comm at a time
#define NODES_MAX_DL_UL		2 // max uploading nodes = 2 and max downloading nodes = 2

#define NUM_OF_NEIGHBORS	NODES_MAX // num of nodes to comm at at time from the ref paper
#define NEIGHBORS_LIST		10 // num of neighbors in the list
#define NODES_UPLOAD		2 // in the leecher mode
#define NODES_DOWNLOAD		2 // in the leecher mode


#define TOTAL_DATA_S		(strlen(seq_idd))	// 4kb of data
#define NUM_CHUNKS_X		32					// constant number of chunks
#define NUM_OF_BLOCKS		4 					// each chunk split into blocks of 4


#define DATA_IN_BYTES 		TOTAL_DATA_S
#define DATA_TOTAL_CHUNKS	NUM_CHUNKS_X
#define DATA_CHUNK_SIZE		( TOTAL_DATA_S / NUM_CHUNKS_X )
#define DATA_CHUNK_ONE		( DATA_CHUNK_SIZE / NUM_OF_BLOCKS )


#define MAX_PAYLOAD_LEN DATA_CHUNK_ONE	// block or 32 bytes of payload in each packet


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
	// INTEREST_INFORMING_C_STATE,
	// INTEREST_INFORMING_UC_STATE,
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
	uip_ipaddr_t node_addr;			// neighbor address
	comm_states_t nnode_state;			// neighbor node state
	interest_state_t nnode_interest;	// neighbor node interest state
	choke_state_t nnode_choke;			// neighbor node choke state
	uint8_t data_chunks[DATA_TOTAL_CHUNKS];				// neighbor nodes data chunks
	// ctrl_msg_t nnode_interest;
	// ctrl_msg_t nnode_choke;
	uint8_t num_upload;
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
bit 7 -> DATAPACKET
*/

// message packet
typedef struct {
	uint32_t self_chunks;	// data chunks a node has
	uint8_t ctrl_msg;		//
	uint8_t data[32];		// 32 bytes in a block
} msg_pckt_t;

/*--------------------------------------------*/


static bool chunk_cnt[DATA_TOTAL_CHUNKS] = {0};
static nnode_state_t *nbr_list[NEIGHBORS_LIST];


msg_pckt_t *prepare_handshake(void);
msg_pckt_t *prepare_request(void);
void nnode_init(int node_i);


void node_handshake(void);		// set HANDSHAKING_STATE with that neighbor
void node_ack_handshake(void);	// set HANDSHAKED_STATE with that neighbor
void node_interest(void);		// set INTEREST_INFORMING with that neighbor
void node_choke_wait();			// wait for 5 seconds
void node_choke_unchoke(void);	// refer point 2
void node_request(void);			// set DOWNLOADING_STATE with that neighbor
void node_upload(void);			// set UPLOADING_STATE with that neighbor

/*------------------------------------------------------------------*/


#endif //P2P_H_






























