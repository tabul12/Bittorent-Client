#ifndef TRACKER_H_  /* Include guard */
#define TRACKER_H_
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>


typedef struct 
{
	char* resp;
	int size;
}resp_t;

resp_t *do_web_request(char *url);

#endif