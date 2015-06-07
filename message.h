typedef struct
{
	char* bitfield; //bitfield where each bit represents a piece that
						//the peer has or doesn’t have
	size_t size;    //size of the bitfiled
} bt_bitfield_t;

typedef struct
{
	int index; //which piece index	
	int begin; // offset within piece
	int length; //amount wanted ,within a power of two
} bt_request_t;
typedef struct
{
	int index; //which piece index
	int begin; //offset within piece
	char piece[0]; //pointer to start of the data for apiece
} bt_piece_t;

typedef struct bt_msg{
	int length; 	//prefix length, size of remaining message
					// 0 length message is a keep-alive message
	unsigned char bt_type; //type of bt_mesage
	
	//payload can be any ofthese
	union{	
		bt_bitfield_t bitfiled;	//send a bitfield
		int have;    // index of piece just completed
		bt_piece_t piece; //a piece message
		bt_request_t request; //requxest messge
		bt_request_t cancel; //cancel message,same type as request
		char data[0];// pointer to start of payload, just for convenience
	}payload;
} bt_msg_t;