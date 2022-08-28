


#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H


typedef enum {INVALID = 0, MULTICAST, REQUEST, RESPONSE} packet_type;


struct payload {
	uint16_t 	seqnum;	//seq number of the packet
	/*
		packettype:
		0: INVALID
		1: MULTICAST for data dissemination
		2: REQUEST for packet
		3: RESPONSE for requested packet
	*/
	uint8_t 	packettype;
	uint8_t		numchunks;
	char		data[MAX];
};

/*
struct recovery {
	uint16_t	seqnum;
	uint8_t 	packettype;
}
*/






#endif


client:
recv:
-> reset timer	//optional
-> check seqnum
-> if the last seqnum is recv or if all the arr[seqnum] are true
		-> break
		-> else timer starts	//optional
			-> when timer ends callback //optional

callback:
			-> request for missing seqnum from arr[seqnum] which are false
				-> unicast reqeust server those missing packets








































