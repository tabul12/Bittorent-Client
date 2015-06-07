#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <openssl/sha.h>
#include <curl/curl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "bencode.h"
#include "construct.h"
#include "tracker.h"
#define HASH_SIZE 20
#define CURL_MAX_SIZE 1024

//announce
void announce(meta_info_t* ret,bencode_t* temp)
{
	if(bencode_is_string(temp))
	{
		int * len=malloc(sizeof(int));
		const char** buf=malloc(sizeof(char*));
		bencode_string_value(temp,buf,len);
		char* val=malloc(*len+1);
		memset(val,0,*len+1);
		val=memcpy(val,*buf,*len);
		ret->announce=val;
		free(buf);
		free(len);
	}
}
//created by
void created_by(meta_info_t* ret,bencode_t* temp)
{
	if(bencode_is_string(temp))
	{
		int * len=malloc(sizeof(int));
		const char** buf=malloc(sizeof(char*));
		bencode_string_value(temp,buf,len);
		char* val=malloc(*len+1);
		memset(val,0,*len+1);
		val=memcpy(val,*buf,*len);
		ret->created_by=val;

		free(buf);
		free(len);
	}
}
//creation date
void creation_date(meta_info_t* ret,bencode_t* temp)
{
	long int *val=malloc(sizeof(long int));
	bencode_int_value(temp,val);
	ret->creation_date=*val;
		free(val);
}
//tu length shemxvda
void length(info_t* info,bencode_t* temp)
{
	long int *val=malloc(sizeof(long int));
	bencode_int_value(temp,val);
	info->length=*val;

	free(val);
}
//tu name shemxvda bencodeshi
void name(info_t* info,bencode_t* temp)
{
	if(bencode_is_string(temp))
	{
		int * len=malloc(sizeof(int));
		const char** buf=malloc(sizeof(char*));
		bencode_string_value(temp,buf,len);
		char* val=malloc(*len+1);
		memset(val,0,*len+1);
		val=memcpy(val,*buf,*len);
		info->name=val;

		free(buf);
		free(len);
	}
}
//abrunebs listis sigrzes
int get_list_len(bencode_t* be)
{
	bencode_t* t=malloc(sizeof(bencode_t));
	bencode_clone(be,t);
	int len=0;
	while(bencode_list_has_next(t))
	{
		bencode_t* temp=malloc(sizeof(bencode_t));
		bencode_list_get_next(t,temp);
		len++;
		free(temp);		
	}
	free(t);
	return len;
}
//tu files shemxvda
void files(info_t* info,bencode_t* be)
{
	int len=get_list_len(be);
	info->num_files=len;
	info->files=malloc(len*sizeof(file_t));
	int ind=0;
	while(bencode_list_has_next(be))
	{
		bencode_t* dic=malloc(sizeof(bencode_t));
		bencode_list_get_next(be,dic);
		while(bencode_dict_has_next(dic))
		{
			bencode_t* cur=malloc(sizeof(bencode_t));
			const char** key=malloc(sizeof(char*));
			int* klen=malloc(sizeof(int));
			
			bencode_dict_get_next(dic,cur,key,klen);			
			char* cur_key=malloc(*klen+1);

			memset(cur_key,0,*klen+1);
			cur_key=memcpy(cur_key,*key,*klen);
			if(strcmp(cur_key,"length")==0)
			{
				long int* val_int=malloc(sizeof(long int));
				bencode_int_value(cur,val_int);
				info->files[ind].length=*val_int;
			}
			int ind_path=0;
			if(strcmp(cur_key,"path")==0)
			{
				int leng_list=get_list_len(cur);
				info->files[ind].path_size=leng_list;//raodenoba mshoblebis
				info->files[ind].path=malloc(leng_list*sizeof(char*));
				while(bencode_list_has_next(cur))
				{
					bencode_t* pt=malloc(sizeof(bencode_t));
					bencode_list_get_next(cur,pt);
					int * len=malloc(sizeof(int));
					const char** buf=malloc(sizeof(char*));
					bencode_string_value(pt,buf,len);
					char* val=malloc(*len+1);
					memset(val,0,*len+1);
					val=memcpy(val,*buf,*len);
					info->files[ind].path[ind_path]=val;
					ind_path++;
					free(buf);
					free(pt);
					free(len);
				}  
			}
			free(cur);
		}
		free(dic);
		ind++;		
	}

}
//tu piece length ze var
void piece_length(info_t* info,bencode_t* temp)
{
	long int *val=malloc(sizeof(long int));
	bencode_int_value(temp,val);
	info->piece_length=*val;
	free(val);
} 
//tu piecebze var
void pieces(info_t* info,bencode_t* be)
{
	
	int * len=malloc(sizeof(int));
	const char** buf=malloc(sizeof(char*));
	bencode_string_value(be,buf,len);
	char* val=malloc(*len+1);
	memset(val,0,*len+1);
	val=memcpy(val,*buf,*len);
	info->num_pieces=*len/HASH_SIZE;
	info->pieces=malloc(sizeof(char*)*info->num_pieces);
	int i;
	for(i = 0; i<info->num_pieces; i++)
	{
		char* cur_piece=malloc(HASH_SIZE+1);
		memset(cur_piece,0,HASH_SIZE+1);
		memcpy(cur_piece,val,HASH_SIZE);
		info->pieces[i]=cur_piece;
		val=val+HASH_SIZE;
	}
	free(len);
	free(buf);
}
//daamushavebs tu infoze var bencodeshi
void info(meta_info_t* ret,bencode_t* info_dict)
{
	const char** start=malloc(sizeof(char*));
	int* len=malloc(sizeof(int));
	bencode_dict_get_start_and_len(info_dict,start,len);

	char* infostr=malloc(*len+1);
	memset(infostr,0,*len+1);
	memcpy(infostr,*start,*len);
	ret->info_as_string=infostr;
	ret->info_len=*len;
	ret->info=malloc(sizeof(info_t));
	while(bencode_dict_has_next(info_dict))
	{
		bencode_t* temp=(bencode_t*)malloc(sizeof(bencode_t));
		const char** key=malloc(sizeof(char*));
		int* klen=malloc(sizeof(int));
		bencode_dict_get_next(info_dict,temp,key,klen);		
		char* cur_key=malloc(*klen+1);
		memset(cur_key,0,*klen+1);
		cur_key=memcpy(cur_key,*key,*klen);
		if(strcmp(cur_key,"files")==0)
			files(ret->info,temp);

		if(strcmp(cur_key,"length")==0)
			length(ret->info,temp);
		if(strcmp(cur_key,"name")==0)
			name(ret->info,temp);
		if(strcmp(cur_key,"piece length")==0)
		{
			piece_length(ret->info,temp);
		}
		if(strcmp(cur_key,"pieces")==0)
		{
			pieces(ret->info,temp);
		}
		free(temp);
		free(klen);
	}
}
//construct meta_info structure
meta_info_t* construct_metainfo(char* file_path){
	char *file_contents;
	long input_file_size;
	FILE *input_file = fopen(file_path, "rb");
	fseek(input_file, 0, SEEK_END);
	input_file_size = ftell(input_file);
	rewind(input_file);
	file_contents = malloc((input_file_size + 1) * (sizeof(char)));
	fread(file_contents, sizeof(char), input_file_size, input_file);
    fclose(input_file);
	file_contents[input_file_size] = 0;

	bencode_t * be;
	be=malloc(sizeof(bencode_t));
	bencode_init(be,file_contents,input_file_size);
	
	meta_info_t* ret=malloc(sizeof(meta_info_t));

	while(bencode_dict_has_next(be))
	{
		bencode_t* temp=(bencode_t*)malloc(sizeof(bencode_t));
		const char** key=malloc(sizeof(char*));
		int* klen=malloc(sizeof(int));
		
		bencode_dict_get_next(be,temp,key,klen);
		
		char* cur_key=malloc(*klen+1);
		memset(cur_key,0,*klen+1);
		cur_key=memcpy(cur_key,*key,*klen);
		if(strcmp(cur_key,"announce")==0)
			announce(ret,temp);
		if(strcmp(cur_key,"created by")==0)
			created_by(ret,temp);
		if(strcmp(cur_key,"creation date")==0)
			creation_date(ret,temp);
		if(strcmp(cur_key,"info")==0)
			info(ret,temp);
		free(temp);
	}
	return ret;
}
//abrunebs curls validurad awyobils
char* construct_valid_curl(meta_info_t* inf,
	char* peer_id,int port, long int uploaded,
	long int downloaded,long int left)
{
	char* curl=malloc(CURL_MAX_SIZE);
	memset(curl,0,CURL_MAX_SIZE);

	char* info_sha1=malloc(HASH_SIZE+1);
	memset(info_sha1,0,HASH_SIZE+1);
	info_sha1=SHA1(inf->info_as_string,inf->info_len,info_sha1);
	CURL * cl=malloc(sizeof(CURL));
	char* percent_sha;
	percent_sha=curl_easy_escape(cl,info_sha1,strlen(info_sha1));
	
	char* per_sha1=malloc(HASH_SIZE+1);
	memset(per_sha1,0,HASH_SIZE+1);
	per_sha1=SHA1(peer_id,strlen(peer_id),per_sha1);

	char* percent_id=curl_easy_escape(cl,per_sha1,strlen(per_sha1));
	strcat(curl,percent_id);
	
	sprintf(curl,"%s?info_hash=%s&peer_id=%s&port=%d&uploaded=%ld&downloaded=%ld&left=%ld&compact=1",
	inf->announce,percent_sha,percent_id,port,uploaded,downloaded,left);
	free(cl);
	return curl;
}
//abrunebs structurashi chatenil tracker is responses
tracker_response_t *construct_tracker_response(resp_t* response_content)
{
	bencode_t * be;
	be=malloc(sizeof(bencode_t));
	
	bencode_init(be,response_content->resp,response_content->size);
	tracker_response_t* ret=malloc(sizeof(tracker_response_t));

	while(bencode_dict_has_next(be))
	{
		bencode_t* temp=(bencode_t*)malloc(sizeof(bencode_t));
		const char** key=malloc(sizeof(char*));
		int* klen=malloc(sizeof(int));
		bencode_dict_get_next(be,temp,key,klen);		
		char* cur_key=malloc(*klen+1);
		memset(cur_key,0,*klen+1);
		cur_key=memcpy(cur_key,*key,*klen);

		if(strcmp(cur_key,"failure reason")==0)
		{
			int * len=malloc(sizeof(int));
			const char** buf=malloc(sizeof(char*));
			bencode_string_value(temp,buf,len);
			char* val=malloc(*len+1);
			memset(val,0,*len+1);
			val=memcpy(val,*buf,*len);
			ret->faiure_reason=val;
			free(len);
			free(buf);
		}
		if(strcmp(cur_key,"interval")==0)
		{
			long int *val=malloc(sizeof(long int));
			bencode_int_value(temp,val);
			ret->interval=*val;
			free(val);
		}
		if(strcmp(cur_key,"complete")==0)
		{
			long int *val=malloc(sizeof(long int));
			bencode_int_value(temp,val);
			ret->complete=*val;
			free(val);
		}
		if(strcmp(cur_key,"incomplete")==0)
		{
			long int *val=malloc(sizeof(long int));
			bencode_int_value(temp,val);
			ret->incomplete=*val;
			free(val);
		}
		if(strcmp(cur_key,"peers")==0)
		{
			if(bencode_is_string(temp))
			{
				int * len=malloc(sizeof(int));
				const char** buf=malloc(sizeof(char*));
				bencode_string_value(temp,buf,len);
				ret->peers=malloc(*len);
				ret->num_peers=*len/6;
				char* val=malloc(*len+1);
				memset(val,0,*len+1);
				val=memcpy(val,*buf,*len);

				int i;
				for(i = 0; i<ret->num_peers; i++)
				{
					ret->peers[i].ip=*(uint32_t*)val;
					val=val+sizeof(uint32_t);
					ret->peers[i].port=*(uint16_t*)val;
					val=val+sizeof(uint16_t);
				

					/*struct sockaddr_in antelope;
					antelope.sin_addr.s_addr=ret->peers[i].ip;
					char* str = inet_ntoa(antelope.sin_addr); */
				}
				free(len);
				free(buf);
			}
		}
		free(temp);
		free(key);
		free(klen);

	}
	return ret;

}
