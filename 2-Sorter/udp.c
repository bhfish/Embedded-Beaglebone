
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "udp.h"
#include "main.h"
#include "sort.h"


/*
* Define constant
*/
#define RX_BUFLEN         128
#define TX_BUFLEN         1600
#define PORT              12345
#define UDP_PKT_SIZE_MAX  1500


const struct timespec delay100ms = {2, 0};

//UDP command reply
const char* reply_help = "Accepted command examples:\n"
"count      -- display number arrays sorted.\n"
"get length -- display length of array currently being sorted.\n"
"get array  -- display the full array being sorted.\n"
"get 10     -- display the tenth element of array currently being sorted.\n"
"stop       -- cause the server program to end.\n";

const char* reply_invalid_cmd = "Unknown command. Type help for command list\n";
const char* reply_stop = "Program Terminating\n";


/*
*   UDP command parser.
*   return  1: Command is found and need to send array
*           0: Command is found and tx message is ready to be sent
*          -1: Command is NOT found
*/
int udp_cmd_parse(char* rx_buffer, char* tx_buffer, int* num)
{

	char* buf_tokens[2];
	int cmd_len = 0;


    if ((buf_tokens[0] = strtok(rx_buffer," \n\t\0")) != NULL) 
    {
   		if ((buf_tokens[1] = strtok(NULL, " \n\t\0")) != NULL)
    	{
    		if (strtok(NULL, " \n\t\0") != NULL)
			{
				//Unknown command
				strcpy(tx_buffer, reply_invalid_cmd);
        *num = strlen(tx_buffer);
				return 0;
			}
			else
			{
				cmd_len = 2;
			}
    	}
    	else
    	{
    		cmd_len = 1;
    	}
    }
    else
    {
    	//Unknown command
		  return -1;
    }


  //Process the commands
	if ((strcmp("help", buf_tokens[0]) == 0) && (cmd_len == 1))
	{
		strcpy(tx_buffer, reply_help);
		*num = strlen(tx_buffer);
	}
	else if ((strcmp("count", buf_tokens[0]) == 0) && (cmd_len == 1))
	{
		sprintf(tx_buffer, "Number of arrays sorted = %lld \n", sort_cntr);
		*num = strlen(tx_buffer);
	}
	else if ((strcmp("get", buf_tokens[0]) == 0) && (cmd_len == 2))
	{
		if ((strcmp("array", buf_tokens[1]) == 0))
		{
			return 1;
		}
		else if ((strcmp("length", buf_tokens[1]) == 0))
		{
			sprintf(tx_buffer, "Current array length = %d \n", array_len);
			*num = strlen(tx_buffer);
		}
		else
		{
			int array_idx =  atoi (buf_tokens[1]);

			//Validate the input array index
			if ((array_idx >= 1) && 
				(array_idx <= array_len) &&
			    (array_sort != NULL))
			{

        //Get mutex to read array being sorted
        pthread_mutex_lock(&arrayRead_mutex);       
        pthread_mutex_lock(&arraySort_mutex);

				sprintf(tx_buffer, "Value %d = %d \n", array_idx, array_sort[array_idx -1]);

        //Unlock mutex to allow keeping sorting
        pthread_mutex_unlock(&arraySort_mutex);
        pthread_mutex_unlock(&arrayRead_mutex); 

				*num = strlen(tx_buffer);
			}
			else
			{
				sprintf(tx_buffer, "Invalid argument. Must be between 1 and %d (array length)", array_len);
				*num = strlen(tx_buffer);
			}
		}

	}
	else if ((strcmp("stop", buf_tokens[0]) == 0) && (cmd_len == 1))
	{
		strcpy(tx_buffer, reply_stop);
		*num = strlen(tx_buffer);
		programExit();
	}
	else
	{
		//Unknown command
		strcpy(tx_buffer, reply_invalid_cmd);
		*num = strlen(tx_buffer);
	}	

	return 0;
}


void udp_task(void)
{
    int udpSocket;
  	char rx_buffer[RX_BUFLEN];
  	char tx_buffer[TX_BUFLEN];
  	struct sockaddr_in udp_server;
  	struct sockaddr_storage udp_Storage;
  	socklen_t addr_size;
  	int tx_byte;
  	int cmd_flag;

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


  	printf(" . . . Start UDP Task\n");
  	while(progRun)
    {

  		//Initialize input and output buffer
  		memset(rx_buffer, 0, sizeof(rx_buffer));
  		memset(tx_buffer, '\0', sizeof(tx_buffer));

    	//Receive incoming UDP message
    	recvfrom(udpSocket, rx_buffer, RX_BUFLEN,0,
    		      (struct sockaddr *)&udp_Storage, &addr_size);

    	//Parse received command
    	cmd_flag = udp_cmd_parse(rx_buffer, tx_buffer, &tx_byte);

    	if (cmd_flag == 0)
    	{
    		//Send response
    		sendto(udpSocket, tx_buffer, tx_byte, 0,
    		   		(struct sockaddr *)&udp_Storage,addr_size);
  		}
      else if (cmd_flag == 1)
      {
        int i = 0;
        int j = 0;
        int n = 0;

        //Get mutex to read array being sorted
        pthread_mutex_lock(&arrayRead_mutex);       
        pthread_mutex_lock(&arraySort_mutex);


        for (i = 0; i< array_len; i++)
        {
          j++;

          //Send data if current tx buffer reaches the largest UDP packet size
          if (n >= 1500)
          {
              //Send data
              sendto(udpSocket, tx_buffer, n, 0,
                    (struct sockaddr *)&udp_Storage,addr_size);
              
              //Clear tx buffer
              memset(tx_buffer, '\0', sizeof(tx_buffer));
              n = 0;
          }
           
          if (i == (array_len - 1))
          {
            //Store the last element of array
            n += sprintf(&tx_buffer[n], "%d\n", array_sort[i]);
          }
          else
          {
            //Store array elements in tx buffer
            n += sprintf(&tx_buffer[n], "%d, ", array_sort[i]);
          }

          //Start a new line for every 10 elements
          if (!(j%10))
          {
            tx_buffer[n] = '\n';
            n ++;
            j = 0;
          }
        }
         
        //Send data     
        sendto(udpSocket, tx_buffer, n, 0,
                (struct sockaddr *)&udp_Storage,addr_size);

        //Unlock mutex to allow keeping sorting
        pthread_mutex_unlock(&arraySort_mutex);
        pthread_mutex_unlock(&arrayRead_mutex); 

      }
  	}  

    printf(" . . . Stop UDP Task\n");

  	//Close socket
  	shutdown(udpSocket, SHUT_RDWR);
    close(udpSocket);
}



/*
* Initialize and start sorter task
*/
void udp_init(void)
{
	udp_task();
	return;
}
