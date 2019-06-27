#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "udpServer.h"
#include "audioCtrl.h"
#include "beatbox.h"

/*
* Define constant
*/
#define RX_BUFLEN         128
#define TX_BUFLEN         1600
#define PORT              12345

//#define UDP_PKT_SIZE_MAX  1500
    

static int udpSocket;
static struct sockaddr_in udp_server;
static struct sockaddr_storage udp_Storage;
static socklen_t addr_size;
static pthread_t udpServerThreadId;

static int udpServerCmdParse(char* rx_buffer, char* tx_buffer, int* num);
static void udpServerTask(void);
static int udpServerBindPort(void);
//static void udpServerCleanup(void);


static int udpServerCmdParse(char* rx_buffer, char* tx_buffer, int* num)
{
	int res = 0;
	char* buf_token;

    if ((buf_token = strtok(rx_buffer," \n\t\0")) == NULL) 
    {	
    	return -1;
    }

  	//Process the commands
	if(strcmp("volumeUp", rx_buffer) == 0)
	{
		//Change volume
		audioCtrlVolIncr(5);
		sprintf(tx_buffer, "volume:%d\n", audioCtrlGetVol());
		*num = strlen(tx_buffer);
	}
	else if(strcmp("volumeDown", rx_buffer) == 0)
	{
		//Change volume
		audioCtrlVolIncr(-5);
		sprintf(tx_buffer, "volume:%d\n", audioCtrlGetVol());
		*num = strlen(tx_buffer);
	}
	else if(strcmp("tempoUp", rx_buffer) == 0)
	{
		//Change tempo
		audioCtrlTempoIncr(5);
		sprintf(tx_buffer, "tempo:%d\n", audioCtrlGetTempo());
		*num = strlen(tx_buffer);
	}
	else if(strcmp("tempoDown", rx_buffer) == 0)
	{
		//Change tempo
		audioCtrlTempoIncr(-5);
		sprintf(tx_buffer, "tempo:%d\n", audioCtrlGetTempo());
		*num = strlen(tx_buffer);
	}
	else if(strcmp("hihat", rx_buffer) == 0)
	{
		audioCtrlPlay(&drumBeatHihat);
		strcpy(tx_buffer, "hihat\n");
		*num = strlen(tx_buffer);
	}
	else if(strcmp("snare", rx_buffer) == 0)
	{
		audioCtrlPlay(&drumBeatSnare);
		strcpy(tx_buffer, "snare\n");
		*num = strlen(tx_buffer);
	}
	else if(strcmp("base", rx_buffer) == 0)
	{
		audioCtrlPlay(&drumBeatBase);
		strcpy(tx_buffer, "base\n");
		*num = strlen(tx_buffer);
	}
	else if(strcmp("rock", rx_buffer) == 0)
	{
		audioCtrlSetBeatMode(BEAT_MODE_ROCK);
		strcpy(tx_buffer, "mode:0\n");
		*num = strlen(tx_buffer);
	}
	else if(strcmp("custom", rx_buffer) == 0)
	{
		audioCtrlSetBeatMode(BEAT_MODE_CUSTOM);
		strcpy(tx_buffer, "mode:1\n");
		*num = strlen(tx_buffer);
	}
	else if(strcmp("none", rx_buffer) == 0)
	{
		audioCtrlSetBeatMode(BEAT_MODE_NONE);
		strcpy(tx_buffer, "mode:2\n");
		*num = strlen(tx_buffer);
	}
	else if(strcmp("update", rx_buffer) == 0)
	{
		sprintf(tx_buffer, "status:%d, %d, %d\n", audioCtrlGetBeatMode(), audioCtrlGetVol(), audioCtrlGetTempo());
		*num = strlen(tx_buffer);
	}

//For debugging	
	else if(strcmp("close", rx_buffer) == 0)
	{
		audioCtrlSetBeatMode(BEAT_MODE_NONE);
		stopping = true;
		strcpy(tx_buffer, "Closing beatbox\n");
		*num = strlen(tx_buffer);
	}
	else
	{

printf("...Not match with any cmd.\n");

		res = -1;
	}

	return res;
}

static void udpServerTask(void)
{
	char rx_buffer[RX_BUFLEN];
  	char tx_buffer[TX_BUFLEN];
  	int res;
  	int tx_byte;

	while(!stopping)
	{
  		//Initialize input and output buffer
  		memset(rx_buffer, 0, sizeof(rx_buffer));
  		memset(tx_buffer, '\0', sizeof(tx_buffer));

    	//Receive incoming UDP message
    	recvfrom(udpSocket, rx_buffer, RX_BUFLEN,0,
    		      (struct sockaddr *)&udp_Storage, &addr_size);

    	//Parse received command
    	res = udpServerCmdParse(rx_buffer, tx_buffer, &tx_byte);

    	if(res == 0)
    	{   
    		//Send response
    		sendto(udpSocket, tx_buffer, tx_byte, 0,
    		   		(struct sockaddr *)&udp_Storage,addr_size);
    	}
	}

//	udpServerCleanup();
}

static int udpServerBindPort(void)
{
	int res = 0;

  	/*Create UDP socket*/
  	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  	udp_server.sin_family = AF_INET;
  	udp_server.sin_port = htons(PORT);
  	udp_server.sin_addr.s_addr = htonl(INADDR_ANY);
  	memset(udp_server.sin_zero, '\0', sizeof (udp_server.sin_zero));  

  	//Bind socket
  	bind(udpSocket, (struct sockaddr *) &udp_server, sizeof(udp_server));

  	//Initialize size
  	addr_size = sizeof (udp_Storage);

  	return res;
}


void udpServerCleanup(void)
{
  	//Close socket
  	printf("...Closing UDP socket.\n");
  	shutdown(udpSocket, SHUT_RDWR);
    close(udpSocket);

   	printf("...Stopping UDP server thread.\n");
	pthread_join(udpServerThreadId, NULL);

}

void udpServerInit(void)
{
	int rt;

	rt = udpServerBindPort();

	if(rt)
	{
		printf("...Error: Failed to bind port.\n");
		return;
	}

	printf("...Creating UDP server thread.\n");
	rt = pthread_create(&udpServerThreadId, NULL, (void *)&udpServerTask, NULL);
	
	if(rt)
	{
		printf("...Error: UDP thread creation failed: %d\n", rt);
		return;
	}

}
