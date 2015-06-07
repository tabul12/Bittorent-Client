#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include "tracker.h"
#include <stdlib.h>
#include <stdio.h>


/* function prototypes to define later */
resp_t *do_web_request(char *url);
size_t static write_callback_func(void *buffer,
                        size_t size,
                        size_t nmemb,
                        void *userp);

/* the function to return the content for a url */
resp_t* do_web_request(char *url)
{
    /* keeps the handle to the curl object */
    CURL *curl_handle = NULL;
    /* to keep the response */
    resp_t *response =(resp_t *) malloc(sizeof(resp_t));

    /* initializing curl and setting the url */
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);

    /* follow locations specified by the response header */
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);

    /* setting a callback function to return the data */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback_func);

    /* passing the pointer to the response as the callback parameter */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);

    /* perform the request */
    curl_easy_perform(curl_handle);

    /* cleaning all curl stuff */
    curl_easy_cleanup(curl_handle);

    return response;
}

/* the function to invoke as the data recieved */
size_t static write_callback_func(void *buffer,
                        size_t size,
                        size_t nmemb,
                        void *userp)
{
    resp_t **response_ptr =  (resp_t **)userp;
    /* assuming the response is a string */
    
    (*response_ptr)->resp = strndup(buffer, (size_t)(size *nmemb));
    (*response_ptr)->size=(int)(size *nmemb);
}