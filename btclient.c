#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "bencode.h"
#include "construct.h"
#include "tracker.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include <sys/types.h>  
#include <semaphore.h>
#include <stdarg.h>
#include <stdint.h>
#define  BUFF_SIZE 1024
#define  HASH_SIZE 20
#define  PEER_ID_SIZE 20
#define PORT 6883
#define BLOCK_SIZE 16384

char* PEER_ID;
meta_info_t* META_INFO;
uint8_t* DOWNLOADED_PIECES;
sem_t * DOWNLOADED_PIECES_SEM;
int LEFT_PIECES_NUM;
sem_t LEFT_PIECES_NUM_SEM;

int FILE_DESCRIPTOR;

typedef struct{
	uint32_t peer_ip;
   uint16_t peer_port;
	meta_info_t *meta_info;
	char* peer_id;
}thread_info_t;

typedef struct
{
   int choke;
   int interested;
   int sockfd;
   uint8_t* bitt_filled;
   sem_t state_sem;
}state_t;
//es meotdi gzavnis 
//requestebs sul 
void* request_sender_thread(void* th_info)
{
   state_t* state=(state_t*)th_info;
   int sockfd=state->sockfd;
   while(1)
   {
      //sem_wait(&(state->state_sem));
      if(state->choke==0 && state->interested==1)
      {
         //sem_post(&(state->state_sem));
         int i;
         for(i=0; i<META_INFO->info->num_pieces;i++)
         {
           sem_wait(&(DOWNLOADED_PIECES_SEM[i]));
           //sem_wait(&(state->state_sem));
           if(DOWNLOADED_PIECES[i]==0 && state->bitt_filled[i]==1)
           {
               DOWNLOADED_PIECES[i]=1;
               sem_post(&(DOWNLOADED_PIECES_SEM[i]));
               //sem_post(&(state->state_sem));
               int piece_length=META_INFO->info->piece_length;
               int num_blocks=piece_length/BLOCK_SIZE;
               int per=piece_length%BLOCK_SIZE;
               int j;
               for(j = 0; j<num_blocks; j++)
               {
                  char request[sizeof(int)+1+3*sizeof(int)];
                  *((int*)request)=htonl(3*sizeof(int)+1);
                  *(request+sizeof(int))=6;
                  *((int*)(request+sizeof(int)+1))=htonl(i);
                  *((int*)(request+sizeof(int)+1+sizeof(int)))=htonl(j*BLOCK_SIZE);
                 
                  *((int*)(request+sizeof(int)+1+sizeof(int)+sizeof(int)))=htonl(BLOCK_SIZE);
           
                  write(sockfd,request,sizeof(int)+1+3*sizeof(int));
               }
               if(per>0) //zustad tu ar chaetia 
               {
                  char request[sizeof(int)+1+3*sizeof(int)];
                  *((int*)request)=htonl(3*sizeof(int)+1);
                  *(request+sizeof(int))=6;
                  *((int*)(request+sizeof(int)+1))=htonl(i);
                  *((int*)(request+sizeof(int)+1+sizeof(int)))=htonl(num_blocks*BLOCK_SIZE);
                  *((int*)(request+sizeof(int)+1+sizeof(int)+sizeof(int)))=htonl(per);
                  write(sockfd,request,sizeof(int)+1+3*sizeof(int));
               }
               //sem_post(&(DOWNLOADED_PIECES_SEM[i]));
               //sem_post(&(state->state_sem));
           }
           else
           {
               //sem_post(&(state->state_sem));
               sem_post(&(DOWNLOADED_PIECES_SEM[i]));
           }
         }
      }
      //else
         //sem_post(&(state->state_sem));


      //tu dainteresebuli agar var unda shevwyvito
      //requestebis gzavna
      //states gadmovcem sredis parametrad
      //da ziaria mshobeli da mimdinare sredistvis
      if(state->interested==0)
      {
         //sem_post(&(state->state_sem));
         break;
      }
      //sem_post(&(state->state_sem));
   }
   pthread_exit(NULL);
}
//sredis metodi
void* peer_thread(void* th_info)
{
   state_t* state=malloc(sizeof(state_t));
   state->choke=1;
   state->interested=0;
   //sem_init(&(state->state_sem),0,1);
   uint8_t bitt_filled[META_INFO->info->num_pieces];
   memset(bitt_filled,0,META_INFO->info->num_pieces);

   state->bitt_filled=bitt_filled;

   int sockfd=*((int*)th_info);
   state->sockfd=sockfd;
   
   int n=0;
   char sendline[BUFF_SIZE];
   char recvline[BUFF_SIZE];
   memset(sendline,0,BUFF_SIZE);
   memset(recvline,0,BUFF_SIZE);

   char* info_sha1=malloc(HASH_SIZE+1);
   memset(info_sha1,0,HASH_SIZE+1);
   info_sha1=SHA1(META_INFO->info_as_string,META_INFO->info_len,info_sha1);
   char* id_sha1=malloc(HASH_SIZE+1);
   memset(id_sha1,0,HASH_SIZE+1);
   id_sha1=SHA1(PEER_ID,strlen(PEER_ID),id_sha1);
   uint8_t b=19;
   *(uint8_t*)sendline=b;
   memcpy(sendline+1,"BitTorrent protocol",19);
   memcpy(sendline+1+b+8,info_sha1,HASH_SIZE);
   memcpy(sendline+1+b+8+HASH_SIZE,id_sha1,PEER_ID_SIZE);
   int handshakesize=1+b+8+HASH_SIZE+PEER_ID_SIZE;
   
   //handshake gavgzavnet
   write(sockfd,sendline,handshakesize);
   n=read(sockfd,recvline,BUFF_SIZE);
   uint8_t b1=*(uint8_t*)recvline;
   
   char* recv_info_sha1=malloc(HASH_SIZE+1);
   memset(recv_info_sha1,0,HASH_SIZE+1);
   memcpy(recv_info_sha1,sendline+b1+1+8,HASH_SIZE);

   //tu mosuli araa toli mashin unda gavwyvitot connection
   if(strcmp(info_sha1,recv_info_sha1)!=0)
   {
      close(sockfd);
      return;
   }
   //interested gavgzavnet
   void* intersted=malloc(sizeof(int)+1);
   ((int*)intersted)[0]= htonl(1);
   ((char*)intersted)[4]=2;
   write(sockfd,intersted,sizeof(int)+1);
   free(intersted);

   //savtartet sredi romelic sul cdilobs gagzavnos request   
   state->interested=1;
   pthread_t* writer_th=malloc(sizeof(pthread_t));
   pthread_create(writer_th,NULL,request_sender_thread,(void*)state);

   //sul mivigot mesijebi 
   //da davamushaot
   while(1)
   {
      int indicator=0;
      //memset(sendline,0,BUFF_SIZE);
      memset(recvline,0,BUFF_SIZE);
      sem_wait(&LEFT_PIECES_NUM_SEM);
      if(LEFT_PIECES_NUM==0)
      {
         sem_post(&LEFT_PIECES_NUM_SEM);
         break;
      }
      sem_post(&LEFT_PIECES_NUM_SEM);

      uint32_t message_len;
      n=read(sockfd,&message_len,sizeof(uint32_t));
      if(n<0)
         return;
      
      message_len=ntohl(message_len);
      
      if(message_len==0)
         continue;
   
      uint8_t message_id;
      n=read(sockfd,&message_id,sizeof(uint8_t));
      if(message_id<0 || message_len<0)
         continue;
      if(n<0)
         return;


      //amoviget len da id da vamowmebt

      //chocheckd
      if(message_id==0)
      {
         //sem_wait(&(state->state_sem));
         state->choke=1;
         indicator=1;
         //sem_post(&(state->state_sem));
      }
      //unchocked
      if(message_id==1)
      {
         //sem_wait(&(state->state_sem));
         state->choke=0;
         indicator=1;
         //sem_post(&(state->state_sem));
      }
      if(message_id==2)
      {

      }
      if(message_id==3){

      }
      //have
      if(message_id==4){
         //sem_wait(&(state->state_sem));
         uint32_t piece_index;
         n=read(sockfd,&piece_index,sizeof(uint32_t));
         piece_index=ntohl(piece_index);
         bitt_filled[piece_index]=(uint8_t)1;
         indicator=1;
         //sem_post(&(state->state_sem));
      }
      //bittfield
      //anu 8 8 bits vigeb da imat vgebulob romeli romelia anu da davsetav mere 1 s, tu aris
      if(message_id==5){
         //sem_wait(&(state->state_sem));
         printf("%u\n",message_len);
         memset(recvline,0,BUFF_SIZE);
         read(sockfd,recvline,message_len-1);
         uint8_t mask=1;
         int by;
         for( by=0; by<META_INFO->info->num_pieces;by++)
         {
            int by_ind=by/8;
            int per=by%8;
            uint8_t one_byte=((uint8_t*)recvline)[by_ind];
            bitt_filled[by]=(one_byte>>(8-per-1))&mask;
         }
         indicator=1;
         //sem_post(&(state->state_sem));
      }
      //request
      if(message_id==6){
      
      }

      //tu piece momivida
      //
      if(message_id==7){

         uint32_t index;
         read(sockfd,&index,sizeof(uint32_t));
         index=ntohl(index);

         uint32_t offset;
         read(sockfd,&offset,sizeof(uint32_t));
         offset=ntohl(offset);

         int block_size=message_len-9;

         char buff[block_size];
         read(sockfd,buff,block_size);

         int d=index*META_INFO->info->piece_length+offset;
         lseek(FILE_DESCRIPTOR,d,SEEK_SET);
         write(FILE_DESCRIPTOR,buff,block_size);
         indicator=1;
      }
      if(message_id==8){

      }
      if(message_id==9){

      } 

      //anu aq tu momivida iseti info romelic ar davamushave
      //aq ubralod vaxden wakitxvas
      if(indicator==0)
      {
         char buff[message_len-1];
         read(sockfd,buff,message_len-1);
      }
   }
   close(sockfd);
   free(writer_th);
   pthread_exit(NULL); 
}
int main(int argc, char const *argv[])
{
   //aq gavwer ragac peer_id ra udna iyos chemi
   PEER_ID=malloc(PEER_ID_SIZE);
   memset(PEER_ID,0,(int)PEER_ID_SIZE);
   memcpy(PEER_ID,"TORNIKEABULADZE",strlen("TORNIKEABULADZE")+1);

   //shevadgent bencodes infos structurashi damushavebuls
   META_INFO=construct_metainfo((char*)argv[1]);

   //gavaketebt urls
	char* url=construct_valid_curl(META_INFO,"TORNIKEABULADZE",PORT,0,0,META_INFO->info->length);
	//gavaketebt trackertan requests
   resp_t * response=do_web_request(url);
   
   if(response->size<0)
      return 0;

   sem_init(&LEFT_PIECES_NUM_SEM,0,1);
   LEFT_PIECES_NUM=META_INFO->info->num_pieces;
   
   DOWNLOADED_PIECES=malloc(META_INFO->info->num_pieces);
   memset(DOWNLOADED_PIECES,0,META_INFO->info->num_pieces);

   
   fopen(META_INFO->info->name,"wb");
   FILE_DESCRIPTOR=open(META_INFO->info->name,"O_RDWR");        

   DOWNLOADED_PIECES_SEM=malloc(META_INFO->info->num_pieces*sizeof(sem_t));
   int j;
   for(j = 0; j<META_INFO->info->num_pieces;j++)
      sem_init(&DOWNLOADED_PIECES_SEM[j],0,1);

   //tracker is responses shevinaxavt structurashi dapasruls
	tracker_response_t * t_resp=construct_tracker_response(response);


   //vuconnectdebit titoeuls da vstartavt sredebs
   int i;
	for(i=0; i<t_resp->num_peers; i++)
	{
      int sockfd=socket(AF_INET,SOCK_STREAM,0);
      if(sockfd==-1)
         continue;
      struct timeval timeout;      
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;

      if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        continue;
      if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        continue;
      struct sockaddr_in addr;
      bzero(&addr,sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr=t_resp->peers[i].ip;
      addr.sin_port=t_resp->peers[i].port;

      if(connect(sockfd,(struct sockaddr *)&addr, sizeof(addr))==0)
      {
           pthread_t p_thread;
           pthread_create(&p_thread, NULL ,peer_thread,(void*)&sockfd);
      }
	}
   pthread_exit(NULL);
	return 0;
}