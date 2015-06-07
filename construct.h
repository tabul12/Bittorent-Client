#ifndef CONSTRUCT_H_  /* Include guard */
#define CONSTRUCT_H_

#include <inttypes.h>
#include "tracker.h"
typedef struct
{
	long int length;
	int path_size;
	char** path;
}file_t;

typedef struct 
{
	int length;
	long int piece_length;
	int num_pieces;
	char* name;
	char** pieces;
	int num_files;
	file_t* files;
}info_t;

typedef struct 
{
	char* announce;
	char* created_by;	
	char* info_as_string;
	int info_len;
	uint64_t creation_date;
	info_t* info;
}meta_info_t;


typedef struct
{
	uint32_t ip;
	uint16_t port;
}peer_t; 
typedef struct
{
	char* faiure_reason;
	int interval;
	int complete;
	int incomplete;
	int num_peers;
	peer_t* peers;
}tracker_response_t;

meta_info_t* construct_metainfo(char* file_path);
char* construct_valid_curl(meta_info_t* inf,
	char* peer_id,int port, long int uploaded,
	long int downloaded,long int left);

tracker_response_t *construct_tracker_response(resp_t* response_content);
#endif